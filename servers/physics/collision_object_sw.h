/*************************************************************************/
/*  collision_object_sw.h                                                */
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
#ifndef COLLISION_OBJECT_SW_H
#define COLLISION_OBJECT_SW_H

#include "shape_sw.h"
#include "servers/physics_server.h"
#include "self_list.h"
#include "broad_phase_sw.h"

#define MAX_OBJECT_DISTANCE 10000000
#define MAX_OBJECT_DISTANCE_X2 (MAX_OBJECT_DISTANCE*MAX_OBJECT_DISTANCE)

class SpaceSW;

class CollisionObjectSW : public ShapeOwnerSW {
public:
	enum Type {
		TYPE_AREA,
		TYPE_BODY
	};
private:

	Type type;
	RID self;
	ObjectID instance_id;
	uint32_t layer_mask;

	struct Shape3D {

		Transform3D xform;
		Transform3D xform_inv;
		BroadPhaseSW::ID bpid;
		AABB aabb_cache; //for rayqueries
		ShapeSW *shape;
		bool trigger;

		Shape3D() { trigger=false; }
	};

	Vector<Shape3D> shapes;
	SpaceSW *space;
	Transform3D transform;
	Transform3D inv_transform;
	bool _static;

	void _update_shapes();

protected:


	void _update_shapes_with_motion(const Vector3& p_motion);
	void _unregister_shapes();

	_FORCE_INLINE_ void _set_transform(const Transform3D& p_transform,bool p_update_shapes=true) {

#ifdef DEBUG_ENABLED

		if (p_transform.origin.length_squared() > MAX_OBJECT_DISTANCE_X2) {
			ERR_EXPLAIN("Object went too far away (more than "+itos(MAX_OBJECT_DISTANCE)+"mts from origin).");
			ERR_FAIL();
		}
#endif

		transform=p_transform; if (p_update_shapes) _update_shapes();

	}
	_FORCE_INLINE_ void _set_inv_transform(const Transform3D& p_transform) { inv_transform=p_transform; }
	void _set_static(bool p_static);

	virtual void _shapes_changed()=0;
	void _set_space(SpaceSW *space);

	bool ray_pickable;


	CollisionObjectSW(Type p_type);
public:

	_FORCE_INLINE_ void set_self(const RID& p_self) { self=p_self; }
	_FORCE_INLINE_ RID get_self() const { return self; }

	_FORCE_INLINE_ void set_instance_id(const ObjectID& p_instance_id) { instance_id=p_instance_id; }
	_FORCE_INLINE_ ObjectID get_instance_id() const { return instance_id; }

	void _shape_changed();

	_FORCE_INLINE_ Type get_type() const { return type; }
	void add_shape(ShapeSW *p_shape,const Transform3D& p_transform=Transform3D());
	void set_shape(int p_index,ShapeSW *p_shape);
	void set_shape_transform(int p_index,const Transform3D& p_transform);
	_FORCE_INLINE_ int get_shape_count() const { return shapes.size(); }
	_FORCE_INLINE_ ShapeSW *get_shape(int p_index) const { return shapes[p_index].shape; }
	_FORCE_INLINE_ const Transform3D& get_shape_transform(int p_index) const { return shapes[p_index].xform; }
	_FORCE_INLINE_ const Transform3D& get_shape_inv_transform(int p_index) const { return shapes[p_index].xform_inv; }
	_FORCE_INLINE_ const AABB& get_shape_aabb(int p_index) const { return shapes[p_index].aabb_cache; }

	_FORCE_INLINE_ Transform3D get_transform() const { return transform; }
	_FORCE_INLINE_ Transform3D get_inv_transform() const { return inv_transform; }
	_FORCE_INLINE_ SpaceSW* get_space() const { return space; }

	_FORCE_INLINE_ void set_ray_pickable(bool p_enable) { ray_pickable=p_enable; }
	_FORCE_INLINE_ bool is_ray_pickable() const { return ray_pickable; }

	_FORCE_INLINE_ void set_shape_as_trigger(int p_idx,bool p_enable) { shapes[p_idx].trigger=p_enable; }
	_FORCE_INLINE_ bool is_shape_set_as_trigger(int p_idx) const { return shapes[p_idx].trigger; }

	_FORCE_INLINE_ void set_layer_mask(uint32_t p_mask) { layer_mask=p_mask; }
	_FORCE_INLINE_ uint32_t get_layer_mask() const { return layer_mask; }

	void remove_shape(ShapeSW *p_shape);
	void remove_shape(int p_index);

	virtual void set_space(SpaceSW *p_space)=0;

	_FORCE_INLINE_ bool is_static() const { return _static;  }

	virtual ~CollisionObjectSW() {}

};

#endif // COLLISION_OBJECT_SW_H
