/*************************************************************************/
/*  camera_2d.h                                                          */
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
#ifndef CAMERA_2D_H
#define CAMERA_2D_H

#include "scene/2d/node_2d.h"
#include "scene/main/viewport.h"


class Camera2D : public Node2D {

	OBJ_TYPE( Camera2D, Node2D );

protected:
	Point2 camera_pos;
	Point2 smoothed_camera_pos;
	bool first;

	Viewport *viewport;

	StringName group_name;
	StringName canvas_group_name;
	RID canvas;
	Vector2 offset;
	Vector2 zoom;
	bool centered;
	bool rotating;
	bool current;
	float smoothing;
	int limit[4];
	float drag_margin[4];

	bool h_drag_enabled;
	bool v_drag_enabled;
	float h_ofs;
	float v_ofs;


	Point2 camera_screen_center;
	void _update_scroll();

	void _make_current(Object *p_which);
	void _set_current(bool p_current);
protected:

	virtual Transform2D get_camera_transform();
	void _notification(int p_what);
	static void _bind_methods();
public:

	void set_offset(const Vector2& p_offset);
	Vector2 get_offset() const;

	void set_centered(bool p_centered);
	bool is_centered() const;

	void set_rotating(bool p_rotating);
	bool is_rotating() const;

	void set_limit(Margin p_margin,int p_limit);
	int get_limit(Margin p_margin) const;


	void set_h_drag_enabled(bool p_enabled);
	bool is_h_drag_enabled() const;

	void set_v_drag_enabled(bool p_enabled);
	bool is_v_drag_enabled() const;

	void set_drag_margin(Margin p_margin,float p_drag_margin);
	float get_drag_margin(Margin p_margin) const;  

	void set_v_offset(float p_offset);
	float get_v_offset() const;

	void set_h_offset(float p_offset);
	float get_h_offset() const;

	void set_follow_smoothing(float p_speed);
	float get_follow_smoothing() const;

	void make_current();
	bool is_current() const;

	void set_zoom(const Vector2& p_zoom);
	Vector2 get_zoom() const;

	Point2 get_camera_screen_center() const;

	Vector2 get_camera_pos() const;
	void force_update_scroll();

	Camera2D();
};

#endif // CAMERA_2D_H
