#ifndef RASTERIZER_GX_H
#define RASTERIZER_GX_H

//I LIED! This is a VisualServer impl, not a Rasterizer :trollface:

#include "servers/visual_server.h"
#include <stdint.h>
#include <gccore.h>

class VisualServerGX : public VisualServer {
private:
	void *gp_fifo;

	static RID_Owner<GXTexObj> texture_owner;


public:
	virtual void set_mipmap_policy(MipMapPolicy p_policy);
	virtual MipMapPolicy get_mipmap_policy() const;

	/* TEXTURE API */

	virtual RID texture_create();
	RID texture_create_from_image(const Image& p_image,uint32_t p_flags=TEXTURE_FLAGS_DEFAULT); // helper
	virtual void texture_allocate(RID p_texture,int p_width, int p_height,Image::Format p_format,uint32_t p_flags=TEXTURE_FLAGS_DEFAULT);
	virtual void texture_set_data(RID p_texture,const Image& p_image,CubeMapSide p_cube_side=CUBEMAP_LEFT);
	virtual Image texture_get_data(RID p_texture,CubeMapSide p_cube_side=CUBEMAP_LEFT) const;
	virtual void texture_set_flags(RID p_texture,uint32_t p_flags) ;
	virtual uint32_t texture_get_flags(RID p_texture) const;
	virtual Image::Format texture_get_format(RID p_texture) const;
	virtual uint32_t texture_get_width(RID p_texture) const;
	virtual uint32_t texture_get_height(RID p_texture) const;
	virtual void texture_set_size_override(RID p_texture,int p_width, int p_height);
	virtual bool texture_can_stream(RID p_texture) const;
	virtual void texture_set_reload_hook(RID p_texture,ObjectID p_owner,const StringName& p_function) const;

	/* SHADER API */

	virtual RID shader_create(ShaderMode p_mode=SHADER_MATERIAL);

	virtual void shader_set_mode(RID p_shader,ShaderMode p_mode);
	virtual ShaderMode shader_get_mode(RID p_shader) const;

	virtual void shader_set_code(RID p_shader, const String& p_vertex, const String& p_fragment,const String& p_light, int p_vertex_ofs=0,int p_fragment_ofs=0,int p_light_ofs=0);
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

	virtual void material_set_flag(RID p_material, MaterialFlag p_flag,bool p_enabled);
	virtual bool material_get_flag(RID p_material,MaterialFlag p_flag) const;

	virtual void material_set_depth_draw_mode(RID p_material, MaterialDepthDrawMode p_mode);
	virtual MaterialDepthDrawMode material_get_depth_draw_mode(RID p_material) const;

	virtual void material_set_blend_mode(RID p_material,MaterialBlendMode p_mode);
	virtual MaterialBlendMode material_get_blend_mode(RID p_material) const;

	virtual void material_set_line_width(RID p_material,float p_line_width);
	virtual float material_get_line_width(RID p_material) const;


	//fixed material api

	virtual RID fixed_material_create();

	virtual void fixed_material_set_flag(RID p_material, FixedMaterialFlags p_flag, bool p_enabled);
	virtual bool fixed_material_get_flag(RID p_material, FixedMaterialFlags p_flag) const;

	virtual void fixed_material_set_param(RID p_material, FixedMaterialParam p_parameter, const Variant& p_value);
	virtual Variant fixed_material_get_param(RID p_material,FixedMaterialParam p_parameter) const;

	virtual void fixed_material_set_texture(RID p_material,FixedMaterialParam p_parameter, RID p_texture);
	virtual RID fixed_material_get_texture(RID p_material,FixedMaterialParam p_parameter) const;

	virtual void fixed_material_set_light_shader(RID p_material,FixedMaterialLightShader p_shader);
	virtual FixedMaterialLightShader fixed_material_get_light_shader(RID p_material) const;

	virtual void fixed_material_set_texcoord_mode(RID p_material,FixedMaterialParam p_parameter, FixedMaterialTexCoordMode p_mode);
	virtual FixedMaterialTexCoordMode fixed_material_get_texcoord_mode(RID p_material,FixedMaterialParam p_parameter) const;

	virtual void fixed_material_set_uv_transform(RID p_material,const Transform3D& p_transform);
	virtual Transform3D fixed_material_get_uv_transform(RID p_material) const;

	virtual void fixed_material_set_point_size(RID p_material,float p_size);
	virtual float fixed_material_get_point_size(RID p_material) const;

	/* MESH API */

	virtual RID mesh_create();

	virtual void mesh_add_surface(RID p_mesh,PrimitiveType p_primitive,const Array& p_arrays,const Array& p_blend_shapes=Array(),bool p_alpha_sort=false);
	virtual Array mesh_get_surface_arrays(RID p_mesh,int p_surface) const;
	virtual Array mesh_get_surface_morph_arrays(RID p_mesh,int p_surface) const;


	virtual void mesh_add_custom_surface(RID p_mesh,const Variant& p_dat); //this is used by each platform in a different way
	virtual void mesh_set_morph_target_count(RID p_mesh,int p_amount);
	virtual int mesh_get_morph_target_count(RID p_mesh) const;

	virtual void mesh_set_morph_target_mode(RID p_mesh,MorphTargetMode p_mode);
	virtual MorphTargetMode mesh_get_morph_target_mode(RID p_mesh) const;

	virtual void mesh_surface_set_material(RID p_mesh, int p_surface, RID p_material,bool p_owned=false);
	virtual RID mesh_surface_get_material(RID p_mesh, int p_surface) const;

	virtual int mesh_surface_get_array_len(RID p_mesh, int p_surface) const;
	virtual int mesh_surface_get_array_index_len(RID p_mesh, int p_surface) const;
	virtual uint32_t mesh_surface_get_format(RID p_mesh, int p_surface) const;
	virtual PrimitiveType mesh_surface_get_primitive_type(RID p_mesh, int p_surface) const;

	virtual void mesh_remove_surface(RID p_mesh,int p_index);
	virtual int mesh_get_surface_count(RID p_mesh) const;

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
	virtual AABB multimesh_get_aabb(RID p_multimesh,const AABB& p_aabb) const;

	virtual Transform3D multimesh_instance_get_transform(RID p_multimesh,int p_index) const;
	virtual Color multimesh_instance_get_color(RID p_multimesh,int p_index) const;

	virtual void multimesh_set_visible_instances(RID p_multimesh,int p_visible);
	virtual int multimesh_get_visible_instances(RID p_multimesh) const;

	/* IMMEDIATE API */

	virtual RID immediate_create();
	virtual void immediate_begin(RID p_immediate,PrimitiveType p_rimitive,RID p_texture=RID());
	virtual void immediate_vertex(RID p_immediate,const Vector3& p_vertex);
	virtual void immediate_normal(RID p_immediate,const Vector3& p_normal);
	virtual void immediate_tangent(RID p_immediate,const Plane& p_tangent);
	virtual void immediate_color(RID p_immediate,const Color& p_color);
	virtual void immediate_uv(RID p_immediate,const Vector2& tex_uv);
	virtual void immediate_uv2(RID p_immediate,const Vector2& tex_uv);
	virtual void immediate_end(RID p_immediate);
	virtual void immediate_clear(RID p_immediate);
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

	virtual void particles_set_variable(RID p_particles, ParticleVariable p_variable,float p_value);
	virtual float particles_get_variable(RID p_particles, ParticleVariable p_variable) const;

	virtual void particles_set_randomness(RID p_particles, ParticleVariable p_variable,float p_randomness);
	virtual float particles_get_randomness(RID p_particles, ParticleVariable p_variable) const;

	virtual void particles_set_color_phases(RID p_particles, int p_phases);
	virtual int particles_get_color_phases(RID p_particles) const;

	virtual void particles_set_color_phase_pos(RID p_particles, int p_phase, float p_pos);
	virtual float particles_get_color_phase_pos(RID p_particles, int p_phase) const;

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

	virtual void particles_set_height_from_velocity(RID p_particles, bool p_enable);
	virtual bool particles_has_height_from_velocity(RID p_particles) const;

	virtual void particles_set_use_local_coordinates(RID p_particles, bool p_enable);
	virtual bool particles_is_using_local_coordinates(RID p_particles) const;

	/* Light API */

	virtual RID light_create(LightType p_type);
	virtual LightType light_get_type(RID p_light) const;

	virtual void light_set_color(RID p_light,LightColor p_type, const Color& p_color);
	virtual Color light_get_color(RID p_light,LightColor p_type) const;

	virtual void light_set_shadow(RID p_light,bool p_enabled);
	virtual bool light_has_shadow(RID p_light) const;

	virtual void light_set_volumetric(RID p_light,bool p_enabled);
	virtual bool light_is_volumetric(RID p_light) const;

	virtual void light_set_projector(RID p_light,RID p_texture);
	virtual RID light_get_projector(RID p_light) const;

	virtual void light_set_param(RID p_light, LightParam p_var, float p_value);
	virtual float light_get_param(RID p_light, LightParam p_var) const;

	virtual void light_set_operator(RID p_light,LightOp p_op);
	virtual LightOp light_get_operator(RID p_light) const;

	// omni light

	virtual void light_omni_set_shadow_mode(RID p_light,LightOmniShadowMode p_mode);
	virtual LightOmniShadowMode light_omni_get_shadow_mode(RID p_light) const;

	// directional light

	virtual void light_directional_set_shadow_mode(RID p_light,LightDirectionalShadowMode p_mode);
	virtual LightDirectionalShadowMode light_directional_get_shadow_mode(RID p_light) const;

	virtual void light_directional_set_shadow_param(RID p_light,LightDirectionalShadowParam p_param, float p_value);
	virtual float light_directional_get_shadow_param(RID p_light,LightDirectionalShadowParam p_param) const;

	//@TODO fallof model and all that stuff

	/* SKELETON API */

	virtual RID skeleton_create();
	virtual void skeleton_resize(RID p_skeleton,int p_bones);
	virtual int skeleton_get_bone_count(RID p_skeleton) const;
	virtual void skeleton_bone_set_transform(RID p_skeleton,int p_bone, const Transform3D& p_transform);
	virtual Transform3D skeleton_bone_get_transform(RID p_skeleton,int p_bone);

	/* ROOM API */

	virtual RID room_create();
	virtual void room_set_bounds(RID p_room, const BSP_Tree& p_bounds);
	virtual BSP_Tree room_get_bounds(RID p_room) const;

	/* PORTAL API */

	// portals are only (x/y) points, forming a convex shape, which its clockwise
	// order points outside. (z is 0);

	virtual RID portal_create();
	virtual void portal_set_shape(RID p_portal, const Vector<Point2>& p_shape);
	virtual Vector<Point2> portal_get_shape(RID p_portal) const;
	virtual void portal_set_enabled(RID p_portal, bool p_enabled);
	virtual bool portal_is_enabled(RID p_portal) const;
	virtual void portal_set_disable_distance(RID p_portal, float p_distance);
	virtual float portal_get_disable_distance(RID p_portal) const;
	virtual void portal_set_disabled_color(RID p_portal, const Color& p_color);
	virtual Color portal_get_disabled_color(RID p_portal) const;
	virtual void portal_set_connect_range(RID p_portal, float p_range) ;
	virtual float portal_get_connect_range(RID p_portal) const ;


	/* BAKED LIGHT API */

	virtual RID baked_light_create();

	virtual void baked_light_set_mode(RID p_baked_light,BakedLightMode p_mode);
	virtual BakedLightMode baked_light_get_mode(RID p_baked_light) const;

	virtual void baked_light_set_octree(RID p_baked_light,const DVector<uint8_t> p_octree);
	virtual DVector<uint8_t> baked_light_get_octree(RID p_baked_light) const;

	virtual void baked_light_set_light(RID p_baked_light,const DVector<uint8_t> p_light);
	virtual DVector<uint8_t> baked_light_get_light(RID p_baked_light) const;

	virtual void baked_light_set_sampler_octree(RID p_baked_light,const DVector<int> &p_sampler);
	virtual DVector<int> baked_light_get_sampler_octree(RID p_baked_light) const;

	virtual void baked_light_set_lightmap_multiplier(RID p_baked_light,float p_multiplier);
	virtual float baked_light_get_lightmap_multiplier(RID p_baked_light) const;

	virtual void baked_light_add_lightmap(RID p_baked_light,const RID p_texture,int p_id);
	virtual void baked_light_clear_lightmaps(RID p_baked_light);

	/* BAKED LIGHT SAMPLER */

	virtual RID baked_light_sampler_create();

	virtual void baked_light_sampler_set_param(RID p_baked_light_sampler,BakedLightSamplerParam p_param,float p_value);
	virtual float baked_light_sampler_get_param(RID p_baked_light_sampler,BakedLightSamplerParam p_param) const;

	virtual void baked_light_sampler_set_resolution(RID p_baked_light_sampler,int p_resolution);
	virtual int baked_light_sampler_get_resolution(RID p_baked_light_sampler) const;

	/* CAMERA API */

	virtual RID camera_create();
	virtual void camera_set_perspective(RID p_camera,float p_fovy_degrees, float p_z_near, float p_z_far);
	virtual void camera_set_orthogonal(RID p_camera,float p_size, float p_z_near, float p_z_far);
	virtual void camera_set_transform(RID p_camera,const Transform3D& p_transform);

	virtual void camera_set_visible_layers(RID p_camera,uint32_t p_layers);
	virtual uint32_t camera_get_visible_layers(RID p_camera) const;

	virtual void camera_set_environment(RID p_camera,RID p_env);
	virtual RID camera_get_environment(RID p_camera) const;

	virtual void camera_set_use_vertical_aspect(RID p_camera,bool p_enable);
	virtual bool camera_is_using_vertical_aspect(RID p_camera,bool p_enable) const;

	/* VIEWPORT API */

	virtual RID viewport_create();

	virtual void viewport_attach_to_screen(RID p_viewport,int p_screen=0);
	virtual void viewport_detach(RID p_viewport);
	virtual void viewport_set_render_target_to_screen_rect(RID p_viewport,const Rect2& p_rect);

	virtual void viewport_set_as_render_target(RID p_viewport,bool p_enable);
	virtual void viewport_set_render_target_update_mode(RID p_viewport,RenderTargetUpdateMode p_mode);
	virtual RenderTargetUpdateMode viewport_get_render_target_update_mode(RID p_viewport) const;
	virtual RID viewport_get_render_target_texture(RID p_viewport) const;
	virtual void viewport_set_render_target_vflip(RID p_viewport,bool p_enable);
	virtual bool viewport_get_render_target_vflip(RID p_viewport) const;

	virtual void viewport_queue_screen_capture(RID p_viewport);
	virtual Image viewport_get_screen_capture(RID p_viewport) const;

	virtual void viewport_set_rect(RID p_viewport,const ViewportRect& p_rect);
	virtual ViewportRect viewport_get_rect(RID p_viewport) const;

	virtual void viewport_set_hide_scenario(RID p_viewport,bool p_hide);
	virtual void viewport_set_hide_canvas(RID p_viewport,bool p_hide);

	virtual void viewport_attach_camera(RID p_viewport,RID p_camera);
	virtual void viewport_set_scenario(RID p_viewport,RID p_scenario);
	virtual RID viewport_get_attached_camera(RID  p_viewport) const;
	virtual RID viewport_get_scenario(RID  p_viewport) const;
	virtual void viewport_attach_canvas(RID p_viewport,RID p_canvas);
	virtual void viewport_remove_canvas(RID p_viewport,RID p_canvas);
	virtual void viewport_set_canvas_transform(RID p_viewport,RID p_canvas,const Transform2D& p_offset);
	virtual Transform2D viewport_get_canvas_transform(RID p_viewport,RID p_canvas) const;
	virtual void viewport_set_transparent_background(RID p_viewport,bool p_enabled);
	virtual bool viewport_has_transparent_background(RID p_viewport) const;


	virtual void viewport_set_global_canvas_transform(RID p_viewport,const Transform2D& p_transform);
	virtual Transform2D viewport_get_global_canvas_transform(RID p_viewport) const;
	virtual void viewport_set_canvas_layer(RID p_viewport,RID p_canvas,int p_layer);



	/* ENVIRONMENT API */

	virtual RID environment_create();

	virtual void environment_set_background(RID p_env,EnvironmentBG p_bg);
	virtual EnvironmentBG environment_get_background(RID p_env) const;

	virtual void environment_set_group(RID p_env,Group p_group, const Variant& p_param);
	virtual Variant environment_get_group(RID p_env, Group p_group) const;

	virtual void environment_set_background_param(RID p_env,EnvironmentBGParam p_param, const Variant& p_value);
	virtual Variant environment_get_background_param(RID p_env,EnvironmentBGParam p_param) const;

	virtual void environment_set_enable_fx(RID p_env,EnvironmentFx p_effect,bool p_enabled);
	virtual bool environment_is_fx_enabled(RID p_env,EnvironmentFx p_mode) const;

	virtual void environment_fx_set_param(RID p_env,EnvironmentFxParam p_effect,const Variant& p_param);
	virtual Variant environment_fx_get_param(RID p_env,EnvironmentFxParam p_effect) const;


	/* SCENARIO API */

	virtual RID scenario_create();

	virtual void scenario_set_debug(RID p_scenario,ScenarioDebugMode p_debug_mode);
	virtual void scenario_set_environment(RID p_scenario, RID p_environment);
	virtual RID scenario_get_environment(RID p_scenario, RID p_environment) const;
	virtual void scenario_set_fallback_environment(RID p_scenario, RID p_environment);


	/* INSTANCING API */

	virtual RID instance_create2(RID p_base, RID p_scenario);

//	virtual RID instance_create(RID p_base,RID p_scenario); // from can be mesh, light,  area and portal so far.
	virtual RID instance_create(); // from can be mesh, light, poly, area and portal so far.

	virtual void instance_set_base(RID p_instance, RID p_base); // from can be mesh, light, poly, area and portal so far.
	virtual RID instance_get_base(RID p_instance) const;

	virtual void instance_set_scenario(RID p_instance, RID p_scenario); // from can be mesh, light, poly, area and portal so far.
	virtual RID instance_get_scenario(RID p_instance) const;

	virtual void instance_set_layer_mask(RID p_instance, uint32_t p_mask);
	virtual uint32_t instance_get_layer_mask(RID p_instance) const;

	virtual AABB instance_get_base_aabb(RID p_instance) const;

	virtual void instance_set_transform(RID p_instance, const Transform3D& p_transform);
	virtual Transform3D instance_get_transform(RID p_instance) const;


	virtual void instance_attach_object_instance_ID(RID p_instance,uint32_t p_ID);
	virtual uint32_t instance_get_object_instance_ID(RID p_instance) const;

	virtual void instance_set_morph_target_weight(RID p_instance,int p_shape, float p_weight);
	virtual float instance_get_morph_target_weight(RID p_instance,int p_shape) const;

	virtual void instance_attach_skeleton(RID p_instance,RID p_skeleton);
	virtual RID instance_get_skeleton(RID p_instance) const;

	virtual void instance_set_exterior( RID p_instance, bool p_enabled );
	virtual bool instance_is_exterior( RID p_instance) const;

	virtual void instance_set_room( RID p_instance, RID p_room );
	virtual RID instance_get_room( RID p_instance ) const ;

	virtual void instance_set_extra_visibility_margin( RID p_instance, real_t p_margin );
	virtual real_t instance_get_extra_visibility_margin( RID p_instance ) const ;

	// don't use these in a game!
	virtual Vector<RID> instances_cull_aabb(const AABB& p_aabb, RID p_scenario=RID()) const;
	virtual Vector<RID> instances_cull_ray(const Vector3& p_from, const Vector3& p_to, RID p_scenario=RID()) const;
	virtual Vector<RID> instances_cull_convex(const Vector<Plane>& p_convex, RID p_scenario=RID()) const;

	virtual void instance_geometry_set_flag(RID p_instance,InstanceFlags p_flags,bool p_enabled);
	virtual bool instance_geometry_get_flag(RID p_instance,InstanceFlags p_flags) const;

	virtual void instance_geometry_set_material_override(RID p_instance, RID p_material);
	virtual RID instance_geometry_get_material_override(RID p_instance) const;

	virtual void instance_geometry_set_draw_range(RID p_instance,float p_min,float p_max);
	virtual float instance_geometry_get_draw_range_max(RID p_instance) const;
	virtual float instance_geometry_get_draw_range_min(RID p_instance) const;

	virtual void instance_geometry_set_baked_light(RID p_instance,RID p_baked_light);
	virtual RID instance_geometry_get_baked_light(RID p_instance) const;

	virtual void instance_geometry_set_baked_light_sampler(RID p_instance,RID p_baked_light_sampler);
	virtual RID instance_geometry_get_baked_light_sampler(RID p_instance) const;

	virtual void instance_geometry_set_baked_light_texture_index(RID p_instance,int p_tex_id);
	virtual int instance_geometry_get_baked_light_texture_index(RID p_instance) const;

	virtual void instance_light_set_enabled(RID p_instance,bool p_enabled);
	virtual bool instance_light_is_enabled(RID p_instance) const;

	/* CANVAS (2D) */

	virtual RID canvas_create();
	virtual void canvas_set_item_mirroring(RID p_canvas,RID p_item,const Point2& p_mirroring);
	virtual Point2 canvas_get_item_mirroring(RID p_canvas,RID p_item) const;


	virtual RID canvas_item_create();
	virtual void canvas_item_set_parent(RID p_item,RID p_parent);
	virtual RID canvas_item_get_parent(RID p_canvas_item) const;

	virtual void canvas_item_set_visible(RID p_item,bool p_visible);
	virtual bool canvas_item_is_visible(RID p_item) const;

	virtual void canvas_item_set_blend_mode(RID p_canvas_item,MaterialBlendMode p_blend);

	virtual void canvas_item_attach_viewport(RID p_item, RID p_viewport);

	//virtual void canvas_item_set_rect(RID p_item, const Rect2& p_rect);

	virtual void canvas_item_set_transform(RID p_item, const Transform2D& p_transform);
	virtual void canvas_item_set_clip(RID p_item, bool p_clip);
	virtual void canvas_item_set_custom_rect(RID p_item, bool p_custom_rect,const Rect2& p_rect=Rect2());
	virtual void canvas_item_set_opacity(RID p_item, float p_opacity);
	virtual float canvas_item_get_opacity(RID p_item, float p_opacity) const;

	virtual void canvas_item_set_self_opacity(RID p_item, float p_self_opacity);
	virtual float canvas_item_get_self_opacity(RID p_item, float p_self_opacity) const;

	virtual void canvas_item_set_on_top(RID p_item, bool p_on_top);
	virtual bool canvas_item_is_on_top(RID p_item) const;

	virtual void canvas_item_add_line(RID p_item, const Point2& p_from, const Point2& p_to,const Color& p_color,float p_width=1.0);
	virtual void canvas_item_add_rect(RID p_item, const Rect2& p_rect, const Color& p_color);
	virtual void canvas_item_add_circle(RID p_item, const Point2& p_pos, float p_radius,const Color& p_color);
	virtual void canvas_item_add_texture_rect(RID p_item, const Rect2& p_rect, RID p_texture,bool p_tile=false,const Color& p_modulate=Color(1,1,1));
	virtual void canvas_item_add_texture_rect_region(RID p_item, const Rect2& p_rect, RID p_texture,const Rect2& p_src_rect,const Color& p_modulate=Color(1,1,1));
	virtual void canvas_item_add_style_box(RID p_item, const Rect2& p_rect, RID p_texture,const Vector2& p_topleft, const Vector2& p_bottomright, bool p_draw_center=true,const Color& p_modulate=Color(1,1,1));
	virtual void canvas_item_add_primitive(RID p_item, const Vector<Point2>& p_points, const Vector<Color>& p_colors,const Vector<Point2>& p_uvs, RID p_texture,float p_width=1.0);
	virtual void canvas_item_add_polygon(RID p_item, const Vector<Point2>& p_points, const Vector<Color>& p_colors,const Vector<Point2>& p_uvs=Vector<Point2>(), RID p_texture=RID());
	virtual void canvas_item_add_triangle_array(RID p_item, const Vector<int>& p_indices, const Vector<Point2>& p_points, const Vector<Color>& p_colors,const Vector<Point2>& p_uvs=Vector<Point2>(), RID p_texture=RID(), int p_count=-1);
	virtual void canvas_item_add_triangle_array_ptr(RID p_item, int p_count, const int* p_indices, const Point2* p_points, const Color* p_colors,const Point2* p_uvs=NULL, RID p_texture=RID());
	virtual void canvas_item_add_set_transform(RID p_item,const Transform2D& p_transform);
	virtual void canvas_item_add_set_blend_mode(RID p_item, MaterialBlendMode p_blend);
	virtual void canvas_item_add_clip_ignore(RID p_item, bool p_ignore);
	virtual void canvas_item_set_sort_children_by_y(RID p_item, bool p_enable);

	virtual void canvas_item_clear(RID p_item);
	virtual void canvas_item_raise(RID p_item);

	/* CURSOR */
	virtual void cursor_set_rotation(float p_rotation, int p_cursor = 0); // radians
	virtual void cursor_set_texture(RID p_texture, const Point2 &p_center_offset = Point2(0, 0), int p_cursor=0);
	virtual void cursor_set_visible(bool p_visible, int p_cursor = 0);
	virtual void cursor_set_pos(const Point2& p_pos, int p_cursor = 0);

	/* BLACK BARS */


	virtual void black_bars_set_margins(int p_left, int p_top, int p_right, int p_bottom);
	virtual void black_bars_set_images(RID p_left, RID p_top, RID p_right, RID p_bottom);


	/* FREE */

	virtual void free( RID p_rid ); ///< free RIDs associated with the visual server

	/* CUSTOM SHADING */

	virtual void custom_shade_model_set_shader(int p_model, RID p_shader);
	virtual RID custom_shade_model_get_shader(int p_model) const;
	virtual void custom_shade_model_set_name(int p_model, const String& p_name);
	virtual String custom_shade_model_get_name(int p_model) const;
	virtual void custom_shade_model_set_param_info(int p_model, const List<PropertyInfo>& p_info);
	virtual void custom_shade_model_get_param_info(int p_model, List<PropertyInfo>* p_info) const;

	/* EVENT QUEUING */

	virtual void draw();
	virtual void flush();
	virtual bool has_changed() const;
	virtual void init();
	virtual void finish();

	/* STATUS INFORMATION */

	virtual int get_render_info(RenderInfo p_info);

	/* Materials for 2D on 3D */

	RID material_2d_get(bool p_shaded, bool p_transparent, bool p_cut_alpha,bool p_opaque_prepass);

	/* TESTING */

	virtual RID get_test_cube();

	virtual RID get_test_texture();
	virtual RID get_white_texture();

	virtual RID make_sphere_mesh(int p_lats,int p_lons,float p_radius);

	virtual void mesh_add_surface_from_mesh_data( RID p_mesh, const Geometry::MeshData& p_mesh_data);
	virtual void mesh_add_surface_from_planes( RID p_mesh, const DVector<Plane>& p_planes);

	virtual void set_boot_image(const Image& p_image, const Color& p_color);
	virtual void set_default_clear_color(const Color& p_color);

	virtual bool has_feature(Features p_feature) const;
};

#endif
