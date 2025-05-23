/*************************************************************************/
/*  rasterizer_gles1.h                                                   */
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
#ifndef RASTERIZER_GLES1_H
#define RASTERIZER_GLES1_H

#include "servers/visual/rasterizer.h"

#if defined(GLES1_ENABLED) || defined(__psp2__) || defined(__WII__)

#include "image.h"
#include "rid.h"
#include "servers/visual_server.h"
#include "list.h"
#include "map.h"
#include "camera_matrix.h"
#include "sort.h"
// #include "tools/editor/scene_tree_editor.h"
#include "platform_config.h"
#if defined(GLES1_INCLUDE_H)
 #include GLES1_INCLUDE_H
#else
 #include <GLES/gl.h>
#endif

#include "servers/visual/particle_system_sw.h"

/**
        @author Juan Linietsky <reduzio@gmail.com>
*/
class RasterizerGLES1 : public Rasterizer {

	enum {

		MAX_SCENE_LIGHTS=2048,
		LIGHT_SPOT_BIT=0x80,
		DEFAULT_SKINNED_BUFFER_SIZE = 1024 * 1024, // 10k vertices
#ifdef __psp2__
		MAX_HW_LIGHTS = 8, //Yay! 8 Lights :blush:
#else
		MAX_HW_LIGHTS = 4,
#endif
	};
	#if defined(PSP) || defined(__WII__)
	void glActiveTexture(int a1) { };
	void glClientActiveTexture(int a1) { };
	#endif

	GLuint BlurTexture;

	uint8_t *skinned_buffer;
	int skinned_buffer_size;
	bool pvr_supported;
	bool s3tc_supported;
	bool etc_supported;
	bool npo2_textures_available;
	bool pack_arrays;
	bool use_reload_hooks;
	bool is_editor;
	

	Image _get_gl_image_and_format(const Image& p_image, Image::Format p_format, uint32_t p_flags,GLenum& r_gl_format,int &r_gl_components,bool &r_has_alpha_cache,bool &r_compressed);


	struct Texture {

		uint32_t flags;
		int width,height;
		int alloc_width, alloc_height;
		Image::Format format;

		GLenum target;
		GLenum gl_format_cache;
		int gl_components_cache;
		int data_size; //original data size, useful for retrieving back
		bool format_has_alpha;
		bool compressed;
		bool disallow_mipmaps;
		int total_data_size;

		Image image[6];

		bool active;
		GLuint tex_id;

		ObjectID reloader;
		StringName reloader_func;

		Texture() {

			flags=width=height=0;
			tex_id=0;
			data_size=0;
			format=Image::FORMAT_GRAYSCALE;
			gl_components_cache=0;
			format_has_alpha=false;
			active=false;
			disallow_mipmaps=false;
//			gen_mipmap=true;
			compressed=false;
			total_data_size=0;
		}

		~Texture() {

			if (tex_id!=0) {

				glDeleteTextures(1,&tex_id);
			}
		}
	};

	mutable RID_Owner<Texture> texture_owner;

	struct Shader {

		String vertex_code;
		String fragment_code;
		String light_code;
		VS::ShaderMode mode;
		Map<StringName,Variant> params;
		int fragment_line;
		int vertex_line;
		int light_line;
		bool valid;
		bool has_alpha;
		bool use_world_transform;

	};

	mutable RID_Owner<Shader> shader_owner;


	struct Material {

		bool fixed_flags[VS::FIXED_MATERIAL_FLAG_MAX];
		bool flags[VS::MATERIAL_FLAG_MAX];
		Variant parameters[VisualServer::FIXED_MATERIAL_PARAM_MAX];
		RID textures[VisualServer::FIXED_MATERIAL_PARAM_MAX];

		VS::MaterialDepthDrawMode depth_draw_mode;

		Transform3D uv_transform;
		VS::FixedMaterialTexCoordMode texcoord_mode[VisualServer::FIXED_MATERIAL_PARAM_MAX];

		VS::MaterialBlendMode blend_mode;

		float line_width;
		float point_size;
		bool has_alpha;

		RID shader; // shader material
		uint64_t last_pass;

		Map<StringName,Variant> shader_params;


		Material() {


			for(int i=0;i<VS::FIXED_MATERIAL_FLAG_MAX;i++)
				flags[i]=false;

			for(int i=0;i<VS::MATERIAL_FLAG_MAX;i++)
				flags[i]=false;
			flags[VS::MATERIAL_FLAG_VISIBLE]=true;

			parameters[VS::FIXED_MATERIAL_PARAM_DIFFUSE] = Color(0.8, 0.8, 0.8);
			parameters[VS::FIXED_MATERIAL_PARAM_SPECULAR_EXP] = 12;

			for (int i=0; i<VisualServer::FIXED_MATERIAL_PARAM_MAX; i++) {
				texcoord_mode[i] = VS::FIXED_MATERIAL_TEXCOORD_UV;
			};
			depth_draw_mode=VS::MATERIAL_DEPTH_DRAW_OPAQUE_ONLY;
			line_width=1;
			has_alpha=false;
			blend_mode=VS::MATERIAL_BLEND_MODE_MIX;
			last_pass = 0;
			point_size = 1.0;

		}
	};
	mutable RID_Owner<Material> material_owner;

	void _material_check_alpha(Material *p_material);


	struct Geometry {

		enum Type {
			GEOMETRY_INVALID,
			GEOMETRY_SURFACE,
			GEOMETRY_POLY,
			GEOMETRY_PARTICLES,
			GEOMETRY_MULTISURFACE,
		};

		Type type;
		RID material;
		bool has_alpha;
		bool material_owned;

		Geometry() { has_alpha=false; material_owned = false; }
		virtual ~Geometry() {};
	};

	struct GeometryOwner {

		virtual ~GeometryOwner() {}
	};

	class Mesh;

	struct Surface : public Geometry {

		struct ArrayData {

			uint32_t ofs,size,datatype,count;
			bool normalize;
			bool bind;

			ArrayData() { ofs=0; size=0; count=0; datatype=0; normalize=0; bind=false;}
		};

		Mesh *mesh;

		Array data;
		Array morph_data;
		ArrayData array[VS::ARRAY_MAX];
		// support for vertex array objects
		GLuint array_object_id;
		// support for vertex buffer object
		GLuint vertex_id; // 0 means, unconfigured
		GLuint index_id; // 0 means, unconfigured
		// no support for the above, array in localmem.
		uint8_t *array_local;
		uint8_t *index_array_local;

		bool packed;

		struct MorphTarget {
			uint32_t configured_format;
			uint8_t *array;
		};

		MorphTarget* morph_targets_local;
		int morph_target_count;
		AABB aabb;

		int array_len;
		int index_array_len;
		int max_bone;

		float vertex_scale;
		float uv_scale;
		float uv2_scale;

		VS::PrimitiveType primitive;

		uint32_t format;
		uint32_t configured_format;

		int stride;
		int local_stride;
		uint32_t morph_format;

		bool active;

		Point2 uv_min;
		Point2 uv_max;

		Surface() {


			array_len=0;
			local_stride=0;
			morph_format=0;
			type=GEOMETRY_SURFACE;
			primitive=VS::PRIMITIVE_POINTS;
			index_array_len=0;
			vertex_scale=1.0;
			uv_scale=1.0;
			uv2_scale=1.0;

			format=0;
			stride=0;
			morph_targets_local=0;
			morph_target_count=0;

			array_local = index_array_local = 0;
			vertex_id = index_id = 0;

			active=false;
			packed=false;
		}

		~Surface() {

		}
	};


	struct Mesh {

		bool active;
		Vector<Surface*> surfaces;
		int morph_target_count;
		VS::MorphTargetMode morph_target_mode;
		AABB custom_aabb;

		mutable uint64_t last_pass;
		Mesh() {
			morph_target_mode=VS::MORPH_MODE_NORMALIZED;
			morph_target_count=0;
			last_pass=0;
			active=false;
		}
	};
	mutable RID_Owner<Mesh> mesh_owner;

	Error _surface_set_arrays(Surface *p_surface, uint8_t *p_mem,uint8_t *p_index_mem,const Array& p_arrays,bool p_main);

	struct MultiMesh;

	struct MultiMeshSurface : public Geometry {

		Surface *surface;
		MultiMeshSurface() { type=GEOMETRY_MULTISURFACE; }
	};

	struct MultiMesh : public GeometryOwner {

		struct Element {

			float matrix[16];
			uint8_t color[4];
			Element() {
				matrix[0]=1;
				matrix[1]=0;
				matrix[2]=0;
				matrix[3]=0;

				matrix[4]=0;
				matrix[5]=1;
				matrix[6]=0;
				matrix[7]=0;

				matrix[8]=0;
				matrix[9]=0;
				matrix[10]=1;
				matrix[11]=0;

				matrix[12]=0;
				matrix[13]=0;
				matrix[14]=0;
				matrix[15]=1;
			};


		};

		AABB aabb;
		RID mesh;
		int visible;

		//IDirect3DVertexBuffer9* instance_buffer;
		Vector<Element> elements;
		Vector<MultiMeshSurface> cache_surfaces;
		mutable uint64_t last_pass;
		GLuint tex_id;
		int tw;
		int th;

		SelfList<MultiMesh> dirty_list;

		MultiMesh() : dirty_list(this) {

			tw=1;
			th=1;
			tex_id=0;
			last_pass=0;
			visible = -1;
		}
	};

	mutable RID_Owner<MultiMesh> multimesh_owner;
	mutable SelfList<MultiMesh>::List _multimesh_dirty_list;

	struct Immediate {

		RID material;
		int empty;
	};

	mutable RID_Owner<Immediate> immediate_owner;

	struct Particles : public Geometry {

		ParticleSystemSW data; // software particle system

		Particles() {
			type=GEOMETRY_PARTICLES;

		}
	};

	mutable RID_Owner<Particles> particles_owner;

	struct ParticlesInstance : public GeometryOwner {

		RID particles;

		ParticleSystemProcessSW particles_process;
		Transform3D transform;

		ParticlesInstance() {  }
	};

	mutable RID_Owner<ParticlesInstance> particles_instance_owner;
	ParticleSystemDrawInfoSW particle_draw_info;

	struct Skeleton {

		Vector<Transform3D> bones;

	};

	mutable RID_Owner<Skeleton> skeleton_owner;


	struct Light {

		VS::LightType type;
		float vars[VS::LIGHT_PARAM_MAX];
		Color colors[3];
		bool shadow_enabled;
		RID projector;
		bool volumetric_enabled;
		Color volumetric_color;


		Light() {

			vars[VS::LIGHT_PARAM_SPOT_ATTENUATION]=1;
			vars[VS::LIGHT_PARAM_SPOT_ANGLE]=45;
			vars[VS::LIGHT_PARAM_ATTENUATION]=1.0;
			vars[VS::LIGHT_PARAM_ENERGY]=1.0;
			vars[VS::LIGHT_PARAM_RADIUS]=1.0;
			vars[VS::LIGHT_PARAM_SHADOW_Z_OFFSET]=0.05;

			colors[VS::LIGHT_COLOR_DIFFUSE]=Color(1,1,1);
			colors[VS::LIGHT_COLOR_SPECULAR]=Color(1,1,1);
			shadow_enabled=false;
			volumetric_enabled=false;
		}
	};


	struct Environment {


		VS::EnvironmentBG bg_mode;
		Variant bg_param[VS::ENV_BG_PARAM_MAX];
		bool fx_enabled[VS::ENV_FX_MAX];
		Variant fx_param[VS::ENV_FX_PARAM_MAX];
		Variant group[VS::ENV_GROUP_MAX];
		
		Environment() {

			bg_mode=VS::ENV_BG_DEFAULT_COLOR;
			bg_param[VS::ENV_BG_PARAM_COLOR]=Color(0,0,0);
			bg_param[VS::ENV_BG_PARAM_TEXTURE]=RID();
			bg_param[VS::ENV_BG_PARAM_CUBEMAP]=RID();
			bg_param[VS::ENV_BG_PARAM_ENERGY]=1.0;

			for(int i=0;i<VS::ENV_FX_MAX;i++)
				fx_enabled[i]=false;

			fx_param[VS::ENV_FX_PARAM_GLOW_BLUR_PASSES]=1;
			fx_param[VS::ENV_FX_PARAM_GLOW_BLOOM]=0.0;
			fx_param[VS::ENV_FX_PARAM_GLOW_BLOOM_TRESHOLD]=0.5;
			fx_param[VS::ENV_FX_PARAM_DOF_BLUR_PASSES]=1;
			fx_param[VS::ENV_FX_PARAM_DOF_BLUR_BEGIN]=100.0;
			fx_param[VS::ENV_FX_PARAM_DOF_BLUR_RANGE]=10.0;
			fx_param[VS::ENV_FX_PARAM_HDR_EXPOSURE]=0.4;
			fx_param[VS::ENV_FX_PARAM_HDR_WHITE]=1.0;
			fx_param[VS::ENV_FX_PARAM_HDR_GLOW_TRESHOLD]=0.95;
			fx_param[VS::ENV_FX_PARAM_HDR_GLOW_SCALE]=0.2;
			fx_param[VS::ENV_FX_PARAM_HDR_MIN_LUMINANCE]=0.4;
			fx_param[VS::ENV_FX_PARAM_HDR_MAX_LUMINANCE]=8.0;
			fx_param[VS::ENV_FX_PARAM_HDR_EXPOSURE_ADJUST_SPEED]=0.5;
			fx_param[VS::ENV_FX_PARAM_FOG_BEGIN]=100.0;
			fx_param[VS::ENV_FX_PARAM_FOG_ATTENUATION]=1.0;
			fx_param[VS::ENV_FX_PARAM_FOG_BEGIN_COLOR]=Color(0,0,0);
			fx_param[VS::ENV_FX_PARAM_FOG_END_COLOR]=Color(0,0,0);
			fx_param[VS::ENV_FX_PARAM_FOG_BG]=true;
			fx_param[VS::ENV_FX_PARAM_BCS_BRIGHTNESS]=1.0;
			fx_param[VS::ENV_FX_PARAM_BCS_CONTRAST]=1.0;
			fx_param[VS::ENV_FX_PARAM_BCS_SATURATION]=1.0;

		}

	};

	mutable RID_Owner<Environment> environment_owner;
	Environment *current_env;
	
	struct SampledLight {

		int w,h;
		GLuint texture;
		float multiplier;
		bool is_float;
	};

	mutable RID_Owner<SampledLight> sampled_light_owner;

	struct ShadowBuffer;

	struct LightInstance {

		struct SplitInfo {

			CameraMatrix camera;
			Transform3D transform;
			float near;
			float far;
		};

		RID light;
		Light *base;
		Transform3D transform;
		CameraMatrix projection;

		Transform3D custom_transform;
		CameraMatrix custom_projection;

		Vector3 light_vector;
		Vector3 spot_vector;
		float linear_att;

		uint64_t shadow_pass;
		uint64_t last_pass;
		uint16_t sort_key;

		ShadowBuffer* shadow_buffer;

		void clear_shadow_buffers() {/*

			for (int i=0;i<shadow_buffer.size();i++) {

				ShadowBuffer *sb=shadow_buffer[i];
				ERR_CONTINUE( sb->owner != this );

				sb->owner=NULL;
			}
			
			shadow_buffers.clear();*/
			shadow_buffer = NULL;
		}

		LightInstance() { shadow_pass=0; last_pass=0; sort_key=0; shadow_buffer=NULL; }

	};
	mutable RID_Owner<Light> light_owner;
	mutable RID_Owner<LightInstance> light_instance_owner;

	LightInstance *light_instances[MAX_SCENE_LIGHTS];
	LightInstance *directional_lights[4];
//	LightInstance *directional_light_instances[MAX_SCENE_LIGHTS];
	int light_instance_count;
	int directional_light_count;
	int last_light_id;


	struct RenderList {

		enum {
			MAX_ELEMENTS=4096,
			MAX_LIGHTS=8192
		};

		struct Element {


			float depth;
			const InstanceData *instance;
			const Skeleton *skeleton;
			union {
				uint16_t lights[MAX_HW_LIGHTS];
				uint64_t light_key;
			};

			const Geometry *geometry;
			const Geometry *geometry_cmp;
			const Material *material;
			const GeometryOwner *owner;
			uint16_t light_count;
			bool mirror;


		};


		Element _elements[MAX_ELEMENTS];
		Element *elements[MAX_ELEMENTS];
		int element_count;

		void clear() {

			element_count=0;
		}

		struct SortZ {

			_FORCE_INLINE_ bool operator()(const Element* A,  const Element* B ) const {

				return A->depth > B->depth;
			}
		};

		void sort_z() {

			SortArray<Element*,SortZ> sorter;
			sorter.sort(elements,element_count);
		}


		struct SortMat {

			_FORCE_INLINE_ bool operator()(const Element* A,  const Element* B ) const {
				// TODO move to a single uint64 (one comparison)
				if (A->material == B->material) {

					return A->light_key < B->light_key;
				} else {

					return (A->material < B->material);
				}
			}
		};

		void sort_mat() {

			SortArray<Element*,SortMat> sorter;
			sorter.sort(elements,element_count);
		}

		struct SortMatLight {

			_FORCE_INLINE_ bool operator()(const Element* A,  const Element* B ) const {

				if (A->material->flags[VS::MATERIAL_FLAG_UNSHADED] == B->material->flags[VS::MATERIAL_FLAG_UNSHADED]) {

					if (A->material == B->material) {

						if (A->geometry == B->geometry) {

							return A->light_key<B->light_key;
						} else
							return (A->geometry < B->geometry);
					} else {

						return (A->material < B->material);
					}
				} else {

					return (int(A->material->flags[VS::MATERIAL_FLAG_UNSHADED]) < int(B->material->flags[VS::MATERIAL_FLAG_UNSHADED]));
				}
			}
		};

		void sort_mat_light() {

			SortArray<Element*,SortMatLight> sorter;
			sorter.sort(elements,element_count);
		}

		_FORCE_INLINE_ Element* add_element() {

			if (element_count>MAX_ELEMENTS)
				return NULL;
			elements[element_count]=&_elements[element_count];
			return elements[element_count++];
		}

		RenderList() {

			element_count = 0;
			for (int i=0;i<MAX_ELEMENTS;i++)
				elements[i]=&_elements[i]; // assign elements
		}
	};

	RenderList opaque_render_list;
	RenderList alpha_render_list;

	RID default_material;

	struct FX {

		bool bgcolor_active;
		Color bgcolor;

		bool skybox_active;
		RID skybox_cubemap;

		bool antialias_active;
		float antialias_tolerance;

		bool glow_active;
		int glow_passes;
		float glow_attenuation;
		float glow_bloom;

		bool ssao_active;
		float ssao_attenuation;
		float ssao_radius;
		float ssao_max_distance;
		float ssao_range_max;
		float ssao_range_min;
		bool ssao_only;

		bool fog_active;
		float fog_near;
		float fog_far;
		float fog_attenuation;
		Color fog_color_near;
		Color fog_color_far;
		bool fog_bg;

		bool toon_active;
		float toon_treshold;
		float toon_soft;

		bool edge_active;
		Color edge_color;
		float edge_size;

		FX();

	};
	mutable RID_Owner<FX> fx_owner;


	FX *scene_fx;
	CameraMatrix camera_projection;
	Transform3D camera_transform;
	Transform3D camera_transform_inverse;
	float camera_z_near;
	float camera_z_far;
	Size2 camera_vp_size;
	Color last_color;

	Plane camera_plane;

	bool keep_copies;

	bool depth_write;
	bool depth_test;
	int blend_mode;
	bool lighting;

	_FORCE_INLINE_ void _add_geometry( const Geometry* p_geometry, const InstanceData *p_instance, const Geometry *p_geometry_cmp, const GeometryOwner *p_owner);

	void _render_list_forward(RenderList *p_render_list,bool p_reverse_cull=false);

	void _setup_light(LightInstance* p_instance, int p_idx);
	void _setup_lights(const uint16_t * p_lights,int p_light_count);

	_FORCE_INLINE_ void _setup_shader_params(const Material *p_material);
	void _setup_fixed_material(const Geometry *p_geometry,const Material *p_material);
	void _setup_material(const Geometry *p_geometry,const Material *p_material);

	Error _setup_geometry(const Geometry *p_geometry, const Material* p_material,const Skeleton *p_skeleton, const float *p_morphs);
	void _render(const Geometry *p_geometry,const Material *p_material, const Skeleton* p_skeleton, const GeometryOwner *p_owner);


	/***********/
	/* SHADOWS */
	/***********/

	struct ShadowBuffer {

		int size;
		GLuint fbo;
		GLuint depth;
		LightInstance *owner;
		void init(int p_size);
		ShadowBuffer() { size=0; depth=0; owner=NULL; }
	};

	Vector<ShadowBuffer> near_shadow_buffers;
	Vector<ShadowBuffer> far_shadow_buffers;

	LightInstance *shadow;
	int shadow_pass;
	void _init_shadow_buffers();

	float shadow_near_far_split_size_ratio;
	bool _allocate_shadow_buffers(LightInstance *p_instance, Vector<ShadowBuffer>& p_buffers);
	void _debug_draw_shadow(GLuint tex, const Rect2& p_rect);
	void _debug_draw_shadows_type(Vector<ShadowBuffer>& p_shadows,Point2& ofs);
	void _debug_shadows();
	void reset_state();

	/***********/
	/*  FBOs   */
	/***********/


	struct FrameBuffer {

		GLuint fbo;
		GLuint color;
		GLuint depth;
		int width,height;
		bool buff16;
		bool active;

		struct Blur {

			GLuint fbo;
			GLuint color;
		} blur[2];

	} framebuffer;

	void _update_framebuffer();
	void _process_glow_and_bloom();

	/*********/
	/* FRAME */
	/*********/

	struct _Rinfo {

		int texture_mem;
		int vertex_count;
		int object_count;
		int mat_change_count;
		int shader_change_count;

	} _rinfo;

	GLuint white_tex;
	RID canvas_tex;
	float canvas_opacity;
	VS::MaterialBlendMode canvas_blend;
	_FORCE_INLINE_ Texture* _bind_canvas_texture(const RID& p_texture);


	int _setup_geometry_vinfo;

	bool cull_front;
	_FORCE_INLINE_ void _set_cull(bool p_front,bool p_reverse_cull=false);

	Size2 window_size;
	VS::ViewportRect viewport;
	double last_time;
	double time_delta;
	uint64_t frame;
	uint64_t scene_pass;

	//void _draw_primitive(int p_points, const Vector3 *p_vertices, const Vector3 *p_normals, const Color* p_colors, const Vector3 *p_uvs,const Plane *p_tangents=NULL,int p_instanced=1);
	//void _draw_textured_quad(const Rect2& p_rect, const Rect2& p_src_region, const Size2& p_tex_size,bool p_h_flip=false, bool p_v_flip=false );
	//void _draw_quad(const Rect2& p_rect);
	void _process_blur(int times, float inc);

public:

	/* TEXTURE API */

	virtual RID texture_create();
	virtual void texture_allocate(RID p_texture,int p_width, int p_height,Image::Format p_format,uint32_t p_flags=VS::TEXTURE_FLAGS_DEFAULT);
	virtual void texture_set_data(RID p_texture,const Image& p_image,VS::CubeMapSide p_cube_side=VS::CUBEMAP_LEFT);
	virtual Image texture_get_data(RID p_texture,VS::CubeMapSide p_cube_side=VS::CUBEMAP_LEFT) const;
	virtual void texture_set_flags(RID p_texture,uint32_t p_flags);
	virtual uint32_t texture_get_flags(RID p_texture) const;
	virtual Image::Format texture_get_format(RID p_texture) const;
	virtual uint32_t texture_get_width(RID p_texture) const;
	virtual uint32_t texture_get_height(RID p_texture) const;
	virtual bool texture_has_alpha(RID p_texture) const;
	virtual void texture_set_size_override(RID p_texture,int p_width, int p_height);
	virtual void texture_set_reload_hook(RID p_texture,ObjectID p_owner,const StringName& p_function) const;

	/* SHADER API */

	virtual RID shader_create(VS::ShaderMode p_mode=VS::SHADER_MATERIAL);

	virtual void shader_set_mode(RID p_shader,VS::ShaderMode p_mode);
	virtual VS::ShaderMode shader_get_mode(RID p_shader) const;

	virtual void shader_set_code(RID p_shader, const String& p_vertex, const String& p_fragment,const String& p_light,int p_vertex_ofs=0,int p_fragment_ofs=0,int p_light_ofs=0);
	virtual String shader_get_fragment_code(RID p_shader) const;
	virtual String shader_get_vertex_code(RID p_shader) const;
	virtual String shader_get_light_code(RID p_shader) const;

	virtual void shader_get_param_list(RID p_shader, List<PropertyInfo> *p_param_list) const;

	/* COMMON MATERIAL API */

	virtual RID material_create();

	virtual void material_set_shader(RID p_shader_material, RID p_shader);
	virtual RID material_get_shader(RID p_shader_material) const;

	virtual void material_set_param(RID p_material, const StringName& p_param, const Variant& p_value);
	virtual Variant material_get_param(RID p_material, const StringName& p_param) const;

	virtual void material_set_flag(RID p_material, VS::MaterialFlag p_flag,bool p_enabled);
	virtual bool material_get_flag(RID p_material,VS::MaterialFlag p_flag) const;

	virtual void material_set_depth_draw_mode(RID p_material, VS::MaterialDepthDrawMode p_mode);
	virtual VS::MaterialDepthDrawMode material_get_depth_draw_mode(RID p_material) const;

	virtual void material_set_blend_mode(RID p_material,VS::MaterialBlendMode p_mode);
	virtual VS::MaterialBlendMode material_get_blend_mode(RID p_material) const;

	virtual void material_set_line_width(RID p_material,float p_line_width);
	virtual float material_get_line_width(RID p_material) const;

	/* FIXED MATERIAL */

	virtual RID fixed_material_create();

	virtual void fixed_material_set_flag(RID p_material, VS::FixedMaterialFlags p_flag, bool p_enabled);
	virtual bool fixed_material_get_flag(RID p_material, VS::FixedMaterialFlags p_flag) const;

	virtual void fixed_material_set_parameter(RID p_material, VS::FixedMaterialParam p_parameter, const Variant& p_value);
	virtual Variant fixed_material_get_parameter(RID p_material,VS::FixedMaterialParam p_parameter) const;

	virtual void fixed_material_set_texture(RID p_material,VS::FixedMaterialParam p_parameter, RID p_texture);
	virtual RID fixed_material_get_texture(RID p_material,VS::FixedMaterialParam p_parameter) const;

	virtual void fixed_material_set_texcoord_mode(RID p_material,VS::FixedMaterialParam p_parameter, VS::FixedMaterialTexCoordMode p_mode);
	virtual VS::FixedMaterialTexCoordMode fixed_material_get_texcoord_mode(RID p_material,VS::FixedMaterialParam p_parameter) const;

	virtual void fixed_material_set_uv_transform(RID p_material,const Transform3D& p_transform);
	virtual Transform3D fixed_material_get_uv_transform(RID p_material) const;

	virtual void fixed_material_set_point_size(RID p_material,float p_size);
	virtual float fixed_material_get_point_size(RID p_material) const;

	/* MESH API */


	virtual RID mesh_create();

	virtual void mesh_add_surface(RID p_mesh,VS::PrimitiveType p_primitive,const Array& p_arrays,const Array& p_blend_shapes=Array(),bool p_alpha_sort=false);
	virtual Array mesh_get_surface_arrays(RID p_mesh,int p_surface) const;
	virtual Array mesh_get_surface_morph_arrays(RID p_mesh,int p_surface) const;
	virtual void mesh_add_custom_surface(RID p_mesh,const Variant& p_dat);

	virtual void mesh_set_morph_target_count(RID p_mesh,int p_amount);
	virtual int mesh_get_morph_target_count(RID p_mesh) const;

	virtual void mesh_set_morph_target_mode(RID p_mesh,VS::MorphTargetMode p_mode);
	virtual VS::MorphTargetMode mesh_get_morph_target_mode(RID p_mesh) const;

	virtual void mesh_surface_set_material(RID p_mesh, int p_surface, RID p_material,bool p_owned=false);
	virtual RID mesh_surface_get_material(RID p_mesh, int p_surface) const;

	virtual int mesh_surface_get_array_len(RID p_mesh, int p_surface) const;
	virtual int mesh_surface_get_array_index_len(RID p_mesh, int p_surface) const;
	virtual uint32_t mesh_surface_get_format(RID p_mesh, int p_surface) const;
	virtual VS::PrimitiveType mesh_surface_get_primitive_type(RID p_mesh, int p_surface) const;

	virtual void mesh_remove_surface(RID p_mesh,int p_index);
	virtual int mesh_get_surface_count(RID p_mesh) const;

	virtual AABB mesh_get_aabb(RID p_mesh,RID p_skeleton=RID()) const;

	virtual void mesh_set_custom_aabb(RID p_mesh,const AABB& p_aabb);
	virtual AABB mesh_get_custom_aabb(RID p_mesh) const;

	/* MULTIMESH API */

	virtual RID multimesh_create();

	virtual void multimesh_set_instance_count(RID p_multimesh,int p_count);
	virtual int multimesh_get_instance_count(RID p_multimesh) const;

	virtual void multimesh_set_mesh(RID p_multimesh,RID p_mesh);
	virtual void multimesh_set_aabb(RID p_multimesh,const AABB& p_aabb);
	virtual void multimesh_instance_set_transform(RID p_multimesh,int p_index,const Transform3D& p_transform);
	virtual void multimesh_instance_set_color(RID p_multimesh,int p_index,const Color& p_color);

	virtual RID multimesh_get_mesh(RID p_multimesh) const;
	virtual AABB multimesh_get_aabb(RID p_multimesh) const;;

	virtual Transform3D multimesh_instance_get_transform(RID p_multimesh,int p_index) const;
	virtual Color multimesh_instance_get_color(RID p_multimesh,int p_index) const;

	virtual void multimesh_set_visible_instances(RID p_multimesh,int p_visible);
	virtual int multimesh_get_visible_instances(RID p_multimesh) const;

	/* IMMEDIATE API */

	virtual RID immediate_create();
	virtual void immediate_begin(RID p_immediate,VS::PrimitiveType p_rimitive,RID p_texture=RID());
	virtual void immediate_vertex(RID p_immediate,const Vector3& p_vertex);
	virtual void immediate_normal(RID p_immediate,const Vector3& p_normal);
	virtual void immediate_tangent(RID p_immediate,const Plane& p_tangent);
	virtual void immediate_color(RID p_immediate,const Color& p_color);
	virtual void immediate_uv(RID p_immediate,const Vector2& tex_uv);
	virtual void immediate_uv2(RID p_immediate,const Vector2& tex_uv);
	virtual void immediate_end(RID p_immediate);
	virtual void immediate_clear(RID p_immediate);
	virtual AABB immediate_get_aabb(RID p_immediate) const;
	virtual void immediate_set_material(RID p_immediate,RID p_material);
	virtual RID immediate_get_material(RID p_immediate) const;


	/* PARTICLES API */

	virtual RID particles_create();

	virtual void particles_set_amount(RID p_particles, int p_amount);
	virtual int particles_get_amount(RID p_particles) const;

	virtual void particles_set_emitting(RID p_particles, bool p_emitting);
	virtual bool particles_is_emitting(RID p_particles) const;

	virtual void particles_set_visibility_aabb(RID p_particles, const AABB& p_visibility);
	virtual AABB particles_get_visibility_aabb(RID p_particles) const;

	virtual void particles_set_emission_half_extents(RID p_particles, const Vector3& p_half_extents);
	virtual Vector3 particles_get_emission_half_extents(RID p_particles) const;

	virtual void particles_set_emission_base_velocity(RID p_particles, const Vector3& p_base_velocity);
	virtual Vector3 particles_get_emission_base_velocity(RID p_particles) const;

	virtual void particles_set_emission_points(RID p_particles, const DVector<Vector3>& p_points);
	virtual DVector<Vector3> particles_get_emission_points(RID p_particles) const;

	virtual void particles_set_gravity_normal(RID p_particles, const Vector3& p_normal);
	virtual Vector3 particles_get_gravity_normal(RID p_particles) const;

	virtual void particles_set_variable(RID p_particles, VS::ParticleVariable p_variable,float p_value);
	virtual float particles_get_variable(RID p_particles, VS::ParticleVariable p_variable) const;

	virtual void particles_set_randomness(RID p_particles, VS::ParticleVariable p_variable,float p_randomness);
	virtual float particles_get_randomness(RID p_particles, VS::ParticleVariable p_variable) const;

	virtual void particles_set_color_phase_pos(RID p_particles, int p_phase, float p_pos);
	virtual float particles_get_color_phase_pos(RID p_particles, int p_phase) const;

	virtual void particles_set_color_phases(RID p_particles, int p_phases);
	virtual int particles_get_color_phases(RID p_particles) const;

	virtual void particles_set_color_phase_color(RID p_particles, int p_phase, const Color& p_color);
	virtual Color particles_get_color_phase_color(RID p_particles, int p_phase) const;

	virtual void particles_set_attractors(RID p_particles, int p_attractors);
	virtual int particles_get_attractors(RID p_particles) const;

	virtual void particles_set_attractor_pos(RID p_particles, int p_attractor, const Vector3& p_pos);
	virtual Vector3 particles_get_attractor_pos(RID p_particles,int p_attractor) const;

	virtual void particles_set_attractor_strength(RID p_particles, int p_attractor, float p_force);
	virtual float particles_get_attractor_strength(RID p_particles,int p_attractor) const;

	virtual void particles_set_material(RID p_particles, RID p_material,bool p_owned=false);
	virtual RID particles_get_material(RID p_particles) const;

	virtual AABB particles_get_aabb(RID p_particles) const;

	virtual void particles_set_height_from_velocity(RID p_particles, bool p_enable);
	virtual bool particles_has_height_from_velocity(RID p_particles) const;

	virtual void particles_set_use_local_coordinates(RID p_particles, bool p_enable);
	virtual bool particles_is_using_local_coordinates(RID p_particles) const;

	/* SKELETON API */

	virtual RID skeleton_create();
	virtual void skeleton_resize(RID p_skeleton,int p_bones);
	virtual int skeleton_get_bone_count(RID p_skeleton) const;
	virtual void skeleton_bone_set_transform(RID p_skeleton,int p_bone, const Transform3D& p_transform);
	virtual Transform3D skeleton_bone_get_transform(RID p_skeleton,int p_bone);


	/* LIGHT API */

	virtual RID light_create(VS::LightType p_type);
	virtual VS::LightType light_get_type(RID p_light) const;

	virtual void light_set_color(RID p_light,VS::LightColor p_type, const Color& p_color);
	virtual Color light_get_color(RID p_light,VS::LightColor p_type) const;

	virtual void light_set_shadow(RID p_light,bool p_enabled);
	virtual bool light_has_shadow(RID p_light) const;

	virtual void light_set_volumetric(RID p_light,bool p_enabled);
	virtual bool light_is_volumetric(RID p_light) const;

	virtual void light_set_projector(RID p_light,RID p_texture);
	virtual RID light_get_projector(RID p_light) const;

	virtual void light_set_var(RID p_light, VS::LightParam p_var, float p_value);
	virtual float light_get_var(RID p_light, VS::LightParam p_var) const;

	virtual void light_set_operator(RID p_light,VS::LightOp p_op);
	virtual VS::LightOp light_get_operator(RID p_light) const;

	virtual void light_omni_set_shadow_mode(RID p_light,VS::LightOmniShadowMode p_mode);
	virtual VS::LightOmniShadowMode light_omni_get_shadow_mode(RID p_light) const;


	virtual void light_directional_set_shadow_mode(RID p_light,VS::LightDirectionalShadowMode p_mode);
	virtual VS::LightDirectionalShadowMode light_directional_get_shadow_mode(RID p_light) const;
	virtual void light_directional_set_shadow_param(RID p_light,VS::LightDirectionalShadowParam p_param, float p_value);
	virtual float light_directional_get_shadow_param(RID p_light,VS::LightDirectionalShadowParam p_param) const;

	virtual AABB light_get_aabb(RID p_poly) const;


	virtual RID light_instance_create(RID p_light);
	virtual void light_instance_set_transform(RID p_light_instance,const Transform3D& p_transform);

	virtual bool light_instance_has_shadow(RID p_light_instance) const;
	virtual bool light_instance_assign_shadow(RID p_light_instance);
	virtual ShadowType light_instance_get_shadow_type(RID p_light_instance) const;
	virtual int light_instance_get_shadow_passes(RID p_light_instance) const;
	virtual bool light_instance_get_pssm_shadow_overlap(RID p_light_instance) const;
	virtual void light_instance_set_custom_transform(RID p_light_instance, int p_index, const CameraMatrix& p_camera, const Transform3D& p_transform, float p_split_near=0,float p_split_far=0);
	virtual int light_instance_get_shadow_size(RID p_light_instance, int p_index=0) const { return 1; }

	virtual ShadowType light_instance_get_shadow_type(RID p_light_instance,bool p_far=false) const;
	virtual void light_instance_set_shadow_transform(RID p_light_instance, int p_index, const CameraMatrix& p_camera, const Transform3D& p_transform, float p_split_near=0,float p_split_far=0);

	virtual void shadow_clear_near();
	virtual bool shadow_allocate_near(RID p_light);
	virtual bool shadow_allocate_far(RID p_light);


	/* PARTICLES INSTANCE */

	virtual RID particles_instance_create(RID p_particles);
	virtual void particles_instance_set_transform(RID p_particles_instance,const Transform3D& p_transform);

	/* VIEWPORT */

	virtual RID viewport_data_create();

	virtual RID render_target_create();
	virtual void render_target_set_size(RID p_render_target, int p_width, int p_height);
	virtual RID render_target_get_texture(RID p_render_target) const;
	virtual bool render_target_renedered_in_frame(RID p_render_target);

	/* RENDER API */
	/* all calls (inside begin/end shadow) are always warranted to be in the following order: */

	virtual void begin_frame();

	virtual void set_viewport(const VS::ViewportRect& p_viewport);
	virtual void set_render_target(RID p_render_target,bool p_transparent_bg=false,bool p_vflip=false);
	virtual void clear_viewport(const Color& p_color);
	virtual void capture_viewport(Image* r_capture);


	virtual void begin_scene(RID p_viewport_data,RID p_env,VS::ScenarioDebugMode p_debug);
	virtual void begin_shadow_map( RID p_light_instance, int p_shadow_pass );

	virtual void set_camera(const Transform3D& p_world,const CameraMatrix& p_projection);

	virtual void add_light( RID p_light_instance ); ///< all "add_light" calls happen before add_geometry calls


	virtual void add_mesh( const RID& p_mesh, const InstanceData *p_data);
	virtual void add_multimesh( const RID& p_multimesh, const InstanceData *p_data);
	virtual void add_immediate( const RID& p_immediate, const InstanceData *p_data) {}
	virtual void add_particles( const RID& p_particle_instance, const InstanceData *p_data);

	virtual void end_scene();
	virtual void end_shadow_map();

	virtual void end_frame();

	/* CANVAS API */

	virtual void canvas_begin();
	virtual void canvas_disable_blending();
	virtual void canvas_set_opacity(float p_opacity);
	virtual void canvas_set_blend_mode(VS::MaterialBlendMode p_mode);
	virtual void canvas_begin_rect(const Transform2D& p_transform);
	virtual void canvas_set_clip(bool p_clip, const Rect2& p_rect);
	virtual void canvas_end_rect();
	virtual void canvas_draw_line(const Point2& p_from, const Point2& p_to,const Color& p_color,float p_width);
	virtual void canvas_draw_rect(const Rect2& p_rect, int p_flags, const Rect2& p_source,RID p_texture,const Color& p_modulate);
	virtual void canvas_draw_style_box(const Rect2& p_rect, RID p_texture,const float *p_margins, bool p_draw_center=true,const Color& p_modulate=Color(1,1,1));
	virtual void canvas_draw_primitive(const Vector<Point2>& p_points, const Vector<Color>& p_colors,const Vector<Point2>& p_uvs, RID p_texture,float p_width);
	virtual void canvas_draw_polygon(int p_vertex_count, const int* p_indices, const Vector2* p_vertices, const Vector2* p_uvs, const Color* p_colors,const RID& p_texture,bool p_singlecolor);
	virtual void canvas_set_transform(const Transform2D& p_transform);

	/* FX */

	virtual RID fx_create();
	virtual void fx_get_effects(RID p_fx,List<String> *p_effects) const;
	virtual void fx_set_active(RID p_fx,const String& p_effect, bool p_active);
	virtual bool fx_is_active(RID p_fx,const String& p_effect) const;
	virtual void fx_get_effect_params(RID p_fx,const String& p_effect,List<PropertyInfo> *p_params) const;
	virtual Variant fx_get_effect_param(RID p_fx,const String& p_effect,const String& p_param) const;
	virtual void fx_set_effect_param(RID p_fx,const String& p_effect, const String& p_param, const Variant& p_pvalue);

	/* ENVIRONMENT */

	virtual RID environment_create();

	virtual void environment_set_background(RID p_env,VS::EnvironmentBG p_bg);
	virtual VS::EnvironmentBG environment_get_background(RID p_env) const;

	virtual void environment_set_background_param(RID p_env,VS::EnvironmentBGParam p_param, const Variant& p_value);
	virtual Variant environment_get_background_param(RID p_env,VS::EnvironmentBGParam p_param) const;

	virtual void environment_set_group(RID p_env,VS::Group p_group, const Variant& p_param);
	virtual Variant environment_get_group(RID p_env, VS::Group p_param) const;
	
	virtual void environment_set_enable_fx(RID p_env,VS::EnvironmentFx p_effect,bool p_enabled);
	virtual bool environment_is_fx_enabled(RID p_env,VS::EnvironmentFx p_effect) const;

	virtual void environment_fx_set_param(RID p_env,VS::EnvironmentFxParam p_param,const Variant& p_value);
	virtual Variant environment_fx_get_param(RID p_env,VS::EnvironmentFxParam p_param) const;

	/* SAMPLED LIGHT */
	virtual RID sampled_light_dp_create(int p_width,int p_height);
	virtual void sampled_light_dp_update(RID p_sampled_light,const Color *p_data,float p_multiplier);


	/*MISC*/

	virtual bool is_texture(const RID& p_rid) const;
	virtual bool is_material(const RID& p_rid) const;
	virtual bool is_mesh(const RID& p_rid) const;
	virtual bool is_multimesh(const RID& p_rid) const;
	virtual bool is_immediate(const RID& p_rid) const;
	virtual bool is_particles(const RID &p_beam) const;

	virtual bool is_light(const RID& p_rid) const;
	virtual bool is_light_instance(const RID& p_rid) const;
	virtual bool is_particles_instance(const RID& p_rid) const;
	virtual bool is_skeleton(const RID& p_rid) const;
	virtual bool is_environment(const RID& p_rid) const;
	virtual bool is_fx(const RID& p_rid) const;
	virtual bool is_shader(const RID& p_rid) const;

	virtual void free(const RID& p_rid);

	virtual void custom_shade_model_set_shader(int p_model, RID p_shader);
	virtual RID custom_shade_model_get_shader(int p_model) const;
	virtual void custom_shade_model_set_name(int p_model, const String& p_name);
	virtual String custom_shade_model_get_name(int p_model) const;
	virtual void custom_shade_model_set_param_info(int p_model, const List<PropertyInfo>& p_info);
	virtual void custom_shade_model_get_param_info(int p_model, List<PropertyInfo>* p_info) const;


	virtual void init();
	virtual void finish();

	virtual int get_render_info(VS::RenderInfo p_info);

	void reload_vram();

	virtual bool needs_to_draw_next_frame() const;

	virtual bool has_feature(VS::Features p_feature) const;


#ifdef TOOLS_ENABLED
	RasterizerGLES1(bool p_keep_copies=true,bool p_use_reload_hooks=false);
#else
	RasterizerGLES1(bool p_keep_copies=false,bool p_use_reload_hooks=false);
#endif
	virtual ~RasterizerGLES1();
};

#endif
#endif
