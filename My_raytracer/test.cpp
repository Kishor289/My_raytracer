#include "random.h"
#include <iostream>
#include "vec3.h"
#include "img_read.h"
#include "utility.h"
#include "procedural_text.h"
#include <ostream>
#include <fstream>

int main(){
    int img_height = 400;
    int img_width = 400;
    std::ofstream file_;
    file_.open("texture_text.ppm");

    if(file_.is_open()){
        file_ << "P3\n";
        file_ << img_width ;
        file_ << " ";
        file_ << img_height;
        file_ << "\n255\n";

        for(int j=0; j<img_height; ++j){
            for(int i=0; i<img_width; ++i){
                double f = 30.8634;
                color c = vec3(1, 1, 1) * perlinNoise(f*i, f*j);
                write_color(file_, c);
            }
        }

    }


    return 0;
}