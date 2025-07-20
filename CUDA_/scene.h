#pragma once
#include "glm/glm.hpp"
#include "utils.h"

#include "Object.h"
#include "camera.h"

#include <vector>     

#include <iostream>

class Scene {
public:
	std::vector<Object> ObjectList;
	Camera cam;
	AABB global_bb;

	Scene(Camera cam) : cam(cam){}
	void addObj(Object& obj) {
		ObjectList.push_back(obj);
	}
	void gen_global_BB() {
		if (ObjectList.empty()) return;
		for (auto& o : ObjectList)
			if (!o.BVH_Tree.empty())
				global_bb.expand(o.BVH_Tree[0].box);
	}

	
};