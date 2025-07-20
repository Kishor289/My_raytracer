
#pragma once
#include <cstdint>
#include <vector_types.h>


struct GPU_AABB {
    float3 min;
    float3 max;
};

struct GPUTriangle {
    float3 v0, v1, v2;
    float3 n;
    int32_t material_id;
};

struct GPUBVHNode {
    GPU_AABB    bbox;
    int32_t left;    
    int32_t right;   
    int32_t start;   
    int32_t count;   
};

struct GPUMaterial {
    float3 albedo;
    float  roughness;
    float  metallic;
    
};

struct GPUObject {
    int32_t tri_offset;
    int32_t tri_count;
    int32_t node_offset;
    int32_t node_count;
    int32_t mat_offset;
    float   transform[16];     
    float   inv_transform[16];  
};

struct GPUCamera
{
    float3 pos;        
    float3 dir;        
    float3 right;      
    float3 up;          
    float  focal_len; 
    float  lens_radius; 
};


struct GPULightSphere
{
    float3 centre;
    float  radius;
    float3 colour;   
    float  intensity;
};
