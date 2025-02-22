#include "rasterizer_gx.h"


RID RasterizerGX::texture_create(){
    return RID();
}

void RasterizerGX::texture_allocate(RID p_texture,int p_width, int p_height,Image::Format p_format,uint32_t p_flags){
}

void RasterizerGX::texture_set_data(RID p_texture,const Image& p_image,VS::CubeMapSide p_cube_side){

}

Image RasterizerGX::texture_get_data(RID p_texture,VS::CubeMapSide p_cube_side) const {
    return Image();
}

void RasterizerGX::texture_set_flags(RID p_texture,uint32_t p_flags){

}

uint32_t RasterizerGX::texture_get_flags(RID p_texture) const{
    return 0;
}

Image::Format RasterizerGX::texture_get_format(RID p_texture) const{
    return Image::Format::FORMAT_CUSTOM;
}

uint32_t RasterizerGX::texture_get_width(RID p_texture) const{
    return 0;
}

uint32_t RasterizerGX::texture_get_height(RID p_texture) const{
    return 0;
}

bool RasterizerGX::texture_has_alpha(RID p_texture) const{
    return false;
}

void RasterizerGX::texture_set_size_override(RID p_texture,int p_width, int p_height){
}

void RasterizerGX::texture_set_reload_hook(RID p_texture,ObjectID p_owner,const StringName& p_function) const{

}

RID RasterizerGX::shader_create(VS::ShaderMode p_mode) {

}

void RasterizerGX::shader_set_mode(RID p_shader,VS::ShaderMode p_mode) {

}

VS::ShaderMode RasterizerGX::shader_get_mode(RID p_shader) const {

}

void RasterizerGX::shader_set_code(RID p_shader, const String& p_vertex, const String& p_fragment,const String& p_light,int p_vertex_ofs,int p_fragment_ofs,int p_light_ofs){

}
String RasterizerGX::shader_get_fragment_code(RID p_shader) const{

}

String RasterizerGX::shader_get_vertex_code(RID p_shader) const{

}

String RasterizerGX::shader_get_light_code(RID p_shader) const{

};

void RasterizerGX::shader_get_param_list(RID p_shader, List<PropertyInfo> *p_param_list) const {

}


RID RasterizerGX::material_create(){
    return RID();
}

void RasterizerGX::material_set_shader(RID p_shader_material, RID p_shader){

}

RID RasterizerGX::material_get_shader(RID p_shader_material) const {
    return RID();
}

void RasterizerGX::material_set_param(RID p_material, const StringName& p_param, const Variant& p_value){

}

Variant RasterizerGX::material_get_param(RID p_material, const StringName& p_param) const {
    return Variant();
}

void RasterizerGX::material_set_flag(RID p_material, VS::MaterialFlag p_flag,bool p_enabled){

}

bool RasterizerGX::material_get_flag(RID p_material,VS::MaterialFlag p_flag) const {
    return false;
}

void RasterizerGX::material_set_depth_draw_mode(RID p_material, VS::MaterialDepthDrawMode p_mode){

}

VS::MaterialDepthDrawMode RasterizerGX::material_get_depth_draw_mode(RID p_material) const {

}

void RasterizerGX::material_set_blend_mode(RID p_material,VS::MaterialBlendMode p_mode){

}

VS::MaterialBlendMode RasterizerGX::material_get_blend_mode(RID p_material) const{

}

void RasterizerGX::material_set_line_width(RID p_material,float p_line_width){

}

float RasterizerGX::material_get_line_width(RID p_material) const {

}

	/* MESH API */

RID RasterizerGX::mesh_create(){
}

void RasterizerGX::mesh_add_surface(RID p_mesh,VS::PrimitiveType p_primitive,const Array& p_arrays,const Array& p_blend_shapes,bool p_alpha_sort){

}

Array RasterizerGX::mesh_get_surface_arrays(RID p_mesh,int p_surface) const {

}

Array RasterizerGX::mesh_get_surface_morph_arrays(RID p_mesh,int p_surface) const {

}

void RasterizerGX::mesh_add_custom_surface(RID p_mesh,const Variant& p_dat){
}

void RasterizerGX::mesh_set_morph_target_count(RID p_mesh,int p_amount){
}

int RasterizerGX::mesh_get_morph_target_count(RID p_mesh) const {
}

void RasterizerGX::mesh_set_morph_target_mode(RID p_mesh,VS::MorphTargetMode p_mode){
}

VS::MorphTargetMode RasterizerGX::mesh_get_morph_target_mode(RID p_mesh) const {
}

void RasterizerGX::mesh_surface_set_material(RID p_mesh, int p_surface, RID p_material,bool p_owned){
}

RID RasterizerGX::mesh_surface_get_material(RID p_mesh, int p_surface) const {
}

int RasterizerGX::mesh_surface_get_array_len(RID p_mesh, int p_surface) const {
}

int RasterizerGX::mesh_surface_get_array_index_len(RID p_mesh, int p_surface) const {
}

uint32_t RasterizerGX::mesh_surface_get_format(RID p_mesh, int p_surface) const {
}

VS::PrimitiveType RasterizerGX::mesh_surface_get_primitive_type(RID p_mesh, int p_surface) const {
}

void RasterizerGX::mesh_remove_surface(RID p_mesh,int p_index){
}

int RasterizerGX::mesh_get_surface_count(RID p_mesh) const {
}

AABB RasterizerGX::mesh_get_aabb(RID p_mesh,RID p_skeleton) const {
}

void RasterizerGX::mesh_set_custom_aabb(RID p_mesh,const AABB& p_aabb){
}

AABB RasterizerGX::mesh_get_custom_aabb(RID p_mesh) const {
}

	/* MULTIMESH API */

RID RasterizerGX::multimesh_create(){
}

void RasterizerGX::multimesh_set_instance_count(RID p_multimesh,int p_count){
}

int RasterizerGX::multimesh_get_instance_count(RID p_multimesh) const {
}

void RasterizerGX::multimesh_set_mesh(RID p_multimesh,RID p_mesh){
}

void RasterizerGX::multimesh_set_aabb(RID p_multimesh,const AABB& p_aabb){
}

void RasterizerGX::multimesh_instance_set_transform(RID p_multimesh,int p_index,const Transform3D& p_transform){
}

void RasterizerGX::multimesh_instance_set_color(RID p_multimesh,int p_index,const Color& p_color){
}

RID RasterizerGX::multimesh_get_mesh(RID p_multimesh) const {
}

AABB RasterizerGX::multimesh_get_aabb(RID p_multimesh) const {
}

Transform3D RasterizerGX::multimesh_instance_get_transform(RID p_multimesh,int p_index) const {
}
Color RasterizerGX::multimesh_instance_get_color(RID p_multimesh,int p_index) const {
}

void RasterizerGX::multimesh_set_visible_instances(RID p_multimesh,int p_visible){
}

int RasterizerGX::multimesh_get_visible_instances(RID p_multimesh) const {
}

	/* BAKED LIGHT */

	/* IMMEDIATE API */

RID RasterizerGX::immediate_create(){
}

void RasterizerGX::immediate_begin(RID p_immediate,VS::PrimitiveType p_rimitive,RID p_texture){
}

void RasterizerGX::immediate_vertex(RID p_immediate,const Vector3& p_vertex){
}

void RasterizerGX::immediate_normal(RID p_immediate,const Vector3& p_normal){
}

void RasterizerGX::immediate_tangent(RID p_immediate,const Plane& p_tangent){
}

void RasterizerGX::immediate_color(RID p_immediate,const Color& p_color){
}

void RasterizerGX::immediate_uv(RID p_immediate,const Vector2& tex_uv){
}

void RasterizerGX::immediate_uv2(RID p_immediate,const Vector2& tex_uv){
}

void RasterizerGX::immediate_end(RID p_immediate){
}

void RasterizerGX::immediate_clear(RID p_immediate){
}

AABB RasterizerGX::immediate_get_aabb(RID p_immediate) const {
}

void RasterizerGX::immediate_set_material(RID p_immediate,RID p_material){
}

RID RasterizerGX::immediate_get_material(RID p_immediate) const {
}

	/* PARTICLES API */

RID RasterizerGX::particles_create(){
}

void RasterizerGX::particles_set_amount(RID p_particles, int p_amount){
}

int RasterizerGX::particles_get_amount(RID p_particles) const {
}

void RasterizerGX::particles_set_emitting(RID p_particles, bool p_emitting){
}

bool RasterizerGX::particles_is_emitting(RID p_particles) const {
}

void RasterizerGX::particles_set_visibility_aabb(RID p_particles, const AABB& p_visibility){
}

AABB RasterizerGX::particles_get_visibility_aabb(RID p_particles) const {
}

void RasterizerGX::particles_set_emission_half_extents(RID p_particles, const Vector3& p_half_extents){
}

Vector3 RasterizerGX::particles_get_emission_half_extents(RID p_particles) const {
}

void RasterizerGX::particles_set_emission_base_velocity(RID p_particles, const Vector3& p_base_velocity){
}
Vector3 RasterizerGX::particles_get_emission_base_velocity(RID p_particles) const {
}

void RasterizerGX::particles_set_emission_points(RID p_particles, const DVector<Vector3>& p_points){
}

DVector<Vector3> RasterizerGX::particles_get_emission_points(RID p_particles) const {
}

void RasterizerGX::particles_set_gravity_normal(RID p_particles, const Vector3& p_normal){
}

Vector3 RasterizerGX::particles_get_gravity_normal(RID p_particles) const {
}

void RasterizerGX::particles_set_variable(RID p_particles, VS::ParticleVariable p_variable,float p_value){
}
float RasterizerGX::particles_get_variable(RID p_particles, VS::ParticleVariable p_variable) const {
}

void RasterizerGX::particles_set_randomness(RID p_particles, VS::ParticleVariable p_variable,float p_randomness){
}

float RasterizerGX::particles_get_randomness(RID p_particles, VS::ParticleVariable p_variable) const {
}

void RasterizerGX::particles_set_color_phase_pos(RID p_particles, int p_phase, float p_pos){
}

float RasterizerGX::particles_get_color_phase_pos(RID p_particles, int p_phase) const {
}

void RasterizerGX::particles_set_color_phases(RID p_particles, int p_phases){
}
int RasterizerGX::particles_get_color_phases(RID p_particles) const {
}

void RasterizerGX::particles_set_color_phase_color(RID p_particles, int p_phase, const Color& p_color){
}

Color RasterizerGX::particles_get_color_phase_color(RID p_particles, int p_phase) const {
}

void RasterizerGX::particles_set_attractors(RID p_particles, int p_attractors){
}

int RasterizerGX::particles_get_attractors(RID p_particles) const {
}

void RasterizerGX::particles_set_attractor_pos(RID p_particles, int p_attractor, const Vector3& p_pos){
}

Vector3 RasterizerGX::particles_get_attractor_pos(RID p_particles,int p_attractor) const {
}

void RasterizerGX::particles_set_attractor_strength(RID p_particles, int p_attractor, float p_force){
}

float RasterizerGX::particles_get_attractor_strength(RID p_particles,int p_attractor) const {
}

void RasterizerGX::particles_set_material(RID p_particles, RID p_material,bool p_owned){
}

RID RasterizerGX::particles_get_material(RID p_particles) const {
}

AABB RasterizerGX::particles_get_aabb(RID p_particles) const {
}

void RasterizerGX::particles_set_height_from_velocity(RID p_particles, bool p_enable){
}
bool RasterizerGX::particles_has_height_from_velocity(RID p_particles) const {
}

void RasterizerGX::particles_set_use_local_coordinates(RID p_particles, bool p_enable){
}
bool RasterizerGX::particles_is_using_local_coordinates(RID p_particles) const {
}

	/* SKELETON API */

RID RasterizerGX::skeleton_create(){
}
void RasterizerGX::skeleton_resize(RID p_skeleton,int p_bones){
}
int RasterizerGX::skeleton_get_bone_count(RID p_skeleton) const {
}
void RasterizerGX::skeleton_bone_set_transform(RID p_skeleton,int p_bone, const Transform3D& p_transform){
}
Transform3D RasterizerGX::skeleton_bone_get_transform(RID p_skeleton,int p_bone){
}


	/* LIGHT API */

RID RasterizerGX::light_create(VS::LightType p_type){
}
VS::LightType RasterizerGX::light_get_type(RID p_light) const {
}

void RasterizerGX::light_set_color(RID p_light,VS::LightColor p_type, const Color& p_color){
}
Color RasterizerGX::light_get_color(RID p_light,VS::LightColor p_type) const {
}

void RasterizerGX::light_set_shadow(RID p_light,bool p_enabled){
}
bool RasterizerGX::light_has_shadow(RID p_light) const {
}

void RasterizerGX::light_set_volumetric(RID p_light,bool p_enabled){
}
bool RasterizerGX::light_is_volumetric(RID p_light) const {
}

void RasterizerGX::light_set_projector(RID p_light,RID p_texture){
}
RID RasterizerGX::light_get_projector(RID p_light) const {
}

void RasterizerGX::light_set_var(RID p_light, VS::LightParam p_var, float p_value){
}
float RasterizerGX::light_get_var(RID p_light, VS::LightParam p_var) const {
}

void RasterizerGX::light_set_operator(RID p_light,VS::LightOp p_op){
}
VS::LightOp RasterizerGX::light_get_operator(RID p_light) const {
}

void RasterizerGX::light_omni_set_shadow_mode(RID p_light,VS::LightOmniShadowMode p_mode){
}
VS::LightOmniShadowMode RasterizerGX::light_omni_get_shadow_mode(RID p_light) const {
}

void RasterizerGX::light_directional_set_shadow_mode(RID p_light,VS::LightDirectionalShadowMode p_mode){
}
VS::LightDirectionalShadowMode RasterizerGX::light_directional_get_shadow_mode(RID p_light) const {
}

void RasterizerGX::light_directional_set_shadow_param(RID p_light,VS::LightDirectionalShadowParam p_param, float p_value){
}

float RasterizerGX::light_directional_get_shadow_param(RID p_light,VS::LightDirectionalShadowParam p_param) const {
}

AABB RasterizerGX::light_get_aabb(RID p_poly) const {
}

RID RasterizerGX::light_instance_create(RID p_light){
}

void RasterizerGX::light_instance_set_transform(RID p_light_instance,const Transform3D& p_transform){
}

Rasterizer::ShadowType RasterizerGX::light_instance_get_shadow_type(RID p_light_instance,bool p_far) const {
}

int RasterizerGX::light_instance_get_shadow_passes(RID p_light_instance) const {
}
void RasterizerGX::light_instance_set_shadow_transform(RID p_light_instance, int p_index, const CameraMatrix& p_camera, const Transform3D& p_transform, float p_split_near,float p_split_far){
}

int RasterizerGX::light_instance_get_shadow_size(RID p_light_instance, int p_index) const {
}

bool RasterizerGX::light_instance_get_pssm_shadow_overlap(RID p_light_instance) const {
}

	/* SHADOWS */

void RasterizerGX::shadow_clear_near(){
}

bool RasterizerGX::shadow_allocate_near(RID p_light){
} //true on successful alloc

bool RasterizerGX::shadow_allocate_far(RID p_light){
} //true on successful alloc

	/* PARTICLES INSTANCE */

RID RasterizerGX::particles_instance_create(RID p_particles){
}

void RasterizerGX::particles_instance_set_transform(RID p_particles_instance,const Transform3D& p_transform){
}


RID RasterizerGX::viewport_data_create(){
}

RID RasterizerGX::render_target_create(){
}

void RasterizerGX::render_target_set_size(RID p_render_target, int p_width, int p_height){
}

RID RasterizerGX::render_target_get_texture(RID p_render_target) const {
}

bool RasterizerGX::render_target_renedered_in_frame(RID p_render_target){
}

void RasterizerGX::begin_frame(){
}

void RasterizerGX::set_viewport(const VS::ViewportRect& p_viewport){
}

void RasterizerGX::set_render_target(RID p_render_target,bool p_transparent_bg,bool p_vflip){
}

void RasterizerGX::clear_viewport(const Color& p_color){
}

void RasterizerGX::capture_viewport(Image* r_capture){
}

void RasterizerGX::begin_scene(RID p_viewport_data,RID p_env,VS::ScenarioDebugMode p_debug){
}

void RasterizerGX::begin_shadow_map( RID p_light_instance, int p_shadow_pass ){
}

void RasterizerGX::set_camera(const Transform3D& p_world,const CameraMatrix& p_projection){
}

void RasterizerGX::add_light( RID p_light_instance ){
} ///< all "add_light" calls happen before add_geometry calls

void RasterizerGX::add_mesh( const RID& p_mesh, const InstanceData *p_data){
}

void RasterizerGX::add_multimesh( const RID& p_multimesh, const InstanceData *p_data){
}

void RasterizerGX::add_immediate( const RID& p_immediate, const InstanceData *p_data){
}

void RasterizerGX::add_particles( const RID& p_particle_instance, const InstanceData *p_data){
}

void RasterizerGX::end_scene(){
}

void RasterizerGX::end_shadow_map(){
}

void RasterizerGX::end_frame(){
}

void RasterizerGX::canvas_begin(){
}

void RasterizerGX::canvas_disable_blending(){
}

void RasterizerGX::canvas_set_opacity(float p_opacity){
}

void RasterizerGX::canvas_set_blend_mode(VS::MaterialBlendMode p_mode){
}

void RasterizerGX::canvas_begin_rect(const Transform2D& p_transform){
}

void RasterizerGX::canvas_set_clip(bool p_clip, const Rect2& p_rect){
}

void RasterizerGX::canvas_end_rect(){
}

void RasterizerGX::canvas_draw_line(const Point2& p_from, const Point2& p_to,const Color& p_color,float p_width){
}

void RasterizerGX::canvas_draw_rect(const Rect2& p_rect, int p_flags, const Rect2& p_source,RID p_texture,const Color& p_modulate){
}
void RasterizerGX::canvas_draw_style_box(const Rect2& p_rect, RID p_texture,const float *p_margins, bool p_draw_center,const Color& p_modulate){
}
void RasterizerGX::canvas_draw_primitive(const Vector<Point2>& p_points, const Vector<Color>& p_colors,const Vector<Point2>& p_uvs, RID p_texture,float p_width){
}
void RasterizerGX::canvas_draw_polygon(int p_vertex_count, const int* p_indices, const Vector2* p_vertices, const Vector2* p_uvs, const Color* p_colors,const RID& p_texture,bool p_singlecolor){
}
void RasterizerGX::canvas_set_transform(const Transform2D& p_transform){
}

	/* ENVIRONMENT */


RID RasterizerGX::environment_create(){
}

void RasterizerGX::environment_set_background(RID p_env,VS::EnvironmentBG p_bg){
}
VS::EnvironmentBG RasterizerGX::environment_get_background(RID p_env) const {
}

void RasterizerGX::environment_set_background_param(RID p_env,VS::EnvironmentBGParam p_param, const Variant& p_value){
}
Variant RasterizerGX::environment_get_background_param(RID p_env,VS::EnvironmentBGParam p_param) const {
}

void RasterizerGX::environment_set_group(RID p_env,VS::Group p_param, const Variant& p_value){
}
Variant RasterizerGX::environment_get_group(RID p_env,VS::Group p_param) const {
}

void RasterizerGX::environment_set_enable_fx(RID p_env,VS::EnvironmentFx p_effect,bool p_enabled){
}

bool RasterizerGX::environment_is_fx_enabled(RID p_env,VS::EnvironmentFx p_effect) const {
}

void RasterizerGX::environment_fx_set_param(RID p_env,VS::EnvironmentFxParam p_param,const Variant& p_value){
}

Variant RasterizerGX::environment_fx_get_param(RID p_env,VS::EnvironmentFxParam p_param) const {
}

	/* SAMPLED LIGHT */
RID RasterizerGX::sampled_light_dp_create(int p_width,int p_height){
}
void RasterizerGX::sampled_light_dp_update(RID p_sampled_light,const Color *p_data,float p_multiplier){
}

	/*MISC*/

bool RasterizerGX::is_texture(const RID& p_rid) const {
}
bool RasterizerGX::is_material(const RID& p_rid) const {
}
bool RasterizerGX::is_mesh(const RID& p_rid) const {
}
bool RasterizerGX::is_multimesh(const RID& p_rid) const {
}
bool RasterizerGX::is_immediate(const RID& p_rid) const {
}

bool RasterizerGX::is_particles(const RID &p_beam) const {
}

bool RasterizerGX::is_light(const RID& p_rid) const {
}

bool RasterizerGX::is_light_instance(const RID& p_rid) const {
}

bool RasterizerGX::is_particles_instance(const RID& p_rid) const {
}

bool RasterizerGX::is_skeleton(const RID& p_rid) const {
}

bool RasterizerGX::is_environment(const RID& p_rid) const {
}

bool RasterizerGX::is_shader(const RID& p_rid) const {
}

void RasterizerGX::free(const RID& p_rid){
}

void RasterizerGX::init(){
}

void RasterizerGX::finish(){
}

bool RasterizerGX::needs_to_draw_next_frame() const {
}

void RasterizerGX::reload_vram(){

}

bool RasterizerGX::has_feature(VS::Features p_feature) const {
}

int RasterizerGX::get_render_info(VS::RenderInfo p_info){
}
