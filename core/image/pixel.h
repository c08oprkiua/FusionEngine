
#ifndef PIXEL_H
#define PIXEL_H

#include "core/variant.h"
#include "core/color.h"
/*
A sister class of sorts to Color, this is specifically used in Images and anything relevant to
literal pixels as opposed to "colors" in a generic sense. This allows for more efficent handling of
pixel data in these cases.

A platform needs to include their used pixel formats from the headers in the types/ subfolder, if
the required pixel type is not included in the image types they include.
*/

template<typename PixelType>
constexpr bool validate_pixel_type(){
    return &PixelType::get_red_bit_depth &&
    &PixelType::get_green_bit_depth &&
    &PixelType::get_blue_bit_depth &&
    &PixelType::get_alpha_bit_depth &&
    &PixelType::has_alpha &&
    &PixelType::from_hex &&
    &PixelType::from_color;
}

struct Pixel {
    //Get bit depth, eg. RGB888 will return 8 for all of these, and RGB565 will return 5, 6, and 5.
    //Replace these with static constexpr functions of the class.
    constexpr static uint32_t get_red_bit_depth(){return 0;}
    constexpr static uint32_t get_green_bit_depth(){return 0;}
    constexpr static uint32_t get_blue_bit_depth(){return 0;}
    constexpr static uint32_t get_alpha_bit_depth(){return 0;}

    constexpr static bool has_alpha(){return true;}

    constexpr static Pixel from_hex(uint32_t p_hex){return Pixel();}
    constexpr static Pixel from_color(uint32_t p_hex){return Pixel();}

    virtual Color to_color(){return Color();};
    virtual void load_from_color(const Color &p_color){};

    //retrieve the unadjusted RGB values of the Color
    virtual uint32_t get_red(){return 0u;}
    virtual uint32_t get_green(){return 0u;}
    virtual uint32_t get_blue(){return 0u;}
    virtual uint32_t get_alpha(){return 0u;}

    virtual void brighten(float p_amount){}
    virtual void darken(float p_amount){}
    virtual void invert(){}

};

#endif
