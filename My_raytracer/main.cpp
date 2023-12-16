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
    int img_width = 400;
    int img_height = 300; 

    //camera parameters
    float focal_length = 3.00;
    float d_fov = 40;

    //creating scene
    world w;

    Sphere s1(vec3(0, 0, -3), 0.4, vec3(1, 0.3, 0));
    w.add_(s1);
    Sphere s3(vec3(-1.6, 0, -10), 1, vec3(0, 0.2, 1));
    w.add_(s3);

    Sphere s2(vec3(0, -100.5, -20), 100, vec3(0, 1, 0.3));
    w.add_(s2);
    
    //creating camera
    camera cam(focal_length, d_fov, img_height, img_width);
    cam.look_from = vec3(0, 0, 0);
    cam.look_at = vec3(0, 0, -1);
    cam.roll_angle = convert_rad(0); // cw rotation
    cam.d_focus_angle = convert_rad(1.4); // half angle of cone/;

    renderer rend_(cam, w, 50);

    rend_.init();

    //auto start = std::chrono::high_resolution_clock::now();
    rend_.render();

    //auto end = std::chrono::high_resolution_clock::now();

    //auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    //std::cout << "Execution time: " << duration << " ms" << std::endl;

    //rend_.flush();

}