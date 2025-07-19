#ifndef RANDOM_H
#define RANDOM_H

#include "cstdlib"
#include "time.h"
#include "vec3.h"
#include "utils.h"
#include <random>
#include <chrono>
#include <cmath>

double rand_no(double a, double b){

    int r = rand() % 456;
    double r_ = static_cast<double>(r)/456; //0 to 1
    return r_*(b-a) + a;

}

vec3 random_unit_vector(){
    auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::default_random_engine e(seed);
    std::normal_distribution<double> d(0, 1);

    vec3 u;
    u.e[0] = d(e);
    u.e[1] = d(e);
    u.e[2] = d(e);

    return u.normalized();
    
}

vec3 rand_disk_point(double r, vec3 x_, vec3 y_){

    //x_ and y_ are unit vectors , we need to return uniform points in a radius r

    double rho = sqrt(rand_no(0, r));
    double p = 3.1415926535 * 2;
    double theta = (rand_no(0, p));

    //vec3 x_ = vec3(-norm.y(), -norm.x(), 0).normalized();
    //vec3 y_ = (x_ ^ norm).normalized();
    return x_ * (rho*cos(theta)) + y_ *(rho*sin(theta));
}

// vec3 rand_ray_bounce_dir(vec3 n){
//     vec3 u;
//     u.e[0] = rand_no(0, 1);
//     u.e[1] = rand_no(0, 1);
//     u.e[2] = sqrt(1-u.e[0]*u.e[0] - u.e[1]*u.e[1]);
//     return (n + u).normalized();
// }

// double random_gaussian(){

//     double pi = PI;
//     double theta =  (2*pi)*rand();
//     double rho = sqrt(-2*log(rand()));
//     return rho*cos(theta);
// }

vec3 rand_ray_bounce_dir(vec3 n){
    return (n.normalized() + random_unit_vector()).normalized();
}

// vec3 random_unit_vector(){
//     srand(time(NULL));
//     vec3 u;
//     u.e[0] = rand_no(-1, 1);
//     u.e[1] = rand_no(-1, 1);
//     u.e[2] = sqrt(1-u.e[0]*u.e[0] - u.e[1]*u.e[1]);

//     return u;
    
// }










#endif