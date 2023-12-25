#ifndef HITTABLE_H
#define HITTABLE_H

#include "renderer.h"
#include "structures.h"
#include <math.h>
#include <cmath>
//#include "stb_image.h"
#include "img_read.h"
#include "utility.h"
#include "color.h"

class hittable{

public:
    vec3 pos;
    param_set p_;
    material m;
    uint8_t *img_text_data;
    int img_text_width;
    int img_text_height;
    int textured;
    color c1;
    color c2;

    hittable(vec3 p, material m_) : pos(p), m(m_){}
    ~hittable(){
        if(textured == 1){
            stbi_image_free(img_text_data);
        }
    }

    virtual bool ray_hit(ray& r, hit_data& h_data) =0;


};

class Sphere : public hittable{

public:


    Sphere(vec3 c, double r, color alb) : hittable(c,  material(alb)){
        p_.params.push_back(r);
        textured = 0;

    }

    Sphere(vec3 c, double r, std::string text_file_name) : hittable(c,  material(color(0, 0, 0))){
        p_.params.push_back(r);

        int bpp;
        stbi_info(text_file_name.c_str(), &img_text_width, &img_text_height, &bpp);
        img_text_data = stbi_load(text_file_name.c_str(), &img_text_width, &img_text_height, &bpp, 3);
        textured = 1;
    }

    bool ray_hit(ray& r, hit_data& h_data) override{

        vec3 oc = r.origin_() - this->pos;
        double a = r.direction()* r.direction();
        double b_h = oc * r.dir;
        double c = oc * oc - this->p_.params[0] * this->p_.params[0];
        double d = b_h*b_h - a*c;

        if(d>=0.001f){

            double t_low = (-b_h - sqrt(d))/a;
            if(t_low<0.0001){
                return false;
            }

            h_data.point_t = t_low;
            h_data.normal = (r.at(t_low) - this->pos).normalized();
            h_data.point = r.at(t_low);
            h_data.index = 1;

            //assign color correctly as per texture
            color t_c;
            if(textured == 1){
                double pi = PI;
                auto p = h_data.point - pos;
                auto theta = acos(-p.normalized().y());
                auto phi = atan2(-p.normalized().z(), p.normalized().x()) + pi;
                auto u = (phi) / (2*pi);
                auto v = theta / pi;

                int p_x = static_cast<int>(u * img_text_width);
                int p_y = static_cast<int>(v * img_text_height);
                int pixel_no = img_text_width * p_y + p_x;

                float r = static_cast<float>(img_text_data[3 * pixel_no]) / 255.0f;
                float g = static_cast<float>(img_text_data[3 * pixel_no + 1]) / 255.0f;
                float b = static_cast<float>(img_text_data[3 * pixel_no + 2]) / 255.0f;

                t_c = color(r, g, b);
            }

            else if(textured == 0){
                t_c = m.albedo;
            }

            else{

                double pi = PI;
                auto p = h_data.point - pos;
                auto theta = acos(-p.normalized().y());
                auto phi = atan2(-p.normalized().z(), p.normalized().x()) + pi;
                auto u = (phi) / (2*pi);
                auto v = theta / pi;

                t_c = checker_texture(u, v, 15, c1, c2);

            }
            //fn end 

            h_data.mat = m;
            h_data.visible_final_color=t_c;

            return true;
        }

        else{
            h_data.index = 0;
            return false;
        }

    }

};


class Quadrilateral :public hittable{

public:
    Quadrilateral(vec3 bot_left, vec3 length_v, vec3 width_v, color col) : hittable(bot_left, material(col)){

        p_.params.push_back(length_v.x());
        p_.params.push_back(length_v.y());
        p_.params.push_back(length_v.z());
        p_.params.push_back(width_v.x());
        p_.params.push_back(width_v.y());
        p_.params.push_back(width_v.z());

        textured = 0;

    }

    Quadrilateral(vec3 bot_left, vec3 length_v, vec3 width_v, std::string text_file_name) : hittable(bot_left, material(color(0, 0, 0))){

        p_.params.push_back(length_v.x());
        p_.params.push_back(length_v.y());
        p_.params.push_back(length_v.z());
        p_.params.push_back(width_v.x());
        p_.params.push_back(width_v.y());
        p_.params.push_back(width_v.z());

        textured = 1;

        int bpp;
        stbi_info(text_file_name.c_str(), &img_text_width, &img_text_height, &bpp);
        img_text_data = stbi_load(text_file_name.c_str(), &img_text_width, &img_text_height, &bpp, 3);

    }



    bool ray_hit(ray& r, hit_data& h_data) override{

        //std::cout << "hh";

        auto u = vec3(p_.params[0], p_.params[1], p_.params[2]);
        auto v = vec3(p_.params[3], p_.params[4], p_.params[5]);

        auto n = (u ^ v); //cross

        // if((r.direction().normalized())*n.normalized() <= 1.0E-20){
        //     h_data.index = 0;
        //     return false;
        // }

        //std::cout << "h";

        auto t_0 = ((pos - r.origin_())*n)/(r.direction()*n);

        if(t_0 <= 0.001){
            h_data.index = 0;
            return false;
        }

        auto w = n/(n*n);

        double a = w*((r.at(t_0) - pos) ^ v);
        double b = w*(u ^ (r.at(t_0) - pos));

        if(!interval(a, 0.0f, 1.0f) || !interval(b, 0.0f, 1.0f)){
            h_data.index = 0;
            return false;
        }

        h_data.point_t = t_0;
        h_data.point = r.at(t_0);
        h_data.index =1;
        h_data.normal = n.normalized();

        color t_c;
        if(textured == 1){

            int p_x = static_cast<int>(a * img_text_width);
            int p_y = static_cast<int>(b * img_text_height);
            int pixel_no = img_text_width * p_y + p_x;

            float r = static_cast<float>(img_text_data[3 * pixel_no]) / 255.0f;
            float g = static_cast<float>(img_text_data[3 * pixel_no + 1]) / 255.0f;
            float b = static_cast<float>(img_text_data[3 * pixel_no + 2]) / 255.0f;

            t_c = color(r, g, b);

        }
        else if(textured == 0){
            t_c = m.albedo;
        }
        else{
            t_c = checker_texture(a, b, 200, c1, c2);
        }

        h_data.mat = m;
        h_data.visible_final_color=t_c;
        
        return true;


    }


};


class Triangle :public hittable{

public:
    Triangle(vec3 bot_left, vec3 length_v, vec3 width_v, color col) : hittable(bot_left, material(col)){

        p_.params.push_back(length_v.x());
        p_.params.push_back(length_v.y());
        p_.params.push_back(length_v.z());
        p_.params.push_back(width_v.x());
        p_.params.push_back(width_v.y());
        p_.params.push_back(width_v.z());

        textured = 0;

    }

    Triangle(vec3 bot_left, vec3 length_v, vec3 width_v, std::string text_file_name) : hittable(bot_left, material(color(0, 0, 0))){

        p_.params.push_back(length_v.x());
        p_.params.push_back(length_v.y());
        p_.params.push_back(length_v.z());
        p_.params.push_back(width_v.x());
        p_.params.push_back(width_v.y());
        p_.params.push_back(width_v.z());

        textured = 1;

        int bpp;
        stbi_info(text_file_name.c_str(), &img_text_width, &img_text_height, &bpp);
        img_text_data = stbi_load(text_file_name.c_str(), &img_text_width, &img_text_height, &bpp, 3);

    }



    bool ray_hit(ray& r, hit_data& h_data) override{

        //std::cout << "hh";

        auto u = vec3(p_.params[0], p_.params[1], p_.params[2]);
        auto v = vec3(p_.params[3], p_.params[4], p_.params[5]);

        auto n = (u ^ v); //cross

        // if((r.direction().normalized())*n.normalized() <= 1.0E-20){
        //     h_data.index = 0;
        //     return false;
        // }

        //std::cout << "h";

        auto t_0 = ((pos - r.origin_())*n)/(r.direction()*n);

        if(t_0 <= 0.001){
            h_data.index = 0;
            return false;
        }

        auto w = n/(n*n);

        double a = w*((r.at(t_0) - pos) ^ v);
        double b = w*(u ^ (r.at(t_0) - pos));

        if(!interval(a, 0.0f, 1.0f) || !interval(b, 0.0f, 1.0f - a)){
            h_data.index = 0;
            return false;
        }

        h_data.point_t = t_0;
        h_data.point = r.at(t_0);
        h_data.index =1;
        h_data.normal = n.normalized();

        color t_c;
        if(textured == 1){

            int p_x = static_cast<int>(a * img_text_width);
            int p_y = static_cast<int>(b * img_text_height);
            int pixel_no = img_text_width * p_y + p_x;

            float r = static_cast<float>(img_text_data[3 * pixel_no]) / 255.0f;
            float g = static_cast<float>(img_text_data[3 * pixel_no + 1]) / 255.0f;
            float b = static_cast<float>(img_text_data[3 * pixel_no + 2]) / 255.0f;

            t_c = color(r, g, b);

        }
        else if(textured == 0){
            t_c = m.albedo;
        }
        else{
            t_c = checker_texture(a, b, 200, c1, c2);
        }

        h_data.mat = m;
        h_data.visible_final_color=t_c;
        
        return true;


    }


};



#endif