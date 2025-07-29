/*************************************************************************/
/*  scene_string_names.h                                                 */
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
#ifndef SCENE_STRING_NAMES_H
#define SCENE_STRING_NAMES_H

#include "types/string_name.h"

namespace SceneStringNames {

	void create();

	extern StringName resized;
	extern StringName dot;
	extern StringName doubledot;
	extern StringName draw;
	extern StringName hide;
	extern StringName visibility_changed;
	extern StringName input_event;
	extern StringName _input_event;
	extern StringName item_rect_changed;
	extern StringName shader_shader;
	extern StringName enter_tree;
	extern StringName exit_tree;
	extern StringName size_flags_changed;
	extern StringName minimum_size_changed;
	extern StringName idle;
	extern StringName iteration;
	extern StringName update;

	extern StringName line_separation;

	extern StringName mouse_enter;
	extern StringName mouse_exit;
	extern StringName focus_enter;
	extern StringName focus_exit;

	extern StringName sort_children;

	extern StringName finished;
	extern StringName animation_changed;

	extern StringName body_enter_shape;
	extern StringName body_enter;
	extern StringName body_exit_shape;
	extern StringName body_exit;

	extern StringName _area_enter_tree;
	extern StringName _area_exit_tree;

	
	extern StringName area_enter_shape;
	extern StringName area_exit_shape;
	
	extern StringName _get_gizmo_geometry;
	extern StringName _can_gizmo_scale;

	extern StringName _fixed_process;
	extern StringName _process;
	extern StringName _enter_world;
	extern StringName _exit_world;
	extern StringName _enter_tree;
	extern StringName _exit_tree;
	extern StringName _draw;
	extern StringName _input;
	extern StringName _ready;

	extern StringName _pressed;
	extern StringName _toggled;

	extern StringName _update_scroll;
	extern StringName _update_xform;

	extern StringName _proxgroup_add;
	extern StringName _proxgroup_remove;

	extern StringName grouped;
	extern StringName ungrouped;

	extern StringName has_point;
	extern StringName get_drag_data;
	extern StringName can_drop_data;
	extern StringName drop_data;

	extern StringName enter_screen;
	extern StringName exit_screen;
	extern StringName enter_viewport;
	extern StringName exit_viewport;
	extern StringName enter_camera;
	extern StringName exit_camera;

	extern StringName _body_enter_tree;
	extern StringName _body_exit_tree;

	extern StringName changed;
	extern StringName _shader_changed;

	extern StringName _spatial_editor_group;
	extern StringName _request_gizmo;

	extern StringName offset;
	extern StringName unit_offset;
	extern StringName rotation_mode;
	extern StringName rotate;
	extern StringName v_offset;
	extern StringName h_offset;

	extern StringName transform_pos;
	extern StringName transform_rot;
	extern StringName transform_scale;

	extern StringName _update_remote;
	extern StringName _update_pairs;

	extern StringName area_enter;
	extern StringName area_exit;

	extern StringName get_minimum_size;

	extern StringName play_play;

	extern StringName _im_update;
	extern StringName _queue_update;

	extern StringName baked_light_changed;
	extern StringName _baked_light_changed;

	extern StringName _mouse_enter;
	extern StringName _mouse_exit;

	extern StringName frame_changed;
};


#endif // SCENE_STRING_NAMES_H
