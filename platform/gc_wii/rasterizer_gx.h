#ifndef RASTERIZER_GX_H
#define RASTERIZER_GX_H

#include "servers/visual/rasterizer.h"

class RasterizerGX : public Rasterizer {


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

	/* BAKED LIGHT */

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

	virtual ShadowType light_instance_get_shadow_type(RID p_light_instance,bool p_far=false) const;
	virtual int light_instance_get_shadow_passes(RID p_light_instance) const;
	virtual void light_instance_set_shadow_transform(RID p_light_instance, int p_index, const CameraMatrix& p_camera, const Transform3D& p_transform, float p_split_near=0,float p_split_far=0);
	virtual int light_instance_get_shadow_size(RID p_light_instance, int p_index=0) const;
	virtual bool light_instance_get_pssm_shadow_overlap(RID p_light_instance) const;

	/* SHADOWS */

	virtual void shadow_clear_near();
	virtual bool shadow_allocate_near(RID p_light); //true on successful alloc
	virtual bool shadow_allocate_far(RID p_light); //true on successful alloc

	/* PARTICLES INSTANCE */

	virtual RID particles_instance_create(RID p_particles);
	virtual void particles_instance_set_transform(RID p_particles_instance,const Transform3D& p_transform);

	/* RENDER API */
	/* all calls (inside begin/end shadow) are always warranted to be in the following order: */

	/* VIEWPORT API */

	virtual RID viewport_data_create();

	virtual RID render_target_create();
	virtual void render_target_set_size(RID p_render_target, int p_width, int p_height);
	virtual RID render_target_get_texture(RID p_render_target) const;
	virtual bool render_target_renedered_in_frame(RID p_render_target);

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
	virtual void add_immediate( const RID& p_immediate, const InstanceData *p_data);
	virtual void add_particles( const RID& p_particle_instance, const InstanceData *p_data);

	virtual void end_scene();
	virtual void end_shadow_map();

	virtual void end_frame();

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

	/* ENVIRONMENT */


	virtual RID environment_create();

	virtual void environment_set_background(RID p_env,VS::EnvironmentBG p_bg);
	virtual VS::EnvironmentBG environment_get_background(RID p_env) const;

	virtual void environment_set_background_param(RID p_env,VS::EnvironmentBGParam p_param, const Variant& p_value);
	virtual Variant environment_get_background_param(RID p_env,VS::EnvironmentBGParam p_param) const;

	virtual void environment_set_group(RID p_env,VS::Group p_param, const Variant& p_value);
	virtual Variant environment_get_group(RID p_env,VS::Group p_param) const;

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
	virtual bool is_shader(const RID& p_rid) const;

	virtual void free(const RID& p_rid);

	virtual void init();
	virtual void finish();

	virtual bool needs_to_draw_next_frame() const;

	virtual void reload_vram();

	virtual bool has_feature(VS::Features p_feature) const;


	virtual int get_render_info(VS::RenderInfo p_info);
};

#endif
