#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"
#include "math.h"
#include "utility.h"

#include <iostream>

using color = vec3;

void write_color(std::ostream &out, color pixel_color) {
    //pixel_color = clamp(pixel_color, 0.0f, 1.0f);
    out << static_cast<int>(255.999 * (pixel_color.x())) << ' '
        << static_cast<int>(255.999 * (pixel_color.y())) << ' '
        << static_cast<int>(255.999 * (pixel_color.z())) << '\n';
}

#endif