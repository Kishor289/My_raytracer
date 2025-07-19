#ifndef IMG_READ_H
#define IMG_READ_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "color.h"

// color readImg(double u, double v){

//     int width, height, bpp;
//     width = 983;
//     height = 492;

//     int p_x = static_cast<int>(u*width) + 1;
//     int p_y = static_cast<int>(v*height) + 1;
//     int pixel_no = width*(p_y-1) + p_x; 
//     uint8_t* img_text_data = stbi_load("earth_text.jpg", &width, &height, &bpp, 3);
//     color t_c = vec3(static_cast<int>(img_text_data[3*(pixel_no)-1])/255.0f, static_cast<int>(img_text_data[3*(pixel_no)])/255.0f, static_cast<int>(img_text_data[3*(pixel_no) + 1])/255.0f);

//     stbi_image_free(img_text_data);
//     return t_c;
// }

color readImg(double u, double v) {
    int width, height, bpp;
    width = 983;
    height = 492;

    int p_x = static_cast<int>(u * width);
    int p_y = static_cast<int>(v * height);
    int pixel_no = width * p_y + p_x;

    uint8_t* img_text_data = stbi_load("earth_text.jpg", &width, &height, &bpp, 3);

    if (!img_text_data) {
        // Handle error (e.g., print an error message, return a default color, etc.)
        return color(0.0f, 0.0f, 0.0f);
    }

    // Ensure pixel_no is within bounds
    pixel_no = std::min(std::max(pixel_no, 0), width * height - 1);

    // Normalize pixel values to the range [0, 1]
    float r = static_cast<float>(img_text_data[3 * pixel_no]) / 255.0f;
    float g = static_cast<float>(img_text_data[3 * pixel_no + 1]) / 255.0f;
    float b = static_cast<float>(img_text_data[3 * pixel_no + 2]) / 255.0f;

    stbi_image_free(img_text_data);

    return color(r, g, b);
}


#endif

