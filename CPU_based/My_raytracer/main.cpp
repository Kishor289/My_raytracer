#include "vec3.h"
#include "color.h"
#include "ray.h"
#include "camera.h"
#include "renderer.h"
#include "utils.h"
//#include "renderer_mthread.h"

#include <iostream>
#include <chrono>

using namespace std;

int main(){

    // auto start = std::chrono::high_resolution_clock::now();
    //img parameters
    int img_width = 200;
    int img_height = 200; 

    //camera parameters
    float focal_length = 8.00;
    float d_fov = 35;

    //creating scene
    world w;

    // Sphere s1(vec3(1, 0.24, -8), 1.4, vec3(0, static_cast<double>(180)/255, static_cast<double>(220)/255)); //180
    // // s1.m.emission_c = vec3(1, 1, 1);
    // s1.m.emission_str = 1.0f;
    Sphere s1(vec3(4.1, 3.24, -12), 3, color(0, 1, 0)); //180
    s1.m.emission_c = color(1, 201/255.0f, 15/255.0f);
    s1.m.emission_str = 20.0f;
    w.add_(s1);
    Sphere s3(vec3(-1, 0.24, -8), 1.4, color(1, 0, 0)); //171 24 57
    //s3.m.emission_c = vec3(1, 1, 1);
    //s3.m.emission_str = 90.0f;
    
    w.add_(s3);

    Sphere s2(vec3(0, -100.5, -20), 100, color(0.7, 0.7, 0.7));
    // Sphere s2(vec3(0, -100.5, -20), 100, "grass_hres_3.jpeg");
    //s2.textured = 3;
    //s2.c1 = color(191.0f, 226.0f, 244.0f)/255.0f;
    //s2.c2 = color(3.0f, 41.0f, 56.0f)/255.0f;
    w.add_(s2);

    // world w2;
    // Quadrilateral q1(vec3(-2, -2, 0), vec3(4, 0, 0), vec3(0, 4, 0), "grass_hres_2.jpeg");
    // //q1.textured = 3;
    // Quadrilateral q2(vec3(2.5, -2, 0), vec3(4*cos(1.25), 0, 4*sin(1.25)), vec3(0, 4, 0), color(0, 0, 1));
    // w2.add_(q2);
    // w2.add_(q1);
    
    //creating camera
    camera cam(focal_length, d_fov, img_height, img_width);
    cam.look_from = vec3(0, 1, 4);
    cam.look_at = vec3(0, 0.6, -1);
    cam.roll_angle = convert_rad(0); // cw rotation
    cam.d_focus_angle = convert_rad(0.0f); // half angle of cone/;

    renderer rend_(cam, w, 400);

    rend_.init();

    auto start = std::chrono::high_resolution_clock ::now();
    rend_.render();

    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Execution time: " << duration << " ms" << std::endl; 

    //rend_.flush();

}