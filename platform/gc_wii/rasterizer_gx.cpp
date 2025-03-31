#include "rasterizer_gx.h"

#include <malloc.h>

#define DEFAULT_FIFO_SIZE	(256*1024)

//A good bit of this code is based on the examples included with libogc2

void VisualServerGX::set_mipmap_policy(MipMapPolicy p_policy){
	WARN_PRINT("Changing mipmap policy on GX is not supported by hardware!");
}

VisualServer::MipMapPolicy VisualServerGX::get_mipmap_policy() const {
	return MIPMAPS_ENABLED_FOR_PO2;
}

RID VisualServerGX::texture_create(){
    return texture_owner.make_rid(NULL);
}

void VisualServerGX::texture_allocate(RID p_texture, int p_width, int p_height, Image::Format p_format, uint32_t p_flags){
	ERR_FAIL_COND(!texture_owner.owns(p_texture));

	GXTexObj *tex = texture_owner.get(p_texture);

	uint16_t w16 = p_width;
	uint16_t h16 = p_height;

	uint8_t format;

	uint8_t wrap = p_flags & TextureFlags::TEXTURE_FLAG_REPEAT ? GX_REPEAT : GX_CLAMP;

	uint8_t mipmaps;

	void *tex_data = malloc(GX_GetTexBufferSize(w16, h16, format, mipmaps, 0));

	GX_InitTexObj(tex, tex_data, w16, h16, format, wrap, wrap, mipmaps);

}

void VisualServerGX::texture_set_data(RID p_texture,const Image& p_image,VS::CubeMapSide p_cube_side){
	DVector<uint8_t> data = p_image.get_data();


}

Image VisualServerGX::texture_get_data(RID p_texture,VS::CubeMapSide p_cube_side) const {
	ERR_FAIL_COND_V(!texture_owner.owns(p_texture), Image());

	GXTexObj *tex = texture_owner.get(p_texture);

	void *raw_data = GX_GetTexObjData(tex);
	uint16_t width = GX_GetTexObjWidth(tex);
	uint16_t height = GX_GetTexObjHeight(tex);


	//TODO: Bumpmap stuff here

	Image new_img = Image(width, height, false, Image::FORMAT_RGBA);

	DVector<uint8_t> data = new_img.get_data();

	//(void *) data.write().ptr() = raw_data;

    return new_img;
}

void VisualServerGX::texture_set_flags(RID p_texture,uint32_t p_flags){

}

uint32_t VisualServerGX::texture_get_flags(RID p_texture) const {
    return 0;
}

Image::Format VisualServerGX::texture_get_format(RID p_texture) const{
	ERR_FAIL_COND_V(!texture_owner.owns(p_texture), Image::FORMAT_MAX);

	GXTexObj *tex = texture_owner.get(p_texture);

	uint8_t tex_fmt = GX_GetTexObjFmt(tex);

	switch (tex_fmt){
		case GX_TF_RGBA8:
			return Image::Format::FORMAT_RGBA;
		default:
			 return Image::Format::FORMAT_CUSTOM;
	}
}

uint32_t VisualServerGX::texture_get_width(RID p_texture) const{
	ERR_FAIL_COND_V(!texture_owner.owns(p_texture), 0);
	GXTexObj *tex = texture_owner.get(p_texture);

    return GX_GetTexObjWidth(tex);
}

uint32_t VisualServerGX::texture_get_height(RID p_texture) const{
	ERR_FAIL_COND_V(!texture_owner.owns(p_texture), 0);
	GXTexObj *tex = texture_owner.get(p_texture);

	return GX_GetTexObjHeight(tex);
}

void VisualServerGX::texture_set_size_override(RID p_texture,int p_width, int p_height){
}

bool VisualServerGX::texture_can_stream(RID p_texture) const{

}

void VisualServerGX::texture_set_reload_hook(RID p_texture,ObjectID p_owner,const StringName& p_function) const{

}

RID VisualServerGX::shader_create(VS::ShaderMode p_mode) {

}

void VisualServerGX::shader_set_mode(RID p_shader,VS::ShaderMode p_mode) {

}

VS::ShaderMode VisualServerGX::shader_get_mode(RID p_shader) const {

}

void VisualServerGX::shader_set_code(RID p_shader, const String& p_vertex, const String& p_fragment,const String& p_light,int p_vertex_ofs,int p_fragment_ofs,int p_light_ofs){

}
String VisualServerGX::shader_get_fragment_code(RID p_shader) const{

}

String VisualServerGX::shader_get_vertex_code(RID p_shader) const{

}

String VisualServerGX::shader_get_light_code(RID p_shader) const{

}

void VisualServerGX::shader_get_param_list(RID p_shader, List<PropertyInfo> *p_param_list) const{

}

RID VisualServerGX::material_create(){
    return RID();
}

void VisualServerGX::material_set_shader(RID p_shader_material, RID p_shader){

}

RID VisualServerGX::material_get_shader(RID p_shader_material) const {
    return RID();
}

void VisualServerGX::material_set_param(RID p_material, const StringName& p_param, const Variant& p_value){

}

Variant VisualServerGX::material_get_param(RID p_material, const StringName& p_param) const {
    return Variant();
}

void VisualServerGX::material_set_flag(RID p_material, VS::MaterialFlag p_flag,bool p_enabled){

}

bool VisualServerGX::material_get_flag(RID p_material,VS::MaterialFlag p_flag) const {
    return false;
}

void VisualServerGX::material_set_depth_draw_mode(RID p_material, VS::MaterialDepthDrawMode p_mode){

}

VS::MaterialDepthDrawMode VisualServerGX::material_get_depth_draw_mode(RID p_material) const {

}

void VisualServerGX::material_set_blend_mode(RID p_material,VS::MaterialBlendMode p_mode){

}

VS::MaterialBlendMode VisualServerGX::material_get_blend_mode(RID p_material) const{

}

void VisualServerGX::material_set_line_width(RID p_material,float p_line_width){

}

float VisualServerGX::material_get_line_width(RID p_material) const {

}

RID VisualServerGX::fixed_material_create(){
	return RID();
}

void VisualServerGX::fixed_material_set_flag(RID p_material, FixedMaterialFlags p_flag, bool p_enabled){

}

bool VisualServerGX::fixed_material_get_flag(RID p_material, FixedMaterialFlags p_flag) const{
	return false;
}

void VisualServerGX::fixed_material_set_param(RID p_material, FixedMaterialParam p_parameter, const Variant& p_value){

}

Variant VisualServerGX::fixed_material_get_param(RID p_material,FixedMaterialParam p_parameter) const {

}

void VisualServerGX::fixed_material_set_texture(RID p_material,FixedMaterialParam p_parameter, RID p_texture){

}

RID VisualServerGX::fixed_material_get_texture(RID p_material,FixedMaterialParam p_parameter) const{
	return RID();
}

void VisualServerGX::fixed_material_set_light_shader(RID p_material,FixedMaterialLightShader p_shader){

}

VS::FixedMaterialLightShader VisualServerGX::fixed_material_get_light_shader(RID p_material) const{
	return FixedMaterialLightShader();
}

void VisualServerGX::fixed_material_set_texcoord_mode(RID p_material,FixedMaterialParam p_parameter, FixedMaterialTexCoordMode p_mode){

}

VS::FixedMaterialTexCoordMode VisualServerGX::fixed_material_get_texcoord_mode(RID p_material,FixedMaterialParam p_parameter) const{

}

void VisualServerGX::fixed_material_set_uv_transform(RID p_material,const Transform3D& p_transform){

}

Transform3D VisualServerGX::fixed_material_get_uv_transform(RID p_material) const{
	return Transform3D();
}

void VisualServerGX::fixed_material_set_point_size(RID p_material,float p_size){

}

float VisualServerGX::fixed_material_get_point_size(RID p_material) const{
	return 0.0f;
}

	/* MESH API */

RID VisualServerGX::mesh_create(){
}

void VisualServerGX::mesh_add_surface(RID p_mesh,VS::PrimitiveType p_primitive,const Array& p_arrays,const Array& p_blend_shapes,bool p_alpha_sort){

}

Array VisualServerGX::mesh_get_surface_arrays(RID p_mesh,int p_surface) const {

}

Array VisualServerGX::mesh_get_surface_morph_arrays(RID p_mesh,int p_surface) const {

}

void VisualServerGX::mesh_add_custom_surface(RID p_mesh,const Variant& p_dat){
}

void VisualServerGX::mesh_set_morph_target_count(RID p_mesh,int p_amount){
}

int VisualServerGX::mesh_get_morph_target_count(RID p_mesh) const {
}

void VisualServerGX::mesh_set_morph_target_mode(RID p_mesh,VS::MorphTargetMode p_mode){
}

VS::MorphTargetMode VisualServerGX::mesh_get_morph_target_mode(RID p_mesh) const {
}

void VisualServerGX::mesh_surface_set_material(RID p_mesh, int p_surface, RID p_material,bool p_owned){
}

RID VisualServerGX::mesh_surface_get_material(RID p_mesh, int p_surface) const {
}

int VisualServerGX::mesh_surface_get_array_len(RID p_mesh, int p_surface) const {
}

int VisualServerGX::mesh_surface_get_array_index_len(RID p_mesh, int p_surface) const {
}

uint32_t VisualServerGX::mesh_surface_get_format(RID p_mesh, int p_surface) const {
}

VS::PrimitiveType VisualServerGX::mesh_surface_get_primitive_type(RID p_mesh, int p_surface) const {
}

void VisualServerGX::mesh_remove_surface(RID p_mesh,int p_index){
}

int VisualServerGX::mesh_get_surface_count(RID p_mesh) const {
}

void VisualServerGX::mesh_set_custom_aabb(RID p_mesh,const AABB& p_aabb){
}

AABB VisualServerGX::mesh_get_custom_aabb(RID p_mesh) const {
}

	/* MULTIMESH API */

RID VisualServerGX::multimesh_create(){
}

void VisualServerGX::multimesh_set_instance_count(RID p_multimesh,int p_count){
}

int VisualServerGX::multimesh_get_instance_count(RID p_multimesh) const {
}

void VisualServerGX::multimesh_set_mesh(RID p_multimesh,RID p_mesh){
}

RID VisualServerGX::multimesh_get_mesh(RID p_multimesh) const {
}

void VisualServerGX::multimesh_set_aabb(RID p_multimesh,const AABB& p_aabb){
}

AABB VisualServerGX::multimesh_get_aabb(RID p_multimesh,const AABB& p_aabb) const {
	return AABB();
}

void VisualServerGX::multimesh_instance_set_transform(RID p_multimesh,int p_index,const Transform3D& p_transform){
}

void VisualServerGX::multimesh_instance_set_color(RID p_multimesh,int p_index,const Color& p_color){
}

Transform3D VisualServerGX::multimesh_instance_get_transform(RID p_multimesh,int p_index) const {
}

Color VisualServerGX::multimesh_instance_get_color(RID p_multimesh,int p_index) const {
}

void VisualServerGX::multimesh_set_visible_instances(RID p_multimesh,int p_visible){
}

int VisualServerGX::multimesh_get_visible_instances(RID p_multimesh) const {
}

	/* BAKED LIGHT */

	/* IMMEDIATE API */

RID VisualServerGX::immediate_create(){
}

void VisualServerGX::immediate_begin(RID p_immediate,VS::PrimitiveType p_rimitive,RID p_texture){
	//Because of API discrepencies, we unfortunately can't simply drop in GX_Begin here.
	//What we can do, however, is store a command queue and then execute that when immediate_end()
	//is called


}

void VisualServerGX::immediate_vertex(RID p_immediate,const Vector3& p_vertex){
}

void VisualServerGX::immediate_normal(RID p_immediate,const Vector3& p_normal){
}

void VisualServerGX::immediate_tangent(RID p_immediate,const Plane& p_tangent){
}

void VisualServerGX::immediate_color(RID p_immediate,const Color& p_color){
}

void VisualServerGX::immediate_uv(RID p_immediate,const Vector2& tex_uv){
}

void VisualServerGX::immediate_uv2(RID p_immediate,const Vector2& tex_uv){
}

void VisualServerGX::immediate_end(RID p_immediate){

}

void VisualServerGX::immediate_clear(RID p_immediate){
	ERR_FAIL_COND(!immedeate_queues.owns(p_immediate));
}

void VisualServerGX::immediate_set_material(RID p_immediate,RID p_material){
}

RID VisualServerGX::immediate_get_material(RID p_immediate) const {
}

	/* PARTICLES API */

RID VisualServerGX::particles_create(){
}

void VisualServerGX::particles_set_amount(RID p_particles, int p_amount){
}

int VisualServerGX::particles_get_amount(RID p_particles) const {
}

void VisualServerGX::particles_set_emitting(RID p_particles, bool p_emitting){
}

bool VisualServerGX::particles_is_emitting(RID p_particles) const {
}

void VisualServerGX::particles_set_visibility_aabb(RID p_particles, const AABB& p_visibility){
}

AABB VisualServerGX::particles_get_visibility_aabb(RID p_particles) const {
}

void VisualServerGX::particles_set_emission_half_extents(RID p_particles, const Vector3& p_half_extents){
}

Vector3 VisualServerGX::particles_get_emission_half_extents(RID p_particles) const {
}

void VisualServerGX::particles_set_emission_base_velocity(RID p_particles, const Vector3& p_base_velocity){
}
Vector3 VisualServerGX::particles_get_emission_base_velocity(RID p_particles) const {
}

void VisualServerGX::particles_set_emission_points(RID p_particles, const DVector<Vector3>& p_points){
}

DVector<Vector3> VisualServerGX::particles_get_emission_points(RID p_particles) const {
}

void VisualServerGX::particles_set_gravity_normal(RID p_particles, const Vector3& p_normal){
}

Vector3 VisualServerGX::particles_get_gravity_normal(RID p_particles) const {
}

void VisualServerGX::particles_set_variable(RID p_particles, VS::ParticleVariable p_variable,float p_value){
}
float VisualServerGX::particles_get_variable(RID p_particles, VS::ParticleVariable p_variable) const {
}

void VisualServerGX::particles_set_randomness(RID p_particles, VS::ParticleVariable p_variable,float p_randomness){
}

float VisualServerGX::particles_get_randomness(RID p_particles, VS::ParticleVariable p_variable) const {
}

void VisualServerGX::particles_set_color_phase_pos(RID p_particles, int p_phase, float p_pos){
}

float VisualServerGX::particles_get_color_phase_pos(RID p_particles, int p_phase) const {
}

void VisualServerGX::particles_set_color_phases(RID p_particles, int p_phases){
}
int VisualServerGX::particles_get_color_phases(RID p_particles) const {
}

void VisualServerGX::particles_set_color_phase_color(RID p_particles, int p_phase, const Color& p_color){
}

Color VisualServerGX::particles_get_color_phase_color(RID p_particles, int p_phase) const {
}

void VisualServerGX::particles_set_attractors(RID p_particles, int p_attractors){
}

int VisualServerGX::particles_get_attractors(RID p_particles) const {
}

void VisualServerGX::particles_set_attractor_pos(RID p_particles, int p_attractor, const Vector3& p_pos){
}

Vector3 VisualServerGX::particles_get_attractor_pos(RID p_particles,int p_attractor) const {
}

void VisualServerGX::particles_set_attractor_strength(RID p_particles, int p_attractor, float p_force){
}

float VisualServerGX::particles_get_attractor_strength(RID p_particles,int p_attractor) const {
}

void VisualServerGX::particles_set_material(RID p_particles, RID p_material,bool p_owned){
}

RID VisualServerGX::particles_get_material(RID p_particles) const {
}

void VisualServerGX::particles_set_height_from_velocity(RID p_particles, bool p_enable){
}

bool VisualServerGX::particles_has_height_from_velocity(RID p_particles) const {
}

void VisualServerGX::particles_set_use_local_coordinates(RID p_particles, bool p_enable){
}

bool VisualServerGX::particles_is_using_local_coordinates(RID p_particles) const {
}

	/* SKELETON API */

RID VisualServerGX::skeleton_create(){
}
void VisualServerGX::skeleton_resize(RID p_skeleton,int p_bones){
}
int VisualServerGX::skeleton_get_bone_count(RID p_skeleton) const {
}
void VisualServerGX::skeleton_bone_set_transform(RID p_skeleton,int p_bone, const Transform3D& p_transform){
}
Transform3D VisualServerGX::skeleton_bone_get_transform(RID p_skeleton,int p_bone){
}

	/* LIGHT API */

RID VisualServerGX::light_create(VS::LightType p_type){
}

VS::LightType VisualServerGX::light_get_type(RID p_light) const {
}

void VisualServerGX::light_set_color(RID p_light,VS::LightColor p_type, const Color& p_color){
}

Color VisualServerGX::light_get_color(RID p_light,VS::LightColor p_type) const {
}

void VisualServerGX::light_set_shadow(RID p_light,bool p_enabled){
}

bool VisualServerGX::light_has_shadow(RID p_light) const {
}

void VisualServerGX::light_set_volumetric(RID p_light,bool p_enabled){
}

bool VisualServerGX::light_is_volumetric(RID p_light) const {
}

void VisualServerGX::light_set_projector(RID p_light,RID p_texture){
}

RID VisualServerGX::light_get_projector(RID p_light) const {
}

void VisualServerGX::light_set_param(RID p_light, LightParam p_var, float p_value){

}

float VisualServerGX::light_get_param(RID p_light, LightParam p_var) const{
	return 0.0f;
}

void VisualServerGX::light_set_operator(RID p_light,VS::LightOp p_op){
}

VS::LightOp VisualServerGX::light_get_operator(RID p_light) const {
}

void VisualServerGX::light_omni_set_shadow_mode(RID p_light,VS::LightOmniShadowMode p_mode){
}

VS::LightOmniShadowMode VisualServerGX::light_omni_get_shadow_mode(RID p_light) const {
}

void VisualServerGX::light_directional_set_shadow_mode(RID p_light,VS::LightDirectionalShadowMode p_mode){
}

VS::LightDirectionalShadowMode VisualServerGX::light_directional_get_shadow_mode(RID p_light) const {
}

void VisualServerGX::light_directional_set_shadow_param(RID p_light,VS::LightDirectionalShadowParam p_param, float p_value){
}

float VisualServerGX::light_directional_get_shadow_param(RID p_light,VS::LightDirectionalShadowParam p_param) const {
}

RID VisualServerGX::room_create(){
	return RID();
}

void VisualServerGX::room_set_bounds(RID p_room, const BSP_Tree& p_bounds){

}

BSP_Tree VisualServerGX::room_get_bounds(RID p_room) const {
	return BSP_Tree();
}

RID VisualServerGX::portal_create(){

}

void VisualServerGX::portal_set_shape(RID p_portal, const Vector<Point2>& p_shape){

}

Vector<Point2> VisualServerGX::portal_get_shape(RID p_portal) const{

}

void VisualServerGX::portal_set_enabled(RID p_portal, bool p_enabled){

}

bool VisualServerGX::portal_is_enabled(RID p_portal) const{

}

void VisualServerGX::portal_set_disable_distance(RID p_portal, float p_distance){

}

float VisualServerGX::portal_get_disable_distance(RID p_portal) const{

}

void VisualServerGX::portal_set_disabled_color(RID p_portal, const Color& p_color){

}

Color VisualServerGX::portal_get_disabled_color(RID p_portal) const{

}

void VisualServerGX::portal_set_connect_range(RID p_portal, float p_range){

}

float VisualServerGX::portal_get_connect_range(RID p_portal) const{

}

/* BAKED LIGHT API */

RID VisualServerGX::baked_light_create(){

}

void VisualServerGX::baked_light_set_mode(RID p_baked_light,BakedLightMode p_mode){

}

VS::BakedLightMode VisualServerGX::baked_light_get_mode(RID p_baked_light) const{

}

void VisualServerGX::baked_light_set_octree(RID p_baked_light,const DVector<uint8_t> p_octree){

}

DVector<uint8_t> VisualServerGX::baked_light_get_octree(RID p_baked_light) const{

}


void VisualServerGX::baked_light_set_light(RID p_baked_light,const DVector<uint8_t> p_light){

}

DVector<uint8_t> VisualServerGX::baked_light_get_light(RID p_baked_light) const{

}


void VisualServerGX::baked_light_set_sampler_octree(RID p_baked_light,const DVector<int> &p_sampler){

}

DVector<int> VisualServerGX::baked_light_get_sampler_octree(RID p_baked_light) const{

}


void VisualServerGX::baked_light_set_lightmap_multiplier(RID p_baked_light,float p_multiplier){

}

float VisualServerGX::baked_light_get_lightmap_multiplier(RID p_baked_light) const{

}


void VisualServerGX::baked_light_add_lightmap(RID p_baked_light,const RID p_texture,int p_id){

}

void VisualServerGX::baked_light_clear_lightmaps(RID p_baked_light){

}


	/* BAKED LIGHT SAMPLER */

RID VisualServerGX::baked_light_sampler_create(){

}


void VisualServerGX::baked_light_sampler_set_param(RID p_baked_light_sampler,BakedLightSamplerParam p_param,float p_value){

}

float VisualServerGX::baked_light_sampler_get_param(RID p_baked_light_sampler,BakedLightSamplerParam p_param) const{

}


void VisualServerGX::baked_light_sampler_set_resolution(RID p_baked_light_sampler,int p_resolution){

}

int VisualServerGX::baked_light_sampler_get_resolution(RID p_baked_light_sampler) const{

}


	/* CAMERA API */

RID VisualServerGX::camera_create(){

}

void VisualServerGX::camera_set_perspective(RID p_camera,float p_fovy_degrees, float p_z_near, float p_z_far){

}

void VisualServerGX::camera_set_orthogonal(RID p_camera,float p_size, float p_z_near, float p_z_far){

}

void VisualServerGX::camera_set_transform(RID p_camera,const Transform3D& p_transform){

}

void VisualServerGX::camera_set_visible_layers(RID p_camera,uint32_t p_layers){

}

uint32_t VisualServerGX::camera_get_visible_layers(RID p_camera) const{

}

void VisualServerGX::camera_set_environment(RID p_camera,RID p_env){

}

RID VisualServerGX::camera_get_environment(RID p_camera) const{

}

void VisualServerGX::camera_set_use_vertical_aspect(RID p_camera,bool p_enable){

}

bool VisualServerGX::camera_is_using_vertical_aspect(RID p_camera,bool p_enable) const{

}

	/* VIEWPORT API */

RID VisualServerGX::viewport_create(){

}

void VisualServerGX::viewport_attach_to_screen(RID p_viewport,int p_screen){

}

void VisualServerGX::viewport_detach(RID p_viewport){

}

void VisualServerGX::viewport_set_render_target_to_screen_rect(RID p_viewport,const Rect2& p_rect){

}

void VisualServerGX::viewport_set_as_render_target(RID p_viewport,bool p_enable){

}

void VisualServerGX::viewport_set_render_target_update_mode(RID p_viewport,RenderTargetUpdateMode p_mode){

}

VS::RenderTargetUpdateMode VisualServerGX::viewport_get_render_target_update_mode(RID p_viewport) const{

}

RID VisualServerGX::viewport_get_render_target_texture(RID p_viewport) const{

}

void VisualServerGX::viewport_set_render_target_vflip(RID p_viewport,bool p_enable){

}

bool VisualServerGX::viewport_get_render_target_vflip(RID p_viewport) const{

}

void VisualServerGX::viewport_queue_screen_capture(RID p_viewport){

}

Image VisualServerGX::viewport_get_screen_capture(RID p_viewport) const{

}

void VisualServerGX::viewport_set_rect(RID p_viewport,const ViewportRect& p_rect){

}

VS::ViewportRect VisualServerGX::viewport_get_rect(RID p_viewport) const{

}

void VisualServerGX::viewport_set_hide_scenario(RID p_viewport,bool p_hide){

}

void VisualServerGX::viewport_set_hide_canvas(RID p_viewport,bool p_hide){

}

void VisualServerGX::viewport_attach_camera(RID p_viewport,RID p_camera){

}

void VisualServerGX::viewport_set_scenario(RID p_viewport,RID p_scenario){

}

RID VisualServerGX::viewport_get_attached_camera(RID  p_viewport) const{

}

RID VisualServerGX::viewport_get_scenario(RID  p_viewport) const{

}

void VisualServerGX::viewport_attach_canvas(RID p_viewport,RID p_canvas){

}

void VisualServerGX::viewport_remove_canvas(RID p_viewport,RID p_canvas){

}

void VisualServerGX::viewport_set_canvas_transform(RID p_viewport,RID p_canvas,const Transform2D& p_offset){

}

Transform2D VisualServerGX::viewport_get_canvas_transform(RID p_viewport,RID p_canvas) const{

}

void VisualServerGX::viewport_set_transparent_background(RID p_viewport,bool p_enabled){

}

bool VisualServerGX::viewport_has_transparent_background(RID p_viewport) const{

}

void VisualServerGX::viewport_set_global_canvas_transform(RID p_viewport,const Transform2D& p_transform){

}

Transform2D VisualServerGX::viewport_get_global_canvas_transform(RID p_viewport) const{

}

void VisualServerGX::viewport_set_canvas_layer(RID p_viewport,RID p_canvas,int p_layer){

}

/* ENVIRONMENT API */

RID VisualServerGX::environment_create(){

}

void VisualServerGX::environment_set_background(RID p_env,EnvironmentBG p_bg){

}

VS::EnvironmentBG VisualServerGX::environment_get_background(RID p_env) const{

}

void VisualServerGX::environment_set_group(RID p_env,Group p_group, const Variant& p_param){

}

Variant VisualServerGX::environment_get_group(RID p_env, Group p_group) const{

}

void VisualServerGX::environment_set_background_param(RID p_env,EnvironmentBGParam p_param, const Variant& p_value){

}

Variant VisualServerGX::environment_get_background_param(RID p_env,EnvironmentBGParam p_param) const{

}

void VisualServerGX::environment_set_enable_fx(RID p_env,EnvironmentFx p_effect,bool p_enabled){

}

bool VisualServerGX::environment_is_fx_enabled(RID p_env,EnvironmentFx p_mode) const{

}

void VisualServerGX::environment_fx_set_param(RID p_env,EnvironmentFxParam p_effect,const Variant& p_param){

}

Variant VisualServerGX::environment_fx_get_param(RID p_env,EnvironmentFxParam p_effect) const{

}

/* SCENARIO API */

RID VisualServerGX::scenario_create(){

}

void VisualServerGX::scenario_set_debug(RID p_scenario,ScenarioDebugMode p_debug_mode){

}

void VisualServerGX::scenario_set_environment(RID p_scenario, RID p_environment){

}

RID VisualServerGX::scenario_get_environment(RID p_scenario, RID p_environment) const{

}

void VisualServerGX::scenario_set_fallback_environment(RID p_scenario, RID p_environment){

}

	/* INSTANCING API */

RID VisualServerGX::instance_create2(RID p_base, RID p_scenario){

}

RID VisualServerGX::instance_create(){

}

void VisualServerGX::instance_set_base(RID p_instance, RID p_base){

}

RID VisualServerGX::instance_get_base(RID p_instance) const{

}

void VisualServerGX::instance_set_scenario(RID p_instance, RID p_scenario){

}

RID VisualServerGX::instance_get_scenario(RID p_instance) const{

}

void VisualServerGX::instance_set_layer_mask(RID p_instance, uint32_t p_mask){

}

uint32_t VisualServerGX::instance_get_layer_mask(RID p_instance) const{

}

AABB VisualServerGX::instance_get_base_aabb(RID p_instance) const{

}

void VisualServerGX::instance_set_transform(RID p_instance, const Transform3D& p_transform){

}

Transform3D VisualServerGX::instance_get_transform(RID p_instance) const{

}

void VisualServerGX::instance_attach_object_instance_ID(RID p_instance,uint32_t p_ID){

}

uint32_t VisualServerGX::instance_get_object_instance_ID(RID p_instance) const{

}

void VisualServerGX::instance_set_morph_target_weight(RID p_instance,int p_shape, float p_weight){

}

float VisualServerGX::instance_get_morph_target_weight(RID p_instance,int p_shape) const{

}

void VisualServerGX::instance_attach_skeleton(RID p_instance,RID p_skeleton){

}

RID VisualServerGX::instance_get_skeleton(RID p_instance) const{

}

void VisualServerGX::instance_set_exterior( RID p_instance, bool p_enabled){

}

bool VisualServerGX::instance_is_exterior( RID p_instance) const{

}

void VisualServerGX::instance_set_room( RID p_instance, RID p_room){

}

RID VisualServerGX::instance_get_room( RID p_instance ) const{

}

void VisualServerGX::instance_set_extra_visibility_margin( RID p_instance, real_t p_margin){

}

real_t VisualServerGX::instance_get_extra_visibility_margin( RID p_instance) const{

}

Vector<RID> VisualServerGX::instances_cull_aabb(const AABB& p_aabb, RID p_scenario) const{

}

Vector<RID> VisualServerGX::instances_cull_ray(const Vector3& p_from, const Vector3& p_to, RID p_scenario) const{

}

Vector<RID> VisualServerGX::instances_cull_convex(const Vector<Plane>& p_convex, RID p_scenario) const{

}


void VisualServerGX::instance_geometry_set_flag(RID p_instance,InstanceFlags p_flags,bool p_enabled){

}

bool VisualServerGX::instance_geometry_get_flag(RID p_instance,InstanceFlags p_flags) const{

}

void VisualServerGX::instance_geometry_set_material_override(RID p_instance, RID p_material){

}

RID VisualServerGX::instance_geometry_get_material_override(RID p_instance) const{

}

void VisualServerGX::instance_geometry_set_draw_range(RID p_instance,float p_min,float p_max){

}

float VisualServerGX::instance_geometry_get_draw_range_max(RID p_instance) const{

}

float VisualServerGX::instance_geometry_get_draw_range_min(RID p_instance) const{

}

void VisualServerGX::instance_geometry_set_baked_light(RID p_instance,RID p_baked_light){

}

RID VisualServerGX::instance_geometry_get_baked_light(RID p_instance) const{

}

void VisualServerGX::instance_geometry_set_baked_light_sampler(RID p_instance,RID p_baked_light_sampler){

}

RID VisualServerGX::instance_geometry_get_baked_light_sampler(RID p_instance) const{

}

void VisualServerGX::instance_geometry_set_baked_light_texture_index(RID p_instance,int p_tex_id){

}

int VisualServerGX::instance_geometry_get_baked_light_texture_index(RID p_instance) const{

}

void VisualServerGX::instance_light_set_enabled(RID p_instance,bool p_enabled){

}

bool VisualServerGX::instance_light_is_enabled(RID p_instance) const{

}

/* CANVAS (2D) */

RID VisualServerGX::canvas_create(){

}

void VisualServerGX::canvas_set_item_mirroring(RID p_canvas,RID p_item,const Point2& p_mirroring){

}

Point2 VisualServerGX::canvas_get_item_mirroring(RID p_canvas,RID p_item) const{

}

RID VisualServerGX::canvas_item_create(){

}

void VisualServerGX::canvas_item_set_parent(RID p_item,RID p_parent){

}

RID VisualServerGX::canvas_item_get_parent(RID p_canvas_item) const{

}

void VisualServerGX::canvas_item_set_visible(RID p_item,bool p_visible){

}

bool VisualServerGX::canvas_item_is_visible(RID p_item) const{

}

void VisualServerGX::canvas_item_set_blend_mode(RID p_canvas_item, MaterialBlendMode p_mode){
 	switch (p_mode){
 		case MATERIAL_BLEND_MODE_ADD:
 			break;
 		case MATERIAL_BLEND_MODE_MIX:
 			break;
 		case MATERIAL_BLEND_MODE_MUL:
 			break;
 		case MATERIAL_BLEND_MODE_PREMULT_ALPHA:
 			break;
 		case MATERIAL_BLEND_MODE_SUB:
 			break;
 	}
}

void VisualServerGX::canvas_item_attach_viewport(RID p_item, RID p_viewport){

}


void VisualServerGX::canvas_item_set_transform(RID p_item, const Transform2D& p_transform){

}

void VisualServerGX::canvas_item_set_clip(RID p_item, bool p_clip){

}

void VisualServerGX::canvas_item_set_custom_rect(RID p_item, bool p_custom_rect,const Rect2& p_rect){

}

void VisualServerGX::canvas_item_set_opacity(RID p_item, float p_opacity){

}

float VisualServerGX::canvas_item_get_opacity(RID p_item, float p_opacity) const{

}

void VisualServerGX::canvas_item_set_self_opacity(RID p_item, float p_self_opacity){

}

float VisualServerGX::canvas_item_get_self_opacity(RID p_item, float p_self_opacity) const{

}


void VisualServerGX::canvas_item_set_on_top(RID p_item, bool p_on_top){

}

bool VisualServerGX::canvas_item_is_on_top(RID p_item) const{

}


void VisualServerGX::canvas_item_add_line(RID p_item, const Point2& p_from, const Point2& p_to,const Color& p_color,float p_width){

}

void VisualServerGX::canvas_item_add_rect(RID p_item, const Rect2& p_rect, const Color& p_color){

}

void VisualServerGX::canvas_item_add_circle(RID p_item, const Point2& p_pos, float p_radius,const Color& p_color){

}

void VisualServerGX::canvas_item_add_texture_rect(RID p_item, const Rect2& p_rect, RID p_texture,bool p_tile, const Color& p_modulate){

}

void VisualServerGX::canvas_item_add_texture_rect_region(RID p_item, const Rect2& p_rect, RID p_texture,const Rect2& p_src_rect,const Color& p_modulate){

}

void VisualServerGX::canvas_item_add_style_box(RID p_item, const Rect2& p_rect, RID p_texture,const Vector2& p_topleft, const Vector2& p_bottomright, bool p_draw_center,const Color& p_modulate){

}

void VisualServerGX::canvas_item_add_primitive(RID p_item, const Vector<Point2>& p_points, const Vector<Color>& p_colors,const Vector<Point2>& p_uvs, RID p_texture,float p_width){

}

void VisualServerGX::canvas_item_add_polygon(RID p_item, const Vector<Point2>& p_points, const Vector<Color>& p_colors,const Vector<Point2>& p_uvs, RID p_texture){

}

void VisualServerGX::canvas_item_add_triangle_array(RID p_item, const Vector<int>& p_indices, const Vector<Point2>& p_points, const Vector<Color>& p_colors,const Vector<Point2>& p_uvs, RID p_texture, int p_count){

}

void VisualServerGX::canvas_item_add_triangle_array_ptr(RID p_item, int p_count, const int* p_indices, const Point2* p_points, const Color* p_colors,const Point2* p_uvs, RID p_texture){

}

void VisualServerGX::canvas_item_add_set_transform(RID p_item,const Transform2D& p_transform){

}

void VisualServerGX::canvas_item_add_set_blend_mode(RID p_item, MaterialBlendMode p_blend){

}

void VisualServerGX::canvas_item_add_clip_ignore(RID p_item, bool p_ignore){

}

void VisualServerGX::canvas_item_set_sort_children_by_y(RID p_item, bool p_enable){

}


void VisualServerGX::canvas_item_clear(RID p_item){

}

void VisualServerGX::canvas_item_raise(RID p_item){

}


/* CURSOR */

void VisualServerGX::cursor_set_rotation(float p_rotation, int p_cursor){

}

void VisualServerGX::cursor_set_texture(RID p_texture, const Point2 &p_center_offset, int p_cursor){

}

void VisualServerGX::cursor_set_visible(bool p_visible, int p_cursor){

}

void VisualServerGX::cursor_set_pos(const Point2& p_pos, int p_cursor){

}


/* BLACK BARS */

void VisualServerGX::black_bars_set_margins(int p_left, int p_top, int p_right, int p_bottom){

}

void VisualServerGX::black_bars_set_images(RID p_left, RID p_top, RID p_right, RID p_bottom){

}

/* FREE */

void VisualServerGX::free(RID p_rid){

}

/* CUSTOM SHADING */

void VisualServerGX::custom_shade_model_set_shader(int p_model, RID p_shader){

}

RID VisualServerGX::custom_shade_model_get_shader(int p_model) const{

}

void VisualServerGX::custom_shade_model_set_name(int p_model, const String& p_name){

}

String VisualServerGX::custom_shade_model_get_name(int p_model) const{

}

void VisualServerGX::custom_shade_model_set_param_info(int p_model, const List<PropertyInfo>& p_info){

}

void VisualServerGX::custom_shade_model_get_param_info(int p_model, List<PropertyInfo>* p_info) const {

}

/* EVENT QUEUING */

void VisualServerGX::draw(){

}

void VisualServerGX::flush(){
	GX_Flush();
}

bool VisualServerGX::has_changed() const {

}

void VisualServerGX::init(){
	gp_fifo = memalign(32,DEFAULT_FIFO_SIZE);
	memset(gp_fifo,0,DEFAULT_FIFO_SIZE);

	GX_Init(gp_fifo,DEFAULT_FIFO_SIZE);

}

void VisualServerGX::finish(){

}

int VisualServerGX::get_render_info(VS::RenderInfo p_info){

}

RID VisualServerGX::material_2d_get(bool p_shaded, bool p_transparent, bool p_cut_alpha,bool p_opaque_prepass){

}

/* TESTING */

RID VisualServerGX::get_test_cube(){

}

RID VisualServerGX::get_test_texture(){

}

RID VisualServerGX::get_white_texture(){

}

RID VisualServerGX::make_sphere_mesh(int p_lats,int p_lons,float p_radius){

}

void VisualServerGX::mesh_add_surface_from_mesh_data( RID p_mesh, const Geometry::MeshData& p_mesh_data){

}

void VisualServerGX::mesh_add_surface_from_planes( RID p_mesh, const DVector<Plane>& p_planes){

}

void VisualServerGX::set_boot_image(const Image& p_image, const Color& p_color){

}

void VisualServerGX::set_default_clear_color(const Color& p_color){
	uint32_t u32_color = p_color.to_ARGB32();

	GXColor gx_color;

	gx_color.a = u32_color & 0xFF'00'00'00;
	gx_color.r = u32_color & 0x00'FF'00'00;
	gx_color.g = u32_color & 0x00'00'FF'00;
	gx_color.b = u32_color & 0x00'00'00'FF;

	GX_SetCopyClear(gx_color, GX_MAX_Z24);
}

bool VisualServerGX::has_feature(VS::Features p_feature) const {
}

