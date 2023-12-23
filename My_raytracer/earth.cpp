#include "vec3.h"
#include "color.h"
#include "ray.h"
#include "camera.h"
#include "renderer.h"
#include "utils.h"
//#include "renderer_mthread.h"

#include <iostream>
#include <chrono>

int main(){
    int img_width = 400;
    int img_height = 300;

    float focal_length = 8.00;
    float d_fov = 40;

    //creating scene
    world w;

    Sphere s1 = Sphere(vec3(0, 0, -8), 2, "earth_2.jpg");
    w.add_(s1);

    //creating camera
    camera cam(focal_length, d_fov, img_height, img_width);
    float pi = PI;
    int n= 45;
    float theta = n*(pi/20);
    cam.look_from = vec3(8*cos(theta), 0, 8*sin(theta)-8);
    cam.look_at = vec3(0, 0, -8);
    cam.roll_angle = convert_rad(0);
    cam.d_focus_angle = convert_rad(3);

    renderer rend_(cam, w, 1);

    rend_.init();

    rend_.render();
    return 0;
}