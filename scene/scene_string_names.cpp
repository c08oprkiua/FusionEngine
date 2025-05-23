/*************************************************************************/
/*  scene_string_names.cpp                                               */
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
#include "scene_string_names.h"

namespace SceneStringNames {

StringName resized;
StringName dot;
StringName doubledot;
StringName draw;
StringName hide;
StringName visibility_changed;
StringName input_event;
StringName _input_event;
StringName item_rect_changed;
StringName shader_shader;
StringName enter_tree;
StringName exit_tree;
StringName size_flags_changed;
StringName minimum_size_changed;
StringName idle;
StringName iteration;
StringName update;

StringName line_separation;

StringName mouse_enter;
StringName mouse_exit;
StringName focus_enter;
StringName focus_exit;

StringName sort_children;

StringName finished;
StringName animation_changed;

StringName body_enter_shape;
StringName body_enter;
StringName body_exit_shape;
StringName body_exit;

StringName _area_enter_tree;
StringName _area_exit_tree;


StringName area_enter_shape;
StringName area_exit_shape;

StringName _get_gizmo_geometry;
StringName _can_gizmo_scale;

StringName _fixed_process;
StringName _process;
StringName _enter_world;
StringName _exit_world;
StringName _enter_tree;
StringName _exit_tree;
StringName _draw;
StringName _input;
StringName _ready;

StringName _pressed;
StringName _toggled;

StringName _update_scroll;
StringName _update_xform;

StringName _proxgroup_add;
StringName _proxgroup_remove;

StringName grouped;
StringName ungrouped;

StringName has_point;
StringName get_drag_data;
StringName can_drop_data;
StringName drop_data;

StringName enter_screen;
StringName exit_screen;
StringName enter_viewport;
StringName exit_viewport;
StringName enter_camera;
StringName exit_camera;

StringName _body_enter_tree;
StringName _body_exit_tree;

StringName changed;
StringName _shader_changed;

StringName _spatial_editor_group;
StringName _request_gizmo;

StringName offset;
StringName unit_offset;
StringName rotation_mode;
StringName rotate;
StringName v_offset;
StringName h_offset;

StringName transform_pos;
StringName transform_rot;
StringName transform_scale;

StringName _update_remote;
StringName _update_pairs;

StringName area_enter;
StringName area_exit;

StringName get_minimum_size;

StringName play_play;

StringName _im_update;
StringName _queue_update;

StringName baked_light_changed;
StringName _baked_light_changed;

StringName _mouse_enter;
StringName _mouse_exit;

StringName frame_changed;

void create() {

	resized=StaticCString::create("resized");
	dot=StaticCString::create(".");
	doubledot=StaticCString::create("..");
	draw=StaticCString::create("draw");
	_draw=StaticCString::create("_draw");
	hide=StaticCString::create("hide");
	visibility_changed=StaticCString::create("visibility_changed");
	input_event=StaticCString::create("input_event");
	shader_shader=StaticCString::create("shader/shader");
	enter_tree=StaticCString::create("enter_tree");
	exit_tree=StaticCString::create("exit_tree");
	item_rect_changed=StaticCString::create("item_rect_changed");
	size_flags_changed=StaticCString::create("size_flags_changed");
	minimum_size_changed=StaticCString::create("minimum_size_changed");

	finished=StaticCString::create("finished");
	animation_changed=StaticCString::create("animation_changed");

	mouse_enter=StaticCString::create("mouse_enter");
	mouse_exit=StaticCString::create("mouse_exit");

	focus_enter=StaticCString::create("focus_enter");
	focus_exit=StaticCString::create("focus_exit");

	sort_children = StaticCString::create("sort_children");

	body_enter_shape = StaticCString::create("body_enter_shape");
	body_enter = StaticCString::create("body_enter");
	body_exit_shape = StaticCString::create("body_exit_shape");
	body_exit = StaticCString::create("body_exit");


	_area_enter_tree = StaticCString::create("_area_enter_tree");
	_area_exit_tree = StaticCString::create("_area_exit_tree");
	
	area_enter_shape = StaticCString::create("area_enter_shape");
	area_exit_shape = StaticCString::create("area_exit_shape");
	
	idle=StaticCString::create("idle");
	iteration=StaticCString::create("iteration");
	update=StaticCString::create("update");

	_get_gizmo_geometry=StaticCString::create("_get_gizmo_geometry");
	_can_gizmo_scale=StaticCString::create("_can_gizmo_scale");

	_fixed_process=StaticCString::create("_fixed_process");
	_process=StaticCString::create("_process");

	_enter_tree=StaticCString::create("_enter_tree");
	_exit_tree=StaticCString::create("_exit_tree");
	_enter_world=StaticCString::create("_enter_world");
	_exit_world=StaticCString::create("_exit_world");
	_ready=StaticCString::create("_ready");

	_update_scroll=StaticCString::create("_update_scroll");
	_update_xform=StaticCString::create("_update_xform");

	_proxgroup_add=StaticCString::create("_proxgroup_add");
	_proxgroup_remove=StaticCString::create("_proxgroup_remove");

	grouped=StaticCString::create("grouped");
	ungrouped=StaticCString::create("ungrouped");

	enter_screen=StaticCString::create("enter_screen");
	exit_screen=StaticCString::create("exit_screen");

	enter_viewport=StaticCString::create("enter_viewport");
	exit_viewport=StaticCString::create("exit_viewport");

	enter_camera=StaticCString::create("enter_camera");
	exit_camera=StaticCString::create("exit_camera");

	_body_enter_tree = StaticCString::create("_body_enter_tree");
	_body_exit_tree = StaticCString::create("_body_exit_tree");

	_input_event=StaticCString::create("_input_event");

	changed=StaticCString::create("changed");
	_shader_changed=StaticCString::create("_shader_changed");

	_spatial_editor_group=StaticCString::create("_spatial_editor_group");
	_request_gizmo=StaticCString::create("_request_gizmo");

	offset=StaticCString::create("offset");
	unit_offset=StaticCString::create("unit_offset");
	rotation_mode=StaticCString::create("rotation_mode");
	rotate=StaticCString::create("rotate");
	h_offset=StaticCString::create("h_offset");
	v_offset=StaticCString::create("v_offset");

	transform_pos=StaticCString::create("transform/pos");
	transform_rot=StaticCString::create("transform/rot");
	transform_scale=StaticCString::create("transform/scale");

	_update_remote=StaticCString::create("_update_remote");
	_update_pairs=StaticCString::create("_update_pairs");

	get_minimum_size=StaticCString::create("get_minimum_size");

	area_enter=StaticCString::create("area_enter");
	area_exit=StaticCString::create("area_exit");

	has_point = StaticCString::create("has_point");

	line_separation = StaticCString::create("line_separation");

	play_play = StaticCString::create("play/play");

	get_drag_data = StaticCString::create("get_drag_data");
	drop_data = StaticCString::create("drop_data");
	can_drop_data = StaticCString::create("can_drop_data");

	_im_update = StaticCString::create("_im_update");
	_queue_update = StaticCString::create("_queue_update");

	baked_light_changed = StaticCString::create("baked_light_changed");
	_baked_light_changed = StaticCString::create("_baked_light_changed");

	_mouse_enter=StaticCString::create("_mouse_enter");
	_mouse_exit=StaticCString::create("_mouse_exit");

	_pressed=StaticCString::create("_pressed");
	_toggled=StaticCString::create("_toggled");

	frame_changed=StaticCString::create("frame_changed");

}

};
