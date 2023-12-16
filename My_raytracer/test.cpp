#include "random.h"
#include <iostream>
#include "vec3.h"

int main(){
    for(int i=0; i<100; ++i){
        std::cout<<rand_disk_point(1, vec3(0, 0, -1), vec3(1, 0, 0), vec3(0, 1, 0))<<std::endl;
    }
    
}