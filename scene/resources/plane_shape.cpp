/*************************************************************************/
/*  plane_shape.cpp                                                      */
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
#include "plane_shape.h"

#include "servers/physics_server.h"



void PlaneShape3D::_update_shape() {

	PhysicsServer::get_singleton()->shape_set_data(get_shape(),plane);
}

void PlaneShape3D::set_plane(Plane p_plane) {

	plane=p_plane;
	_update_shape();
	notify_change_to_owners();
	_change_notify("plane");
}

Plane PlaneShape3D::get_plane() const {

	return plane;
}


void PlaneShape3D::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("set_plane","plane"),&PlaneShape3D::set_plane);
	ObjectTypeDB::bind_method(_MD("get_plane"),&PlaneShape3D::get_plane);

	ADD_PROPERTY( PropertyInfo(Variant::PLANE,"plane"), _SCS("set_plane"), _SCS("get_plane") );

}

PlaneShape3D::PlaneShape3D() : Shape3D( PhysicsServer::get_singleton()->shape_create(PhysicsServer::SHAPE_PLANE)) {

	set_plane(Plane(0,1,0,0));
}
