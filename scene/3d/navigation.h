#ifndef NAVIGATION_H
#define NAVIGATION_H

#include "scene/3d/spatial.h"
#include "scene/3d/navigation_mesh.h"

class Navigation : public Node3D {

	OBJ_TYPE( Navigation, Node3D);


	union Point {

		struct {
			int64_t x:21;
			int64_t y:22;
			int64_t z:21;
		};

		uint64_t key;
		bool operator<(const Point& p_key) const { return key < p_key.key; }
	};


	struct EdgeKey {

		Point a;
		Point b;

		bool operator<(const EdgeKey& p_key) const {
			return (a.key==p_key.a.key)?(b.key<p_key.b.key):(a.key<p_key.a.key);
		};

		EdgeKey(const Point& p_a=Point(),const Point& p_b=Point()) {
			a=p_a;
			b=p_b;
			if (a.key > b.key) {
				SWAP(a,b);
			}
		}
	};


	struct NavMesh;


	struct Polygon {

		struct Edge {
			Point point;
			Polygon *C; //connection
			int C_edge;
			Edge() { C=NULL; C_edge=-1; }
		};

		Vector<Edge> edges;

		Vector3 center;

		float distance;
		int prev_edge;

		NavMesh *owner;
	};


	struct Connection {

		Polygon *A;
		int A_edge;
		Polygon *B;
		int B_edge;
		Connection() { A=NULL; B=NULL; A_edge=-1; B_edge=-1;}
	};

	Map<EdgeKey,Connection> connections;


	struct NavMesh {

		Object *owner;
		Transform3D xform;
		bool linked;
		Ref<NavigationMesh> navmesh;
		List<Polygon> polygons;

	};



	_FORCE_INLINE_ Point _get_point(const Vector3& p_pos) const {

		int x = int(Math::floor(p_pos.x/cell_size));
		int y = int(Math::floor(p_pos.y/cell_size));
		int z = int(Math::floor(p_pos.z/cell_size));

		Point p;
		p.key=0;
		p.x=x;
		p.y=y;
		p.z=z;
		return p;

	}

	_FORCE_INLINE_ Vector3 _get_vertex(const Point& p_point) const {

		return Vector3(p_point.x,p_point.y,p_point.z)*cell_size;
	}



	void _navmesh_link(int p_id);
	void _navmesh_unlink(int p_id);

	float cell_size;
	Map<int,NavMesh> navmesh_map;
	int last_id;

	Vector3 up;
	void _clip_path(Vector<Vector3>& path,Polygon *from_poly, const Vector3& p_to_point, Polygon* p_to_poly);

protected:

	static void _bind_methods();

public:

	void set_up_vector(const Vector3& p_up);
	Vector3 get_up_vector() const;

	//API should be as dynamic as possible
	int navmesh_create(const Ref<NavigationMesh>& p_mesh,const Transform3D& p_xform,Object* p_owner=NULL);
	void navmesh_set_transform(int p_id, const Transform3D& p_xform);
	void navmesh_remove(int p_id);

	Vector<Vector3> get_simple_path(const Vector3& p_start, const Vector3& p_end,bool p_optimize=true);
	Vector3 get_closest_point_to_segment(const Vector3& p_from,const Vector3& p_to);
	Vector3 get_closest_point(const Vector3& p_point);
	Vector3 get_closest_point_normal(const Vector3& p_point);

	Navigation();
};

#endif // NAVIGATION_H
