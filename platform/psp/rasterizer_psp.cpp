/*************************************************************************/
/*  rasterizer_psp.cpp                                                 */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#ifdef PSP

#define BUDDY_ALLOC_IMPLEMENTATION

#include "rasterizer_psp.h"
#include "os/os.h"
#include "globals.h"
#include <stdio.h>
#include "drivers/gl_context/context_gl.h"
#include "servers/visual/shader_language.h"
#include "servers/visual/particle_system_sw.h"
#include "gl_context/context_gl.h"
#include <string.h>

_FORCE_INLINE_ static void _gl_load_transform(const Transform3D& tr) {

	sceGumLoadMatrix(gumake<ScePspFMatrix4>({ /* build a 16x16 matrix */
		tr.basis.elements[0][0],
		tr.basis.elements[1][0],
		tr.basis.elements[2][0],
		0,
		tr.basis.elements[0][1],
		tr.basis.elements[1][1],
		tr.basis.elements[2][1],
		0,
		tr.basis.elements[0][2],
		tr.basis.elements[1][2],
		tr.basis.elements[2][2],
		0,
		tr.origin.x,
		tr.origin.y,
		tr.origin.z,
		1
	}));
};


_FORCE_INLINE_ static void _gl_mult_transform(const Transform3D& tr) {

	sceGumMultMatrix(gumake<ScePspFMatrix4>({ /* build a 16x16 matrix */
		tr.basis.elements[0][0],
		tr.basis.elements[1][0],
		tr.basis.elements[2][0],
		0,
		tr.basis.elements[0][1],
		tr.basis.elements[1][1],
		tr.basis.elements[2][1],
		0,
		tr.basis.elements[0][2],
		tr.basis.elements[1][2],
		tr.basis.elements[2][2],
		0,
		tr.origin.x,
		tr.origin.y,
		tr.origin.z,
		1
	}));
};

_FORCE_INLINE_ static void _gl_mult_transform(const Transform2D& tr) {

	sceGumMultMatrix(gumake<ScePspFMatrix4>({ /* build a 16x16 matrix */
		tr.elements[0][0],
		tr.elements[0][1],
		0,
		0,
		tr.elements[1][0],
		tr.elements[1][1],
		0,
		0,
		0,
		0,
		1,
		0,
		tr.elements[2][0],
		tr.elements[2][1],
		0,
		1
	}));
};

static unsigned int staticOffset = 0;

static unsigned int getMemorySize(unsigned int width, unsigned int height, unsigned int psm)
{
	switch (psm)
	{
		case GU_PSM_T4:
			return (width * height) >> 1;

		case GU_PSM_T8:
			return width * height;

		case GU_PSM_5650:
		case GU_PSM_5551:
		case GU_PSM_4444:
		case GU_PSM_T16:
			return 2 * width * height;

		case GU_PSM_8888:
		case GU_PSM_T32:
			return 4 * width * height;

		default:
			return 0;
	}
}

void* getStaticVramBuffer(unsigned int width, unsigned int height, unsigned int psm)
{
	unsigned int memSize = getMemorySize(width,height,psm);
	void* result = (void*)staticOffset;
	staticOffset += memSize;

	return result;
}

RasterizerPSP::FX::FX() {

	bgcolor_active=false;
	bgcolor=Color(0,1,0,1);

	skybox_active=false;

	glow_active=false;
	glow_passes=4;
	glow_attenuation=0.7;
	glow_bloom=0.0;

	antialias_active=true;
	antialias_tolerance=15;

	ssao_active=true;
	ssao_attenuation=0.7;
	ssao_radius=0.18;
	ssao_max_distance=1.0;
	ssao_range_min=0.25;
	ssao_range_max=0.48;
	ssao_only=false;


	fog_active=false;
	fog_near=5;
	fog_far=100;
	fog_attenuation=1.0;
	fog_color_near=Color(1,1,1,1);
	fog_color_far=Color(1,1,1,1);
	fog_bg=false;

	toon_active=false;
	toon_treshold=0.4;
	toon_soft=0.001;

	edge_active=false;
	edge_color=Color(0,0,0,1);
	edge_size=1.0;

}

static const int prim_type[]={GU_POINTS,GU_LINES,GU_TRIANGLES,GU_TRIANGLE_FAN};

static void _draw_primitive(int p_points, const Vector3 *p_vertices, const Vector3 *p_normals, const Color* p_colors, const Vector3 *p_uvs,const Plane *p_tangents=NULL,int p_instanced=1) {
	ERR_FAIL_COND(!p_vertices);
	ERR_FAIL_COND(p_points <1 || p_points>4);

	int type = prim_type[p_points - 1];

	VertexPool vp{p_points};

	vp.vertex(p_vertices);
	vp.normal(p_normals);
	vp.color(p_colors);
	vp.uv(p_uvs);

	sceGumDrawArray(type, vp.attrs()|GU_TRANSFORM_3D, vp.size(), nullptr, vp.pack());
};

/* TEXTURE API */
/*
Image RasterizerPSP::_get_gl_image_and_format(const Image& p_image, Image::Format p_format, uint32_t p_flags,GLenum& r_gl_format,int &r_gl_components,bool &r_has_alpha_cache,bool &r_compressed) {

	r_has_alpha_cache=false;
	r_compressed=false;
	Image image=p_image;

	switch(p_format) {

		case Image::FORMAT_GRAYSCALE: {
			r_gl_components=1;
			r_gl_format=GL_LUMINANCE;

		} break;
		case Image::FORMAT_INTENSITY: {

			if (!image.empty())
				image.convert(Image::FORMAT_RGBA);
			r_gl_components=4;
			r_gl_format=GL_RGBA;
			r_has_alpha_cache=true;
		} break;
		case Image::FORMAT_GRAYSCALE_ALPHA: {

			//image.convert(Image::FORMAT_RGBA);
			r_gl_components=2;
			r_gl_format=GL_LUMINANCE_ALPHA;
			r_has_alpha_cache=true;
		} break;

		case Image::FORMAT_INDEXED: {

			if (!image.empty())
				image.convert(Image::FORMAT_RGB);
			r_gl_components=3;
			r_gl_format=GL_RGB;

		} break;

		case Image::FORMAT_INDEXED_ALPHA: {

			if (!image.empty())
				image.convert(Image::FORMAT_RGBA);
			r_gl_components=4;
			r_gl_format=GL_RGBA;
			r_has_alpha_cache=true;

		} break;
		case Image::FORMAT_RGB: {

			r_gl_components=3;
			r_gl_format=GL_RGB;
		} break;
		case Image::FORMAT_RGBA: {

			r_gl_components=4;
			r_gl_format=GL_RGBA;
			r_has_alpha_cache=true;
		} break;
		case Image::FORMAT_BC1: {

			r_gl_components=1; //doesn't matter much
			r_gl_format=_EXT_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			r_compressed=true;

		} break;
		case Image::FORMAT_BC2: {
			r_gl_components=1; //doesn't matter much
			r_gl_format=_EXT_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			r_has_alpha_cache=true;
			r_compressed=true;

		} break;
		case Image::FORMAT_BC3: {

			r_gl_components=1; //doesn't matter much
			r_gl_format=_EXT_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			r_has_alpha_cache=true;
			r_compressed=true;

		} break;
		case Image::FORMAT_BC4: {

			r_gl_format=_EXT_COMPRESSED_RED_RGTC1;
			r_gl_components=1; //doesn't matter much
			r_compressed=true;

		} break;
		case Image::FORMAT_BC5: {

			r_gl_format=_EXT_COMPRESSED_RG_RGTC2;
			r_gl_components=1; //doesn't matter much
			r_compressed=true;
		} break;
		case Image::FORMAT_PVRTC2: {

			if (!pvr_supported) {

				if (!image.empty())
					image.decompress();
				r_gl_components=4;
				r_gl_format=GL_RGBA;
				r_has_alpha_cache=true;
				print_line("Load Compat PVRTC2");

			} else {

				r_gl_format=_EXT_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
				r_gl_components=1; //doesn't matter much
				r_compressed=true;
				print_line("Load Normal PVRTC2");
			}

		} break;
		case Image::FORMAT_PVRTC2_ALPHA: {

			if (!pvr_supported) {

				if (!image.empty())
					image.decompress();
				r_gl_components=4;
				r_gl_format=GL_RGBA;
				r_has_alpha_cache=true;
				print_line("Load Compat PVRTC2A");

			} else {

				r_gl_format=_EXT_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
				r_gl_components=1; //doesn't matter much
				r_compressed=true;
				print_line("Load Normal PVRTC2A");
			}

		} break;
		case Image::FORMAT_PVRTC4: {

			if (!pvr_supported) {

				if (!image.empty())
					image.decompress();
				r_gl_components=4;
				r_gl_format=GL_RGBA;
				r_has_alpha_cache=true;
				print_line("Load Compat PVRTC4");
			} else {

				r_gl_format=_EXT_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
				r_gl_components=1; //doesn't matter much
				r_compressed=true;
				print_line("Load Normal PVRTC4");
			}

		} break;
		case Image::FORMAT_PVRTC4_ALPHA: {

			if (!pvr_supported) {

				if (!image.empty())
					image.decompress();
				r_gl_components=4;
				r_gl_format=GL_RGBA;
				r_has_alpha_cache=true;
				print_line("Load Compat PVRTC4A");

			} else {

				r_gl_format=_EXT_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
				r_gl_components=1; //doesn't matter much
				r_compressed=true;
				print_line("Load Normal PVRTC4A");
			}

		} break;
		case Image::FORMAT_ETC: {

			if (!pvr_supported) {

				if (!image.empty())
					image.decompress();
			} else {

				r_gl_format=_EXT_ETC1_RGB8_OES;
				r_gl_components=1; //doesn't matter much
				r_compressed=true;
			}

		} break;
		case Image::FORMAT_YUV_422:
		case Image::FORMAT_YUV_444: {

			if (!image.empty())
				image.convert(Image::FORMAT_RGB);
			r_gl_format=GL_RGB;
			r_gl_components=3;

		} break;

		default: {

			ERR_FAIL_V(Image());
		}
	}

	return image;
}
*/

RID RasterizerPSP::texture_create() {

	Texture *texture = memnew(Texture);
	ERR_FAIL_COND_V(!texture,RID());
	// glGenTextures(1, &texture->tex_id);
	texture->active=false;
	texture->total_data_size=0;

	return texture_owner.make_rid( texture );

}

void RasterizerPSP::texture_allocate(RID p_texture,int p_width, int p_height,Image::Format p_format,uint32_t p_flags) {

	bool has_alpha_cache;
	int components;
	// GLenum format;
	bool compressed = false;
	switch (p_format) {
	case Image::FORMAT_BC1:
	case Image::FORMAT_BC2:
	case Image::FORMAT_BC3:
	case Image::FORMAT_BC4:
	case Image::FORMAT_BC5:
	case Image::FORMAT_ETC:
	case Image::FORMAT_PVRTC2:
	case Image::FORMAT_PVRTC2_ALPHA:
	case Image::FORMAT_PVRTC4:
	case Image::FORMAT_PVRTC4_ALPHA:
		compressed = true;
		break;
	}

	int po2_width = nearest_power_of_2(p_width);
	int po2_height = nearest_power_of_2(p_height);
	if (po2_width < 16) {
		po2_width = 16;
	}

	Texture *texture = texture_owner.get( p_texture );
	ERR_FAIL_COND(!texture);
	texture->width=p_width;
	texture->height=p_height;
	texture->format=p_format;
	texture->flags=p_flags;
	texture->swizzle=true;
	// texture->target = /*(p_flags & VS::TEXTURE_FLAG_CUBEMAP) ? GL_TEXTURE_CUBE_MAP :*/ GL_TEXTURE_2D;

	texture->alloc_width = po2_width;
	texture->alloc_height = po2_height;

	// _get_gl_image_and_format(Image(),texture->format,texture->flags,format,components,has_alpha_cache,compressed);

	texture->gl_components_cache=components;
	texture->format_has_alpha=has_alpha_cache;
	texture->compressed=compressed;
	texture->data_size=0;

	switch (texture->format) {
	case Image::FORMAT_RGBA:
		texture->gu_format_cache=GU_PSM_8888;
		texture->swizzle=true;
		break;
#if 0
	case Image::FORMAT_BC1:
		texture->gu_format_cache=GU_PSM_DXT1;
		texture->swizzle=false;
		break;
	case Image::FORMAT_BC2:
		texture->gu_format_cache=GU_PSM_DXT3;
		texture->swizzle=false;
		break;
	case Image::FORMAT_BC3:
		texture->gu_format_cache=GU_PSM_DXT5;
		texture->swizzle=false;
		break;
#endif
	default:
		texture->format=Image::FORMAT_RGBA;
		texture->gu_format_cache=GU_PSM_8888;
		texture->swizzle=true;

		ERR_PRINT("Unsupported texture format");

		break;
	}


	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(texture->target, texture->tex_id);
	//texture->tex_id = (unsigned char*)memalign(16, texture->alloc_width*texture->alloc_height*4);
	// texture->tex_id = 1;
	/*
	if (compressed) {

		glTexParameteri( texture->target, GL_GENERATE_MIPMAP, GL_FALSE );
	} else {
		if (texture->flags&VS::TEXTURE_FLAG_MIPMAPS) {
			glTexParameteri( texture->target, GL_GENERATE_MIPMAP, GL_TRUE );
		} else {
			glTexParameteri( texture->target, GL_GENERATE_MIPMAP, GL_FALSE );
		}

	}


	if (texture->flags&VS::TEXTURE_FLAG_MIPMAPS)
		glTexParameteri(texture->target,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	else
		glTexParameteri(texture->target,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

	if (texture->flags&VS::TEXTURE_FLAG_FILTER) {

		glTexParameteri(texture->target,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	// Linear Filtering

	} else {

		glTexParameteri(texture->target,GL_TEXTURE_MAG_FILTER,GL_NEAREST);	// raw Filtering

	}*/

	texture->active=true;
}

void RasterizerPSP::texture_set_data(RID p_texture,const Image& p_image,VS::CubeMapSide p_cube_side) {

	Texture * texture = texture_owner.get(p_texture);

	ERR_FAIL_COND(!texture);
	ERR_FAIL_COND(!texture->active);
	// ERR_FAIL_COND(texture->format != p_image.get_format() );

	int components;
	// GLenum format;
	bool alpha;
	bool compressed;

	if (keep_copies && !(texture->flags&VS::TEXTURE_FLAG_VIDEO_SURFACE) && !(use_reload_hooks && texture->reloader)) {
		texture->image[p_cube_side]=p_image;
	}


	Image img = p_image;
	//_get_gl_image_and_format(p_image, p_image.get_format(),texture->flags,format,components,alpha,compressed);
	
	if (img.get_format() != texture->format) {
		WARN_PRINT("Image format conversion required");
		if (texture->compressed) {
			img.decompress();
			texture->compressed = false;
		}
		img.convert(texture->format);
	}

	texture->alloc_width = nearest_power_of_2(img.get_width());
	texture->alloc_height = nearest_power_of_2(img.get_height());
	if (texture->alloc_width < 16) {
		texture->alloc_width = 16;
	}
	if (texture->alloc_width != img.get_width() || texture->alloc_height != img.get_height()) {

		img.resize(texture->alloc_width, texture->alloc_height, Image::INTERPOLATE_BILINEAR);
	};

	// GLenum blit_target = /*(texture->target == GL_TEXTURE_CUBE_MAP)?_cube_side_enum[p_cube_side]:*/GL_TEXTURE_2D;

	texture->data_size=img.get_data().size();
	DVector<uint8_t>::Read read = img.get_data().read();

	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(texture->target, texture->tex_id);

	int mipmaps=(texture->flags&VS::TEXTURE_FLAG_MIPMAPS && img.get_mipmaps()>0) ? img.get_mipmaps() +1 : 1;

	//if (mipmaps > 1) {
	//	texture->swizzle = false;
	//}

	int w=img.get_width();
	int h=img.get_height();

	texture->_free_mipmaps();

	int tsize=0;
	for(int i=0;i<mipmaps;i++) {

		int size,ofs;
		img.get_mipmap_offset_and_size(i,ofs,size);

		ERR_BREAK((w % 16) != 0)

		//ERR_FAIL_COND(texture->compressed);

		// glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//			glTexImage2D(blit_target, i, format==GL_RGB?GL_RGB8:format, w, h, 0, format, GL_UNSIGNED_BYTE,&read[ofs]);
		// glTexImage2D(blit_target, i, format, w, h, 0, format, GL_UNSIGNED_BYTE,&read[ofs]);
		//glTexSubImage2D( blit_target, i, 0,0,w,h,format,GL_UNSIGNED_BYTE,&read[ofs] );
		if (w > 128 && h > 128) {
			printf("large mipmap: %i x %i - size: %i\n",w,h,size);
			texture->mipmaps.push_back(reinterpret_cast<uint8_t *>(memalloc(size)));
		} else {
			printf("small mipmap: %i x %i - size: %i\n",w,h,size);
			texture->mipmaps.push_back(reinterpret_cast<uint8_t *>(gualloc(size)));
		}
		if (texture->swizzle)
			swizzle(texture->mipmaps[i], &read[ofs], static_cast<int>(w * (static_cast<float>(size) / w / h)), h);
		else
			memcpy(texture->mipmaps[i], &read[ofs], size);

		tsize+=size;

		w = MAX(1,w>>1);
		h = MAX(1,h>>1);
	}

	//sceGuTexSync();

	_rinfo.texture_mem-=texture->total_data_size;
	texture->total_data_size=tsize;
	_rinfo.texture_mem+=texture->total_data_size;

	printf("texture: %i x %i - size: %i - total: %i\n",texture->width,texture->height,tsize,_rinfo.texture_mem);

/*

	if (mipmaps==1 && texture->flags&VS::TEXTURE_FLAG_MIPMAPS) {
		glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );

	} else {
		glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE );

	}*/

	if (mipmaps>1) {

		//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmaps-1 ); - assumed to have all, always
	}

	//texture_set_flags(p_texture,texture->flags);


}

Image RasterizerPSP::texture_get_data(RID p_texture,VS::CubeMapSide p_cube_side) const {

	Texture * texture = texture_owner.get(p_texture);

	ERR_FAIL_COND_V(!texture,Image());
	ERR_FAIL_COND_V(!texture->active,Image());

	return texture->image[p_cube_side];
#if 0

	Texture * texture = texture_owner.get(p_texture);

	ERR_FAIL_COND_V(!texture,Image());
	ERR_FAIL_COND_V(!texture->active,Image());
	ERR_FAIL_COND_V(texture->data_size==0,Image());

	DVector<uint8_t> data;
	GLenum format,type=GL_UNSIGNED_BYTE;
	Image::Format fmt;
	int pixelsize=0;
	int pixelshift=0;
	int minw=1,minh=1;
	bool compressed=false;

	fmt=texture->format;

	switch(texture->format) {

		case Image::FORMAT_GRAYSCALE: {

			format=GL_LUMINANCE;
			type=GL_UNSIGNED_BYTE;
			data.resize(texture->alloc_width*texture->alloc_height);
			pixelsize=1;

		} break;
		case Image::FORMAT_INTENSITY: {
			return Image();
		} break;
		case Image::FORMAT_GRAYSCALE_ALPHA: {

			format=GL_LUMINANCE_ALPHA;
			type=GL_UNSIGNED_BYTE;
			pixelsize=2;

		} break;
		case Image::FORMAT_RGB: {
			format=GL_RGB;
			type=GL_UNSIGNED_BYTE;
			pixelsize=3;
		} break;
		case Image::FORMAT_RGBA: {

			format=GL_RGBA;
			type=GL_UNSIGNED_BYTE;
			pixelsize=4;
		} break;
		case Image::FORMAT_INDEXED: {

			format=GL_RGB;
			type=GL_UNSIGNED_BYTE;
			fmt=Image::FORMAT_RGB;
			pixelsize=3;
		} break;
		case Image::FORMAT_INDEXED_ALPHA: {

			format=GL_RGBA;
			type=GL_UNSIGNED_BYTE;
			fmt=Image::FORMAT_RGBA;
			pixelsize=4;

		} break;
		case Image::FORMAT_BC1: {

			pixelsize=1; //doesn't matter much
			format=GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			compressed=true;
			pixelshift=1;
			minw=minh=4;

		} break;
		case Image::FORMAT_BC2: {
			pixelsize=1; //doesn't matter much
			format=GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			compressed=true;
			minw=minh=4;

		} break;
		case Image::FORMAT_BC3: {

			pixelsize=1; //doesn't matter much
			format=GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			compressed=true;
			minw=minh=4;

		} break;
		case Image::FORMAT_BC4: {

			format=GL_COMPRESSED_RED_RGTC1;
			pixelsize=1; //doesn't matter much
			compressed=true;
			pixelshift=1;
			minw=minh=4;

		} break;
		case Image::FORMAT_BC5: {

			format=GL_COMPRESSED_RG_RGTC2;
			pixelsize=1; //doesn't matter much
			compressed=true;
			minw=minh=4;

		} break;

		default:{}
	}

	data.resize(texture->data_size);
	DVector<uint8_t>::Write wb = data.write();

	glActiveTexture(GL_TEXTURE0);
	int ofs=0;
	glBindTexture(texture->target,texture->tex_id);

	int w=texture->alloc_width;
	int h=texture->alloc_height;
	for(int i=0;i<texture->mipmaps+1;i++) {

		if (compressed) {

			glPixelStorei(GL_PACK_ALIGNMENT, 4);
			glGetCompressedTexImage(texture->target,i,&wb[ofs]);

		} else {
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			glGetTexImage(texture->target,i,format,type,&wb[ofs]);
		}

		int size = (w*h*pixelsize)>>pixelshift;
		ofs+=size;

		w=MAX(minw,w>>1);
		h=MAX(minh,h>>1);

	}


	wb=DVector<uint8_t>::Write();

	Image img(texture->alloc_width,texture->alloc_height,texture->mipmaps,fmt,data);

	if (texture->format<Image::FORMAT_INDEXED && (texture->alloc_width!=texture->width || texture->alloc_height!=texture->height))
		img.resize(texture->width,texture->height);

	return img;
#endif
}

void RasterizerPSP::texture_set_flags(RID p_texture,uint32_t p_flags) {

	Texture *texture = texture_owner.get( p_texture );
	ERR_FAIL_COND(!texture);
/*
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(texture->target, texture->tex_id);
	uint32_t cube = texture->flags & VS::TEXTURE_FLAG_CUBEMAP;
	texture->flags=p_flags|cube; // can't remove a cube from being a cube

	bool force_clamp_to_edge = !(p_flags&VS::TEXTURE_FLAG_MIPMAPS) && (nearest_power_of_2(texture->alloc_height)!=texture->alloc_height || nearest_power_of_2(texture->alloc_width)!=texture->alloc_width);

	if (!force_clamp_to_edge && texture->flags&VS::TEXTURE_FLAG_REPEAT) {

		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	} else {
		//glTexParameterf( texture->target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
		glTexParameterf( texture->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameterf( texture->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	}


	if (texture->flags&VS::TEXTURE_FLAG_FILTER) {

		glTexParameteri(texture->target,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	// Linear Filtering
		if (texture->flags&VS::TEXTURE_FLAG_MIPMAPS)
			glTexParameteri(texture->target,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		else
			glTexParameteri(texture->target,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	// Linear Filtering

	} else {

		glTexParameteri(texture->target,GL_TEXTURE_MAG_FILTER,GL_NEAREST);	// nearest
	}*/
}
uint32_t RasterizerPSP::texture_get_flags(RID p_texture) const {

	Texture * texture = texture_owner.get(p_texture);

	ERR_FAIL_COND_V(!texture,0);

	return texture->flags;

}
Image::Format RasterizerPSP::texture_get_format(RID p_texture) const {

	Texture * texture = texture_owner.get(p_texture);

	ERR_FAIL_COND_V(!texture,Image::FORMAT_GRAYSCALE);

	return texture->format;
}
uint32_t RasterizerPSP::texture_get_width(RID p_texture) const {

	Texture * texture = texture_owner.get(p_texture);

	ERR_FAIL_COND_V(!texture,0);

	return texture->width;
}
uint32_t RasterizerPSP::texture_get_height(RID p_texture) const {

	Texture * texture = texture_owner.get(p_texture);

	ERR_FAIL_COND_V(!texture,0);

	return texture->height;
}

bool RasterizerPSP::texture_has_alpha(RID p_texture) const {

	Texture * texture = texture_owner.get(p_texture);

	ERR_FAIL_COND_V(!texture,0);

	return false;

}

void RasterizerPSP::texture_set_size_override(RID p_texture,int p_width, int p_height) {

	Texture * texture = texture_owner.get(p_texture);

	ERR_FAIL_COND(!texture);

	ERR_FAIL_COND(p_width<=0 || p_width>4096);
	ERR_FAIL_COND(p_height<=0 || p_height>4096);
	//real texture size is in alloc width and height
	texture->width=p_width;
	texture->height=p_height;

}

void RasterizerPSP::texture_set_reload_hook(RID p_texture,ObjectID p_owner,const StringName& p_function) const {

	Texture * texture = texture_owner.get(p_texture);

	ERR_FAIL_COND(!texture);

	texture->reloader=p_owner;
	texture->reloader_func=p_function;
	if (use_reload_hooks && p_owner && keep_copies) {

		for(int i=0;i<6;i++)
			texture->image[i]=Image();
	}


}

/* SHADER API */

/* SHADER API */

RID RasterizerPSP::shader_create(VS::ShaderMode p_mode) {

	Shader *shader = memnew( Shader );
	shader->mode=p_mode;
	shader->valid=false;
	shader->has_alpha=false;
	shader->fragment_line=0;
	shader->vertex_line=0;
	shader->light_line=0;
	RID rid = shader_owner.make_rid(shader);
	shader_set_mode(rid,p_mode);
//	_shader_make_dirty(shader);

	return rid;

}



void RasterizerPSP::shader_set_mode(RID p_shader,VS::ShaderMode p_mode) {

	ERR_FAIL_INDEX(p_mode,3);
	Shader *shader=shader_owner.get(p_shader);
	ERR_FAIL_COND(!shader);
//	if (shader->custom_code_id && p_mode==shader->mode)
//		return;

	shader->mode=p_mode;

}
VS::ShaderMode RasterizerPSP::shader_get_mode(RID p_shader) const {

	Shader *shader=shader_owner.get(p_shader);
	ERR_FAIL_COND_V(!shader,VS::SHADER_MATERIAL);
	return shader->mode;
}



void RasterizerPSP::shader_set_code(RID p_shader, const String& p_vertex, const String& p_fragment,const String& p_light,int p_vertex_ofs,int p_fragment_ofs,int p_light_ofs) {


	Shader *shader=shader_owner.get(p_shader);
	ERR_FAIL_COND(!shader);

#ifdef DEBUG_ENABLED
	if (shader->vertex_code==p_vertex && shader->fragment_code==p_fragment && shader->light_code==p_light)
		return;
#endif
	shader->fragment_code=p_fragment;
	shader->vertex_code=p_vertex;
	shader->light_code=p_light;
	shader->fragment_line=p_fragment_ofs;
	shader->vertex_line=p_vertex_ofs;
	shader->light_line=p_light_ofs;

}

String RasterizerPSP::shader_get_vertex_code(RID p_shader) const {

	Shader *shader=shader_owner.get(p_shader);
	ERR_FAIL_COND_V(!shader,String());
	return shader->vertex_code;

}

String RasterizerPSP::shader_get_fragment_code(RID p_shader) const {

	Shader *shader=shader_owner.get(p_shader);
	ERR_FAIL_COND_V(!shader,String());
	return shader->fragment_code;

}

String RasterizerPSP::shader_get_light_code(RID p_shader) const {

	Shader *shader=shader_owner.get(p_shader);
	ERR_FAIL_COND_V(!shader,String());
	return shader->light_code;

}

void RasterizerPSP::shader_get_param_list(RID p_shader, List<PropertyInfo> *p_param_list) const {

	Shader *shader=shader_owner.get(p_shader);
	ERR_FAIL_COND(!shader);
#if 0

	if (shader->dirty_list.in_list())
		_update_shader(shader); // ok should be not anymore dirty


	Map<int,StringName> order;


	for(Map<StringName,ShaderLanguage::Uniform>::Element *E=shader->uniforms.front();E;E=E->next()) {


		order[E->get().order]=E->key();
	}


	for(Map<int,StringName>::Element *E=order.front();E;E=E->next()) {

		PropertyInfo pi;
		ShaderLanguage::Uniform &u=shader->uniforms[E->get()];
		pi.name=E->get();
		switch(u.type) {

			case ShaderLanguage::TYPE_VOID:
			case ShaderLanguage::TYPE_BOOL:
			case ShaderLanguage::TYPE_FLOAT:
			case ShaderLanguage::TYPE_VEC2:
			case ShaderLanguage::TYPE_VEC3:
			case ShaderLanguage::TYPE_MAT3:
			case ShaderLanguage::TYPE_MAT4:
			case ShaderLanguage::TYPE_VEC4:
				pi.type=u.default_value.get_type();
				break;
			case ShaderLanguage::TYPE_TEXTURE:
				pi.type=Variant::_RID;
				pi.hint=PROPERTY_HINT_RESOURCE_TYPE;
				pi.hint_string="Texture";
				break;
			case ShaderLanguage::TYPE_CUBEMAP:
				pi.type=Variant::_RID;
				pi.hint=PROPERTY_HINT_RESOURCE_TYPE;
				pi.hint_string="Texture";
				break;
		};

		p_param_list->push_back(pi);

	}
#endif

}

/* COMMON MATERIAL API */


RID RasterizerPSP::material_create() {

	return material_owner.make_rid( memnew( Material ) );
}

void RasterizerPSP::material_set_shader(RID p_material, RID p_shader) {

	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND(!material);
	material->shader=p_shader;

}

RID RasterizerPSP::material_get_shader(RID p_material) const {

	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND_V(!material,RID());
	return material->shader;
}

#if 0

void RasterizerPSP::_material_check_alpha(Material *p_material) {

	p_material->has_alpha=false;
	Color diffuse=p_material->parameters[VS::FIXED_MATERIAL_PARAM_DIFFUSE];
	if (diffuse.a<0.98) {

		p_material->has_alpha=true;
		return;
	}

	if (p_material->textures[VS::FIXED_MATERIAL_PARAM_DIFFUSE].is_valid()) {

		Texture *tex = texture_owner.get(p_material->textures[VS::FIXED_MATERIAL_PARAM_DIFFUSE]);
		if (!tex)
			return;
		if (tex->has_alpha) {

			p_material->has_alpha=true;
			return;
		}
	}
}

#endif
void RasterizerPSP::material_set_param(RID p_material, const StringName& p_param, const Variant& p_value) {

	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND(!material);

	if (p_value.get_type()==Variant::NIL)
		material->shader_params.erase(p_param);
	else
		material->shader_params[p_param]=p_value;
}
Variant RasterizerPSP::material_get_param(RID p_material, const StringName& p_param) const {

	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND_V(!material,Variant());

	if (material->shader_params.has(p_param))
		return material->shader_params[p_param];
	else
		return Variant();
}


void RasterizerPSP::material_set_flag(RID p_material, VS::MaterialFlag p_flag,bool p_enabled) {

	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND(!material);
	ERR_FAIL_INDEX(p_flag,VS::MATERIAL_FLAG_MAX);
	material->flags[p_flag]=p_enabled;

}
bool RasterizerPSP::material_get_flag(RID p_material,VS::MaterialFlag p_flag) const {

	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND_V(!material,false);
	ERR_FAIL_INDEX_V(p_flag,VS::MATERIAL_FLAG_MAX,false);
	return material->flags[p_flag];


}

void RasterizerPSP::material_set_depth_draw_mode(RID p_material, VS::MaterialDepthDrawMode p_mode) {

	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND(!material);
	material->depth_draw_mode=p_mode;
}

VS::MaterialDepthDrawMode RasterizerPSP::material_get_depth_draw_mode(RID p_material) const{


	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND_V(!material,VS::MATERIAL_DEPTH_DRAW_ALWAYS);
	return material->depth_draw_mode;
}


void RasterizerPSP::material_set_blend_mode(RID p_material,VS::MaterialBlendMode p_mode) {

	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND(!material);
	material->blend_mode=p_mode;

}
VS::MaterialBlendMode RasterizerPSP::material_get_blend_mode(RID p_material) const {

	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND_V(!material,VS::MATERIAL_BLEND_MODE_ADD);
	return material->blend_mode;
}

void RasterizerPSP::material_set_line_width(RID p_material,float p_line_width) {

	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND(!material);
	material->line_width=p_line_width;

}
float RasterizerPSP::material_get_line_width(RID p_material) const {

	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND_V(!material,0);

	return material->line_width;
}

/* FIXED MATERIAL */


RID RasterizerPSP::fixed_material_create() {

	return material_create();
}

void RasterizerPSP::fixed_material_set_flag(RID p_material, VS::FixedMaterialFlags p_flag, bool p_enabled) {

	Material *m=material_owner.get( p_material );
	ERR_FAIL_COND(!m);
	ERR_FAIL_INDEX(p_flag, 3);
	m->fixed_flags[p_flag]=p_enabled;
}

bool RasterizerPSP::fixed_material_get_flag(RID p_material, VS::FixedMaterialFlags p_flag) const {

	Material *m=material_owner.get( p_material );
	ERR_FAIL_COND_V(!m,false);
	ERR_FAIL_INDEX_V(p_flag,VS::FIXED_MATERIAL_FLAG_MAX, false);
	return m->fixed_flags[p_flag];
}

void RasterizerPSP::fixed_material_set_parameter(RID p_material, VS::FixedMaterialParam p_parameter, const Variant& p_value) {

	Material *m=material_owner.get( p_material );
	ERR_FAIL_COND(!m);
	ERR_FAIL_INDEX(p_parameter, VisualServer::FIXED_MATERIAL_PARAM_MAX);

	m->parameters[p_parameter] = p_value;

}

Variant RasterizerPSP::fixed_material_get_parameter(RID p_material,VS::FixedMaterialParam p_parameter) const {

	Material *m=material_owner.get( p_material );
	ERR_FAIL_COND_V(!m, Variant());
	ERR_FAIL_INDEX_V(p_parameter, VisualServer::FIXED_MATERIAL_PARAM_MAX, Variant());

	return m->parameters[p_parameter];
}

void RasterizerPSP::fixed_material_set_texture(RID p_material,VS::FixedMaterialParam p_parameter, RID p_texture) {

	Material *m=material_owner.get( p_material );
	ERR_FAIL_COND(!m);
	ERR_FAIL_INDEX(p_parameter, VisualServer::FIXED_MATERIAL_PARAM_MAX);

	m->textures[p_parameter] = p_texture;

}
RID RasterizerPSP::fixed_material_get_texture(RID p_material,VS::FixedMaterialParam p_parameter) const {

	Material *m=material_owner.get( p_material );
	ERR_FAIL_COND_V(!m, RID());
	ERR_FAIL_INDEX_V(p_parameter, VisualServer::FIXED_MATERIAL_PARAM_MAX, Variant());

	return m->textures[p_parameter];
}


void RasterizerPSP::fixed_material_set_texcoord_mode(RID p_material,VS::FixedMaterialParam p_parameter, VS::FixedMaterialTexCoordMode p_mode) {

	Material *m=material_owner.get( p_material );
	ERR_FAIL_COND(!m);
	ERR_FAIL_INDEX(p_parameter, VisualServer::FIXED_MATERIAL_PARAM_MAX);
	ERR_FAIL_INDEX(p_mode,4);

	m->texcoord_mode[p_parameter] = p_mode;
}

VS::FixedMaterialTexCoordMode RasterizerPSP::fixed_material_get_texcoord_mode(RID p_material,VS::FixedMaterialParam p_parameter) const {

	Material *m=material_owner.get( p_material );
	ERR_FAIL_COND_V(!m, VS::FIXED_MATERIAL_TEXCOORD_UV);
	ERR_FAIL_INDEX_V(p_parameter, VisualServer::FIXED_MATERIAL_PARAM_MAX, VS::FIXED_MATERIAL_TEXCOORD_UV);

	return m->texcoord_mode[p_parameter]; // for now
}

void RasterizerPSP::fixed_material_set_uv_transform(RID p_material,const Transform3D& p_transform) {

	Material *m=material_owner.get( p_material );
	ERR_FAIL_COND(!m);

	m->uv_transform = p_transform;
}

Transform3D RasterizerPSP::fixed_material_get_uv_transform(RID p_material) const {

	Material *m=material_owner.get( p_material );
	ERR_FAIL_COND_V(!m, Transform3D());

	return m->uv_transform;
}

void RasterizerPSP::fixed_material_set_point_size(RID p_material,float p_size) {

	Material *m=material_owner.get( p_material );
	ERR_FAIL_COND(!m);
	m->point_size=p_size;

}
float RasterizerPSP::fixed_material_get_point_size(RID p_material) const {

	const Material *m=material_owner.get( p_material );
	ERR_FAIL_COND_V(!m, 0);
	return m->point_size;
}


/* MESH API */


RID RasterizerPSP::mesh_create() {


	return mesh_owner.make_rid( memnew( Mesh ) );
}



void RasterizerPSP::mesh_add_surface(RID p_mesh,VS::PrimitiveType p_primitive,const Array& p_arrays,const Array& p_blend_shapes,bool p_alpha_sort) {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND(!mesh);

	ERR_FAIL_INDEX( p_primitive, VS::PRIMITIVE_MAX );
	ERR_FAIL_COND(p_arrays.size()!=VS::ARRAY_MAX);

	uint32_t format=0;

	// validation
	int index_array_len=0;
	int array_len=0;

	for(int i=0;i<p_arrays.size();i++) {

		if (p_arrays[i].get_type()==Variant::NIL)
			continue;

		format|=(1<<i);

		if (i==VS::ARRAY_VERTEX) {

			array_len=PackedVector3Array(p_arrays[i]).size();
			ERR_FAIL_COND(array_len==0);
		} else if (i==VS::ARRAY_INDEX) {

			index_array_len=PackedIntArray(p_arrays[i]).size();
		}
	}

	ERR_FAIL_COND((format&VS::ARRAY_FORMAT_VERTEX)==0); // mandatory


	Surface *surface = memnew( Surface );
	ERR_FAIL_COND( !surface );

	int total_elem_size=0;

	for (int i=0;i<VS::ARRAY_MAX;i++) {


		Surface::ArrayData&ad=surface->array[i];
		ad.size=0;
		ad.ofs=0;
		int elem_size=0;
		int elem_count=0;
		bool valid_local=true;
		// enum datatype;
		bool normalize=false;
		bool bind=false;

		if (!(format&(1<<i))) // no array
			continue;


		switch(i) {

			case VS::ARRAY_VERTEX: {

				if (surface->packed) {
					elem_size=3*sizeof(int16_t); // vertex
					// datatype=int16_t;
					normalize=true;

				} else {
					elem_size=3*sizeof(float); // vertex
					// datatype=float;
				}
				bind=true;
				elem_count=3;

			} break;
			case VS::ARRAY_NORMAL: {

				if (surface->packed) {
					elem_size=3*sizeof(int8_t); // vertex
					// datatype=GL_BYTE;
					normalize=true;
				} else {
					elem_size=3*sizeof(float); // vertex
					// datatype=float;
				}
				bind=true;
				elem_count=3;
			} break;
			case VS::ARRAY_TANGENT: {
				if (surface->packed) {
					elem_size=4*sizeof(int8_t); // vertex
					// datatype=GL_BYTE;
					normalize=true;
				} else {
					elem_size=4*sizeof(float); // vertex
					// datatype=float;
				}
				bind=true;
				elem_count=4;

			} break;
			case VS::ARRAY_COLOR: {

				elem_size=4*sizeof(uint8_t); /* RGBA */
				// datatype=uint8_t;
				elem_count=4;
				bind=true;
				normalize=true;
			} break;
			case VS::ARRAY_TEX_UV:
			case VS::ARRAY_TEX_UV2: {
				if (surface->packed) {
					elem_size=2*sizeof(int16_t); // vertex
					// datatype=int16_t;
					normalize=true;
				} else {
					elem_size=2*sizeof(float); // vertex
					// datatype=float;
				}
				bind=true;
				elem_count=2;

			} break;
			case VS::ARRAY_WEIGHTS: {

				elem_size=VS::ARRAY_WEIGHTS_SIZE*sizeof(float);
				elem_count=VS::ARRAY_WEIGHTS_SIZE;
				valid_local=false;
				// datatype=float;

			} break;
			case VS::ARRAY_BONES: {

				elem_size=VS::ARRAY_WEIGHTS_SIZE*sizeof(unsigned int);
				elem_count=VS::ARRAY_WEIGHTS_SIZE;
				valid_local=false;
				// datatype=unsigned int;


			} break;
			case VS::ARRAY_INDEX: {

				if (index_array_len<=0) {
					ERR_PRINT("index_array_len==NO_INDEX_ARRAY");
					break;
				}
				/* determine wether using 16 or 32 bits indices */
				elem_size=2;
				// datatype=unsigned short;

				surface->index_array_len=index_array_len; // only way it can exist
				ad.ofs=0;
				ad.size=elem_size;


				continue;
			} break;
			default: {
				ERR_FAIL( );
			}
		}

		ad.ofs=total_elem_size;
		ad.size=elem_size;
		// ad.datatype=datatype;
		ad.normalize=normalize;
		ad.bind=bind;
		ad.count=elem_count;
		total_elem_size+=elem_size;
		if (valid_local) {
			surface->local_stride+=elem_size;
			surface->morph_format|=(1<<i);
		}


	}

	surface->stride=total_elem_size;
	surface->array_len=array_len;
	surface->format=format;
	surface->primitive=p_primitive;
	surface->configured_format=0;
	if (keep_copies) {
		surface->data=p_arrays;
		surface->morph_data=p_blend_shapes;
	}

	uint8_t *array_ptr=NULL;
	uint8_t *index_array_ptr=NULL;

	/* create pointers */
	surface->array_local = (uint8_t*)vtalloc(surface->array_len*surface->stride);
	array_ptr=(uint8_t*)surface->array_local;
	if (surface->index_array_len) {
		surface->index_array_local = reinterpret_cast<uint8_t *>(vtalloc(index_array_len*surface->array[VS::ARRAY_INDEX].size));
		index_array_ptr=(uint8_t*)surface->index_array_local;
	}

	_surface_set_arrays(surface,array_ptr,index_array_ptr,p_arrays,true);

	mesh->surfaces.push_back(surface);

}

Error RasterizerPSP::_surface_set_arrays(Surface *p_surface, uint8_t *p_mem,uint8_t *p_index_mem,const Array& p_arrays,bool p_main) {

	p_surface->vp.resize(p_surface->array_len);

	uint32_t stride = p_main ? p_surface->stride : p_surface->local_stride;

	for(int ai=0;ai<VS::ARRAY_MAX;ai++) {
		if (ai>=p_arrays.size())
			break;
		if (p_arrays[ai].get_type()==Variant::NIL)
			continue;
		Surface::ArrayData &a=p_surface->array[ai];

		switch(ai) {


			case VS::ARRAY_VERTEX: {

				ERR_FAIL_COND_V( p_arrays[ai].get_type() != Variant::VECTOR3_ARRAY, ERR_INVALID_PARAMETER );

				DVector<Vector3> array = p_arrays[ai];
				ERR_FAIL_COND_V( array.size() != p_surface->array_len, ERR_INVALID_PARAMETER );


				DVector<Vector3>::Read read = array.read();
				const Vector3* src=read.ptr();

				// setting vertices means regenerating the AABB
				AABB aabb;

				float scale=1;
				float max=0;


				for (int i=0;i<p_surface->array_len;i++) {


					float vector[3]={ src[i].x, src[i].y, src[i].z };

					copymem(&p_mem[a.ofs+i*stride], vector, a.size);

					if (i==0) {

						aabb=AABB(src[i],Vector3());
					} else {

						aabb.expand_to( src[i] );
					}
				}

				//if (p_main) {
					p_surface->aabb=aabb;
					p_surface->vertex_scale=scale;
				//}

				p_surface->vp.vertex(reinterpret_cast<Vector3 *>(&p_mem[a.ofs]), stride, true);

			} break;
			case VS::ARRAY_NORMAL: {

				ERR_FAIL_COND_V( p_arrays[ai].get_type() != Variant::VECTOR3_ARRAY, ERR_INVALID_PARAMETER );

				DVector<Vector3> array = p_arrays[ai];
				ERR_FAIL_COND_V( array.size() != p_surface->array_len, ERR_INVALID_PARAMETER );


				DVector<Vector3>::Read read = array.read();
				const Vector3* src=read.ptr();

				// setting vertices means regenerating the AABB

				for (int i=0;i<p_surface->array_len;i++) {


					float vector[3]={ src[i].x, src[i].y, src[i].z };
					copymem(&p_mem[a.ofs+i*stride], vector, a.size);

				}

				p_surface->vp.normal(reinterpret_cast<Vector3 *>(&p_mem[a.ofs]), stride, true);

			} break;
			case VS::ARRAY_TANGENT: {

				ERR_FAIL_COND_V( p_arrays[ai].get_type() != Variant::REAL_ARRAY, ERR_INVALID_PARAMETER );

				DVector<real_t> array = p_arrays[ai];

				ERR_FAIL_COND_V( array.size() != p_surface->array_len*4, ERR_INVALID_PARAMETER );


				DVector<real_t>::Read read = array.read();
				const real_t* src = read.ptr();

				for (int i=0;i<p_surface->array_len;i++) {

					float xyzw[4]={
						src[i*4+0],
						src[i*4+1],
						src[i*4+2],
						src[i*4+3]
					};

					copymem(&p_mem[a.ofs+i*stride], xyzw, a.size);

				}

			} break;
			case VS::ARRAY_COLOR: {

				ERR_FAIL_COND_V( p_arrays[ai].get_type() != Variant::COLOR_ARRAY, ERR_INVALID_PARAMETER );


				DVector<Color> array = p_arrays[ai];

				ERR_FAIL_COND_V( array.size() != p_surface->array_len, ERR_INVALID_PARAMETER );


				DVector<Color>::Read read = array.read();
				const Color* src = read.ptr();
				bool alpha=false;

				for (int i=0;i<p_surface->array_len;i++) {

					if (src[i].a<0.98) // tolerate alpha a bit, for crappy exporters
						alpha=true;

					uint8_t colors[4];

					for(int j=0;j<4;j++) {

						colors[j]=CLAMP( int((src[i][j])*255.0), 0,255 );
					}

						copymem(&p_mem[a.ofs+i*stride], colors, a.size);

				}

				if (p_main)
					p_surface->has_alpha=alpha;

				p_surface->vp.color(reinterpret_cast<Color *>(&p_mem[a.ofs]), stride);

			} break;
			case VS::ARRAY_TEX_UV:
			case VS::ARRAY_TEX_UV2: {

				ERR_FAIL_COND_V( p_arrays[ai].get_type() != Variant::VECTOR3_ARRAY && p_arrays[ai].get_type() != Variant::VECTOR2_ARRAY, ERR_INVALID_PARAMETER );

				DVector<Vector2> array = p_arrays[ai];

				ERR_FAIL_COND_V( array.size() != p_surface->array_len , ERR_INVALID_PARAMETER);

				DVector<Vector2>::Read read = array.read();

				const Vector2 * src=read.ptr();
				float scale=1.0;

				Rect2 uv_bb;

				for (int i=0;i<p_surface->array_len;i++) {

					Vector2 uv{ src[i].x , src[i].y };
					
					if (i == 0) {
						uv_bb = {uv, {0.f, 0.f}};
					} else {
						uv_bb.expand_to(uv);
					}

					copymem(&p_mem[a.ofs+i*stride], &uv, a.size);

				}

				if (p_main) {

					if  (ai==VS::ARRAY_TEX_UV) {

						p_surface->uv_scale=scale;
					}
					if  (ai==VS::ARRAY_TEX_UV2) {

						p_surface->uv2_scale=scale;
					}
				}

				p_surface->uv_bb = uv_bb;

				if  (ai==VS::ARRAY_TEX_UV) {
					p_surface->vp.uv(reinterpret_cast<Vector2 *>(&p_mem[a.ofs]), stride, false);
				}

			} break;
			case VS::ARRAY_BONES:
			case VS::ARRAY_WEIGHTS: {


				ERR_FAIL_COND_V( p_arrays[ai].get_type() != Variant::REAL_ARRAY, ERR_INVALID_PARAMETER );

				DVector<real_t> array = p_arrays[ai];

				ERR_FAIL_COND_V( array.size() != p_surface->array_len*VS::ARRAY_WEIGHTS_SIZE, ERR_INVALID_PARAMETER );


				DVector<real_t>::Read read = array.read();

				const real_t * src = read.ptr();

				p_surface->max_bone=0;

				for (int i=0;i<p_surface->array_len;i++) {

					float data[VS::ARRAY_WEIGHTS_SIZE];
					for (int j=0;j<VS::ARRAY_WEIGHTS_SIZE;j++) {
						data[j]=src[i*VS::ARRAY_WEIGHTS_SIZE+j];
						if (ai==VS::ARRAY_BONES) {

							p_surface->max_bone=MAX(data[j],p_surface->max_bone);
						}
					}

					copymem(&p_mem[a.ofs+i*stride], data, a.size);


				}

				if (ai==VS::ARRAY_WEIGHTS) {
					p_surface->vp.weight(reinterpret_cast<float *>(&p_mem[a.ofs]), stride, true);
				}

			} break;
			case VS::ARRAY_INDEX: {

				ERR_FAIL_COND_V( p_surface->index_array_len<=0, ERR_INVALID_DATA );
				ERR_FAIL_COND_V( p_arrays[ai].get_type() != Variant::INT_ARRAY, ERR_INVALID_PARAMETER );

				DVector<int> indices = p_arrays[ai];
				ERR_FAIL_COND_V( indices.size() == 0, ERR_INVALID_PARAMETER );
				ERR_FAIL_COND_V( indices.size() != p_surface->index_array_len, ERR_INVALID_PARAMETER );

				/* determine wether using 16 or 32 bits indices */

				DVector<int>::Read read = indices.read();
				const int *src=read.ptr();

				for (int i=0;i<p_surface->index_array_len;i++) {


					if (a.size==2) {
						uint16_t v=src[i];

						copymem(&p_index_mem[i*a.size], &v, a.size);
					} else {
						uint32_t v=src[i];

						copymem(&p_index_mem[i*a.size], &v, a.size);
					}
				}


			} break;


			default: { ERR_FAIL_V(ERR_INVALID_PARAMETER);}
		}

		p_surface->configured_format|=(1<<ai);
	}

	p_surface->psp_array_local = p_surface->vp.pack<vtalloc>(p_surface->aabb, p_surface->uv_bb);
	p_surface->psp_vattribs = p_surface->vp.attrs();

	//vtfree(p_surface->array_local);
	//p_surface->array_local = nullptr;

	return OK;
}



void RasterizerPSP::mesh_add_custom_surface(RID p_mesh,const Variant& p_dat) {

	ERR_EXPLAIN("OpenGL Rasterizer does not support custom surfaces. Running on wrong platform?");
	ERR_FAIL_V();
}

Array RasterizerPSP::mesh_get_surface_arrays(RID p_mesh,int p_surface) const {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,Array());
	ERR_FAIL_INDEX_V(p_surface, mesh->surfaces.size(), Array() );
	Surface *surface = mesh->surfaces[p_surface];
	ERR_FAIL_COND_V( !surface, Array() );

	return surface->data;


}
Array RasterizerPSP::mesh_get_surface_morph_arrays(RID p_mesh,int p_surface) const{

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,Array());
	ERR_FAIL_INDEX_V(p_surface, mesh->surfaces.size(), Array() );
	Surface *surface = mesh->surfaces[p_surface];
	ERR_FAIL_COND_V( !surface, Array() );

	return surface->morph_data;

}


void RasterizerPSP::mesh_set_morph_target_count(RID p_mesh,int p_amount) {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND(!mesh);
	ERR_FAIL_COND( mesh->surfaces.size()!=0 );

	mesh->morph_target_count=p_amount;

}

int RasterizerPSP::mesh_get_morph_target_count(RID p_mesh) const{

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,-1);

	return mesh->morph_target_count;

}

void RasterizerPSP::mesh_set_morph_target_mode(RID p_mesh,VS::MorphTargetMode p_mode) {

	ERR_FAIL_INDEX(p_mode,2);
	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND(!mesh);

	mesh->morph_target_mode=p_mode;

}

VS::MorphTargetMode RasterizerPSP::mesh_get_morph_target_mode(RID p_mesh) const {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,VS::MORPH_MODE_NORMALIZED);

	return mesh->morph_target_mode;

}



void RasterizerPSP::mesh_surface_set_material(RID p_mesh, int p_surface, RID p_material,bool p_owned) {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND(!mesh);
	ERR_FAIL_INDEX(p_surface, mesh->surfaces.size() );
	Surface *surface = mesh->surfaces[p_surface];
	ERR_FAIL_COND( !surface);

	if (surface->material_owned && surface->material.is_valid())
		free(surface->material);

	surface->material_owned=p_owned;

	surface->material=p_material;
}

RID RasterizerPSP::mesh_surface_get_material(RID p_mesh, int p_surface) const {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,RID());
	ERR_FAIL_INDEX_V(p_surface, mesh->surfaces.size(), RID() );
	Surface *surface = mesh->surfaces[p_surface];
	ERR_FAIL_COND_V( !surface, RID() );

	return surface->material;
}

int RasterizerPSP::mesh_surface_get_array_len(RID p_mesh, int p_surface) const {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,-1);
	ERR_FAIL_INDEX_V(p_surface, mesh->surfaces.size(), -1 );
	Surface *surface = mesh->surfaces[p_surface];
	ERR_FAIL_COND_V( !surface, -1 );

	return surface->array_len;
}
int RasterizerPSP::mesh_surface_get_array_index_len(RID p_mesh, int p_surface) const {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,-1);
	ERR_FAIL_INDEX_V(p_surface, mesh->surfaces.size(), -1 );
	Surface *surface = mesh->surfaces[p_surface];
	ERR_FAIL_COND_V( !surface, -1 );

	return surface->index_array_len;
}
uint32_t RasterizerPSP::mesh_surface_get_format(RID p_mesh, int p_surface) const {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,0);
	ERR_FAIL_INDEX_V(p_surface, mesh->surfaces.size(), 0 );
	Surface *surface = mesh->surfaces[p_surface];
	ERR_FAIL_COND_V( !surface, 0 );

	return surface->format;
}
VS::PrimitiveType RasterizerPSP::mesh_surface_get_primitive_type(RID p_mesh, int p_surface) const {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,VS::PRIMITIVE_POINTS);
	ERR_FAIL_INDEX_V(p_surface, mesh->surfaces.size(), VS::PRIMITIVE_POINTS );
	Surface *surface = mesh->surfaces[p_surface];
	ERR_FAIL_COND_V( !surface, VS::PRIMITIVE_POINTS );

	return surface->primitive;
}

void RasterizerPSP::mesh_remove_surface(RID p_mesh,int p_index) {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND(!mesh);
	ERR_FAIL_INDEX(p_index, mesh->surfaces.size() );
	Surface *surface = mesh->surfaces[p_index];
	ERR_FAIL_COND( !surface);

	if (mesh->morph_target_count) {
		for(int i=0;i<mesh->morph_target_count;i++)
			memfree(surface->morph_targets_local[i].array);
		memfree( surface->morph_targets_local );
	}

	memdelete( mesh->surfaces[p_index] );
	mesh->surfaces.remove(p_index);

}
int RasterizerPSP::mesh_get_surface_count(RID p_mesh) const {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,-1);

	return mesh->surfaces.size();
}

AABB RasterizerPSP::mesh_get_aabb(RID p_mesh,RID p_skeleton) const {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,AABB());

	if (mesh->custom_aabb!=AABB())
		return mesh->custom_aabb;

	AABB aabb;

	for (int i=0;i<mesh->surfaces.size();i++) {

		if (i==0)
			aabb=mesh->surfaces[i]->aabb;
		else
			aabb.merge_with(mesh->surfaces[i]->aabb);
	}

	return aabb;
}

void RasterizerPSP::mesh_set_custom_aabb(RID p_mesh,const AABB& p_aabb) {

	Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND(!mesh);

	mesh->custom_aabb=p_aabb;

}

AABB RasterizerPSP::mesh_get_custom_aabb(RID p_mesh) const {

	const Mesh *mesh = mesh_owner.get( p_mesh );
	ERR_FAIL_COND_V(!mesh,AABB());

	return mesh->custom_aabb;
}


/* MULTIMESH API */

RID RasterizerPSP::multimesh_create() {

	return multimesh_owner.make_rid( memnew( MultiMesh ));
}

void RasterizerPSP::multimesh_set_instance_count(RID p_multimesh,int p_count) {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND(!multimesh);

			if (nearest_power_of_2(p_count)!=nearest_power_of_2(multimesh->elements.size())) {
			if (multimesh->tex_id) {
				memfree((void*)multimesh->tex_id);
				multimesh->tex_id=0;
			}
			/*
			if (p_count) {

				uint32_t po2 = nearest_power_of_2(p_count);
				if (po2&0xAAAAAAAA) {
					//half width

					multimesh->tw=Math::sqrt(po2*2);
					multimesh->th=multimesh->tw/2;
				} else {

					multimesh->tw=Math::sqrt(po2);
					multimesh->th=multimesh->tw;

				}
				multimesh->tw*=4;
				if (multimesh->th==0)
					multimesh->th=1;



				glGenTextures(1, &multimesh->tex_id);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D,multimesh->tex_id);

#ifdef GLEW_ENABLED
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, multimesh->tw, multimesh->th, 0, GL_RGBA, GL_FLOAT,NULL);
#else
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, multimesh->tw, multimesh->th, 0, GL_RGBA, GL_FLOAT,NULL);
#endif
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				//multimesh->pixel_size=1.0/ps;

				glBindTexture(GL_TEXTURE_2D,0);
			}*/

		}
	multimesh->elements.resize(p_count);

}
int RasterizerPSP::multimesh_get_instance_count(RID p_multimesh) const {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND_V(!multimesh,-1);

	return multimesh->elements.size();
}

void RasterizerPSP::multimesh_set_mesh(RID p_multimesh,RID p_mesh) {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND(!multimesh);

	multimesh->mesh=p_mesh;

}
void RasterizerPSP::multimesh_set_aabb(RID p_multimesh,const AABB& p_aabb) {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND(!multimesh);
	multimesh->aabb=p_aabb;
}
void RasterizerPSP::multimesh_instance_set_transform(RID p_multimesh,int p_index,const Transform3D& p_transform) {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND(!multimesh);
	ERR_FAIL_INDEX(p_index,multimesh->elements.size());
	MultiMesh::Element &e=multimesh->elements[p_index];

	e.matrix[0]=p_transform.basis.elements[0][0];
	e.matrix[1]=p_transform.basis.elements[1][0];
	e.matrix[2]=p_transform.basis.elements[2][0];
	e.matrix[3]=0;
	e.matrix[4]=p_transform.basis.elements[0][1];
	e.matrix[5]=p_transform.basis.elements[1][1];
	e.matrix[6]=p_transform.basis.elements[2][1];
	e.matrix[7]=0;
	e.matrix[8]=p_transform.basis.elements[0][2];
	e.matrix[9]=p_transform.basis.elements[1][2];
	e.matrix[10]=p_transform.basis.elements[2][2];
	e.matrix[11]=0;
	e.matrix[12]=p_transform.origin.x;
	e.matrix[13]=p_transform.origin.y;
	e.matrix[14]=p_transform.origin.z;
	e.matrix[15]=1;

}
void RasterizerPSP::multimesh_instance_set_color(RID p_multimesh,int p_index,const Color& p_color) {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND(!multimesh)
	ERR_FAIL_INDEX(p_index,multimesh->elements.size());
	MultiMesh::Element &e=multimesh->elements[p_index];
	e.color[0]=CLAMP(p_color.r*255,0,255);
	e.color[1]=CLAMP(p_color.g*255,0,255);
	e.color[2]=CLAMP(p_color.b*255,0,255);
	e.color[3]=CLAMP(p_color.a*255,0,255);


}

RID RasterizerPSP::multimesh_get_mesh(RID p_multimesh) const {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND_V(!multimesh,RID());

	return multimesh->mesh;
}
AABB RasterizerPSP::multimesh_get_aabb(RID p_multimesh) const {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND_V(!multimesh,AABB());

	return multimesh->aabb;
}

Transform3D RasterizerPSP::multimesh_instance_get_transform(RID p_multimesh,int p_index) const {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND_V(!multimesh,Transform3D());

	ERR_FAIL_INDEX_V(p_index,multimesh->elements.size(),Transform3D());
	MultiMesh::Element &e=multimesh->elements[p_index];

	Transform3D tr;

	tr.basis.elements[0][0]=e.matrix[0];
	tr.basis.elements[1][0]=e.matrix[1];
	tr.basis.elements[2][0]=e.matrix[2];
	tr.basis.elements[0][1]=e.matrix[4];
	tr.basis.elements[1][1]=e.matrix[5];
	tr.basis.elements[2][1]=e.matrix[6];
	tr.basis.elements[0][2]=e.matrix[8];
	tr.basis.elements[1][2]=e.matrix[9];
	tr.basis.elements[2][2]=e.matrix[10];
	tr.origin.x=e.matrix[12];
	tr.origin.y=e.matrix[13];
	tr.origin.z=e.matrix[14];

	return tr;
}
Color RasterizerPSP::multimesh_instance_get_color(RID p_multimesh,int p_index) const {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND_V(!multimesh,Color());
	ERR_FAIL_INDEX_V(p_index,multimesh->elements.size(),Color());
	MultiMesh::Element &e=multimesh->elements[p_index];
	Color c;
	c.r=e.color[0]/255.0;
	c.g=e.color[1]/255.0;
	c.b=e.color[2]/255.0;
	c.a=e.color[3]/255.0;

	return c;

}

void RasterizerPSP::multimesh_set_visible_instances(RID p_multimesh,int p_visible) {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND(!multimesh);
	multimesh->visible=p_visible;

}

int RasterizerPSP::multimesh_get_visible_instances(RID p_multimesh) const {

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND_V(!multimesh,-1);
	return multimesh->visible;

}

/* IMMEDIATE API */


RID RasterizerPSP::immediate_create() {

	Immediate *im = memnew( Immediate );
	return immediate_owner.make_rid(im);

}

void RasterizerPSP::immediate_begin(RID p_immediate, VS::PrimitiveType p_rimitive, RID p_texture){


}
void RasterizerPSP::immediate_vertex(RID p_immediate,const Vector3& p_vertex){


}
void RasterizerPSP::immediate_normal(RID p_immediate,const Vector3& p_normal){


}
void RasterizerPSP::immediate_tangent(RID p_immediate,const Plane& p_tangent){


}
void RasterizerPSP::immediate_color(RID p_immediate,const Color& p_color){


}
void RasterizerPSP::immediate_uv(RID p_immediate,const Vector2& tex_uv){


}
void RasterizerPSP::immediate_uv2(RID p_immediate,const Vector2& tex_uv){


}

void RasterizerPSP::immediate_end(RID p_immediate){


}
void RasterizerPSP::immediate_clear(RID p_immediate) {


}

AABB RasterizerPSP::immediate_get_aabb(RID p_immediate) const {

	return AABB(Vector3(-1,-1,-1),Vector3(2,2,2));
}

void RasterizerPSP::immediate_set_material(RID p_immediate,RID p_material) {

	Immediate *im = immediate_owner.get(p_immediate);
	ERR_FAIL_COND(!im);
	im->material=p_material;
}

RID RasterizerPSP::immediate_get_material(RID p_immediate) const {

	const Immediate *im = immediate_owner.get(p_immediate);
	ERR_FAIL_COND_V(!im,RID());
	return im->material;

}


/* PARTICLES API */

RID RasterizerPSP::particles_create() {

	Particles3D *particles = memnew( Particles3D );
	ERR_FAIL_COND_V(!particles,RID());
	return particles_owner.make_rid(particles);
}

void RasterizerPSP::particles_set_amount(RID p_particles, int p_amount) {

	ERR_FAIL_COND(p_amount<1);
	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	particles->data.amount=p_amount;

}

int RasterizerPSP::particles_get_amount(RID p_particles) const {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,-1);
	return particles->data.amount;

}

void RasterizerPSP::particles_set_emitting(RID p_particles, bool p_emitting) {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	particles->data.emitting=p_emitting;;

}
bool RasterizerPSP::particles_is_emitting(RID p_particles) const {

	const Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,false);
	return particles->data.emitting;

}

void RasterizerPSP::particles_set_visibility_aabb(RID p_particles, const AABB& p_visibility) {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	particles->data.visibility_aabb=p_visibility;

}

void RasterizerPSP::particles_set_emission_half_extents(RID p_particles, const Vector3& p_half_extents) {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);

	particles->data.emission_half_extents=p_half_extents;
}
Vector3 RasterizerPSP::particles_get_emission_half_extents(RID p_particles) const {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,Vector3());

	return particles->data.emission_half_extents;
}

void RasterizerPSP::particles_set_emission_base_velocity(RID p_particles, const Vector3& p_base_velocity) {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);

	particles->data.emission_base_velocity=p_base_velocity;
}

Vector3 RasterizerPSP::particles_get_emission_base_velocity(RID p_particles) const {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,Vector3());

	return particles->data.emission_base_velocity;
}


void RasterizerPSP::particles_set_emission_points(RID p_particles, const DVector<Vector3>& p_points) {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);

	particles->data.emission_points=p_points;
}

DVector<Vector3> RasterizerPSP::particles_get_emission_points(RID p_particles) const {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,DVector<Vector3>());

	return particles->data.emission_points;

}

void RasterizerPSP::particles_set_gravity_normal(RID p_particles, const Vector3& p_normal) {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);

	particles->data.gravity_normal=p_normal;

}
Vector3 RasterizerPSP::particles_get_gravity_normal(RID p_particles) const {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,Vector3());

	return particles->data.gravity_normal;
}


AABB RasterizerPSP::particles_get_visibility_aabb(RID p_particles) const {

	const Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,AABB());
	return particles->data.visibility_aabb;

}

void RasterizerPSP::particles_set_variable(RID p_particles, VS::ParticleVariable p_variable,float p_value) {

	ERR_FAIL_INDEX(p_variable,VS::PARTICLE_VAR_MAX);

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	particles->data.particle_vars[p_variable]=p_value;

}
float RasterizerPSP::particles_get_variable(RID p_particles, VS::ParticleVariable p_variable) const {

	const Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,-1);
	return particles->data.particle_vars[p_variable];
}

void RasterizerPSP::particles_set_randomness(RID p_particles, VS::ParticleVariable p_variable,float p_randomness) {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	particles->data.particle_randomness[p_variable]=p_randomness;

}
float RasterizerPSP::particles_get_randomness(RID p_particles, VS::ParticleVariable p_variable) const {

	const Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,-1);
	return particles->data.particle_randomness[p_variable];

}

void RasterizerPSP::particles_set_color_phases(RID p_particles, int p_phases) {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	ERR_FAIL_COND( p_phases<0 || p_phases>VS::MAX_PARTICLE_COLOR_PHASES );
	particles->data.color_phase_count=p_phases;

}
int RasterizerPSP::particles_get_color_phases(RID p_particles) const {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,-1);
	return particles->data.color_phase_count;
}


void RasterizerPSP::particles_set_color_phase_pos(RID p_particles, int p_phase, float p_pos) {

	ERR_FAIL_INDEX(p_phase, VS::MAX_PARTICLE_COLOR_PHASES);
	if (p_pos<0.0)
		p_pos=0.0;
	if (p_pos>1.0)
		p_pos=1.0;

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	particles->data.color_phases[p_phase].pos=p_pos;

}
float RasterizerPSP::particles_get_color_phase_pos(RID p_particles, int p_phase) const {

	ERR_FAIL_INDEX_V(p_phase, VS::MAX_PARTICLE_COLOR_PHASES, -1.0);

	const Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,-1);
	return particles->data.color_phases[p_phase].pos;

}

void RasterizerPSP::particles_set_color_phase_color(RID p_particles, int p_phase, const Color& p_color) {

	ERR_FAIL_INDEX(p_phase, VS::MAX_PARTICLE_COLOR_PHASES);
	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	particles->data.color_phases[p_phase].color=p_color;

	//update alpha
	particles->has_alpha=false;
	for(int i=0;i<VS::MAX_PARTICLE_COLOR_PHASES;i++) {
		if (particles->data.color_phases[i].color.a<0.99)
			particles->has_alpha=true;
	}

}

Color RasterizerPSP::particles_get_color_phase_color(RID p_particles, int p_phase) const {

	ERR_FAIL_INDEX_V(p_phase, VS::MAX_PARTICLE_COLOR_PHASES, Color());

	const Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,Color());
	return particles->data.color_phases[p_phase].color;

}

void RasterizerPSP::particles_set_attractors(RID p_particles, int p_attractors) {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	ERR_FAIL_COND( p_attractors<0 || p_attractors>VisualServer::MAX_PARTICLE_ATTRACTORS );
	particles->data.attractor_count=p_attractors;

}
int RasterizerPSP::particles_get_attractors(RID p_particles) const {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,-1);
	return particles->data.attractor_count;
}

void RasterizerPSP::particles_set_attractor_pos(RID p_particles, int p_attractor, const Vector3& p_pos) {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	ERR_FAIL_INDEX(p_attractor,particles->data.attractor_count);
	particles->data.attractors[p_attractor].pos=p_pos;;
}
Vector3 RasterizerPSP::particles_get_attractor_pos(RID p_particles,int p_attractor) const {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,Vector3());
	ERR_FAIL_INDEX_V(p_attractor,particles->data.attractor_count,Vector3());
	return particles->data.attractors[p_attractor].pos;
}

void RasterizerPSP::particles_set_attractor_strength(RID p_particles, int p_attractor, float p_force) {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	ERR_FAIL_INDEX(p_attractor,particles->data.attractor_count);
	particles->data.attractors[p_attractor].force=p_force;
}

float RasterizerPSP::particles_get_attractor_strength(RID p_particles,int p_attractor) const {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,0);
	ERR_FAIL_INDEX_V(p_attractor,particles->data.attractor_count,0);
	return particles->data.attractors[p_attractor].force;
}

void RasterizerPSP::particles_set_material(RID p_particles, RID p_material,bool p_owned) {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	if (particles->material_owned && particles->material.is_valid())
		free(particles->material);

	particles->material_owned=p_owned;

	particles->material=p_material;

}
RID RasterizerPSP::particles_get_material(RID p_particles) const {

	const Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,RID());
	return particles->material;

}

void RasterizerPSP::particles_set_use_local_coordinates(RID p_particles, bool p_enable) {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	particles->data.local_coordinates=p_enable;

}

bool RasterizerPSP::particles_is_using_local_coordinates(RID p_particles) const {

	const Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,false);
	return particles->data.local_coordinates;
}
bool RasterizerPSP::particles_has_height_from_velocity(RID p_particles) const {

	const Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,false);
	return particles->data.height_from_velocity;
}

void RasterizerPSP::particles_set_height_from_velocity(RID p_particles, bool p_enable) {

	Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND(!particles);
	particles->data.height_from_velocity=p_enable;

}

AABB RasterizerPSP::particles_get_aabb(RID p_particles) const {

	const Particles3D* particles = particles_owner.get( p_particles );
	ERR_FAIL_COND_V(!particles,AABB());
	return particles->data.visibility_aabb;
}

/* SKELETON API */

RID RasterizerPSP::skeleton_create() {

	Skeleton3D *skeleton = memnew( Skeleton3D );
	ERR_FAIL_COND_V(!skeleton,RID());
	return skeleton_owner.make_rid( skeleton );
}
void RasterizerPSP::skeleton_resize(RID p_skeleton,int p_bones) {

	Skeleton3D *skeleton = skeleton_owner.get( p_skeleton );
	ERR_FAIL_COND(!skeleton);
	if (p_bones == skeleton->bones.size()) {
		return;
	};

	skeleton->bones.resize(p_bones);

}
int RasterizerPSP::skeleton_get_bone_count(RID p_skeleton) const {

	Skeleton3D *skeleton = skeleton_owner.get( p_skeleton );
	ERR_FAIL_COND_V(!skeleton, -1);
	return skeleton->bones.size();
}
void RasterizerPSP::skeleton_bone_set_transform(RID p_skeleton,int p_bone, const Transform3D& p_transform) {

	Skeleton3D *skeleton = skeleton_owner.get( p_skeleton );
	ERR_FAIL_COND(!skeleton);
	ERR_FAIL_INDEX( p_bone, skeleton->bones.size() );

	skeleton->bones[p_bone] = p_transform;
}

Transform3D RasterizerPSP::skeleton_bone_get_transform(RID p_skeleton,int p_bone) {

	Skeleton3D *skeleton = skeleton_owner.get( p_skeleton );
	ERR_FAIL_COND_V(!skeleton, Transform3D());
	ERR_FAIL_INDEX_V( p_bone, skeleton->bones.size(), Transform3D() );

	// something
	return skeleton->bones[p_bone];
}


/* LIGHT API */

RID RasterizerPSP::light_create(VS::LightType p_type) {

	Light *light = memnew( Light );
	light->type=p_type;
	return light_owner.make_rid(light);
}

VS::LightType RasterizerPSP::light_get_type(RID p_light) const {

	Light *light = light_owner.get(p_light);
	ERR_FAIL_COND_V(!light,VS::LIGHT_OMNI);
	return light->type;
}

void RasterizerPSP::light_set_color(RID p_light,VS::LightColor p_type, const Color& p_color) {

	Light *light = light_owner.get(p_light);
	ERR_FAIL_COND(!light);
	ERR_FAIL_INDEX( p_type, 3 );
	light->colors[p_type]=p_color;
}
Color RasterizerPSP::light_get_color(RID p_light,VS::LightColor p_type) const {

	Light *light = light_owner.get(p_light);
	ERR_FAIL_COND_V(!light, Color());
	ERR_FAIL_INDEX_V( p_type, 3, Color() );
	return light->colors[p_type];
}

void RasterizerPSP::light_set_shadow(RID p_light,bool p_enabled) {

	Light *light = light_owner.get(p_light);
	ERR_FAIL_COND(!light);
	light->shadow_enabled=p_enabled;
}

bool RasterizerPSP::light_has_shadow(RID p_light) const {

	Light *light = light_owner.get(p_light);
	ERR_FAIL_COND_V(!light,false);
	return light->shadow_enabled;
}

void RasterizerPSP::light_set_volumetric(RID p_light,bool p_enabled) {

	Light *light = light_owner.get(p_light);
	ERR_FAIL_COND(!light);
	light->volumetric_enabled=p_enabled;

}
bool RasterizerPSP::light_is_volumetric(RID p_light) const {

	Light *light = light_owner.get(p_light);
	ERR_FAIL_COND_V(!light,false);
	return light->volumetric_enabled;
}

void RasterizerPSP::light_set_projector(RID p_light,RID p_texture) {

	Light *light = light_owner.get(p_light);
	ERR_FAIL_COND(!light);
	light->projector=p_texture;
}
RID RasterizerPSP::light_get_projector(RID p_light) const {

	Light *light = light_owner.get(p_light);
	ERR_FAIL_COND_V(!light,RID());
	return light->projector;
}

void RasterizerPSP::light_set_var(RID p_light, VS::LightParam p_var, float p_value) {

	Light * light = light_owner.get( p_light );
	ERR_FAIL_COND(!light);
	ERR_FAIL_INDEX( p_var, VS::LIGHT_PARAM_MAX );

	light->vars[p_var]=p_value;
}
float RasterizerPSP::light_get_var(RID p_light, VS::LightParam p_var) const {

	Light * light = light_owner.get( p_light );
	ERR_FAIL_COND_V(!light,0);

	ERR_FAIL_INDEX_V( p_var, VS::LIGHT_PARAM_MAX,0 );

	return light->vars[p_var];
}

void RasterizerPSP::light_set_operator(RID p_light,VS::LightOp p_op) {

	Light * light = light_owner.get( p_light );
	ERR_FAIL_COND(!light);


};

VS::LightOp RasterizerPSP::light_get_operator(RID p_light) const {

	return VS::LightOp(0);
};

void RasterizerPSP::light_omni_set_shadow_mode(RID p_light,VS::LightOmniShadowMode p_mode) {


}

VS::LightOmniShadowMode RasterizerPSP::light_omni_get_shadow_mode(RID p_light) const{

	return VS::LightOmniShadowMode(0);
}

void RasterizerPSP::light_directional_set_shadow_mode(RID p_light,VS::LightDirectionalShadowMode p_mode) {


}

VS::LightDirectionalShadowMode RasterizerPSP::light_directional_get_shadow_mode(RID p_light) const {

	return VS::LIGHT_DIRECTIONAL_SHADOW_ORTHOGONAL;
}

void RasterizerPSP::light_directional_set_shadow_param(RID p_light,VS::LightDirectionalShadowParam p_param, float p_value) {


}

float RasterizerPSP::light_directional_get_shadow_param(RID p_light,VS::LightDirectionalShadowParam p_param) const {

	return 0;
}


AABB RasterizerPSP::light_get_aabb(RID p_light) const {

	Light *light = light_owner.get( p_light );
	ERR_FAIL_COND_V(!light,AABB());

	switch( light->type ) {

		case VS::LIGHT_SPOT: {

			float len=light->vars[VS::LIGHT_PARAM_RADIUS];
			float size=Math::tan(Math::deg2rad(light->vars[VS::LIGHT_PARAM_SPOT_ANGLE]))*len;
			return AABB( Vector3( -size,-size,-len ), Vector3( size*2, size*2, len ) );
		} break;
		case VS::LIGHT_OMNI: {

			float r = light->vars[VS::LIGHT_PARAM_RADIUS];
			return AABB( -Vector3(r,r,r), Vector3(r,r,r)*2 );
		} break;
		case VS::LIGHT_DIRECTIONAL: {

			return AABB();
		} break;
		default: {}
	}

	ERR_FAIL_V( AABB() );
}


RID RasterizerPSP::light_instance_create(RID p_light) {

	Light *light = light_owner.get( p_light );
	ERR_FAIL_COND_V(!light, RID());

	LightInstance *light_instance = memnew( LightInstance );

	light_instance->light=p_light;
	light_instance->base=light;
	light_instance->last_pass=0;

	return light_instance_owner.make_rid( light_instance );
}
void RasterizerPSP::light_instance_set_transform(RID p_light_instance,const Transform3D& p_transform) {

	LightInstance *lighti = light_instance_owner.get( p_light_instance );
	ERR_FAIL_COND(!lighti);
	lighti->transform=p_transform;

}

bool RasterizerPSP::light_instance_has_shadow(RID p_light_instance) const {

	return false;

	/*
	LightInstance *lighti = light_instance_owner.get( p_light_instance );
	ERR_FAIL_COND_V(!lighti, false);

	if (!lighti->base->shadow_enabled)
		return false;

	if (lighti->base->type==VS::LIGHT_DIRECTIONAL) {
		if (lighti->shadow_pass!=scene_pass)
			return false;

	} else {
		if (lighti->shadow_pass!=frame)
			return false;
	}*/



	//return !lighti->shadow_buffers.empty();

}


bool RasterizerPSP::light_instance_assign_shadow(RID p_light_instance) {

	return false;

}


Rasterizer::ShadowType RasterizerPSP::light_instance_get_shadow_type(RID p_light_instance) const {

	LightInstance *lighti = light_instance_owner.get( p_light_instance );
	ERR_FAIL_COND_V(!lighti,Rasterizer::SHADOW_NONE);

	switch(lighti->base->type) {

		case VS::LIGHT_DIRECTIONAL: return SHADOW_PSM; break;
		case VS::LIGHT_OMNI: return SHADOW_DUAL_PARABOLOID; break;
		case VS::LIGHT_SPOT: return SHADOW_SIMPLE; break;
	}

	return Rasterizer::SHADOW_NONE;
}

Rasterizer::ShadowType RasterizerPSP::light_instance_get_shadow_type(RID p_light_instance,bool p_far) const {

	return SHADOW_NONE;
}
void RasterizerPSP::light_instance_set_shadow_transform(RID p_light_instance, int p_index, const CameraMatrix& p_camera, const Transform3D& p_transform, float p_split_near,float p_split_far) {


}

int RasterizerPSP::light_instance_get_shadow_passes(RID p_light_instance) const {

	return 0;
}

bool RasterizerPSP::light_instance_get_pssm_shadow_overlap(RID p_light_instance) const {

	return false;
}

void RasterizerPSP::light_instance_set_custom_transform(RID p_light_instance, int p_index, const CameraMatrix& p_camera, const Transform3D& p_transform, float p_split_near,float p_split_far) {

	LightInstance *lighti = light_instance_owner.get( p_light_instance );
	ERR_FAIL_COND(!lighti);

	ERR_FAIL_COND(lighti->base->type!=VS::LIGHT_DIRECTIONAL);
	ERR_FAIL_INDEX(p_index,1);

	lighti->custom_projection=p_camera;
	lighti->custom_transform=p_transform;

}
void RasterizerPSP::shadow_clear_near() {


}

bool RasterizerPSP::shadow_allocate_near(RID p_light) {

	return false;
}

bool RasterizerPSP::shadow_allocate_far(RID p_light) {

	return false;
}

/* PARTICLES INSTANCE */

RID RasterizerPSP::particles_instance_create(RID p_particles) {

	ERR_FAIL_COND_V(!particles_owner.owns(p_particles),RID());
	ParticlesInstance *particles_instance = memnew( ParticlesInstance );
	ERR_FAIL_COND_V(!particles_instance, RID() );
	particles_instance->particles=p_particles;
	return particles_instance_owner.make_rid(particles_instance);
}

void RasterizerPSP::particles_instance_set_transform(RID p_particles_instance,const Transform3D& p_transform) {

	ParticlesInstance *particles_instance=particles_instance_owner.get(p_particles_instance);
	ERR_FAIL_COND(!particles_instance);
	particles_instance->transform=p_transform;
}


/* RENDER API */
/* all calls (inside begin/end shadow) are always warranted to be in the following order: */


RID RasterizerPSP::viewport_data_create() {

	return RID();
}

RID RasterizerPSP::render_target_create(){

	return RID();

}
void RasterizerPSP::render_target_set_size(RID p_render_target, int p_width, int p_height){


}
RID RasterizerPSP::render_target_get_texture(RID p_render_target) const{

	return RID();

}
bool RasterizerPSP::render_target_renedered_in_frame(RID p_render_target){

	return false;
}


void RasterizerPSP::begin_frame() {

	// printf("begin_frame\n");
	window_size = Size2( OS::get_singleton()->get_video_mode().width, OS::get_singleton()->get_video_mode().height );
	//print_line("begin frame - winsize: "+window_size);

	double time = (OS::get_singleton()->get_ticks_usec()/1000); // get msec
	time/=1000.0; // make secs
	time_delta=time-last_time;
	last_time=time;
	frame++;
	clear_viewport(Color(1,0,0.5));

	_rinfo.vertex_count=0;
	_rinfo.object_count=0;
	_rinfo.mat_change_count=0;
	_rinfo.shader_change_count=0;
	sceGuStart(GU_DIRECT,list);
}

void RasterizerPSP::capture_viewport(Image* r_capture) {


}


void RasterizerPSP::clear_viewport(const Color& p_color) {

	sceGuScissor( viewport.x, window_size.height-(viewport.height+viewport.y), viewport.width,viewport.height );
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuClearColor(MK_RGBA(p_color.r*255,p_color.g*255,p_color.b*255,p_color.a*255));
	sceGuClear(GU_COLOR_BUFFER_BIT); //should not clear if anything else cleared..
	sceGuDisable(GU_SCISSOR_TEST);

};

void RasterizerPSP::set_viewport(const VS::ViewportRect& p_viewport) {



	viewport=p_viewport;
	//print_line("viewport: "+itos(p_viewport.x)+","+itos(p_viewport.y)+","+itos(p_viewport.width)+","+itos(p_viewport.height));

	// TODO:XXX:
	//sceGuViewport( viewport.x, window_size.height-(viewport.height+viewport.y), viewport.width,viewport.height );
}

void RasterizerPSP::set_render_target(RID p_render_target, bool p_transparent_bg, bool p_vflip) {


}


void RasterizerPSP::begin_scene(RID p_viewport_data,RID p_env,VS::ScenarioDebugMode p_debug) {
	opaque_render_list.clear();
	alpha_render_list.clear();
	light_instance_count=0;
	scene_fx = NULL; // p_env.is_valid() ? fx_owner.get(p_env) : NULL;
	scene_pass++;
	last_light_id=0;
	directional_light_count=0;
	current_env = environment_owner.get(p_env);

	//set state

	// sceGuCull(GL_FRONT);
	sceGuFrontFace(GU_CW);
	cull_front=true;
};

void RasterizerPSP::begin_shadow_map( RID p_light_instance, int p_shadow_pass ) {

}

void RasterizerPSP::set_camera(const Transform3D& p_world,const CameraMatrix& p_projection) {

	camera_transform=p_world;
	camera_transform_inverse=camera_transform.inverse();
	camera_projection=p_projection;
	camera_plane = Plane( camera_transform.origin, camera_transform.basis.get_axis(2) );
	camera_z_near=camera_projection.get_z_near();
	camera_z_far=camera_projection.get_z_far();
	camera_projection.get_viewport_size(camera_vp_size.x,camera_vp_size.y);
}

void RasterizerPSP::add_light( RID p_light_instance ) {

#define LIGHT_FADE_TRESHOLD 0.05

	ERR_FAIL_COND( light_instance_count >= MAX_SCENE_LIGHTS );

	LightInstance *li = light_instance_owner.get(p_light_instance);
	ERR_FAIL_COND(!li);


	/* make light hash */

	// actually, not really a hash, but helps to sort the lights
	// and avoid recompiling redudant shader versions


	li->last_pass=scene_pass;
	li->sort_key=light_instance_count;

	 switch(li->base->type) {

		case VisualServer::LIGHT_DIRECTIONAL: {

			li->light_vector = camera_transform_inverse.basis.xform(li->transform.basis.get_axis(2)).normalized();
			if (directional_light_count<MAX_HW_LIGHTS) {

				directional_lights[directional_light_count++]=li;
			}

		} break;
		case VisualServer::LIGHT_OMNI: {

			  float radius = li->base->vars[VisualServer::LIGHT_PARAM_RADIUS];
			  if (radius==0)
				  radius=0.0001;
			  li->linear_att=(LIGHT_FADE_TRESHOLD)/radius;
			  li->light_vector = camera_transform_inverse.xform(li->transform.origin);

		} break;
		case VisualServer::LIGHT_SPOT: {

			float radius = li->base->vars[VisualServer::LIGHT_PARAM_RADIUS];
			if (radius==0)
				radius=0.0001;
			li->linear_att=(LIGHT_FADE_TRESHOLD)/radius;
			li->light_vector = camera_transform_inverse.xform(li->transform.origin);
			li->spot_vector = -camera_transform_inverse.basis.xform(li->transform.basis.get_axis(2)).normalized();
			//li->sort_key|=LIGHT_SPOT_BIT; // this way, omnis go first, spots go last and less shader versions are generated

			/*
			if (li->base->projector.is_valid()) {

				float far = li->base->vars[ VS::LIGHT_VAR_RADIUS ];
				ERR_FAIL_COND( far<=0 );
				float near= far/200.0;
				if (near<0.05)
					near=0.05;

				float angle = li->base->vars[ VS::LIGHT_VAR_SPOT_ANGLE ];

				//CameraMatrix proj;
				//proj.set_perspective( angle*2.0, 1.0, near, far );

				//Transform3D modelview=Transform3D(camera_transform_inverse * li->transform).inverse();
				//li->projector_mtx= proj * modelview;

			}*/
		} break;
	 }

	light_instances[light_instance_count++]=li;

}

void RasterizerPSP::_add_geometry( const Geometry* p_geometry, const InstanceData *p_instance, const Geometry *p_geometry_cmp, const GeometryOwner *p_owner) {

	Material *m=NULL;
	RID m_src=p_instance->material_override.is_valid() ? p_instance->material_override : p_geometry->material;

	if (m_src)
		m=material_owner.get( m_src );

	if (!m) {
		m=material_owner.get( default_material );
	}

	ERR_FAIL_COND(!m);


	if (m->last_pass!=frame) {

		m->last_pass=frame;
	}


	LightInstance *lights[RenderList::MAX_LIGHTS];
	int light_count=0;

	RenderList *render_list=&opaque_render_list;
	if (m->fixed_flags[VS::FIXED_MATERIAL_FLAG_USE_ALPHA] || m->blend_mode!=VS::MATERIAL_BLEND_MODE_MIX) {
		render_list = &alpha_render_list;
	};

	if (!m->flags[VS::MATERIAL_FLAG_UNSHADED]) {

		int lis = p_instance->light_instances.size();

		for(int i=0;i<lis;i++) {
			if (light_count>=RenderList::MAX_LIGHTS)
				break;

			LightInstance *li=light_instance_owner.get( p_instance->light_instances[i] );

			if (!li || li->last_pass!=scene_pass) //lit by light not in visible scene
				continue;
			lights[light_count++]=li;
		}
	}

	RenderList::Element *e = render_list->add_element();

	e->geometry=p_geometry;
//	e->geometry_cmp=p_geometry_cmp;
	e->material=m;
	e->instance=p_instance;
	//e->depth=camera_plane.distance_to(p_world->origin);
	e->depth=camera_transform.origin.distance_to(p_instance->transform.origin);
	e->owner=p_owner;
	if (p_instance->skeleton.is_valid())
		e->skeleton=skeleton_owner.get(p_instance->skeleton);
	else
		e->skeleton=NULL;
	e->mirror=p_instance->mirror;
	if (m->flags[VS::MATERIAL_FLAG_INVERT_FACES])
		e->mirror=!e->mirror;

	e->light_key=0;
	e->light_count=0;


	if (!shadow) {


		if (m->flags[VS::MATERIAL_FLAG_UNSHADED]) {


			e->light_key--; //special key for all the shadeless people
		} else if (light_count) {

			for(int i=0;i<light_count;i++) {

				e->lights[i]=lights[i]->sort_key;			
			}

			e->light_count=light_count;
			int poslight_count=light_count;
			if (poslight_count>1) {
				SortArray<uint16_t> light_sort;
				light_sort.sort(&e->lights[0],poslight_count); //generate an equal sort key
			}
		}

	}

}


void RasterizerPSP::add_mesh( const RID& p_mesh, const InstanceData *p_data) {

	Mesh *mesh = mesh_owner.get(p_mesh);
	ERR_FAIL_COND(!mesh);

	int ssize = mesh->surfaces.size();

	for (int i=0;i<ssize;i++) {

		Surface *s = mesh->surfaces[i];
		_add_geometry(s,p_data,s,NULL);
	}

	mesh->last_pass=frame;

}

void RasterizerPSP::add_multimesh( const RID& p_multimesh, const InstanceData *p_data){

	MultiMesh *multimesh = multimesh_owner.get(p_multimesh);
	ERR_FAIL_COND(!multimesh);

	if (!multimesh->mesh.is_valid())
		return;
	if (multimesh->elements.empty())
		return;

	Mesh *mesh = mesh_owner.get(multimesh->mesh);
	ERR_FAIL_COND(!mesh);

	int surf_count = mesh->surfaces.size();
	if (multimesh->last_pass!=scene_pass) {

		multimesh->cache_surfaces.resize(surf_count);
		for(int i=0;i<surf_count;i++) {

			multimesh->cache_surfaces[i].material=mesh->surfaces[i]->material;
			multimesh->cache_surfaces[i].has_alpha=mesh->surfaces[i]->has_alpha;
			multimesh->cache_surfaces[i].surface=mesh->surfaces[i];
		}

		multimesh->last_pass=scene_pass;
	}

	for(int i=0;i<surf_count;i++) {

		_add_geometry(&multimesh->cache_surfaces[i],p_data,multimesh->cache_surfaces[i].surface,multimesh);
	}


}

void RasterizerPSP::add_particles( const RID& p_particle_instance, const InstanceData *p_data){

	//print_line("adding particles");
	ParticlesInstance *particles_instance = particles_instance_owner.get(p_particle_instance);
	ERR_FAIL_COND(!particles_instance);
	Particles3D *p=particles_owner.get( particles_instance->particles );
	ERR_FAIL_COND(!p);

	_add_geometry(p,p_data,p,particles_instance);

}


void RasterizerPSP::_set_cull(bool p_front,bool p_reverse_cull) {

	bool front = p_front;
	if (p_reverse_cull)
		front=!front;

	if (front!=cull_front) {

		// glCullFace(front?GL_FRONT:GL_BACK);
		if (cull_front) {
			sceGuFrontFace(GU_CW);
		} else {
			sceGuFrontFace(GU_CCW);
		}
		cull_front=front;
	}
}

void RasterizerPSP::_setup_texture(const Texture *texture) {
	if (!texture) {
		sceGuDisable(GU_TEXTURE_2D);
		return;
	}

	sceGuEnable(GU_TEXTURE_2D);
	sceGuTexMode(texture->gu_format_cache, texture->mipmaps.size()-1, 0, texture->swizzle);
	sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
	// if (texture->flags&VS::TEXTURE_FLAG_REPEAT) {
	// 	sceGuTexWrap(GU_REPEAT, GU_REPEAT);
	// } else {
		sceGuTexWrap(GU_CLAMP, GU_CLAMP);
	// }

	if (texture->mipmaps.size() > 1) {
		sceGuTexLevelMode(GU_TEXTURE_AUTO, 0.f);
		sceGuTexFilter(GU_LINEAR_MIPMAP_LINEAR, GU_LINEAR);
	} else {
		sceGuTexFilter(GU_LINEAR, GU_LINEAR);
	}

	int w = texture->alloc_width;
	int h = texture->alloc_height;
	for (int i = 0; i < texture->mipmaps.size(); ++i) {
		sceGuTexImage(i,w,h,w,texture->mipmaps[i]);

		w>>=1;
		h>>=1;
	}
}

void RasterizerPSP::_setup_fixed_material(const Geometry *p_geometry,const Material *p_material) {
	if (!shadow) {

		///ambient @TODO offer global ambient group option
		///diffuse
		Color diffuse_color=p_material->parameters[VS::FIXED_MATERIAL_PARAM_DIFFUSE];
		const auto diffuse_rgba = MK_RGBA_C(diffuse_color);
		sceGuColor(diffuse_rgba);

		//color array overrides this
		sceGuColorMaterial(GU_DIFFUSE | GU_AMBIENT | GU_SPECULAR);

		// sceGuAmbient(0x00222222);
		// sceGuAmbient(0xff000000);
		// sceGuSpecular(12.0f);
		// sceGuAmbientColor(diffuse_rgba);

		last_color=diffuse_color;
		sceGuMaterial(GU_AMBIENT, diffuse_rgba);
		sceGuSendCommandi(88, (diffuse_rgba >> 24) & 0xFF);

		sceGuMaterial(GU_DIFFUSE,diffuse_rgba);
		//specular

		const Color specular_color=p_material->parameters[VS::FIXED_MATERIAL_PARAM_SPECULAR];
		sceGuMaterial(GU_SPECULAR, MK_RGBA_F(
			specular_color.r,
			specular_color.g,
			specular_color.b,
			1.0
		));

		const Color emission=p_material->parameters[VS::FIXED_MATERIAL_PARAM_EMISSION];
		sceGuSendCommandi(84, MK_RGBA_F( // EMISSIVE
			emission.r,
			emission.g,
			emission.b,
			1.0 //p_material->parameters[VS::FIXED_MATERIAL_PARAM_DETAIL_MIX]
		));

		sceGuSpecular(p_material->parameters[VS::FIXED_MATERIAL_PARAM_SPECULAR_EXP]);

		sceGuShadeModel(GU_SMOOTH);

		//Plane sparams=p_material->parameters[VS::FIXED_MATERIAL_PARAM_SHADE_PARAM];
		//depth test?


	}


	if (p_material->textures[VS::FIXED_MATERIAL_PARAM_DIFFUSE].is_valid()) {

		Texture *texture = texture_owner.get( p_material->textures[VS::FIXED_MATERIAL_PARAM_DIFFUSE] );
		ERR_FAIL_COND(!texture);

		_setup_texture(texture);

	} else {

		sceGuDisable(GU_TEXTURE_2D);
	}
}

void RasterizerPSP::_setup_material(const Geometry *p_geometry,const Material *p_material) {

	if (p_material->flags[VS::MATERIAL_FLAG_DOUBLE_SIDED])
		sceGuDisable(GU_CULL_FACE);
	else {
		sceGuEnable(GU_CULL_FACE);
	}

/*	if (p_material->flags[VS::MATERIAL_FLAG_WIREFRAME])
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);*/
/*
	if (p_material->line_width > 0)
		glLineWidth(p_material->line_width);*/

	if (!shadow) {


		if (blend_mode!=p_material->blend_mode) {/*
			switch(p_material->blend_mode) {


				 case VS::MATERIAL_BLEND_MODE_MIX: {
					//glBlendEquation(GL_FUNC_ADD);
					glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

				 } break;
				 case VS::MATERIAL_BLEND_MODE_ADD: {

					//glBlendEquation(GL_FUNC_ADD);
					glBlendFunc(GL_SRC_ALPHA,GL_ONE);

				 } break;
				 case VS::MATERIAL_BLEND_MODE_SUB: {

					//glBlendEquation(GL_FUNC_SUBTRACT);
					glBlendFunc(GL_SRC_ALPHA,GL_ONE);
				 } break;
				case VS::MATERIAL_BLEND_MODE_MUL: {
					//glBlendEquation(GL_FUNC_ADD);
					glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

				} break;

			}*/
			switch(p_material->blend_mode) {

				case VS::MATERIAL_BLEND_MODE_MIX: {

					// sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA);
					sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA,0,0);

				} break;
				case VS::MATERIAL_BLEND_MODE_ADD: {
					sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_DST_ALPHA, 0, 0);

				} break;
				case VS::MATERIAL_BLEND_MODE_SUB: {
					sceGuBlendFunc(GU_SUBTRACT, GU_SRC_ALPHA, GU_DST_ALPHA, GU_SRC_ALPHA, GU_DST_ALPHA);
				} break;
				case VS::MATERIAL_BLEND_MODE_MUL: {
					// sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA);
					sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA,0,0);

				} break;

			}
			blend_mode=p_material->blend_mode;
		}

		if (lighting!=!p_material->flags[VS::MATERIAL_FLAG_UNSHADED]) {
			if (p_material->flags[VS::MATERIAL_FLAG_UNSHADED]) {
				// glDisable(GL_LIGHTING);
				//sceGuDisable(GU_LIGHTING);
			} else {
				// glEnable(GL_LIGHTING);
				sceGuEnable(GU_LIGHTING);
			}
			lighting=!p_material->flags[VS::MATERIAL_FLAG_UNSHADED];
		}

	}

	bool current_depth_write=p_material->depth_draw_mode!=VS::MATERIAL_DEPTH_DRAW_ALWAYS; //broken
	bool current_depth_test=!p_material->flags[VS::MATERIAL_FLAG_ONTOP];


	_setup_fixed_material(p_geometry,p_material);

	if (current_depth_write!=depth_write) {

		depth_write=current_depth_write;
		sceGuDepthMask(!depth_write);
	}

	if (current_depth_test!=depth_test) {

		depth_test=current_depth_test;
		if (depth_test)
			sceGuEnable(GU_DEPTH_TEST);
		else
			sceGuDisable(GU_DEPTH_TEST);
	}
}
/*
static const MaterialShaderGLES1::Conditionals _gl_light_version[4][3]={
	{MaterialShaderGLES1::LIGHT_0_DIRECTIONAL,MaterialShaderGLES1::LIGHT_0_OMNI,MaterialShaderGLES1::LIGHT_0_SPOT},
	{MaterialShaderGLES1::LIGHT_1_DIRECTIONAL,MaterialShaderGLES1::LIGHT_1_OMNI,MaterialShaderGLES1::LIGHT_1_SPOT},
	{MaterialShaderGLES1::LIGHT_2_DIRECTIONAL,MaterialShaderGLES1::LIGHT_2_OMNI,MaterialShaderGLES1::LIGHT_2_SPOT},
	{MaterialShaderGLES1::LIGHT_3_DIRECTIONAL,MaterialShaderGLES1::LIGHT_3_OMNI,MaterialShaderGLES1::LIGHT_3_SPOT}
};

static const MaterialShaderGLES1::Conditionals _gl_light_shadow[4]={
	MaterialShaderGLES1::LIGHT_0_SHADOW,
	MaterialShaderGLES1::LIGHT_1_SHADOW,
	MaterialShaderGLES1::LIGHT_2_SHADOW,
	MaterialShaderGLES1::LIGHT_3_SHADOW
};
*/


void RasterizerPSP::_setup_light(LightInstance* p_instance, int p_idx) {

	Light* ld = p_instance->base;

	int glid = GU_LIGHT0+p_idx;

	ERR_FAIL_COND(p_idx > MAX_HW_LIGHTS);

	Color diff_color = ld->colors[VS::LIGHT_COLOR_DIFFUSE];
	float emult = ld->vars[VS::LIGHT_PARAM_ENERGY];

	if (ld->type!=VS::LIGHT_DIRECTIONAL)
		emult*=4.0;

	// sceGuLightMode(GU_SEPARATE_SPECULAR_COLOR);
	
	switch(ld->type) {

		case VS::LIGHT_DIRECTIONAL: {

			sceGuLight(p_idx, GU_DIRECTIONAL, GU_DIFFUSE_AND_SPECULAR, gumake<ScePspFVector3>({
				p_instance->light_vector.x,
				p_instance->light_vector.y,
				p_instance->light_vector.z,
			}));

			sceGuLightAtt(p_idx, 1, 0, 0);

		} break;

		case VS::LIGHT_OMNI: {
			
			
			sceGuLight(p_idx, GU_POINTLIGHT, GU_DIFFUSE_AND_SPECULAR, gumake<ScePspFVector3>({
				p_instance->light_vector.x,
				p_instance->light_vector.y,
				p_instance->light_vector.z,
			})); //at modelview

			sceGuLightAtt(p_idx, 0, p_instance->linear_att, 0);

		} break;
		case VS::LIGHT_SPOT: {
			
			sceGuLight(p_idx, GU_SPOTLIGHT, GU_DIFFUSE_AND_SPECULAR, gumake<ScePspFVector3>({
				p_instance->light_vector.x,
				p_instance->light_vector.y,
				p_instance->light_vector.z,
			})); //at modelview

			sceGuLightSpot(p_idx, gumake<ScePspFVector3>({
					p_instance->spot_vector.x,
					p_instance->spot_vector.y,
					p_instance->spot_vector.z,
				}), 5.0, 1/ld->vars[VS::LIGHT_PARAM_SPOT_ANGLE]);

			sceGuLightAtt(p_idx, 0, p_instance->linear_att, 0);

		} break;

		default: return;
	}

	sceGuLightColor(p_idx, GU_DIFFUSE, MK_RGBA_F(
		diff_color.r*emult,
		diff_color.g*emult,
		diff_color.b*emult,
		1.0
	));

	sceGuLightColor(p_idx, GU_AMBIENT, MK_RGBA_F(0, 0, 0, 1));

	Color spec_color = ld->colors[VS::LIGHT_COLOR_SPECULAR];
	sceGuLightColor(p_idx, GU_SPECULAR, MK_RGBA_F(
		spec_color.r,
		spec_color.g,
		spec_color.b,
		1.0
	));
};





void RasterizerPSP::_setup_lights(const uint16_t * p_lights,int p_light_count) {

	if (shadow)
		return;
	
	// Color amb = Color(0,0,0,1);
	
	for (int i=directional_light_count; i<MAX_HW_LIGHTS; i++) {

		if (i<(directional_light_count+p_light_count)) {
			
			sceGuEnable(GU_LIGHT0 + i);
			_setup_light(light_instances[p_lights[i]], i);

		} else {

			sceGuDisable(GU_LIGHT0 + i);

		}
	}
	// sceGuAmbient(MK_RGBA_C(amb));
}


/*
static const int gl_client_states[] = {

	GL_VERTEX_ARRAY,
	GL_NORMAL_ARRAY,
	0, // ARRAY_TANGENT
	0,//GL_COLOR_ARRAY,
	GL_TEXTURE_COORD_ARRAY, // ARRAY_TEX_UV
	0,//GL_TEXTURE_COORD_ARRAY, // ARRAY_TEX_UV2
	0, // ARRAY_BONES
	0, // ARRAY_WEIGHTS
};

static const int gl_texcoord_index[VS::ARRAY_MAX-1] = {

	-1,
	-1,
	-1, // ARRAY_TANGENT
	-1,
	0, // ARRAY_TEX_UV
	-1,//1, // ARRAY_TEX_UV2
	-1, // ARRAY_BONES
	-1, // ARRAY_WEIGHTS
};*/


Error RasterizerPSP::_setup_geometry(const Geometry *p_geometry, const Material* p_material, const Skeleton3D *p_skeleton,const float *p_morphs) {
	switch(p_geometry->type) {

		case Geometry::GEOMETRY_MULTISURFACE:
		case Geometry::GEOMETRY_SURFACE: {



			const Surface *surf=NULL;
			if (p_geometry->type==Geometry::GEOMETRY_SURFACE)
				surf=static_cast<const Surface*>(p_geometry);
			else if (p_geometry->type==Geometry::GEOMETRY_MULTISURFACE)
				surf=static_cast<const MultiMeshSurface*>(p_geometry)->surface;


			if (surf->format != surf->configured_format) {
				if (OS::get_singleton()->is_stdout_verbose()) {

					print_line("has format: "+itos(surf->format));
					print_line("configured format: "+itos(surf->configured_format));
				}
				ERR_EXPLAIN("Missing arrays (not set) in surface");
			}
			ERR_FAIL_COND_V( surf->format != surf->configured_format, ERR_UNCONFIGURED );
			uint8_t *base=0;
			int stride=surf->stride;
			_setup_geometry_vinfo=surf->array_len;

			bool skeleton_valid = p_skeleton && (surf->format&VS::ARRAY_FORMAT_BONES) && (surf->format&VS::ARRAY_FORMAT_WEIGHTS) && !p_skeleton->bones.empty() && p_skeleton->bones.size() > surf->max_bone;


			base = surf->array_local;
			bool can_copy_to_local=surf->local_stride * surf->array_len <= skinned_buffer_size;
			if (!can_copy_to_local)
				skeleton_valid=false;

			for (int i=0;i<(VS::ARRAY_MAX-1);i++) {

				const Surface::ArrayData& ad=surf->array[i];

//				if (!gl_texcoord_shader[i])
//					continue;

				// TODO:XXX:
				if (ad.size==0 || i==VS::ARRAY_BONES || i==VS::ARRAY_WEIGHTS /*|| gl_client_states[i]==0*/ ) {

					//if (gl_texcoord_index[i] != -1) {
					//	glClientActiveTexture(GL_TEXTURE0+gl_texcoord_index[i]);
					//}

					//if (gl_client_states[i] != 0)
					//	glDisableClientState(gl_client_states[i]);

					//if (i == VS::ARRAY_COLOR) {
					//	glColor4f(last_color.r,last_color.g,last_color.b,last_color.a);
					//};
					continue; // this one is disabled.
				}

				//if (gl_texcoord_index[i] != -1) {
				//	glClientActiveTexture(GL_TEXTURE0+gl_texcoord_index[i]);
				//}

				//glEnableClientState(gl_client_states[i]);

				switch (i) {

				case VS::ARRAY_VERTEX: {

				} break;
				case VS::ARRAY_NORMAL: {

				} break;
				case VS::ARRAY_COLOR: {

				} break;
				case VS::ARRAY_TEX_UV:
				case VS::ARRAY_TEX_UV2: {

				} break;
				case VS::ARRAY_TANGENT: {

					//glVertexAttribPointer(i, 4, use_VBO?GL_BYTE:GL_FLOAT, use_VBO?GL_TRUE:GL_FALSE, stride, &base[ad.ofs]);

				} break;
				case VS::ARRAY_BONES: {

					// FIXME:XXX: this is wrong; see the GLES1 renderer; morph transforms are stored somewhere else
					if (skeleton_valid) {
						for (int k = 0; k < VS::ARRAY_WEIGHTS_SIZE; ++k) {
							const auto t = reinterpret_cast<const Transform3D *>(&base[ad.ofs])[k];
							auto *m = gumake<ScePspFMatrix4>({});
							gumLoadIdentity(m);
							const auto euler = t.basis.get_euler();
							gumRotateXYZ(m, reinterpret_cast<const ScePspFVector3 *>(&euler));
							const auto scale = t.basis.get_scale();
							gumScale(m, reinterpret_cast<const ScePspFVector3 *>(&scale));
							gumTranslate(m, reinterpret_cast<const ScePspFVector3 *>(&t.origin));

							sceGuBoneMatrix(k, m);
							sceGuMorphWeight(k, p_morphs[k]);
						}
					}

				} break;
				case VS::ARRAY_WEIGHTS: {

					//do none
					//glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, surf->stride, &base[ad.ofs]);

				} break;
				case VS::ARRAY_INDEX:
					ERR_PRINT("Bug");
					break;
				};
			}

			if (!skeleton_valid) {
				for (int k = 0; k < VS::ARRAY_WEIGHTS_SIZE; ++k) {
					auto *m = gumake<ScePspFMatrix4>({});
					gumLoadIdentity(m);

					sceGuBoneMatrix(k, m);
					sceGuMorphWeight(k, 1.f);
				}
			}

		} break;

		default: break;

	};

	return OK;
};
static const int gl_primitive[]={
	GU_POINTS,
	GU_LINES,
	GU_LINE_STRIP,
	-1, //GU_LINE_LOOP,
	GU_TRIANGLES,
	GU_TRIANGLE_STRIP,
	GU_TRIANGLE_FAN
};

static const int gl_poly_primitive[4]={
	GU_POINTS,
	GU_LINES,
	GU_TRIANGLES,
	//GL_QUADS

};


void RasterizerPSP::_render(const Geometry *p_geometry,const Material *p_material, const Skeleton3D* p_skeleton, const GeometryOwner *p_owner) {

	_rinfo.object_count++;

	switch(p_geometry->type) {
		case Geometry::GEOMETRY_SURFACE: {
			Surface *s = (Surface*)p_geometry;

			_rinfo.vertex_count+=s->array_len;

			const auto t = s->aabb.pos;
			sceGumTranslate(reinterpret_cast<const ScePspFVector3 *>(&t));
			sceGumScale(reinterpret_cast<const ScePspFVector3 *>(&s->aabb.size));

			//sceGumMatrixMode(GU_TEXTURE);
			//sceGumPushMatrix();
			//const Vector3 t2{-s->uv_bb.pos.x, -s->uv_bb.pos.y, .0f};
			//sceGumTranslate(reinterpret_cast<const ScePspFVector3 *>(&t2));
			//const Vector3 s2{s->uv_bb.size.width, s->uv_bb.size.height, .0f};
			//sceGumScale(reinterpret_cast<const ScePspFVector3 *>(&s2));

			sceGumDrawArray(gl_primitive[s->primitive], s->psp_vattribs|GU_INDEX_16BIT|GU_TRANSFORM_3D, s->index_array_len, s->index_array_local, s->psp_array_local);

			//sceGumPopMatrix();
			//sceGumMatrixMode(GU_MODEL);
		} break;

		case Geometry::GEOMETRY_MULTISURFACE: {

			Surface *s = static_cast<const MultiMeshSurface*>(p_geometry)->surface;
			const MultiMesh *mm = static_cast<const MultiMesh*>(p_owner);
			int element_count=mm->elements.size();

			if (element_count==0)
				return;

			const MultiMesh::Element *elements=&mm->elements[0];

			_rinfo.vertex_count+=s->array_len*element_count;
			for(int i=0;i<element_count;i++) {
				sceGumPopMatrix();
				sceGumPushMatrix();

				sceGumMultMatrix(reinterpret_cast<const ScePspFMatrix4 *>(elements[i].matrix));

				const auto t = s->aabb.pos;
				sceGumTranslate(reinterpret_cast<const ScePspFVector3 *>(&t));
				sceGumScale(reinterpret_cast<const ScePspFVector3 *>(&s->aabb.size));

				sceGumDrawArray(gl_primitive[s->primitive], s->psp_vattribs|GU_INDEX_16BIT|GU_TRANSFORM_3D, s->index_array_len, s->index_array_local, s->psp_array_local);
			}
		 } break;
		case Geometry::GEOMETRY_PARTICLES: {


			//print_line("particulinas");
			const Particles3D *particles = static_cast<const Particles3D*>( p_geometry );
			ERR_FAIL_COND(!p_owner);
			ParticlesInstance *particles_instance = (ParticlesInstance*)p_owner;

			ParticleSystemProcessSW &pp = particles_instance->particles_process;
			float td = time_delta; //MIN(time_delta,1.0/10.0);
			pp.process(&particles->data,particles_instance->transform,td);
			ERR_EXPLAIN("A parameter in the particle system is not correct.");
			ERR_FAIL_COND(!pp.valid);


			Transform3D camera;
			if (shadow)
				camera=shadow->transform;
			else
				camera=camera_transform;

			particle_draw_info.prepare(&particles->data,&pp,particles_instance->transform,camera);

			_rinfo.vertex_count+=4*particles->data.amount;

			{
				static const Vector3 points[4]={
					Vector3(-1.0,1.0,0),
					Vector3(1.0,1.0,0),
					Vector3(1.0,-1.0,0),
					Vector3(-1.0,-1.0,0)
				};
				static const Vector3 uvs[4]={
					Vector3(0.0,0.0,0.0),
					Vector3(1.0,0.0,0.0),
					Vector3(1.0,1.0,0.0),
					Vector3(0,1.0,0.0)
				};
				static const Vector3 normals[4]={
					Vector3(0,0,1),
					Vector3(0,0,1),
					Vector3(0,0,1),
					Vector3(0,0,1)
				};

				static const Plane tangents[4]={
					Plane(Vector3(1,0,0),0),
					Plane(Vector3(1,0,0),0),
					Plane(Vector3(1,0,0),0),
					Plane(Vector3(1,0,0),0)
				};


				sceGumMatrixMode(GU_MODEL);
				sceGumPushMatrix();
				_gl_load_transform(camera_transform_inverse);
				for(int i=0;i<particles->data.amount;i++) {

					ParticleSystemDrawInfoSW::ParticleDrawInfo &pinfo=*particle_draw_info.draw_info_order[i];
					if (!pinfo.data->active)
						continue;
					sceGumPushMatrix();
					_gl_mult_transform(pinfo.transform);

					sceGuColor(MK_RGBA(pinfo.color.r*last_color.r*255,pinfo.color.g*last_color.g*255,pinfo.color.b*last_color.b*255,pinfo.color.a*last_color.a*255));
					_draw_primitive(4,points,normals,NULL,uvs,tangents);
					sceGumPopMatrix();

				}
				sceGumPopMatrix();

			}

		} break;
		 default: break;
	};
};

void RasterizerPSP::_setup_shader_params(const Material *p_material) {

}

void RasterizerPSP::_render_list_forward(RenderList *p_render_list,bool p_reverse_cull) {
	const Material *prev_material=NULL;
	uint64_t prev_light_key=0;
	const Skeleton3D *prev_skeleton=NULL;
	const Geometry *prev_geometry=NULL;

	Geometry::Type prev_geometry_type=Geometry::GEOMETRY_INVALID;

	for (int i=0;i<p_render_list->element_count;i++) {

		RenderList::Element *e = p_render_list->elements[i];
		const Material *material = e->material;
		uint64_t light_key = e->light_key;
		const Skeleton3D *skeleton = e->skeleton;
		const Geometry *geometry = e->geometry;
		
		if (material!=prev_material || geometry->type!=prev_geometry_type) {
			_setup_material(e->geometry,material);
			_rinfo.mat_change_count++;
			//_setup_material_overrides(e->material,NULL,material_overrides);
			//_setup_material_skeleton(material,skeleton);
		} else {

			if (prev_skeleton!=skeleton) {
				//_setup_material_skeleton(material,skeleton);
			};
		}


		if (geometry!=prev_geometry || geometry->type!=prev_geometry_type  || prev_skeleton!=skeleton) {

			_setup_geometry(geometry, material,e->skeleton,e->instance->morph_values.ptr());
		};

		// if (i==0 || light_key!=prev_light_key)
			_setup_lights(e->lights,e->light_count);

		_set_cull(e->mirror,p_reverse_cull);

		sceGumMatrixMode(GU_MODEL);
		sceGumPopMatrix();
		sceGumPushMatrix();


		if (e->instance->billboard || e->instance->depth_scale) {

			Transform3D xf=e->instance->transform;
			if (e->instance->depth_scale) {

				if (camera_projection.matrix[3][3]) {
					//orthogonal matrix, try to do about the same
					//with viewport size
					//real_t w = Math::abs( 1.0/(2.0*(p_projection.matrix[0][0])) );
					real_t h = Math::abs( 1.0/(2.0*camera_projection.matrix[1][1]) );
					float sc = (h*2.0); //consistent with Y-fov
					xf.basis.scale( Vector3(sc,sc,sc));
				} else {
					//just scale by depth
					real_t sc = -camera_plane.distance_to(xf.origin);
					xf.basis.scale( Vector3(sc,sc,sc));
				}
			}

			if (e->instance->billboard) {

				Vector3 scale = xf.basis.get_scale();
				xf.set_look_at(xf.origin,xf.origin+camera_transform.get_basis().get_axis(2),camera_transform.get_basis().get_axis(1));
				xf.basis.scale(scale);
			}
			_gl_mult_transform(xf); // for fixed pipeline

		} else {
			_gl_mult_transform(e->instance->transform); // for fixed pipeline
		}

		//bool changed_shader = material_shader.bind();
		//if ( changed_shader && material->shader_cache && !material->shader_cache->params.empty())
		//	_setup_shader_params(material);

		_render(geometry, material, skeleton,e->owner);

		prev_material=material;
		prev_skeleton=skeleton;
		prev_geometry=geometry;
		prev_light_key=e->light_key;
		prev_geometry_type=geometry->type;
	}
};



void RasterizerPSP::end_scene() {
	sceGuEnable(GU_BLEND);
	sceGuDepthMask(GU_FALSE);
	// sceGuDepthRange(0,20000);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuDepthFunc(GU_LEQUAL);
	sceGuDisable(GU_SCISSOR_TEST);
	sceGuDisable(GU_FOG);
	depth_write=true;
	depth_test=true;

	sceGuLightMode(GU_SEPARATE_SPECULAR_COLOR);

	sceGuColor(MK_RGBA(255, 255, 255, 255));
	// sceGuAmbient(MK_RGBA(127, 127, 127, 255));
	//sceGuAmbientColor(MK_RGBA(127, 127, 127, 255));

	//sceGuAmbient(MK_RGBA(255, 255, 255, 255));
	
	sceGuAmbient(0x00222222);

	if (current_env) {

		switch(current_env->bg_mode) {

			case VS::ENV_BG_KEEP: {
				//copy from framebuffer if framebuffer
				// glClear(GL_DEPTH_BUFFER_BIT);
				sceGuClearDepth(0xFFFF);
				sceGuClear(GU_DEPTH_BUFFER_BIT);
			} break;
			case VS::ENV_BG_DEFAULT_COLOR:
			case VS::ENV_BG_COLOR: {
				//only in the runner
				if(VS::get_singleton()->get_editor() != true) {
					Color bgcolor;
					if (current_env->bg_mode==VS::ENV_BG_COLOR)
						bgcolor = current_env->bg_param[VS::ENV_BG_PARAM_COLOR];
					else
						bgcolor = Globals::get_singleton()->get("render/default_clear_color");

					// bgcolor = _convert_color(bgcolor);
					//sceGuAmbient(MK_RGBA_C(bgcolor));
					sceGuClearColor(MK_RGBA(bgcolor.r*255,bgcolor.g*255,bgcolor.b*255,255));
					sceGuClearDepth(0xFFFF);
					// glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
					sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
				} else {
					// glClear(GL_DEPTH_BUFFER_BIT);
					sceGuClearDepth(0xFFFF);
					sceGuClear(GU_DEPTH_BUFFER_BIT);
				}
			} break;
			case VS::ENV_BG_TEXTURE:
			case VS::ENV_BG_CUBEMAP:
			case VS::ENV_BG_TEXTURE_RGBE:
			case VS::ENV_BG_CUBEMAP_RGBE: {
				//a bit broken for now

				// glClear(GL_DEPTH_BUFFER_BIT);
				sceGuClearDepth(0xFFFF);
				sceGuClear(GU_DEPTH_BUFFER_BIT);
			} break;

		}
	} else {

		Color c = Color(0.3,0.3,0.3);
		sceGuClearColor(MK_RGBA_F(c.r,c.g,c.b,0.f));
		// glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		sceGuClearDepth(0xFFFF);
		sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	}
#ifdef GLES_OVER_GL
	//glClearDepth(1.0);
#else
	// glClearDepthf(1.0);
#endif
	// glClear(GL_DEPTH_BUFFER_BIT);

	if (current_env->fx_enabled[VS::ENV_FX_FOG]) {

		sceGuEnable(GU_FOG);
		Color col_end = current_env->fx_param[VS::ENV_FX_PARAM_FOG_END_COLOR];
		sceGuFog(current_env->fx_param[VS::ENV_FX_PARAM_FOG_BEGIN], camera_z_far, MK_RGBA_C(col_end));
		/*
		glEnable(GL_FOG);
		glFogf(GL_FOG_MODE,GL_LINEAR);
		glFogf(GL_FOG_DENSITY,scene_fx->fog_attenuation);
		glFogf(GL_FOG_START,scene_fx->fog_near);
		glFogf(GL_FOG_END,scene_fx->fog_far);
		glFogfv(GL_FOG_COLOR,scene_fx->fog_color_far.components);
		glLightfv(GL_LIGHT5,GL_DIFFUSE,scene_fx->fog_color_near.components);

		material_shader.set_conditional( MaterialShaderGLES1::USE_FOG,true);
		*/
	}

	for(int i=0;i<directional_light_count;i++) {

		sceGuEnable(GU_LIGHT0+i);
		_setup_light(directional_lights[i],i);
	}

	opaque_render_list.sort_mat_light();

	//material_shader.set_uniform_camera(MaterialShaderGLES1::PROJECTION_MATRIX, camera_projection);

	/*
	printf("setting projection to ");
	for (int i=0; i<16; i++) {
		printf("%f, ", ((float*)camera_projection.matrix)[i]);
	};
	printf("\n");

	print_line(String("setting camera to ")+camera_transform_inverse);
	*/
//	material_shader.set_uniform_default(MaterialShaderGLES1::CAMERA_INVERSE, camera_transform_inverse);

	//projection
	//glEnable(GL_RESCALE_NORMAL);
	// glEnable(GL_NORMALIZE);

	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadMatrix((const ScePspFMatrix4 *)&camera_projection.matrix[0][0]);
	// sceGumPerspective(70 + 1.0f, 480*272, 6, 4096);

	sceGumMatrixMode(GU_VIEW);
	_gl_load_transform(camera_transform_inverse);
	//sceGumPushMatrix();

	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();
	sceGumPushMatrix();

	sceGuDisable(GU_BLEND);

	blend_mode=VS::MATERIAL_BLEND_MODE_MIX;
	lighting=true;
	sceGuEnable(GU_LIGHTING);
	// glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	_render_list_forward(&opaque_render_list);

	alpha_render_list.sort_z();
	sceGuEnable(GU_BLEND);

	_render_list_forward(&alpha_render_list);

	//sceGumMatrixMode(GU_VIEW);
	//sceGumPopMatrix();
	sceGumMatrixMode(GU_MODEL);
	sceGumPopMatrix();


//	material_shader.set_conditional( MaterialShaderGLES1::USE_FOG,false);

	_debug_shadows();
}
void RasterizerPSP::end_shadow_map() {
}

void RasterizerPSP::_debug_draw_shadow(ShadowBuffer *p_buffer, const Rect2& p_rect) {

}

void RasterizerPSP::_debug_draw_shadows_type(Vector<ShadowBuffer>& p_shadows,Point2& ofs) {


//	Size2 debug_size(128,128);
	Size2 debug_size(512,512);

	for (int i=0;i<p_shadows.size();i++) {

		ShadowBuffer *sb=&p_shadows[i];

		if (!sb->owner)
			continue;

		if (sb->owner->base->type==VS::LIGHT_DIRECTIONAL) {

			if (sb->owner->shadow_pass!=scene_pass-1)
				continue;
		} else {

			if (sb->owner->shadow_pass!=frame)
				continue;
		}
		_debug_draw_shadow(sb, Rect2( ofs, debug_size ));
		ofs.x+=debug_size.x;
		if ( (ofs.x+debug_size.x) > viewport.width ) {

			ofs.x=0;
			ofs.y+=debug_size.y;
		}
	}

}


void RasterizerPSP::_debug_shadows() {

	return;
#if 0
	canvas_begin();
	glUseProgram(0);
	glDisable(GL_BLEND);
	Size2 ofs;

	/*
	for(int i=0;i<16;i++) {
		glActiveTexture(GL_TEXTURE0+i);
		//glDisable(GL_TEXTURE_2D);
	}
	glActiveTexture(GL_TEXTURE0);
	//glEnable(GL_TEXTURE_2D);
	*/


	_debug_draw_shadows_type(near_shadow_buffers,ofs);
	_debug_draw_shadows_type(far_shadow_buffers,ofs);
#endif
}

void RasterizerPSP::end_frame() {

	/*
	if (framebuffer.active) {

		canvas_begin(); //resets stuff and goes back to fixedpipe
		glBindFramebuffer(GL_FRAMEBUFFER,0);

		//copy to main bufferz
		glEnable(GL_TEXTURE_2D);
		
		glBindTexture(GL_TEXTURE_2D,framebuffer.color);		
		glBegin(GL_QUADS);
		glTexCoord2f(0,0);
		glVertex2f(-1,-1);
		glTexCoord2f(0,1);
		glVertex2f(-1,+1);
		glTexCoord2f(1,1);
		glVertex2f(+1,+1);
		glTexCoord2f(1,0);
		glVertex2f(+1,-1);
		glEnd();


	}
	*/

	//print_line("VTX: "+itos(_rinfo.vertex_count)+" OBJ: "+itos(_rinfo.object_count)+" MAT: "+itos(_rinfo.mat_change_count)+" SHD: "+itos(_rinfo.shader_change_count));

	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuSwapBuffers();
	// printf("end_frame done\n");
}

/* CANVAS API */


void RasterizerPSP::reset_state() {


	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0); //unbind
	// glBindBuffer(GL_ARRAY_BUFFER,0);
	

	// glActiveTexture(GL_TEXTURE0);
	// glClientActiveTexture(GL_TEXTURE0);
	// glMatrixMode(GL_TEXTURE);
	sceGumMatrixMode(GU_TEXTURE);
	sceGumLoadIdentity();
	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadIdentity();
	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();
	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();
	sceGuColor(MK_RGBA_F(1,1,1,1));

	// glDisable(GL_CULL_FACE);
	// glDisable(GL_DEPTH_TEST);
	// glEnable(GL_BLEND);
	//sceGuDisable(GU_FOG);
	sceGuDisable(GU_CULL_FACE);
	sceGuDisable(GU_DEPTH_TEST);
	sceGuEnable(GU_BLEND);
//	glBlendEquation(GL_FUNC_ADD);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
//	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	canvas_blend=VS::MATERIAL_BLEND_MODE_MIX;
	// glLineWidth(1.0);
	// glDisable(GL_LIGHTING);
	sceGuEnable(GU_LIGHTING);

}

_FORCE_INLINE_ static void _set_glcoloro(const Color& p_color,const float p_opac) {

	const auto color = MK_RGBA_F(p_color.r,p_color.g,p_color.b,p_color.a*p_opac);
	sceGuColor(color);
	//sceGuAmbientColor(color);
	//sceGuAmbient(color);
}


void RasterizerPSP::canvas_begin() {
	reset_state();
	canvas_opacity=1.0;
	sceGuEnable(GU_BLEND);
}

void RasterizerPSP::canvas_disable_blending() {
	sceGuDisable(GU_BLEND);
}

void RasterizerPSP::canvas_set_opacity(float p_opacity) {

	canvas_opacity = p_opacity;
}

void RasterizerPSP::canvas_set_blend_mode(VS::MaterialBlendMode p_mode) {

	switch(p_mode) {

		 case VS::MATERIAL_BLEND_MODE_MIX: {
			//glBlendEquation(GL_FUNC_ADD);
			// glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA);

		 } break;
		 case VS::MATERIAL_BLEND_MODE_ADD: {

			//glBlendEquation(GL_FUNC_ADD);
			// glBlendFunc(GL_SRC_ALPHA,GL_ONE);
			sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_DST_ALPHA, GU_SRC_ALPHA, GU_DST_ALPHA);

		 } break;
		 case VS::MATERIAL_BLEND_MODE_SUB: {

			//glBlendEquation(GL_FUNC_SUBTRACT);
			// glBlendFunc(GL_SRC_ALPHA,GL_ONE);
			sceGuBlendFunc(GU_SUBTRACT, GU_SRC_ALPHA, GU_DST_ALPHA, GU_SRC_ALPHA, GU_DST_ALPHA);
		 } break;
		case VS::MATERIAL_BLEND_MODE_MUL: {
			//glBlendEquation(GL_FUNC_ADD);
			// glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA);

		} break;

	}

}


void RasterizerPSP::canvas_begin_rect(const Transform2D& p_transform) {
	// glMatrixMode(GL_MODELVIEW);
	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();
	sceGumScale(gumake<ScePspFVector3>({2.f / viewport.width, -2.f / viewport.height, 0}));
	sceGumTranslate(gumake<ScePspFVector3>({(-(viewport.width / 2.f)), (-(viewport.height / 2.f)), 0}));
	_gl_mult_transform(p_transform);

	sceGumPushMatrix();
}

void RasterizerPSP::canvas_set_clip(bool p_clip, const Rect2& p_rect) {

	if (p_clip) {

		sceGuEnable(GU_SCISSOR_TEST);
	//	glScissor(viewport.x+p_rect.pos.x,viewport.y+ (viewport.height-(p_rect.pos.y+p_rect.size.height)),
		//p_rect.size.width,p_rect.size.height);
		//glScissor(p_rect.pos.x,(viewport.height-(p_rect.pos.y+p_rect.size.height)),p_rect.size.width,p_rect.size.height);
		sceGuScissor(viewport.x+p_rect.pos.x,viewport.y+ (window_size.y-(p_rect.pos.y+p_rect.size.height)), p_rect.size.width,p_rect.size.height);
	} else {

		sceGuDisable(GU_SCISSOR_TEST);
	}


}

void RasterizerPSP::canvas_end_rect() {
	sceGumPopMatrix();
}

void RasterizerPSP::canvas_draw_line(const Point2& p_from, const Point2& p_to,const Color& p_color,float p_width) {

	sceGuEnable(GU_TEXTURE_2D);
	_set_glcoloro( p_color,canvas_opacity );

	Vector3 verts[2]={
		Vector3(p_from.x,p_from.y,0),
		Vector3(p_to.x,p_to.y,0)
	};
	Color colors[2]={
		p_color,
		p_color
	};
	colors[0].a*=canvas_opacity;
	colors[1].a*=canvas_opacity;
	// glLineWidth(p_width);
	_draw_primitive(2,verts,0,colors,0);

}

static void _draw_textured_quad(const Rect2& p_rect, const Rect2& p_src_region, const Size2& p_tex_size,bool p_flip_h=false,bool p_flip_v=false ) {
	Vector3 texcoords[4]= {
		Vector3( p_src_region.pos.x/p_tex_size.width,
		p_src_region.pos.y/p_tex_size.height, 0),

		Vector3((p_src_region.pos.x+p_src_region.size.width)/p_tex_size.width,
		p_src_region.pos.y/p_tex_size.height, 0),

		Vector3( (p_src_region.pos.x+p_src_region.size.width)/p_tex_size.width,
		(p_src_region.pos.y+p_src_region.size.height)/p_tex_size.height, 0),

		Vector3( p_src_region.pos.x/p_tex_size.width,
		(p_src_region.pos.y+p_src_region.size.height)/p_tex_size.height, 0)
	};


	if (p_flip_h) {
		SWAP( texcoords[0], texcoords[1] );
		SWAP( texcoords[2], texcoords[3] );
	}
	if (p_flip_v) {
		SWAP( texcoords[1], texcoords[2] );
		SWAP( texcoords[0], texcoords[3] );
	}

	Vector3 coords[4]= {
		Vector3( p_rect.pos.x, p_rect.pos.y, 0 ),
		Vector3( p_rect.pos.x+p_rect.size.width, p_rect.pos.y, 0 ),
		Vector3( p_rect.pos.x+p_rect.size.width, p_rect.pos.y+p_rect.size.height, 0 ),
		Vector3( p_rect.pos.x,p_rect.pos.y+p_rect.size.height, 0 )
	};

	_draw_primitive(4,coords,0,0,texcoords);
}

static void _draw_quad(const Rect2& p_rect) {
	Vector3 coords[4]= {
		Vector3( p_rect.pos.x,p_rect.pos.y, 0 ),
		Vector3( p_rect.pos.x+p_rect.size.width,p_rect.pos.y, 0 ),
		Vector3( p_rect.pos.x+p_rect.size.width,p_rect.pos.y+p_rect.size.height, 0 ),
		Vector3( p_rect.pos.x,p_rect.pos.y+p_rect.size.height, 0 )
	};

	_draw_primitive(4,coords,0,0,0);
}


void RasterizerPSP::canvas_draw_rect(const Rect2& p_rect, int p_flags, const Rect2& p_source,RID p_texture,const Color& p_modulate) {

	_set_glcoloro( p_modulate,canvas_opacity );

	if ( p_texture.is_valid() ) {

		Texture *texture = texture_owner.get( p_texture );
		ERR_FAIL_COND(!texture);

		_setup_texture(texture);

		if (!(p_flags&CANVAS_RECT_REGION)) {

			Rect2 region = Rect2(0,0,texture->width,texture->height);
			_draw_textured_quad(p_rect,region,region.size,p_flags&CANVAS_RECT_FLIP_H,p_flags&CANVAS_RECT_FLIP_V);

		} else {


			_draw_textured_quad(p_rect, p_source, Size2(texture->width,texture->height),p_flags&CANVAS_RECT_FLIP_H,p_flags&CANVAS_RECT_FLIP_V );

		}
		
	} else {

		WARN_PRINT("p_texture is not valid");

		sceGuDisable(GU_TEXTURE_2D);
		_draw_quad( p_rect );

	}


}
void RasterizerPSP::canvas_draw_style_box(const Rect2& p_rect, RID p_texture,const float *p_margin, bool p_draw_center,const Color& p_modulate) {

	_set_glcoloro( p_modulate,canvas_opacity );


	Texture *texture = texture_owner.get( p_texture );
	ERR_FAIL_COND(!texture);

	_setup_texture(texture);

	/* CORNERS */

	_draw_textured_quad( // top left
		Rect2( p_rect.pos, Size2(p_margin[MARGIN_LEFT],p_margin[MARGIN_TOP])),
		Rect2( Point2(), Size2(p_margin[MARGIN_LEFT],p_margin[MARGIN_TOP])),
		Size2( texture->width, texture->height ) );

	_draw_textured_quad( // top right
		Rect2( Point2( p_rect.pos.x + p_rect.size.width - p_margin[MARGIN_RIGHT], p_rect.pos.y), Size2(p_margin[MARGIN_RIGHT],p_margin[MARGIN_TOP])),
		Rect2( Point2(texture->width-p_margin[MARGIN_RIGHT],0), Size2(p_margin[MARGIN_RIGHT],p_margin[MARGIN_TOP])),
		Size2( texture->width, texture->height ) );


	_draw_textured_quad( // bottom left
		Rect2( Point2(p_rect.pos.x,p_rect.pos.y + p_rect.size.height - p_margin[MARGIN_BOTTOM]), Size2(p_margin[MARGIN_LEFT],p_margin[MARGIN_BOTTOM])),
		Rect2( Point2(0,texture->height-p_margin[MARGIN_BOTTOM]), Size2(p_margin[MARGIN_LEFT],p_margin[MARGIN_BOTTOM])),
		Size2( texture->width, texture->height ) );

	_draw_textured_quad( // bottom right
		Rect2( Point2( p_rect.pos.x + p_rect.size.width - p_margin[MARGIN_RIGHT], p_rect.pos.y + p_rect.size.height - p_margin[MARGIN_BOTTOM]), Size2(p_margin[MARGIN_RIGHT],p_margin[MARGIN_BOTTOM])),
		Rect2( Point2(texture->width-p_margin[MARGIN_RIGHT],texture->height-p_margin[MARGIN_BOTTOM]), Size2(p_margin[MARGIN_RIGHT],p_margin[MARGIN_BOTTOM])),
		Size2( texture->width, texture->height ) );

	Rect2 rect_center( p_rect.pos+Point2( p_margin[MARGIN_LEFT], p_margin[MARGIN_TOP]), Size2( p_rect.size.width - p_margin[MARGIN_LEFT] - p_margin[MARGIN_RIGHT], p_rect.size.height - p_margin[MARGIN_TOP] - p_margin[MARGIN_BOTTOM] ));

	Rect2 src_center( Point2( p_margin[MARGIN_LEFT], p_margin[MARGIN_TOP]), Size2( texture->width - p_margin[MARGIN_LEFT] - p_margin[MARGIN_RIGHT], texture->height - p_margin[MARGIN_TOP] - p_margin[MARGIN_BOTTOM] ));


	_draw_textured_quad( // top
		Rect2( Point2(rect_center.pos.x,p_rect.pos.y),Size2(rect_center.size.width,p_margin[MARGIN_TOP])),
		Rect2( Point2(p_margin[MARGIN_LEFT],0), Size2(src_center.size.width,p_margin[MARGIN_TOP])),
		Size2( texture->width, texture->height ) );

	_draw_textured_quad( // bottom
		Rect2( Point2(rect_center.pos.x,rect_center.pos.y+rect_center.size.height),Size2(rect_center.size.width,p_margin[MARGIN_BOTTOM])),
		Rect2( Point2(p_margin[MARGIN_LEFT],src_center.pos.y+src_center.size.height), Size2(src_center.size.width,p_margin[MARGIN_BOTTOM])),
		Size2( texture->width, texture->height ) );

	_draw_textured_quad( // left
		Rect2( Point2(p_rect.pos.x,rect_center.pos.y),Size2(p_margin[MARGIN_LEFT],rect_center.size.height)),
		Rect2( Point2(0,p_margin[MARGIN_TOP]), Size2(p_margin[MARGIN_LEFT],src_center.size.height)),
		Size2( texture->width, texture->height ) );

	_draw_textured_quad( // right
		Rect2( Point2(rect_center.pos.x+rect_center.size.width,rect_center.pos.y),Size2(p_margin[MARGIN_RIGHT],rect_center.size.height)),
		Rect2( Point2(src_center.pos.x+src_center.size.width,p_margin[MARGIN_TOP]), Size2(p_margin[MARGIN_RIGHT],src_center.size.height)),
		Size2( texture->width, texture->height ) );

	if (p_draw_center) {

		_draw_textured_quad(
			rect_center,
			src_center,
			Size2( texture->width, texture->height ));
	}

}
void RasterizerPSP::canvas_draw_primitive(const Vector<Point2>& p_points, const Vector<Color>& p_colors,const Vector<Point2>& p_uvs, RID p_texture,float p_width) {

	ERR_FAIL_COND(p_points.size()<1);
	Vector3 verts[4];
	Vector3 uvs[4];

	_set_glcoloro( Color(1,1,1),canvas_opacity );

	for(int i=0;i<p_points.size();i++) {

		verts[i]=Vector3(p_points[i].x,p_points[i].y,0);
	}

	for(int i=0;i<p_uvs.size();i++) {

		uvs[i]=Vector3(p_uvs[i].x,p_uvs[i].y,0);
	}

	if (p_texture.is_valid()) {
		_setup_texture(texture_owner.get( p_texture ));
	}

	// glLineWidth(p_width);
	_draw_primitive(p_points.size(),&verts[0],NULL,p_colors.size()?&p_colors[0]:NULL,p_uvs.size()?uvs:NULL);

}

static const int _max_draw_poly_indices = 8*1024;
static uint16_t _draw_poly_indices[_max_draw_poly_indices];
static float _verts3[_max_draw_poly_indices];

void RasterizerPSP::canvas_draw_polygon(int p_vertex_count, const int* p_indices, const Vector2* p_vertices, const Vector2* p_uvs, const Color* p_colors,const RID& p_texture,bool p_singlecolor) {

	bool do_colors=false;
	//TODO: Draw polygons
/*
	//reset_state();
	if (p_singlecolor) {
		Color m = *p_colors;
		m.a*=canvas_opacity;
		glColor4f(m.r, m.g, m.b, m.a);
	} else if (!p_colors) {
		glColor4f(1, 1, 1, canvas_opacity);
	} else
		do_colors=true;

	glColor4f(1, 1, 1, 1);

	Texture* texture = NULL;
	if (p_texture.is_valid()) {
		glEnable(GL_TEXTURE_2D);
		texture = texture_owner.get( p_texture );
		if (texture) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture( GL_TEXTURE_2D,texture->tex_id );
		}
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, (GLvoid*)p_vertices);
	if (do_colors) {

		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4,GL_FLOAT, 0, p_colors);

	} else {
		glDisableClientState(GL_COLOR_ARRAY);
	}

	if (texture && p_uvs) {

		glClientActiveTexture(GL_TEXTURE0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, p_uvs);

	} else {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	if (p_indices) {

		for (int i=0; i<p_vertex_count; i++) {
			_draw_poly_indices[i] = p_indices[i];
		};
		glDrawElements(GL_TRIANGLES, p_vertex_count, GL_UNSIGNED_SHORT, _draw_poly_indices );
	} else {

		glDrawArrays(GL_TRIANGLES,0,p_vertex_count);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
*/
}

void RasterizerPSP::canvas_set_transform(const Transform2D& p_transform) {

	//restore
	sceGumPopMatrix();
	sceGumPushMatrix();
	//set
	_gl_mult_transform(p_transform);
}

/* FX */

RID RasterizerPSP::fx_create() {

	FX *fx = memnew( FX );
	ERR_FAIL_COND_V(!fx,RID());
	return fx_owner.make_rid(fx);

}
void RasterizerPSP::fx_get_effects(RID p_fx,List<String> *p_effects) const {

	FX *fx = fx_owner.get(p_fx);
	ERR_FAIL_COND(!fx);

	p_effects->clear();
	p_effects->push_back("bgcolor");
	p_effects->push_back("skybox");
	p_effects->push_back("antialias");
	//p_effects->push_back("hdr");
	p_effects->push_back("glow");	// glow has a bloom parameter, too
	p_effects->push_back("ssao");
	p_effects->push_back("fog");
	p_effects->push_back("dof_blur");
	p_effects->push_back("toon");
	p_effects->push_back("edge");

}
void RasterizerPSP::fx_set_active(RID p_fx,const String& p_effect, bool p_active) {

	FX *fx = fx_owner.get(p_fx);
	ERR_FAIL_COND(!fx);

	if (p_effect=="bgcolor")
		fx->bgcolor_active=p_active;
	else if (p_effect=="skybox")
		fx->skybox_active=p_active;
	else if (p_effect=="antialias")
		fx->antialias_active=p_active;
	else if (p_effect=="glow")
		fx->glow_active=p_active;
	else if (p_effect=="ssao")
		fx->ssao_active=p_active;
	else if (p_effect=="fog")
		fx->fog_active=p_active;
//	else if (p_effect=="dof_blur")
//		fx->dof_blur_active=p_active;
	else if (p_effect=="toon")
		fx->toon_active=p_active;
	else if (p_effect=="edge")
		fx->edge_active=p_active;
}
bool RasterizerPSP::fx_is_active(RID p_fx,const String& p_effect) const {

	FX *fx = fx_owner.get(p_fx);
	ERR_FAIL_COND_V(!fx,false);

	if (p_effect=="bgcolor")
		return fx->bgcolor_active;
	else if (p_effect=="skybox")
		return fx->skybox_active;
	else if (p_effect=="antialias")
		return fx->antialias_active;
	else if (p_effect=="glow")
		return fx->glow_active;
	else if (p_effect=="ssao")
		return fx->ssao_active;
	else if (p_effect=="fog")
		return fx->fog_active;
	//else if (p_effect=="dof_blur")
	//	return fx->dof_blur_active;
	else if (p_effect=="toon")
		return fx->toon_active;
	else if (p_effect=="edge")
		return fx->edge_active;

	return false;
}
void RasterizerPSP::fx_get_effect_params(RID p_fx,const String& p_effect,List<PropertyInfo> *p_params) const {

	FX *fx = fx_owner.get(p_fx);
	ERR_FAIL_COND(!fx);


	if (p_effect=="bgcolor") {

		p_params->push_back( PropertyInfo( Variant::COLOR, "color" ) );
	} else if (p_effect=="skybox") {
		p_params->push_back( PropertyInfo( Variant::_RID, "cubemap" ) );
	} else if (p_effect=="antialias") {

		p_params->push_back( PropertyInfo( Variant::REAL, "tolerance", PROPERTY_HINT_RANGE,"1,128,1" ) );

	} else if (p_effect=="glow") {

		p_params->push_back( PropertyInfo( Variant::INT, "passes", PROPERTY_HINT_RANGE,"1,4,1" ) );
		p_params->push_back( PropertyInfo( Variant::REAL, "attenuation", PROPERTY_HINT_RANGE,"0.01,8.0,0.01" ) );
		p_params->push_back( PropertyInfo( Variant::REAL, "bloom", PROPERTY_HINT_RANGE,"-1.0,1.0,0.01" ) );

	} else if (p_effect=="ssao") {

		p_params->push_back( PropertyInfo( Variant::REAL, "radius", PROPERTY_HINT_RANGE,"0.0,16.0,0.01" ) );
		p_params->push_back( PropertyInfo( Variant::REAL, "max_distance", PROPERTY_HINT_RANGE,"0.0,256.0,0.01" ) );
		p_params->push_back( PropertyInfo( Variant::REAL, "range_max", PROPERTY_HINT_RANGE,"0.0,1.0,0.01" ) );
		p_params->push_back( PropertyInfo( Variant::REAL, "range_min", PROPERTY_HINT_RANGE,"0.0,1.0,0.01" ) );
		p_params->push_back( PropertyInfo( Variant::REAL, "attenuation", PROPERTY_HINT_RANGE,"0.0,8.0,0.01" ) );

	} else if (p_effect=="fog") {

		p_params->push_back( PropertyInfo( Variant::REAL, "begin", PROPERTY_HINT_RANGE,"0.0,8192,0.01" ) );
		p_params->push_back( PropertyInfo( Variant::REAL, "end", PROPERTY_HINT_RANGE,"0.0,8192,0.01" ) );
		p_params->push_back( PropertyInfo( Variant::REAL, "attenuation", PROPERTY_HINT_RANGE,"0.0,8.0,0.01" ) );
		p_params->push_back( PropertyInfo( Variant::COLOR, "color_begin" ) );
		p_params->push_back( PropertyInfo( Variant::COLOR, "color_end" ) );
		p_params->push_back( PropertyInfo( Variant::BOOL, "fog_bg" ) );

//	} else if (p_effect=="dof_blur") {
//		return fx->dof_blur_active;
	} else if (p_effect=="toon") {
		p_params->push_back( PropertyInfo( Variant::REAL, "treshold", PROPERTY_HINT_RANGE,"0.0,1.0,0.01" ) );
		p_params->push_back( PropertyInfo( Variant::REAL, "soft", PROPERTY_HINT_RANGE,"0.001,1.0,0.001" ) );
	} else if (p_effect=="edge") {

	}
}
Variant RasterizerPSP::fx_get_effect_param(RID p_fx,const String& p_effect,const String& p_param) const {

	FX *fx = fx_owner.get(p_fx);
	ERR_FAIL_COND_V(!fx,Variant());

	if (p_effect=="bgcolor") {

		if (p_param=="color")
			return fx->bgcolor;
	} else if (p_effect=="skybox") {
		if (p_param=="cubemap")
			return fx->skybox_cubemap;
	} else if (p_effect=="antialias") {

		if (p_param=="tolerance")
			return fx->antialias_tolerance;

	} else if (p_effect=="glow") {

		if (p_param=="passes")
			return fx->glow_passes;
		if (p_param=="attenuation")
			return fx->glow_attenuation;
		if (p_param=="bloom")
			return fx->glow_bloom;

	} else if (p_effect=="ssao") {

		if (p_param=="attenuation")
			return fx->ssao_attenuation;
		if (p_param=="max_distance")
			return fx->ssao_max_distance;
		if (p_param=="range_max")
			return fx->ssao_range_max;
		if (p_param=="range_min")
			return fx->ssao_range_min;
		if (p_param=="radius")
			return fx->ssao_radius;

	} else if (p_effect=="fog") {

		if (p_param=="begin")
			return fx->fog_near;
		if (p_param=="end")
			return fx->fog_far;
		if (p_param=="attenuation")
			return fx->fog_attenuation;
		if (p_param=="color_begin")
			return fx->fog_color_near;
		if (p_param=="color_end")
			return fx->fog_color_far;
		if (p_param=="fog_bg")
			return fx->fog_bg;
//	} else if (p_effect=="dof_blur") {
//		return fx->dof_blur_active;
	} else if (p_effect=="toon") {
		if (p_param=="treshold")
			return fx->toon_treshold;
		if (p_param=="soft")
			return fx->toon_soft;

	} else if (p_effect=="edge") {

	}
	return Variant();
}
void RasterizerPSP::fx_set_effect_param(RID p_fx,const String& p_effect, const String& p_param, const Variant& p_value) {

	FX *fx = fx_owner.get(p_fx);
	ERR_FAIL_COND(!fx);

	if (p_effect=="bgcolor") {

		if (p_param=="color")
			fx->bgcolor=p_value;
	} else if (p_effect=="skybox") {
		if (p_param=="cubemap")
			fx->skybox_cubemap=p_value;

	} else if (p_effect=="antialias") {

		if (p_param=="tolerance")
			fx->antialias_tolerance=p_value;

	} else if (p_effect=="glow") {

		if (p_param=="passes")
			fx->glow_passes=p_value;
		if (p_param=="attenuation")
			fx->glow_attenuation=p_value;
		if (p_param=="bloom")
			fx->glow_bloom=p_value;

	} else if (p_effect=="ssao") {

		if (p_param=="attenuation")
			fx->ssao_attenuation=p_value;
		if (p_param=="radius")
			fx->ssao_radius=p_value;
		if (p_param=="max_distance")
			fx->ssao_max_distance=p_value;
		if (p_param=="range_max")
			fx->ssao_range_max=p_value;
		if (p_param=="range_min")
			fx->ssao_range_min=p_value;

	} else if (p_effect=="fog") {

		if (p_param=="begin")
			fx->fog_near=p_value;
		if (p_param=="end")
			fx->fog_far=p_value;
		if (p_param=="attenuation")
			fx->fog_attenuation=p_value;
		if (p_param=="color_begin")
			fx->fog_color_near=p_value;
		if (p_param=="color_end")
			fx->fog_color_far=p_value;
		if (p_param=="fog_bg")
			fx->fog_bg=p_value;
//	} else if (p_effect=="dof_blur") {
//		fx->dof_blur_active=p_value;
	} else if (p_effect=="toon") {

		if (p_param=="treshold")
			fx->toon_treshold=p_value;
		if (p_param=="soft")
			fx->toon_soft=p_value;

	} else if (p_effect=="edge") {

	}

}

/* ENVIRONMENT */

RID RasterizerPSP::environment_create() {

	Environment * env = memnew( Environment );
	return environment_owner.make_rid(env);
}

void RasterizerPSP::environment_set_background(RID p_env,VS::EnvironmentBG p_bg) {

	ERR_FAIL_INDEX(p_bg,VS::ENV_BG_MAX);
	Environment * env = environment_owner.get(p_env);
	ERR_FAIL_COND(!env);
	env->bg_mode=p_bg;
}

VS::EnvironmentBG RasterizerPSP::environment_get_background(RID p_env) const{

	const Environment * env = environment_owner.get(p_env);
	ERR_FAIL_COND_V(!env,VS::ENV_BG_MAX);
	return env->bg_mode;
}

void RasterizerPSP::environment_set_background_param(RID p_env,VS::EnvironmentBGParam p_param, const Variant& p_value){

	ERR_FAIL_INDEX(p_param,VS::ENV_BG_PARAM_MAX);
	Environment * env = environment_owner.get(p_env);
	ERR_FAIL_COND(!env);
	env->bg_param[p_param]=p_value;

}
Variant RasterizerPSP::environment_get_background_param(RID p_env,VS::EnvironmentBGParam p_param) const{

	ERR_FAIL_INDEX_V(p_param,VS::ENV_BG_PARAM_MAX,Variant());
	const Environment * env = environment_owner.get(p_env);
	ERR_FAIL_COND_V(!env,Variant());
	return env->bg_param[p_param];

}
void RasterizerPSP::environment_set_group(RID p_env,VS::Group p_param, const Variant& p_value){

	ERR_FAIL_INDEX(p_param,VS::ENV_GROUP_MAX);
	Environment * env = environment_owner.get(p_env);
	ERR_FAIL_COND(!env);
	env->group[p_param]=p_value;

}
Variant RasterizerPSP::environment_get_group(RID p_env,VS::Group p_param) const{

	ERR_FAIL_INDEX_V(p_param,VS::ENV_GROUP_MAX,Variant());
	const Environment * env = environment_owner.get(p_env);
	ERR_FAIL_COND_V(!env,Variant());
	return env->group[p_param];

}
void RasterizerPSP::environment_set_enable_fx(RID p_env,VS::EnvironmentFx p_effect,bool p_enabled){

	ERR_FAIL_INDEX(p_effect,VS::ENV_FX_MAX);
	Environment * env = environment_owner.get(p_env);
	ERR_FAIL_COND(!env);
	env->fx_enabled[p_effect]=p_enabled;
}
bool RasterizerPSP::environment_is_fx_enabled(RID p_env,VS::EnvironmentFx p_effect) const{

	ERR_FAIL_INDEX_V(p_effect,VS::ENV_FX_MAX,false);
	const Environment * env = environment_owner.get(p_env);
	ERR_FAIL_COND_V(!env,false);
	return env->fx_enabled[p_effect];

}

void RasterizerPSP::environment_fx_set_param(RID p_env,VS::EnvironmentFxParam p_param,const Variant& p_value){

	ERR_FAIL_INDEX(p_param,VS::ENV_FX_PARAM_MAX);
	Environment * env = environment_owner.get(p_env);
	ERR_FAIL_COND(!env);
	env->fx_param[p_param]=p_value;
}
Variant RasterizerPSP::environment_fx_get_param(RID p_env,VS::EnvironmentFxParam p_param) const{

	ERR_FAIL_INDEX_V(p_param,VS::ENV_FX_PARAM_MAX,Variant());
	const Environment * env = environment_owner.get(p_env);
	ERR_FAIL_COND_V(!env,Variant());
	return env->fx_param[p_param];

}

/* SAMPLED LIGHT */

RID RasterizerPSP::sampled_light_dp_create(int p_width,int p_height) {

	return sampled_light_owner.make_rid(memnew(SampledLight));
}

void RasterizerPSP::sampled_light_dp_update(RID p_sampled_light, const Color *p_data, float p_multiplier) {


}

/*MISC*/

bool RasterizerPSP::is_texture(const RID& p_rid) const {

	return texture_owner.owns(p_rid);
}
bool RasterizerPSP::is_material(const RID& p_rid) const {

	return material_owner.owns(p_rid);
}
bool RasterizerPSP::is_mesh(const RID& p_rid) const {

	return mesh_owner.owns(p_rid);
}

bool RasterizerPSP::is_immediate(const RID& p_rid) const {

	return immediate_owner.owns(p_rid);
}

bool RasterizerPSP::is_multimesh(const RID& p_rid) const {

	return multimesh_owner.owns(p_rid);
}
bool RasterizerPSP::is_particles(const RID &p_beam) const {

	return particles_owner.owns(p_beam);
}

bool RasterizerPSP::is_light(const RID& p_rid) const {

	return light_owner.owns(p_rid);
}
bool RasterizerPSP::is_light_instance(const RID& p_rid) const {

	return light_instance_owner.owns(p_rid);
}
bool RasterizerPSP::is_particles_instance(const RID& p_rid) const {

	return particles_instance_owner.owns(p_rid);
}
bool RasterizerPSP::is_skeleton(const RID& p_rid) const {

	return skeleton_owner.owns(p_rid);
}
bool RasterizerPSP::is_environment(const RID& p_rid) const {

	return environment_owner.owns(p_rid);
}
bool RasterizerPSP::is_fx(const RID& p_rid) const {

	return fx_owner.owns(p_rid);
}
bool RasterizerPSP::is_shader(const RID& p_rid) const {

	return false;
}

void RasterizerPSP::free(const RID& p_rid) {

	if (texture_owner.owns(p_rid)) {

		// delete the texture
		Texture *texture = texture_owner.get(p_rid);

		texture->_free_mipmaps();
		_rinfo.texture_mem-=texture->total_data_size;
		texture_owner.free(p_rid);
		memdelete(texture);

	} else if (shader_owner.owns(p_rid)) {

		// delete the texture
		Shader *shader = shader_owner.get(p_rid);



		shader_owner.free(p_rid);
		memdelete(shader);

	} else if (material_owner.owns(p_rid)) {

		Material *material = material_owner.get( p_rid );
		ERR_FAIL_COND(!material);

		material_owner.free(p_rid);
		memdelete(material);

	} else if (mesh_owner.owns(p_rid)) {

		Mesh *mesh = mesh_owner.get(p_rid);
		ERR_FAIL_COND(!mesh);
		for (int i=0;i<mesh->surfaces.size();i++) {

			Surface *surface = mesh->surfaces[i];
			if (surface->psp_array_local != 0) {
				vtfree(surface->psp_array_local);
			};
			if (surface->array_local != 0) {
				vtfree(surface->array_local);
			};
			if (surface->index_array_local != 0) {
				vtfree(surface->index_array_local);
			};

			if (mesh->morph_target_count>0) {

				for(int i=0;i<mesh->morph_target_count;i++) {

					memfree(surface->morph_targets_local[i].array);
				}
				memfree(surface->morph_targets_local);
				surface->morph_targets_local=NULL;
			}

			// if (surface->vertex_id)
			// 	glDeleteBuffers(1,&surface->vertex_id);
			// if (surface->index_id)
			// 	glDeleteBuffers(1,&surface->index_id);

			memdelete( surface );
		};

		mesh->surfaces.clear();

		mesh_owner.free(p_rid);
		memdelete(mesh);

	} else if (multimesh_owner.owns(p_rid)) {

	       MultiMesh *multimesh = multimesh_owner.get(p_rid);
	       ERR_FAIL_COND(!multimesh);

	       multimesh_owner.free(p_rid);
	       memdelete(multimesh);

	} else if (particles_owner.owns(p_rid)) {

		Particles3D *particles = particles_owner.get(p_rid);
		ERR_FAIL_COND(!particles);

		particles_owner.free(p_rid);
		memdelete(particles);
	} else if (immediate_owner.owns(p_rid)) {

		Immediate *immediate = immediate_owner.get(p_rid);
		ERR_FAIL_COND(!immediate);

		immediate_owner.free(p_rid);
		memdelete(immediate);
	} else if (particles_instance_owner.owns(p_rid)) {

		ParticlesInstance *particles_isntance = particles_instance_owner.get(p_rid);
		ERR_FAIL_COND(!particles_isntance);

		particles_instance_owner.free(p_rid);
		memdelete(particles_isntance);

	} else if (skeleton_owner.owns(p_rid)) {

		Skeleton3D *skeleton = skeleton_owner.get( p_rid );
		ERR_FAIL_COND(!skeleton)

		skeleton_owner.free(p_rid);
		memdelete(skeleton);

	} else if (light_owner.owns(p_rid)) {

		Light *light = light_owner.get( p_rid );
		ERR_FAIL_COND(!light)

		light_owner.free(p_rid);
		memdelete(light);

	} else if (light_instance_owner.owns(p_rid)) {

		LightInstance *light_instance = light_instance_owner.get( p_rid );
		ERR_FAIL_COND(!light_instance);
		light_instance->clear_shadow_buffers();
		light_instance_owner.free(p_rid);
		memdelete( light_instance );

	} else if (fx_owner.owns(p_rid)) {

		FX *fx = fx_owner.get( p_rid );
		ERR_FAIL_COND(!fx);

		fx_owner.free(p_rid);
		memdelete( fx );

	} else if (environment_owner.owns(p_rid)) {

		Environment *env = environment_owner.get( p_rid );
		ERR_FAIL_COND(!env);

		environment_owner.free(p_rid);
		memdelete( env );
	} else if (sampled_light_owner.owns(p_rid)) {

		SampledLight *sampled_light = sampled_light_owner.get( p_rid );
		ERR_FAIL_COND(!sampled_light);

		sampled_light_owner.free(p_rid);
		memdelete( sampled_light );
	};
}


void RasterizerPSP::custom_shade_model_set_shader(int p_model, RID p_shader) {


};

RID RasterizerPSP::custom_shade_model_get_shader(int p_model) const {

	return RID();
};

void RasterizerPSP::custom_shade_model_set_name(int p_model, const String& p_name) {

};

String RasterizerPSP::custom_shade_model_get_name(int p_model) const {

	return String();
};

void RasterizerPSP::custom_shade_model_set_param_info(int p_model, const List<PropertyInfo>& p_info) {

};

void RasterizerPSP::custom_shade_model_get_param_info(int p_model, List<PropertyInfo>* p_info) const {

};


void RasterizerPSP::ShadowBuffer::init(int p_size) {


#if 0
	size=p_size;

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &depth);
	ERR_FAIL_COND(depth==0);

	/* Setup Depth Texture */
	glBindTexture(GL_TEXTURE_2D, depth);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, p_size, p_size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float border_color[]={1.0f, 1.0f, 1.0f, 1.0f};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

	/* Create FBO */
	glGenFramebuffers(1, &fbo);

	ERR_FAIL_COND( fbo==0 );

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
	glDrawBuffer(GL_FALSE);
	glReadBuffer(GL_FALSE);

	/* Check FBO creation */
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER);

	ERR_FAIL_COND( status==GL_FRAMEBUFFER_UNSUPPORTED );

	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
#endif

}

void RasterizerPSP::_init_shadow_buffers() {

	int near_shadow_size=GLOBAL_DEF("rasterizer/near_shadow_size",512);
	int far_shadow_size=GLOBAL_DEF("rasterizer/far_shadow_size",64);

	near_shadow_buffers.resize( GLOBAL_DEF("rasterizer/near_shadow_count",4) );
	far_shadow_buffers.resize( GLOBAL_DEF("rasterizer/far_shadow_count",16) );

	shadow_near_far_split_size_ratio = GLOBAL_DEF("rasterizer/shadow_near_far_split_size_ratio",0.3);

	for (int i=0;i<near_shadow_buffers.size();i++) {

		near_shadow_buffers[i].init(near_shadow_size );
	}

	for (int i=0;i<far_shadow_buffers.size();i++) {

		far_shadow_buffers[i].init(far_shadow_size);
	}

}


void RasterizerPSP::_update_framebuffer() {

	return;

#if 0
	bool want_16 = GLOBAL_DEF("rasterizer/support_hdr",true);
	int blur_buffer_div=GLOBAL_DEF("rasterizer/blur_buffer_div",4);
	bool use_fbo = GLOBAL_DEF("rasterizer/use_fbo",true);


	if (blur_buffer_div<1)
		blur_buffer_div=2;


	if (use_fbo==framebuffer.active && framebuffer.width==window_size.width && framebuffer.height==window_size.height && framebuffer.buff16==want_16)
		return; //nuthin to change

	if (framebuffer.fbo!=0) {

		WARN_PRINT("Resizing the screen multiple times while using to FBOs may decrease performance on some hardware.");
		//free the framebuffarz
		glDeleteRenderbuffers(1,&framebuffer.fbo);
		glDeleteTextures(1,&framebuffer.depth);
		glDeleteTextures(1,&framebuffer.color);
		for(int i=0;i<2;i++) {
			glDeleteRenderbuffers(1,&framebuffer.blur[i].fbo);
			glDeleteTextures(1,&framebuffer.blur[i].color);

		}

		framebuffer.fbo=0;
	}

	framebuffer.active=use_fbo;
	framebuffer.width=window_size.width;
	framebuffer.height=window_size.height;
	framebuffer.buff16=want_16;


	if (!use_fbo)
		return;


	glGenFramebuffers(1, &framebuffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

	print_line("generating fbo, id: "+itos(framebuffer.fbo));
	//depth
	glGenTextures(1, &framebuffer.depth);

	glBindTexture(GL_TEXTURE_2D, framebuffer.depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,  framebuffer.width, framebuffer.height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE );
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, framebuffer.depth, 0);
	//color
	glGenTextures(1, &framebuffer.color);
	glBindTexture(GL_TEXTURE_2D, framebuffer.color);
	glTexImage2D(GL_TEXTURE_2D, 0, want_16?GL_RGB16F:GL_RGBA8,  framebuffer.width, framebuffer.height, 0, GL_RGBA, want_16?GL_HALF_FLOAT:GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.color, 0);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	ERR_FAIL_COND( status != GL_FRAMEBUFFER_COMPLETE );

	for(int i=0;i<2;i++) {

		glGenFramebuffers(1, &framebuffer.blur[i].fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.blur[i].fbo);

		glGenTextures(1, &framebuffer.blur[i].color);
		glBindTexture(GL_TEXTURE_2D, framebuffer.blur[i].color);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  framebuffer.width/blur_buffer_div, framebuffer.height/blur_buffer_div, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.blur[i].color, 0);

		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		ERR_FAIL_COND( status != GL_FRAMEBUFFER_COMPLETE );
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}

void RasterizerPSP::init() {

	scene_pass=1;
	// if (ContextGL::get_singleton())
	// 	ContextGL::get_singleton()->make_current();

	fbp0 = reinterpret_cast<void *>(FBP0_OFFSET);
	fbp1 = reinterpret_cast<void *>(FBP1_OFFSET);
	zbp = reinterpret_cast<void *>(FBP2_OFFSET);
/*
	Set<String> extensions;
	Vector<String> strings = String((const char*)glGetString( GL_EXTENSIONS )).split(" ",false);
	for(int i=0;i<strings.size();i++) {

		extensions.insert(strings[i]);
//		print_line(strings[i]);
	}*/

	sceGuInit();
	sceGuStart(GU_DIRECT, list);
	// sceGuDrawBuffer(GU_PSM_5650,fbp0,BUF_WIDTH);
	sceGuDrawBuffer(GU_PSM_8888,fbp0,BUF_WIDTH);
	sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,fbp1,BUF_WIDTH);
	sceGuDepthBuffer(zbp,BUF_WIDTH);
	sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
	sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
	sceGuDepthRange(0,0xffff);
	sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuFrontFace(GU_CW);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuEnable(GU_BLEND);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0,0);
	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);


	const ScePspIMatrix4 dith{
		{-4,  0, -3,  1},
		{ 2, -2,  3, -1},
		{-3,  1, -4,  0},
		{ 3, -1,  2, -2}};
	sceGuSetDither(&dith);
	sceGuEnable(GU_DITHER);

	sceGuDisable(GU_CLIP_PLANES);

	// glEnable(GL_DEPTH_TEST);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuDepthFunc(GU_LEQUAL);
	sceGuDepthMask(GU_FALSE);
	// glDepthFunc(GL_LEQUAL);
	// glFrontFace(GL_CW);
	//glEnable(GL_TEXTURE_2D);

	default_material=create_default_material();

	_init_shadow_buffers();

	shadow=NULL;
	shadow_pass=0;

	framebuffer.fbo=0;
	framebuffer.width=0;
	framebuffer.height=0;
	framebuffer.buff16=false;
	framebuffer.blur[0].fbo=false;
	framebuffer.blur[1].fbo=false;
	framebuffer.active=false;

	//do a single initial clear
	// glClearColor(0,0,0,1);
	// sceGuClearColor(MK_RGBA(0,0,0,1));
	//glClearDepth(1.0);
	// glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	// sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);

	//skinned_buffer_size = GLOBAL_DEF("rasterizer/skinned_buffer_size",DEFAULT_SKINNED_BUFFER_SIZE);
	//skinned_buffer = memnew_arr( uint8_t, skinned_buffer_size );
	skinned_buffer_size = 0;

	// glGenTextures(1, &white_tex);
	// unsigned char whitetexdata[8*8*3];
	// for(int i=0;i<8*8*3;i++) {
	// 	whitetexdata[i]=255;
	// }
	// sceGuTexMode(GU_PSM_8888,0,0,0);
	// sceGuTexImage(0,8,8,8,whitetexdata);
	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D,white_tex);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 8, 8, 0, GL_RGB, GL_UNSIGNED_BYTE,whitetexdata);

	pvr_supported=false;
	etc_supported=false;
	s3tc_supported=true;
	_rinfo.texture_mem=0;


}

void RasterizerPSP::finish() {
	sceGuTerm();
	//memdelete(skinned_buffer);
}

int RasterizerPSP::get_render_info(VS::RenderInfo p_info) {

	switch(p_info) {

		case VS::INFO_OBJECTS_IN_FRAME: {

			return _rinfo.object_count;
		} break;
		case VS::INFO_VERTICES_IN_FRAME: {

			return _rinfo.vertex_count;
		} break;
		case VS::INFO_MATERIAL_CHANGES_IN_FRAME: {

			return _rinfo.mat_change_count;
		} break;
		case VS::INFO_SHADER_CHANGES_IN_FRAME: {

			return _rinfo.shader_change_count;
		} break;
		case VS::INFO_USAGE_VIDEO_MEM_TOTAL: {

			return 0;
		} break;
		case VS::INFO_VIDEO_MEM_USED: {

			return get_render_info(VS::INFO_TEXTURE_MEM_USED)+get_render_info(VS::INFO_VERTEX_MEM_USED);
		} break;
		case VS::INFO_TEXTURE_MEM_USED: {

			_rinfo.texture_mem;
		} break;
		case VS::INFO_VERTEX_MEM_USED: {

			return 0;
		} break;
	}

	return false;
}

bool RasterizerPSP::needs_to_draw_next_frame() const {

	return false;
}

void RasterizerPSP::reload_vram() {

	//sceGuEnable(GU_DEPTH_TEST);
	//sceGuDepthFunc(GU_LEQUAL);
	//sceGuFrontFace(GU_CW);

	//sceGuClearColor(MK_RGBA(0,0,0,255));
	////glClearDepth(1.0);
	//sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);

	//sceGuEnable(GU_TEXTURE_2D);
	// TODO: Reload vram
	/*
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glFrontFace(GL_CW);

	//do a single initial clear
	glClearColor(0,0,0,1);
	//glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);


	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);

	List<RID> textures;
	texture_owner.get_owned_list(&textures);
	keep_copies=false;
	for(List<RID>::Element *E=textures.front();E;E=E->next()) {

		RID tid = E->get();
		Texture *t=texture_owner.get(tid);
		ERR_CONTINUE(!t);
		t->tex_id=0;
		t->data_size=0;
		glGenTextures(1, &t->tex_id);
		t->active=false;
		texture_allocate(tid,t->width,t->height,t->format,t->flags);
		bool had_image=false;
		for(int i=0;i<6;i++) {
			if (!t->image[i].empty()) {
				texture_set_data(tid,t->image[i],VS::CubeMapSide(i));
				had_image=true;
			}
		}

		if (!had_image && t->reloader) {
			Object *rl = ObjectDB::get_instance(t->reloader);
			if (rl)
				rl->call(t->reloader_func,tid);
		}
	}

	keep_copies=true;
*/

}

bool RasterizerPSP::has_feature(VS::Features p_feature) const {

	switch( p_feature) {
		case VS::FEATURE_SHADERS: return false;
		case VS::FEATURE_NEEDS_RELOAD_HOOK: return use_reload_hooks;
		default: return false;

	}

}


RasterizerPSP::RasterizerPSP(bool p_keep_copies,bool p_use_reload_hooks) {
	keep_copies=p_keep_copies;
	pack_arrays=false;
	use_reload_hooks=p_use_reload_hooks;

	frame = 0;

	memnew(MemoryPoolEDRAM);
#ifndef PSP_NET
	memnew(MemoryPoolPSPVolatile);
#endif
};

RasterizerPSP::~RasterizerPSP() {

};


#endif
