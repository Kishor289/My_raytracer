#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <vector>
#include "renderer.h"

using namespace std;
class param_set{
public:
    vector<double> params;

};

class material{
    public:
    vec3 albedo;

    material():albedo(vec3(1, 1, 1)){}
    material(vec3 alb):albedo(alb){}

};

class hit_data{
    public:
    double point_t;
    vec3 point;
    vec3 normal;

};
#endif