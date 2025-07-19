#ifndef RAY_H
#define RAY_H

#include "vec3.h"

class ray{
public:
    vec3 origin;
    vec3 dir;

    ray(vec3 origin, vec3 dir){
        this->origin = origin;
        this->dir = dir;
    }

    vec3 at(float t){
        return this->origin + this->dir*t;
    }

    vec3 origin_(){
        return origin;
    }

    vec3 direction(){
        return dir;
    }

};

#endif