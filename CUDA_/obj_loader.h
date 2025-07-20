#pragma once

#include "tiny_obj_loader.h"
#include "utils.h"

#include <vector>     


void ReadObj(const char* filename, std::vector<Tris>& tri_data);