#include "immediate_geometry.h"


void ImmediateGeometry3D::begin(Mesh::PrimitiveType p_primitive,const Ref<Texture>& p_texture) {

	VS::get_singleton()->immediate_begin(im,(VS::PrimitiveType)p_primitive,p_texture.is_valid()?p_texture->get_rid():RID());
	if (p_texture.is_valid())
		cached_textures.push_back(p_texture);

}

void ImmediateGeometry3D::set_normal(const Vector3& p_normal){

	VS::get_singleton()->immediate_normal(im,p_normal);
}

void ImmediateGeometry3D::set_tangent(const Plane& p_tangent){

	VS::get_singleton()->immediate_tangent(im,p_tangent);

}

void ImmediateGeometry3D::set_color(const Color& p_color){

	VS::get_singleton()->immediate_color(im,p_color);

}

void ImmediateGeometry3D::set_uv(const Vector2& p_uv){

	VS::get_singleton()->immediate_uv(im,p_uv);

}

void ImmediateGeometry3D::set_uv2(const Vector2& p_uv2){

	VS::get_singleton()->immediate_uv2(im,p_uv2);

}

void ImmediateGeometry3D::add_vertex(const Vector3& p_vertex){

	VS::get_singleton()->immediate_vertex(im,p_vertex);
	if (empty) {
		aabb.pos=p_vertex;
		aabb.size=Vector3();
	} else {
		aabb.expand_to(p_vertex);
	}
}

void ImmediateGeometry3D::end(){

	VS::get_singleton()->immediate_end(im);

}

void ImmediateGeometry3D::clear(){

	VS::get_singleton()->immediate_clear(im);
	empty=true;
	cached_textures.clear();

}

AABB ImmediateGeometry3D::get_aabb() const {

	return aabb;
}
DVector<Face3> ImmediateGeometry3D::get_faces(uint32_t p_usage_flags) const {

	return DVector<Face3>();
}



void ImmediateGeometry3D::add_sphere(int p_lats,int p_lons,float p_radius) {

	for(int i = 1; i <= p_lats; i++) {
		double lat0 = Math_PI * (-0.5 + (double) (i - 1) / p_lats);
		double z0  = Math::sin(lat0);
		double zr0 =  Math::cos(lat0);

		double lat1 = Math_PI * (-0.5 + (double) i / p_lats);
		double z1 = Math::sin(lat1);
		double zr1 = Math::cos(lat1);

		for(int j = p_lons; j >= 1; j--) {

			double lng0 = 2 * Math_PI * (double) (j - 1) / p_lons;
			double x0 = Math::cos(lng0);
			double y0 = Math::sin(lng0);

			double lng1 = 2 * Math_PI * (double) (j) / p_lons;
			double x1 = Math::cos(lng1);
			double y1 = Math::sin(lng1);


			Vector3 v[4]={
				Vector3(x1 * zr0, z0, y1 *zr0),
				Vector3(x1 * zr1, z1, y1 *zr1),
				Vector3(x0 * zr1, z1, y0 *zr1),
				Vector3(x0 * zr0, z0, y0 *zr0)
			};

#define ADD_POINT(m_idx)\
	set_normal(v[m_idx]);\
	add_vertex(v[m_idx]*p_radius);

			ADD_POINT(0);
			ADD_POINT(1);
			ADD_POINT(2);

			ADD_POINT(2);
			ADD_POINT(3);
			ADD_POINT(0);
		}
	}

}

void ImmediateGeometry3D::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("begin","primitive","texture:Texture"),&ImmediateGeometry3D::begin);
	ObjectTypeDB::bind_method(_MD("set_normal","normal"),&ImmediateGeometry3D::set_normal);
	ObjectTypeDB::bind_method(_MD("set_tangent","tangent"),&ImmediateGeometry3D::set_tangent);
	ObjectTypeDB::bind_method(_MD("set_color","color"),&ImmediateGeometry3D::set_color);
	ObjectTypeDB::bind_method(_MD("set_uv","uv"),&ImmediateGeometry3D::set_uv);
	ObjectTypeDB::bind_method(_MD("set_uv2","uv"),&ImmediateGeometry3D::set_uv2);
	ObjectTypeDB::bind_method(_MD("add_vertex","color"),&ImmediateGeometry3D::add_vertex);
	ObjectTypeDB::bind_method(_MD("add_sphere","lats","lons","radius"),&ImmediateGeometry3D::add_sphere);
	ObjectTypeDB::bind_method(_MD("end"),&ImmediateGeometry3D::end);
	ObjectTypeDB::bind_method(_MD("clear"),&ImmediateGeometry3D::clear);

}



ImmediateGeometry3D::ImmediateGeometry3D() {

	im = VisualServer::get_singleton()->immediate_create();
	set_base(im);
	empty=true;

}


ImmediateGeometry3D::~ImmediateGeometry3D() {

	VisualServer::get_singleton()->free(im);

}
