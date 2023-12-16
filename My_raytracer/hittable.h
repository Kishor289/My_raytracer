#ifndef HITTABLE_H
#define HITTABLE_H

#include "renderer.h"
#include "structures.h"

class hittable{

public:
    vec3 pos;
    param_set p_;
    material m;

    hittable(vec3 p, material m_) : pos(p), m(m_){}

    virtual bool ray_hit(ray& r, hit_data& h_data) const =0;

};

class Sphere : public hittable{

public:

    Sphere(vec3 c, double r, color alb) : hittable(c,  material(alb)){
        p_.params.push_back(r);
    }

    bool ray_hit(ray& r, hit_data& h_data) const override{

        vec3 oc = r.origin_() - this->pos;
        double a = r.direction()* r.direction();
        double b_h = oc * r.dir;
        double c = oc * oc - this->p_.params[0] * this->p_.params[0];
        double d = b_h*b_h - a*c;

        if(d>=0){

            double t_low = (-b_h - sqrt(d))/a;
            if(t_low<0){
                return false;
            }

            h_data.point_t = t_low;
            h_data.normal = (r.at(t_low) - this->pos).normalized();
            h_data.point = r.at(t_low);

            return true;
        }

        else{
            return false;
        }

    }



};

#endif