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

};
#endif