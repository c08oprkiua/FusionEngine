#ifndef IMAGE_TYPE_H
#define IMAGE_TYPE_H

#include "pixel.h"
#include "error_macros.h"

/*
Images can either come in the form of RawImage or PaletteImage. RawImage is an image with direct
pixel data (eg. RGB, RGBA, greyscale). PaletteImage is an indexed image format: Colors used in the image
are stored in a palette and each pixel is represented as an index into that palette instead of the color directly.

A platform needs to include their used image formats from the headers in the types/ subfolder. These files
should NOT be included anywhere else in the engine, except the editor for the sake of exporting to these
formats.
*/

struct NewImage {
	enum {
		MAX_WIDTH = 16384, // force a limit somehow
		MAX_HEIGHT = 16384 // force a limit somehow
	};

	/*constexpr*/ static String get_image_format(){return "";}
	/*constexpr*/ static String get_pixel_format(){return "";}

	virtual uint32_t get_width() const = 0; ///< Get image width
	virtual uint32_t get_height() const = 0; ///< Get image height
	virtual uint32_t get_mipmaps() const = 0;


	virtual uint32_t get_mipmap_offset(uint32_t p_mipmap) const = 0; //get where the mipmap begins in data
	virtual void get_mipmap_offset_and_size(uint32_t p_mipmap, int &r_ofs, int &r_size) const = 0; //get where the mipmap begins in data
	virtual void get_mipmap_offset_and_size(uint32_t p_mipmap, int &r_ofs, int &r_size, int &r_w, int &r_h) const = 0; //get where the mipmap begins in data

	/**
	 * returns true when the image is empty (0,0) in size
	 */
	virtual bool empty() const = 0;
};


//PixelFormat should be an implementation of the Pixel class
template <class PixelFormat>
struct RawImage : public NewImage {

    DVector<PixelFormat> data;

	PixelFormat get_pixel(uint32_t p_x, uint32_t p_y, uint32_t p_mipmap = 0) const = 0;

	void put_pixel(uint32_t p_x, uint32_t p_y, const PixelFormat& p_color, uint32_t p_mipmap = 0) = 0;
};

//PixelFormat should be an implementation of the Pixel class
//IndexEntry should be a type like uint16_t or uint32_t
template <class PixelFormat, typename IndexEntry>
struct PaletteImage : public NewImage {
public:
    DVector<PixelFormat> palette;
    DVector<IndexEntry> data;

    constexpr static uint32_t get_format_pixel_size(){
        return PixelFormat::get_red_bit_depth() +
        PixelFormat::get_blue_bit_depth() +
        PixelFormat::get_green_bit_depth() +
        PixelFormat::get_alpha_bit_depth();
    };

	/**
	 * Get a pixel from the image.
	 */
	virtual uint32_t get_pixel(uint32_t p_x, uint32_t p_y, uint32_t p_mipmap = 0) const = 0;
	/**
	 * Set a pixel into the image.
	 */
	virtual void put_pixel(uint32_t p_x, uint32_t p_y, const uint32_t p_color_idx, uint32_t p_mipmap = 0) = 0;
};

#endif
