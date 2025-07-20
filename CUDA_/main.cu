#define GLM_ENABLE_EXPERIMENTAL
typedef unsigned int uint;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include <glm/gtc/type_ptr.hpp>  
#include <vector>
#include "gpu_types.h"
#include <iostream>
#include <chrono>
#include <math_constants.h>

#include "obj_loader.h"
#include "utils.h"
#include "Object.h"
#include "camera.h"
#include "scene.h"

#include <algorithm> 
#include <stdexcept>  
#include <cmath>      
#include <cfloat>    
#include <vector>     

__device__ __forceinline__ float3 mix(const float3& a, const float3& b, float t)
{
    return make_float3(a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t);
}

__device__ uint wangHash(uint s) {
    s = (s ^ 61u) ^ (s >> 16);
    s *= 9u;  s = s ^ (s >> 4);  s *= 0x27d4eb2du;
    s = s ^ (s >> 15); return s;
}

__device__ float rand(uint& state)
{
    state = wangHash(state);  return state * 2.3283064365387e-10f;
}

__host__ __device__ inline float3  operator+(const float3& a,
    const float3& b)
{
    return make_float3(a.x + b.x, a.y + b.y, a.z + b.z);
}

__host__ __device__ inline float3  operator-(const float3& a,
    const float3& b)
{
    return make_float3(a.x - b.x, a.y - b.y, a.z - b.z);
}

__host__ __device__ inline float3  operator*(const float3& a, const float b)
{
    return make_float3(a.x * b, a.y * b, a.z * b);
}

__host__ __device__ inline float3  operator*(const float b, const float3& a)
{
    return make_float3(a.x * b, a.y * b, a.z * b);
}

__host__ __device__ inline float3  operator/(const float3& a, const float b)
{
    float inv = 1.0f / b;  return make_float3(a.x * inv, a.y * inv, a.z * inv);
}

__host__ __device__ inline float3  operator*(const float3& a,
    const float3& b)
{
    return make_float3(a.x * b.x, a.y * b.y, a.z * b.z);
}


struct GPU_Ray { float3 orig, dir; };
__device__ inline float3  cross(const float3& a, const float3& b) { return make_float3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }
__device__ inline float   dot3(const float3& a, const float3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
__device__ inline float3  normalize3(float3 v) { float inv = rsqrtf(dot3(v, v)); return make_float3(v.x * inv, v.y * inv, v.z * inv); }

__device__ __forceinline__ float3 clamp01(float3 v)
{
    return make_float3(fminf(fmaxf(v.x, 0.f), 1.f),
        fminf(fmaxf(v.y, 0.f), 1.f),
        fminf(fmaxf(v.z, 0.f), 1.f));
}

__device__ float3 sampleHemisphereCosine(float3 N, float r1, float r2)
{
    float phi = 2.f * CUDART_PI_F * r1;
    float cosT = sqrtf(1.f - r2);
    float sinT = sqrtf(r2);
    float3 T, B;
    if (fabsf(N.z) < .999f) T = normalize3(cross(make_float3(0, 0, 1), N));
    else                    T = normalize3(cross(make_float3(0, 1, 0), N));
    B = cross(N, T);
    return normalize3(cosT * N + cosf(phi) * sinT * T + sinf(phi) * sinT * B);
}

GPUTriangle* d_tris = nullptr;
int32_t* d_tri_indices = nullptr;
GPUBVHNode* d_bvh_nodes = nullptr;
GPUMaterial* d_materials = nullptr;
GPUObject* d_objects = nullptr;
GPU_AABB host_global_bb;

static uchar4* d_fb = nullptr;

GPULightSphere* d_lights = nullptr;

static float3* d_accum = nullptr;

static unsigned int frameID = 0;

std::vector<GPUTriangle> all_tris;
std::vector<int32_t>     all_tri_indices;
std::vector<GPUBVHNode>  all_bvh_nodes;
std::vector<GPUMaterial> all_materials;
std::vector<GPUObject>   all_objects;

GPUCamera* d_cam;

std::vector<GPULightSphere> h_lights;

__device__ __forceinline__ float  clamp01(float v)
{
    return fminf(fmaxf(v, 0.f), 1.f);
}

__device__ bool hitAABB(const GPU_AABB& b, const GPU_Ray& r, float tMin, float tMax)
{
    float3 invD = make_float3(1.0f / r.dir.x, 1.0f / r.dir.y, 1.0f / r.dir.z);
    float3 t0 = (b.min - r.orig) * invD;
    float3 t1 = (b.max - r.orig) * invD;
    float3 tsm = make_float3(fminf(t0.x, t1.x), fminf(t0.y, t1.y), fminf(t0.z, t1.z));
    float3 tbg = make_float3(fmaxf(t0.x, t1.x), fmaxf(t0.y, t1.y), fmaxf(t0.z, t1.z));
    tMin = fmaxf(tMin, fmaxf(tsm.x, fmaxf(tsm.y, tsm.z)));
    tMax = fminf(tMax, fminf(tbg.x, fminf(tbg.y, tbg.z)));
    return tMax >= tMin;
}

__device__ bool hitTriangle(const GPUTriangle& tri, const GPU_Ray& r, float& t, float3& N)
{
    const float3 e1 = tri.v1 - tri.v0;
    const float3 e2 = tri.v2 - tri.v0;
    const float3 p = cross(r.dir, e2);
    const float  det = dot3(e1, p);
    if (fabsf(det) < 1e-8f) return false;
    const float inv = 1.0f / det;
    const float3 s = r.orig - tri.v0;
    const float  u = dot3(s, p) * inv; if (u < 0 || u > 1) return false;
    const float3 q = cross(s, e1);
    const float  v = dot3(r.dir, q) * inv; if (v < 0 || u + v > 1) return false;
    const float  tt = dot3(e2, q) * inv;     if (tt < 0.001f)      return false;
    t = tt;  N = normalize3(tri.n);
    return true;
}

__global__ void render_kernel(float3* accum,const GPUCamera* cam, const GPULightSphere* lights, int lightCnt, uint frameID, uchar4* fb,
    int                width,
    int                height,

    const GPUTriangle* tris,
    const int32_t* triIndices,
    const GPUBVHNode* nodes,
    const GPUMaterial* mats,
    const GPUObject* objects,
    int                objectCount,

    const GPU_AABB         globalBB)
{
    int px = blockIdx.x * blockDim.x + threadIdx.x;
    int py = blockIdx.y * blockDim.y + threadIdx.y;
    if (px >= width || py >= height) return;


    uint seed = ((py * width + px) << 16) ^ frameID;
    GPU_Ray ray;
    ray.orig = cam->pos;

    float sx = ((px + 0.5f) / width * 2.f - 1.f);
    float sy = ((py + 0.5f) / height * 2.f - 1.f);

    ray.dir = normalize3(
        cam->dir * cam->focal_len +
        cam->right * sx +
        cam->up * sy);


    float  bestT = 1e30f;
    float3 bestN = make_float3(0, 0, 0);
    GPUMaterial bestMat; bestMat.albedo = make_float3(1.0f, 1.0f, 1.0f);

    for (int o = 0; o < objectCount; ++o)
    {
        const GPUObject& obj = objects[o];
        int  stack[32];   int stackPtr = 0;
        stack[stackPtr++] = obj.node_offset;
        while (stackPtr)
        {
            int nodeIdx = stack[--stackPtr];
            const GPUBVHNode& n = nodes[nodeIdx];
            if (!hitAABB(n.bbox, ray, 0.001f, bestT)) continue;

            if (n.left == -1) {
                for (int i = 0; i < n.count; ++i) {
                    int triID = obj.tri_offset + triIndices[n.start + i];
                    float  t; float3 N;
                    if (hitTriangle(tris[triID], ray, t, N) && t < bestT) {
                        if (dot3(N, ray.dir) > 0.0f) N = N * -1.0f;

                        bestT = t;
                        bestN = N;
                        bestMat = mats[obj.mat_offset];
                    }
                }
            }
            else {
                stack[stackPtr++] = n.left;
                stack[stackPtr++] = n.right;
            }
        }
    }

    uchar4 out;



    float3 throughput = make_float3(1.0f, 1.0f, 1.0f);   
    float3 radiance = make_float3(0.0f, 0.0f, 0.0f);   

    for (int depth = 0; depth < 8; ++depth)
    {
    
        float  bestT = 1e30f;
        float3 bestN = make_float3(0.0f, 0.0f, 0.0f);
        GPUMaterial bestMat; bestMat.albedo = make_float3(1.0f, 1.0f, 1.0f);

        {                               
            for (int o = 0; o < objectCount; ++o) {
                const GPUObject& obj = objects[o];
                int stack[32], sp = 0;           stack[sp++] = obj.node_offset;
                while (sp) {
                    const GPUBVHNode& N = nodes[stack[--sp]];
                    if (!hitAABB(N.bbox, ray, 1e-3f, bestT)) continue;
                    if (N.left == -1) {
                        for (int i = 0; i < N.count; ++i) {
                            int id = obj.tri_offset + triIndices[N.start + i];
                            float t; float3 Nn;
                            if (hitTriangle(tris[id], ray, t, Nn) && t < bestT) {
                                if (dot3(Nn, ray.dir) > 0) Nn = Nn * -1;
                                bestT = t; bestN = Nn; bestMat = mats[obj.mat_offset];
                            }
                        }
                    }
                    else { stack[sp++] = N.left; stack[sp++] = N.right; }
                }
            }
        }

        if (bestT > 1e29f) {
            float  t = clamp01(ray.dir.y * .5f + .5f);
            const float3 skyTop = make_float3(0.65f, 0.80f, 1.0f);
            const float3 skyBottom = make_float3(1.00f, 1.00f, 1.00f);
            radiance = radiance+ throughput * mix(skyBottom, skyTop, t);
            break;
        }

       
        float3 hitP = ray.orig + bestT * ray.dir + bestN * 1e-3f;
        float3 Ldir = normalize3(make_float3(-0.2f, 0.55f, -0.4f));   
        float  nDotL = fmaxf(0.f, dot3(bestN, Ldir));

        bool inShadow = false;    
        if (!inShadow && nDotL > 0) {
            const float3 Li = make_float3(25.f, 25.f, 25.f);                 
            radiance = radiance + throughput * Li * nDotL;
        }

        float3 newDir = sampleHemisphereCosine(bestN, rand(seed), rand(seed));
        float  cosTheta = fmaxf(0.f, dot3(bestN, newDir));
        throughput = throughput* bestMat.albedo * (cosTheta / CUDART_PI_F);

        
        float q = fmaxf(throughput.x, fmaxf(throughput.y, throughput.z));
        if (rand(seed) > q) break;
        throughput = throughput / q;

        ray.orig = hitP;
        ray.dir = newDir;
    }

    uint idxPix = py * width + px;
    float3 old = accum[idxPix];
    uint   sampleCount = frameID + 1;       
    accum[idxPix] = (old * frameID + radiance) / sampleCount;
    float exposure = 1.5f; 
    float3 mapped = accum[idxPix] * exposure;     
    mapped = make_float3(mapped.x / (1.0f + mapped.x),
        mapped.y / (1.0f + mapped.y),
        mapped.z / (1.0f + mapped.z));
    mapped = clamp01(mapped);                    
    fb[idxPix] = make_uchar4(mapped.x * 255, mapped.y * 255, mapped.z * 255, 255);

}


GLuint tex;
cudaGraphicsResource* cuda_tex_resource;
int width = 1280, height = 720;

// Setup GL texture
void createTexture() {
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    
    cudaGraphicsGLRegisterImage(&cuda_tex_resource, tex,
        GL_TEXTURE_2D, cudaGraphicsRegisterFlagsSurfaceLoadStore);
}

void setupScene() {
   
    //main defn 
    Camera c1(8.0f, 35.0f, height, width);
    Scene s1(c1);
    material m;
    m.albedo = glm::vec3(1.0f, 0.0f, 0.0f);
    m.metallic = 0.0f;
    m.roughness = 1.0f;
    glm::vec3 rotEuler = glm::vec3(glm::radians(0.0f),
        glm::radians(0.0f),
        0.0f);
    Object o1("street_new.obj", glm::vec3(0.0f, -15.0f, -125.0f), rotEuler, m);
    o1.BuildBVH(0, o1.Tri_index.size());
    s1.addObj(o1);
    s1.gen_global_BB();

    //send data to GPU

    GPULightSphere sun;
    sun.centre = make_float3(-50, 470, -90);
    sun.radius = 0.55f;
    sun.colour = make_float3(1.0f, 0.96f, 0.86f);
    sun.intensity = 1.5e1;
    h_lights.push_back(sun);


   
    size_t tri_off = 0,
        idx_off = 0,
        node_off = 0,
        mat_off = 0;

    int matid = 0;
    for (auto& obj : s1.ObjectList) {

        for (auto& t : obj.Tri_data) {
            GPUTriangle gt;
            gt.v0 = { t.v1.x, t.v1.y, t.v1.z };
            gt.v1 = { t.v2.x, t.v2.y, t.v2.z };
            gt.v2 = { t.v3.x, t.v3.y, t.v3.z };
            gt.n = { t.n.x, t.n.y, t.n.z };
            gt.material_id = matid;
            all_tris.push_back(gt);
        }

        for (int i : obj.Tri_index)
            all_tri_indices.push_back(i);


        for (auto& n : obj.BVH_Tree) {
            GPUBVHNode gn;
            gn.bbox.min = { n.box.min.x, n.box.min.y, n.box.min.z };
            gn.bbox.max = { n.box.max.x, n.box.max.y, n.box.max.z };
            gn.left = n.left;
            gn.right = n.right;
            gn.start = n.start;
            gn.count = n.count;
            all_bvh_nodes.push_back(gn);
        }


        m = obj.m;
        GPUMaterial gm;
        gm.albedo = { m.albedo.r, m.albedo.g, m.albedo.b };
        gm.roughness = m.roughness;
        gm.metallic = m.metallic;
        all_materials.push_back(gm);

        GPUObject go;
        go.tri_offset = tri_off;
        go.tri_count = (int)obj.Tri_index.size();
        go.node_offset = node_off;
        go.node_count = (int)obj.BVH_Tree.size();
        go.mat_offset = matid;


        all_objects.push_back(go);


        tri_off += obj.Tri_data.size();
        idx_off += obj.Tri_index.size();
        node_off += obj.BVH_Tree.size();
        mat_off += 1;

        matid++;
    }

    
    host_global_bb.min = { s1.global_bb.min.x,
                           s1.global_bb.min.y,
                           s1.global_bb.min.z };
    host_global_bb.max = { s1.global_bb.max.x,
                           s1.global_bb.max.y,
                           s1.global_bb.max.z };


    cudaMalloc(&d_tris, all_tris.size() * sizeof(GPUTriangle));
    cudaMalloc(&d_tri_indices, all_tri_indices.size() * sizeof(int32_t));
    cudaMalloc(&d_bvh_nodes, all_bvh_nodes.size() * sizeof(GPUBVHNode));
    cudaMalloc(&d_materials, all_materials.size() * sizeof(GPUMaterial));
    cudaMalloc(&d_objects, all_objects.size() * sizeof(GPUObject));
    cudaMalloc(&d_fb, width * height * sizeof(uchar4));
    cudaMalloc(&d_accum, width * height * sizeof(float3));


    cudaMemcpy(d_tris, all_tris.data(), all_tris.size() * sizeof(GPUTriangle), cudaMemcpyHostToDevice);
    cudaMemcpy(d_tri_indices, all_tri_indices.data(), all_tri_indices.size() * sizeof(int32_t), cudaMemcpyHostToDevice);
    cudaMemcpy(d_bvh_nodes, all_bvh_nodes.data(), all_bvh_nodes.size() * sizeof(GPUBVHNode), cudaMemcpyHostToDevice);
    cudaMemcpy(d_materials, all_materials.data(), all_materials.size() * sizeof(GPUMaterial), cudaMemcpyHostToDevice);
    cudaMemcpy(d_objects, all_objects.data(), all_objects.size() * sizeof(GPUObject), cudaMemcpyHostToDevice);


    GPUCamera h_cam;
    h_cam.pos = make_float3(0, 1.0f, 3.0f);

    glm::vec3 fwd = glm::normalize(glm::vec3(0, 0, -1));
    glm::vec3 right = glm::normalize(glm::cross(fwd, { 0,1,0 }));
    glm::vec3 up = glm::cross(right, fwd);

    h_cam.dir = make_float3(fwd.x, fwd.y, fwd.z);
    h_cam.right = make_float3(right.x, right.y, right.z);
    h_cam.up = make_float3(up.x, up.y, up.z);
    h_cam.focal_len = 1.0f;
    h_cam.lens_radius = h_cam.focal_len * tanf(glm::radians(0.5f)); 

    cudaMalloc(&d_cam, sizeof(GPUCamera));
    cudaMemcpy(d_cam, &h_cam, sizeof(GPUCamera), cudaMemcpyHostToDevice);

    
    cudaMalloc(&d_lights, h_lights.size() * sizeof(GPULightSphere));
    cudaMemcpy(d_lights, h_lights.data(),
        h_lights.size() * sizeof(GPULightSphere),
        cudaMemcpyHostToDevice);

    cudaMalloc(&d_accum, width* height * sizeof(float3));
    cudaMemset(d_accum, 0, width* height * sizeof(float3));

}


float runCuda() {
    cudaArray_t array;
    cudaGraphicsMapResources(1, &cuda_tex_resource);
    cudaGraphicsSubResourceGetMappedArray(&array, cuda_tex_resource, 0, 0);

    cudaResourceDesc desc{};
    desc.resType = cudaResourceTypeArray;
    desc.res.array.array = array;
    
    //end def
    dim3 block(32, 16);
    dim3 grid((width + block.x - 1) / block.x,
        (height + block.y - 1) / block.y);

    // CUDA Timing
    cudaEvent_t start, stop;
    float milliseconds = 0;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);

    render_kernel << <grid, block >> > (
        d_accum,
        d_cam,
        d_lights,
        h_lights.size(),
        frameID++,
        d_fb,
        width, height,
        d_tris,
        d_tri_indices,
        d_bvh_nodes,
        d_materials,
        d_objects,
        static_cast<int>(all_objects.size()),
        host_global_bb);

    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&milliseconds, start, stop);

    cudaMemcpy2DToArray(array, 0, 0,
        d_fb, width * sizeof(uchar4),
        width * sizeof(uchar4), height,
        cudaMemcpyDeviceToDevice);

    cudaGraphicsUnmapResources(1, &cuda_tex_resource);

    cudaEventDestroy(start);
    cudaEventDestroy(stop);

   

    return milliseconds;
}


void drawTexture() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glBegin(GL_QUADS);
 
    glTexCoord2f(0, 1);  glVertex2f(-1, 1);


    glTexCoord2f(1, 1);  glVertex2f(1, 1);


    glTexCoord2f(1, 0);  glVertex2f(1, -1);

    glTexCoord2f(0, 0);  glVertex2f(-1, -1);
    glEnd();
}

int main() {
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(width, height, "My Raytracer", nullptr, nullptr);
    if (!window) return -1;
    glfwMakeContextCurrent(window);

    glewInit();
    createTexture();

    
    using Clock = std::chrono::high_resolution_clock;
    auto lastTime = Clock::now();
    int frameCount = 0;
    setupScene();
    while (!glfwWindowShouldClose(window)) {
        auto frameStart = Clock::now();

        float kernelTime = runCuda();

        glClear(GL_COLOR_BUFFER_BIT);
        drawTexture();

        glfwSwapBuffers(window);
        glfwPollEvents();

        frameCount++;
        auto now = Clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime).count();
        if (elapsed >= 1000) {
            std::cout << "FPS: " << frameCount << " | Kernel time: " << kernelTime << " ms" << std::endl;
            frameCount = 0;
            lastTime = now;
        }
    }

    cudaGraphicsUnregisterResource(cuda_tex_resource);
    glDeleteTextures(1, &tex);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
