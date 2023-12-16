#ifndef RENDERER_H
#define RENDERER_H

#include "vec3.h"
#include "color.h"
#include "ray.h"
#include "camera.h"
#include "world.h"
#include "structures.h"
#include "random.h"

#include <iostream>
#include <cmath>
#include <string>
#include <fstream>
#include <ostream>
#include <chrono>

using namespace std;

class renderer{

public:
    camera c;
    vec3 pixel_00;
    vec3 delta_x;
    vec3 delta_y;
    world w;
    int max_samples;

    renderer(camera c_, world w_, int s) : c(c_), w(w_), max_samples(s){}

    void init(){

        vec3 view_dir = c.look_at-c.look_from;
        vec3 viewport_center = c.look_from + (view_dir.normalized()) * (c.focal_length);

        vec3 up = vec3(sin(-c.roll_angle), cos(-c.roll_angle), 0);
        vec3 y_ = up.perp_to(view_dir).normalized();
        vec3 x_ = (y_ ^ view_dir).normalized();
        vec3 viewport_corner = viewport_center + x_ * c.viewport_width/2 + y_ * c.viewport_height/2;

        delta_x = x_ * (-c.viewport_width/c.img_width);
        delta_y = y_ * (-c.viewport_height/c.img_height);

        pixel_00 = viewport_corner + (delta_x+delta_y) * 0.5;

    }

    void render(){
        ofstream file_;
        file_.open("output.ppm");

        if(file_.is_open()){
            file_ << "P3\n";
            file_ << c.img_width ;
            file_ << " ";
            file_ << c.img_height;
            file_ << "\n255\n";

            for(int j=0; j<c.img_height; ++j){
                for(int i=0; i<c.img_width; ++i){

                    color c_f = color(0, 0, 0);
                    for(int samples = 0 ; samples<max_samples; ++samples){
                        
                        double r_ = c.focal_length * tan(c.d_focus_angle);

                        vec3 disk_point = c.look_from + rand_disk_point(r_,delta_x.normalized(), delta_y.normalized());

                        vec3 dir = pixel_00 + (delta_x*rand_no(0.995, 1.002))*i + (delta_y*rand_no(0.995, 1.002))*j - disk_point;
                        
                        ray r = ray(disk_point, dir);

                        double t_lowest_till = 100000;
                        hittable* obj = NULL;
                        hit_data h_d;
                        hit_data h_d_front_obj;
                        for(auto i = w.hittable_list.begin() ; i!=w.hittable_list.end(); ++i){
                            
                            if((*i)->ray_hit(r, h_d) == true){
                                if(h_d.point_t < t_lowest_till){
                                    t_lowest_till = h_d.point_t;
                                    obj = &(**i);
                                    h_d_front_obj = h_d;
                                }
                            }

                        }

                        if(obj == NULL){
                            //sky
                            double a = 0.5*r.direction().normalized().y()+1;
                            c_f = c_f + color(1.0, 1.0, 1.0) * (1-a) + color(0.5, 0.7, 1.0) * a;
                        }
                        else{
                            //lighting code
                            
                            

                            //vec3 light = vec3(1, 0, 0).normalized();
                            //float a = 0.5*(h_d.normal * light)+0.5; 
                            //color c_ = vec3(1, 1, 1)*a + ((*obj).m.albedo)*(1-a);
                            c_f = c_f + (*obj).m.albedo;
                            //c_f = c_f + c_;
                        }

                    }

                    write_color(file_, c_f/max_samples);


                    
                }
            }

        }




    }

};


#endif