/*************************************************************************/
/*  spatial.h                                                            */
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
#ifndef SPATIAL_H
#define SPATIAL_H

#include "scene/main/node.h"
#include "scene/main/scene_main_loop.h"

/**
	@author Juan Linietsky <reduzio@gmail.com>
*/

class SpatialGizmo : public RefCounted {

	OBJ_TYPE(SpatialGizmo,RefCounted);


public:

	virtual void create()=0;
	virtual void transform()=0;
	virtual void clear()=0;
	virtual void redraw()=0;
	virtual void free()=0;

	SpatialGizmo();
};


class Node3D : public Node {

	OBJ_TYPE( Node3D, Node );
	OBJ_CATEGORY("3D");
	
	enum TransformDirty {
		DIRTY_NONE=0,
		DIRTY_VECTORS=1,
		DIRTY_LOCAL=2,
		DIRTY_GLOBAL=4
	};

	mutable SelfList<Node> xform_change;

	struct Data {
	


		mutable Transform3D global_transform;
		mutable Transform3D local_transform;
		mutable Vector3 rotation;
		mutable Vector3 scale;

		mutable int dirty;

		Viewport *viewport;


		bool toplevel_active;
		bool toplevel;
		bool inside_world;
	
		int children_lock;
		Node3D *parent;
		List<Node3D*> children;
		List<Node3D*>::Element *C;
		
		bool ignore_notification;

		bool visible;

#ifdef TOOLS_ENABLED
		Ref<SpatialGizmo> gizmo;
		bool gizmo_disabled;
		bool gizmo_dirty;
		Transform3D import_transform;
#endif

	} data;
#ifdef TOOLS_ENABLED

	void _update_gizmo();
#endif
	void _notify_dirty();
	void _propagate_transform_changed(Node3D *p_origin);

	void _set_rotation_deg(const Vector3& p_deg);
	Vector3 _get_rotation_deg() const;

	void _propagate_visibility_changed();


protected:

	_FORCE_INLINE_ void set_ignore_transform_notification(bool p_ignore) { data.ignore_notification=p_ignore; }

	_FORCE_INLINE_ void _update_local_transform() const;

	void _notification(int p_what);
	static void _bind_methods();

	void _set_visible_(bool p_visible);
	bool _is_visible_() const;
public:

	enum {
	
		NOTIFICATION_TRANSFORM_CHANGED=SceneTree::NOTIFICATION_TRANSFORM_CHANGED,
		NOTIFICATION_ENTER_WORLD=41,
		NOTIFICATION_EXIT_WORLD=42,
		NOTIFICATION_VISIBILITY_CHANGED=43,
	};

	Node3D *get_parent_spatial() const;


	Ref<World3D> get_world() const;

	void set_translation(const Vector3& p_translation);
	void set_rotation(const Vector3& p_euler);
	void set_scale(const Vector3& p_scale);

	Vector3 get_translation() const;
	Vector3 get_rotation() const;
	Vector3 get_scale() const;

	void set_transform(const Transform3D& p_transform);
	void set_global_transform(const Transform3D& p_transform);
	
	Transform3D get_transform() const;
	Transform3D get_global_transform() const;

	void set_as_toplevel(bool p_enabled);
	bool is_set_as_toplevel() const;

	void set_disable_gizmo(bool p_enabled);
	void update_gizmo();
	void set_gizmo(const Ref<SpatialGizmo>& p_gizmo);
	Ref<SpatialGizmo> get_gizmo() const;

	_FORCE_INLINE_ bool is_inside_world() const { return data.inside_world; }

	Transform3D get_relative_transform(const Node *p_parent) const;

	void show();
	void hide();
	bool is_visible() const;
	bool is_hidden() const;

#ifdef TOOLS_ENABLED
	void set_import_transform(const Transform3D& p_transform)	;
	Transform3D get_import_transform() const;
#endif

	Node3D();	
	~Node3D();

};

#endif
