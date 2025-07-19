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

                        vec3 dir = pixel_00 + (delta_x*rand_no(0.992, 1.008))*i + (delta_y*rand_no(0.992, 1.008))*j - disk_point;
                        
                        ray r = ray(disk_point, dir);

                        //takes a ray and world to give its hit_data

                        hit_data h_d_front_obj = w.hit(r);
                        
                        //fn end


                        if(h_d_front_obj.index == 0){
                            //sky
                            //c_f = c_f + color(0, 0, 0);
                            //c_f = c_f + color(static_cast<double>(125)/255, static_cast<double>(187)/255, static_cast<double>(210)/255); //bg color
                            c_f = c_f + w.get_env_color(r);
                            
                        }
                        else{
                            
                            //lighting 
                            int max_bounce = 1000;
                            //ray r_b = r;
                            //color sky_color(1, 1, 1);
                            

                            //run this in a loop and avg c_obs;
                            int max_pass_per_pixel = 1200;
                            color c_obs_avg  = color(0, 0, 0);

                            for(int passes =0; passes<max_pass_per_pixel; ++passes){
                            //color c_obs = h_d_front_obj.visible_final_color;
                            color ray_color = h_d_front_obj.visible_final_color; 
                            color inc_light = h_d_front_obj.mat.emission_c * h_d_front_obj.mat.emission_str;
                            ray r_b = r;
                            hit_data h_d_front_obj_c = h_d_front_obj;

                            for(int count =0; count < max_bounce; ++count){
                                r_b = ray(h_d_front_obj_c.point, rand_ray_bounce_dir(h_d_front_obj_c.normal));
                                h_d_front_obj_c = w.hit(r_b);

                                if(h_d_front_obj_c.index == 0){
                                    //c_obs_avg = c_obs_avg + c_obs;
                                    inc_light = inc_light + elm_multiply(ray_color, w.get_env_color(r_b));
                                    break;
                                }

                                else{

                                    ray_color = elm_multiply(ray_color, h_d_front_obj_c.visible_final_color);
                                    inc_light = inc_light + h_d_front_obj_c.mat.emission_c * h_d_front_obj_c.mat.emission_str;
                                    //c_obs = c_obs * 0.1;
                                    //color emit_light = h_d_front_obj_c.mat.emission_c * h_d_front_obj_c.mat.emission_str;
                                    //inc_light = inc_light + elm_multiply(emit_light, c_obs);
                                    // c_obs = elm_multiply(c_obs, h_d_front_obj_c.visible_final_color);

                                }
                                

                            }
                            c_obs_avg = c_obs_avg + inc_light;
                            //c_obs_avg = c_obs_avg + c_obs;
                            


                            }

                            c_obs_avg = c_obs_avg/max_pass_per_pixel;
                            c_f = c_f + c_obs_avg;

                            //vec3 light = vec3(1, 0, 0).normalized();
                            //float a = 0.5*(h_d.normal * light)+0.5; 
                            //color c_ = vec3(1, 1, 1)*a + ((*obj).m.albedo)*(1-a);
                            //c_f = c_f + (*obj).m.albedo;
                            //c_f = c_f + c_;
                        }

                        //c_f = c_f + c_obs_avg;

                    }

                    write_color(file_, c_f/max_samples);


                    
                }
            }

        }




    }

};


#endif