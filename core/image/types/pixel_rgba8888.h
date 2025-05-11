#ifndef PIXEL_RGBA888_H
#define PIXEL_RGBA888_H

#include "core/image/pixel.h"

struct PixRGBA8888 : public Pixel {

    uint8_t r, g, b, a;

    constexpr static uint32_t get_red_bit_depth(){return 8;}
    constexpr static uint32_t get_green_bit_depth(){return 8;}
    constexpr static uint32_t get_blue_bit_depth(){return 8;}
    constexpr static uint32_t get_alpha_bit_depth(){return 8;}

    constexpr static bool has_alpha(){return true;}

    virtual Color to_color(){
        return Color(r / 255.0, g / 255.0, b / 255.0, a / 255.0);
    };
    virtual void load_from_color(const Color &p_color){
        r = (uint8_t) p_color.r * 255;
        g = (uint8_t) p_color.g * 255;
        b = (uint8_t) p_color.b * 255;
        a = (uint8_t) p_color.a * 255;
    };

    //retrieve the unadjusted RGB values of the Color
    virtual uint32_t get_red(){
        return r;
    };
    virtual uint32_t get_green(){
        return g;
    };
    virtual uint32_t get_blue(){
        return b;
    };
    virtual uint32_t get_alpha(){
        return a;
    };
};

#endif
