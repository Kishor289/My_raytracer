#ifndef WORLD_H
#define WORLD_H

#include "renderer.h"
#include "hittable.h"
#include <vector>

class world{

public:
    vector<hittable*> hittable_list;

    void add_(hittable& h){
        hittable_list.push_back(&h);
    }

    hit_data hit(ray r_b){
        double t_lowest_till = 100000;
        hit_data h_d;
        hit_data h_d_front_obj;
        for(auto i = this->hittable_list.begin() ; i!=this->hittable_list.end(); ++i){
                            
            if((*i)->ray_hit(r_b, h_d) == true){
                if(h_d.point_t < t_lowest_till){
                    t_lowest_till = h_d.point_t;
                    h_d_front_obj = h_d;
                }
            }

        }

        return h_d_front_obj;
    }

    color get_env_color(ray r){
        return color(0,0,0);
        // double a = 0.5*r.direction().normalized().y()+0.5;
        // if(a<=0.2){
        //     return color(1, 1, 1);
        // }
        // else{
        //     a = 1.25f*(a-0.2); //0 to 0.8
        //     return color(1.0, 1.0, 1.0) * (1-a) + color(static_cast<double>(22)/255, static_cast<double>(75)/255, static_cast<double>(118)/255) * a;
        // }
    }

};
#endif