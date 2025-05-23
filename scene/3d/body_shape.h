/*************************************************************************/
/*  body_shape.h                                                         */
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
#ifndef COLLISION_SHAPE_H
#define COLLISION_SHAPE_H

#include "scene/3d/spatial.h"
#include "scene/resources/shape.h"

class CollisionShape3D : public Node3D {

	OBJ_TYPE( CollisionShape3D, Node3D );
	OBJ_CATEGORY("3D Physics Nodes");

	Ref<Shape3D> shape;

	/*
	RID _get_visual_instance_rid() const;


	void _update_indicator();

	RID material;
	RID indicator;
	RID indicator_instance;
	*/

	void resource_changed(RES res);

	bool updating_body;
	bool unparenting;
	bool trigger;

	void _update_body();
	void _add_to_collision_object(Object* p_cshape);
protected:

	void _notification(int p_what);
	static void _bind_methods();

public:

	void make_convex_from_brothers();

	void set_shape(const Ref<Shape3D> &p_shape);
	Ref<Shape3D> get_shape() const;

	void set_updating_body(bool p_update);
	bool is_updating_body() const;

    void set_trigger(bool p_trigger);
    bool is_trigger() const;

	CollisionShape3D();
	~CollisionShape3D();
};

#endif // BODY_VOLUME_H

