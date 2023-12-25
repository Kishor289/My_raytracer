#include "random.h"
#include <iostream>
#include "vec3.h"
#include "img_read.h"
#include "utility.h"
#include "procedural_text.h"
#include <ostream>
#include <fstream>

int main(){
    
    //vec3 u(0, 0, 0);
    double s =0;
    int  k = 100000;
    for(int i=0; i<k; i++){
        vec3 u = random_unit_vector();
        s+= u.x()*u.x() + u.y()*u.y();

    }

    s=s/k;

    std::cout <<  s <<std::endl;

    //moi about z axis



    return 0;
}