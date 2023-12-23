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
    vec3 emission_c = vec3(0, 0, 0);
    double emission_str = 0.0f;

    material():albedo(vec3(1, 1, 1)), emission_c(vec3(0, 0, 0)), emission_str(0){}
    material(vec3 alb):albedo(alb){}

};

class hit_data{
    public:
    double point_t;
    vec3 point;
    vec3 normal;
    int index = 0;
    material mat;
    color visible_final_color = color(0, 0, 0);

};
#endif