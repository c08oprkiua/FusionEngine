/*************************************************************************/
/*  path.h                                                               */
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
#ifndef PATH_H
#define PATH_H

#include "scene/resources/curve.h"
#include "scene/3d/spatial.h"

class Path3D : public Node3D {

	OBJ_TYPE( Path3D, Node3D );

	Ref<Curve3D> curve;

	void _curve_changed();


protected:

	void _notification(int p_what);
	static void _bind_methods();
public:

	void set_curve(const Ref<Curve3D>& p_curve);
	Ref<Curve3D> get_curve() const;


	Path3D();
};

class PathFollow3D : public Node3D {

	OBJ_TYPE(PathFollow3D,Node3D);
public:

	enum RotationMode {

		ROTATION_NONE,
		ROTATION_Y,
		ROTATION_XY,
		ROTATION_XYZ
	};

private:
	Path3D *path;
	real_t offset;
	real_t h_offset;
	real_t v_offset;
	real_t lookahead;
	bool cubic;
	bool loop;
	RotationMode rotation_mode;

	void _update_transform();


protected:

	bool _set(const StringName& p_name, const Variant& p_value);
	bool _get(const StringName& p_name,Variant &r_ret) const;
	void _get_property_list( List<PropertyInfo> *p_list) const;

	void _notification(int p_what);
	static void _bind_methods();
public:

	void set_offset(float p_offset);
	float get_offset() const;

	void set_h_offset(float p_h_offset);
	float get_h_offset() const;

	void set_v_offset(float p_v_offset);
	float get_v_offset() const;

	void set_unit_offset(float p_unit_offset);
	float get_unit_offset() const;

	void set_lookahead(float p_lookahead);
	float get_lookahead() const;

	void set_loop(bool p_loop);
	bool has_loop() const;

	void set_rotation_mode(RotationMode p_rotation_mode);
	RotationMode get_rotation_mode() const;

	void set_cubic_interpolation(bool p_enable);
	bool get_cubic_interpolation() const;

	PathFollow3D();
};

VARIANT_ENUM_CAST(PathFollow3D::RotationMode);

#endif // PATH_H
