#define GLM_ENABLE_EXPERIMENTAL

#include "Object.h"
#include <iostream>
#include "utils.h"

#include <algorithm> 
#include <stdexcept>  
#include <cmath>     
#include <cfloat>    
#include <vector>    


void Object::load_obj(const char* file) {
	ReadObj(file, this->Tri_data);
	for (int i = 0; i < this->Tri_data.size(); i++) {
		this->Tri_index.push_back(i);
	}

}

void Object::apply_transform_to_vertices()
{
	for (Tris& t : Tri_data) {
		t.v1 = glm::vec3(transform * glm::vec4(t.v1, 1.0f));
		t.v2 = glm::vec3(transform * glm::vec4(t.v2, 1.0f));
		t.v3 = glm::vec3(transform * glm::vec4(t.v3, 1.0f));

		// recompute flat normal in world space
		glm::vec3 e1 = t.v2 - t.v1;
		glm::vec3 e2 = t.v3 - t.v1;
		t.n = glm::normalize(glm::cross(e1, e2));
	}
}


void Object::set_pos_rot(glm::vec3 pos, glm::vec3 rot) {
	this->transform = glm::mat4(1.0f);
	this->transform = glm::translate(this->transform, pos);
	this->transform = this->transform * (glm::yawPitchRoll(rot.y, rot.x, rot.z));
	this->inv_transform = glm::inverse(this->transform);
	this->apply_transform_to_vertices();


}

void Object::set_material(material mat) {
	this->m = mat;
}

Object::Object(const char* file, glm::vec3 pos, glm::vec3 rot, material m) {
	load_obj(file);
	set_pos_rot(pos, rot);
	set_material(m);

}

void Object::BuildBVH(int start, int count) {
	BVHNode node;
	for (int i = start; i < start + count; i++) {
		node.box.expand(this->Tri_data[this->Tri_index[i]].v1);
		node.box.expand(this->Tri_data[this->Tri_index[i]].v2);
		node.box.expand(this->Tri_data[this->Tri_index[i]].v3);
	}

	if (count <= this->leaf_limit) {
		node.left = -1;
		node.right = -1;
		node.start = start;
		node.count = count;
		(this->BVH_Tree).push_back(node);
		return;
	}

	glm::vec3 c = node.box.max - node.box.min;
	int split_axis;
	if (c.x >= c.y && c.x >= c.z) split_axis = 0;
	else if (c.y >= c.x && c.y >= c.z) split_axis = 1;
	else split_axis = 2;

	//now put tris to bucket along this chosen axis
	std::vector<Bucket> buckets(this->BUCKET_SIZE);
	for (int i = start; i < start + count; i++) {
		int c_bkt;
		float extent = node.box.max[split_axis] - node.box.min[split_axis];
		if (fabs(extent) > 1e-6f * std::max(1.0f, fabs(node.box.min[split_axis]) + fabs(node.box.max[split_axis]))) c_bkt = ((Tri_data[Tri_index[i]].get_centroid()[split_axis] - node.box.min[split_axis]) * (BUCKET_SIZE)) / (node.box.max[split_axis] - node.box.min[split_axis]);
		else c_bkt = 0;

		c_bkt = clamp(c_bkt, 0, BUCKET_SIZE - 1);

		buckets[c_bkt].elements_box.expand(Tri_data[Tri_index[i]].v1);
		buckets[c_bkt].elements_box.expand(Tri_data[Tri_index[i]].v2);
		buckets[c_bkt].elements_box.expand(Tri_data[Tri_index[i]].v3);
		buckets[c_bkt].size++;
	}

	//iterate ovre all partition optios bucket_size-1
	int chosen_bkt = 0;
	float cost = FLT_MAX;
	int chosen_l_cnt = 0;
	int chosen_r_cnt = 0;
	for (int i = 1; i < BUCKET_SIZE; i++) {
		//calculate SAH for the partition after bucket i
		AABB lb;
		AABB rb;
		int left_cnt = 0;
		int right_cnt = 0;
		for (int j = 0; j < i; j++) {
			if (buckets[j].size)  lb.expand(buckets[j].elements_box);
			left_cnt += buckets[j].size;
		}
		for (int j = i; j < BUCKET_SIZE; j++) {
			if(buckets[j].size) rb.expand(buckets[j].elements_box);
			right_cnt += buckets[j].size;
		}
		float SAH_cost = 1.0f + ((lb.surface_area() * left_cnt) / node.box.surface_area()) + ((rb.surface_area() * right_cnt) / node.box.surface_area());
		if (SAH_cost < cost) {
				cost = SAH_cost;
				chosen_bkt = i;
				chosen_l_cnt = left_cnt;
				chosen_r_cnt = right_cnt;
		}

	}
		//chosen bkt will be atleast 1 and upto N-1;
		//now we hav chosenthe optimum bvh set, just partition and recurse;
		//ipdate the index array and push to vec thsi node na d call recussive correctly
	int k = start;
	for (int i = start; i < start + count; i++) {

		float extent = node.box.max[split_axis] - node.box.min[split_axis];
		int c_bkt;
		if (fabs(extent) > 1e-6f * std::max(1.0f, fabs(node.box.min[split_axis]) + fabs(node.box.max[split_axis]))) c_bkt = ((Tri_data[Tri_index[i]].get_centroid()[split_axis] - node.box.min[split_axis]) * (BUCKET_SIZE)) / (node.box.max[split_axis] - node.box.min[split_axis]);
		else c_bkt = 0;

		c_bkt = clamp(c_bkt, 0, BUCKET_SIZE - 1);
		if (c_bkt < chosen_bkt) {
			//swap with k and increment k;
			std::swap(Tri_index[i], Tri_index[k]);
			k++;
		}
	}

	if (chosen_l_cnt == 0 || chosen_r_cnt == 0) {
		node.left = node.right = -1;
		node.start = start;
		node.count = count;
		BVH_Tree.push_back(node);
		return;
	}


	node.start = start;
	node.count = count;
	node.left = BVH_Tree.size() + 1;
	int curr_index = BVH_Tree.size();
	this->BVH_Tree.push_back(node);
	BuildBVH(start, chosen_l_cnt);
	BVH_Tree[curr_index].right = BVH_Tree.size();
	BuildBVH(start + chosen_l_cnt, chosen_r_cnt);

}


