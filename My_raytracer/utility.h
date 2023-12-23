#ifndef UTILITY_H
#define UTILITY_H

#include "vec3.h"

bool interval(double p, double min_, double max_){
    if(p>=min_ && p<=max_){
        return true;
    }
    else{
        return false;
    }

}

bool interval(vec3 p, vec3 min_, vec3 max_){
    auto a = interval(p.x(), min_.x(), max_.x());
    auto b = interval(p.y(), min_.y(), max_.y());
    auto c = interval(p.z(), min_.z(), max_.z());
    if(a && b && c){
        return true;
    }
    else{
        return false;
    }
}

bool interval(vec3 p, double min_, double max_){
    auto a = interval(p.x(), min_, max_);
    auto b = interval(p.y(), min_, max_);
    auto c = interval(p.z(), min_, max_);

    if(a && b && c){
        return true;
    }
    else{
        return false;
    }
}

double clamp(double p, double min_, double max_){
    if(p>max_){
        return max_;
    }
    else if(p<min_){
        return min_;
    }
    else{
        return p;
    }
}

vec3 clamp(vec3 p, double min_, double max_){
    return vec3(clamp(p.x(), min_, max_), clamp(p.y(), min_, max_), clamp(p.z(), min_, max_));
}

color checker_texture(double u, double v, int d, color c1, color c2){
    
    int text_width = 400;
    int text_height = 800;

    int u_ = static_cast<int>(u*text_width) % d;
    int v_ = static_cast<int>(v*text_height) % d;

    color t_c;
    
    //check psattern
    if(u_ < d/2 && v_< d/2){
        t_c = c1;
    }
    else if(u_ < d/2 && v_> d/2){
        t_c = c2;
    }
    else if(u_ > d/2 && v_> d/2){
        t_c = c1;
    }
    else{
        t_c = c2;
    }

    return t_c;
}



#endif