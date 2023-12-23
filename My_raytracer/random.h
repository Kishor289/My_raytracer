#ifndef RANDOM_H
#define RANDOM_H

#include "cstdlib"
#include "time.h"
#include "vec3.h"
#include "utils.h"

double rand_no(double a, double b){
    //srand(time(NULL));

    int r = rand() % 456;
    double r_ = static_cast<double>(r)/456; //0 to 1
    return r_*(b-a) + a;

}

vec3 rand_disk_point(double r, vec3 x_, vec3 y_){
    double x = rand_no(0, r);
    double p = 3.14159 * 2;
    double y= rand_no(0, p);

    //vec3 x_ = vec3(-norm.y(), -norm.x(), 0).normalized();
    //vec3 y_ = (x_ ^ norm).normalized();
    return x_ * (x*cos(y)) + y_ *(x*sin(y));
}

vec3 rand_ray_bounce_dir(vec3 n){
    vec3 u;
    u.e[0] = rand_no(0, 1);
    u.e[1] = rand_no(0, 1);
    u.e[2] = sqrt(1-u.e[0]*u.e[0] - u.e[1]*u.e[1]);
    return n + u;
}



#endif