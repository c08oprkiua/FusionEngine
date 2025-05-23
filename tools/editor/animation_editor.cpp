/*************************************************************************/
/*  animation_editor.cpp                                                 */
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
#include "animation_editor.h"
#include "editor_settings.h"
#include "os/keyboard.h"
#include "os/os.h"
#include "io/resource_saver.h"
#include "pair.h"
#include "scene/gui/separator.h"
/* Missing to fix:

  *Set
  *Find better source for hint for edited value keys
  * + button on track to add a key
  * when clicked for first time, erase selection of not selected at first
  * automatically create discrete/continuous tracks!!
  *when create track do undo/redo
*/


class AnimationKeyEdit : public Object {

	OBJ_TYPE(AnimationKeyEdit,Object);
public:
	bool setting;

	static void _bind_methods() {

		ObjectTypeDB::bind_method("_update_obj",&AnimationKeyEdit::_update_obj);
		ObjectTypeDB::bind_method("_key_ofs_changed",&AnimationKeyEdit::_key_ofs_changed);
	}

	PopupDialog *ke_dialog;

	void _update_obj(const Ref<Animation> &p_anim) {
		if (setting)
			return;
		if (!ke_dialog->is_visible())
			return;
		if (!(animation==p_anim))
			return;
		notify_change();
	}

	void _key_ofs_changed(const Ref<Animation> &p_anim,float from, float to) {
		if (!ke_dialog->is_visible())
			return;
		if (!(animation==p_anim))
			return;
		if (from!=key_ofs)
			return;
		key_ofs=to;
		if (setting)
			return;
		notify_change();
	}

	bool _set(const StringName& p_name, const Variant& p_value) {

		int key = animation->track_find_key(track,key_ofs,true);
		ERR_FAIL_COND_V(key==-1,false);

		String name=p_name;
		if (name=="time") {

			float new_time = p_value;
			if (new_time==key_ofs)
				return true;

			int existing = animation->track_find_key(track,new_time,true);

			setting=true;
			undo_redo->create_action("Move Add Key",false);

			Variant val = animation->track_get_key_value(track,key);
			float trans = animation->track_get_key_transition(track,key);

			undo_redo->add_do_method(animation.ptr(),"track_remove_key",track,key);
			undo_redo->add_do_method(animation.ptr(),"track_insert_key",track,new_time,val,trans);
			undo_redo->add_do_method(this,"_key_ofs_changed",animation,key_ofs,new_time);
			undo_redo->add_undo_method(animation.ptr(),"track_remove_key_at_pos",track,new_time);
			undo_redo->add_undo_method(animation.ptr(),"track_insert_key",track,key_ofs,val,trans);
			undo_redo->add_undo_method(this,"_key_ofs_changed",animation,new_time,key_ofs);


			if (existing!=-1) {
				Variant v = animation->track_get_key_value(track,existing);
				float trans = animation->track_get_key_transition(track,existing);
				undo_redo->add_undo_method(animation.ptr(),"track_insert_key",track,new_time,v,trans);
			}

			undo_redo->commit_action();
			setting=false;

			return true;
		} else if (name=="easing") {

			float val = p_value;
			float prev_val = animation->track_get_key_transition(track,key);
			setting=true;
			undo_redo->create_action("Anim Change Transition",true);
			undo_redo->add_do_method(animation.ptr(),"track_set_key_transition",track,key,val);
			undo_redo->add_undo_method(animation.ptr(),"track_set_key_transition",track,key,prev_val);
			undo_redo->add_do_method(this,"_update_obj",animation);
			undo_redo->add_undo_method(this,"_update_obj",animation);
			undo_redo->commit_action();
			setting=false;
			return true;
		}



		switch(animation->track_get_type(track)) {


			case Animation::TYPE_TRANSFORM: {

				Dictionary d_old = animation->track_get_key_value(track,key);
				Dictionary d_new = d_old;
				d_new[p_name]=p_value;
				setting=true;
				undo_redo->create_action("Anim Change Transform3D");
				undo_redo->add_do_method(animation.ptr(),"track_set_key_value",track,key,d_new);
				undo_redo->add_undo_method(animation.ptr(),"track_set_key_value",track,key,d_old);
				undo_redo->add_do_method(this,"_update_obj",animation);
				undo_redo->add_undo_method(this,"_update_obj",animation);
				undo_redo->commit_action();
				setting=false;
				return true;

			} break;
			case Animation::TYPE_VALUE: {

				if (name=="value") {
					setting=true;
					undo_redo->create_action("Anim Change Value",true);
					Variant prev =  animation->track_get_key_value(track,key);
					undo_redo->add_do_method(animation.ptr(),"track_set_key_value",track,key,p_value);
					undo_redo->add_undo_method(animation.ptr(),"track_set_key_value",track,key,prev);
					undo_redo->add_do_method(this,"_update_obj",animation);
					undo_redo->add_undo_method(this,"_update_obj",animation);
					undo_redo->commit_action();
					setting=false;
					return true;
				}



			} break;
			case Animation::TYPE_METHOD: {

				Dictionary d_old = animation->track_get_key_value(track,key);
				Dictionary d_new = d_old;

				bool change_notify_deserved=false;
				bool mergeable=false;

				if (name=="name") {

					d_new["method"]=p_value;

				}

				if (name=="arg_count") {

					Vector<Variant> args = d_old["args"];
					args.resize(p_value);
					d_new["args"]=args;
					change_notify_deserved=true;
				}

				if (name.begins_with("args/")) {


					Vector<Variant> args = d_old["args"];
					int idx = name.get_slice("/",1).to_int();
					ERR_FAIL_INDEX_V(idx,args.size(),false);

					String what = name.get_slice("/",2);
					if (what=="type") {
						Variant::Type t = Variant::Type(int(p_value));

						if (t!=args[idx].get_type()) {
							Variant::CallError err;
							if (Variant::can_convert(args[idx].get_type(),t)) {
								Variant old=args[idx];
								Variant *ptrs[1]={&old};
								args[idx]=Variant::construct(t,(const Variant**)ptrs,1,err);
							} else {

								args[idx]=Variant::construct(t,NULL,0,err);
							}
							change_notify_deserved=true;
							d_new["args"]=args;
						}

					}
					if (what=="value") {
						args[idx]=p_value;
						d_new["args"]=args;
						mergeable=true;
					}
				}

				undo_redo->create_action("Anim Change Call",mergeable);
				Variant prev =  animation->track_get_key_value(track,key);
				setting=true;
				undo_redo->add_do_method(animation.ptr(),"track_set_key_value",track,key,d_new);
				undo_redo->add_undo_method(animation.ptr(),"track_set_key_value",track,key,d_old);
				undo_redo->add_do_method(this,"_update_obj",animation);
				undo_redo->add_undo_method(this,"_update_obj",animation);
				undo_redo->commit_action();
				setting=false;
				if (change_notify_deserved)
					notify_change();
				return true;
			} break;
		}

		return false;

		return false;

	}

	bool _get(const StringName& p_name,Variant &r_ret) const {

		int key = animation->track_find_key(track,key_ofs,true);
		ERR_FAIL_COND_V(key==-1,false);

		String name=p_name;
		if (name=="time") {
			r_ret = key_ofs;
			return true;
		} else if (name=="easing") {
			r_ret = animation->track_get_key_transition(track,key);
			return true;
		}



		switch(animation->track_get_type(track)) {


			case Animation::TYPE_TRANSFORM: {

				Dictionary d = animation->track_get_key_value(track,key);
				ERR_FAIL_COND_V(!d.has(name),false);
				r_ret = d[p_name];
				return true;

			} break;
			case Animation::TYPE_VALUE: {

				if (name=="value") {
					r_ret = animation->track_get_key_value(track,key);;
					return true;
				}



			} break;
			case Animation::TYPE_METHOD: {

				Dictionary d = animation->track_get_key_value(track,key);

				if (name=="name") {

					ERR_FAIL_COND_V(!d.has("method"),false);
					r_ret=d["method"];
					return true;
				}

				ERR_FAIL_COND_V(!d.has("args"),false);

				Vector<Variant> args = d["args"];


				if (name=="arg_count") {

					r_ret=args.size();
					return true;
				}


				if (name.begins_with("args/")) {

					int idx = name.get_slice("/",1).to_int();
					ERR_FAIL_INDEX_V(idx,args.size(),false);

					String what = name.get_slice("/",2);
					if (what=="type") {
						r_ret=args[idx].get_type();
						return true;
					}
					if (what=="value") {
						r_ret=args[idx];
						return true;
					}
				}

			} break;
		}

		return false;
	}
	void _get_property_list( List<PropertyInfo> *p_list) const {

		if (animation.is_null())
			return;

		ERR_FAIL_INDEX(track,animation->get_track_count());
		int key = animation->track_find_key(track,key_ofs,true);
		ERR_FAIL_COND(key==-1);

		p_list->push_back( PropertyInfo( Variant::REAL, "time", PROPERTY_HINT_RANGE,"0,"+rtos(animation->get_length())+",0.01") );

		switch(animation->track_get_type(track)) {


			case Animation::TYPE_TRANSFORM: {

				p_list->push_back( PropertyInfo( Variant::VECTOR3, "loc"));
				p_list->push_back( PropertyInfo( Variant::QUAT, "rot"));
				p_list->push_back( PropertyInfo( Variant::VECTOR3, "scale"));

			} break;
			case Animation::TYPE_VALUE: {

				Variant v = animation->track_get_key_value(track,key);



				if (hint.type!=Variant::NIL) {

					PropertyInfo pi=hint;
					pi.name="value";
					p_list->push_back( pi );
				} else {

					PropertyHint hint= PROPERTY_HINT_NONE;
					String hint_string;

					if (v.get_type()==Variant::OBJECT) {
						//could actually check the object property if exists..? yes i will!
						Ref<Resource> res = v;
						if (res.is_valid()) {

							hint=PROPERTY_HINT_RESOURCE_TYPE;
							hint_string=res->get_type();
						}
					}

					if (v.get_type()!=Variant::NIL)
						p_list->push_back( PropertyInfo( v.get_type(), "value", hint,hint_string));
				}

			} break;
			case Animation::TYPE_METHOD: {

				p_list->push_back( PropertyInfo( Variant::STRING, "name"));
				p_list->push_back( PropertyInfo( Variant::INT, "arg_count",PROPERTY_HINT_RANGE,"0,5,1"));

				Dictionary d = animation->track_get_key_value(track,key);
				ERR_FAIL_COND(!d.has("args"));
				Vector<Variant> args=d["args"];
				String vtypes;
				for(int i=0;i<Variant::VARIANT_MAX;i++) {

					if (i>0)
						vtypes+=",";
					vtypes+=Variant::get_type_name( Variant::Type(i) );
				}

				for(int i=0;i<args.size();i++) {

					p_list->push_back( PropertyInfo( Variant::INT, "args/"+itos(i)+"/type", PROPERTY_HINT_ENUM,vtypes));
					if (args[i].get_type()!=Variant::NIL)
						p_list->push_back( PropertyInfo( args[i].get_type(), "args/"+itos(i)+"/value"));
				}

			} break;
		}

		if (animation->track_get_type(track)!=Animation::TYPE_METHOD)
			p_list->push_back( PropertyInfo( Variant::REAL, "easing", PROPERTY_HINT_EXP_EASING));
	}

	UndoRedo *undo_redo;
	Ref<Animation> animation;
	int track;
	float key_ofs;

	PropertyInfo hint;


	void notify_change() {

		_change_notify();
	}

	AnimationKeyEdit() { key_ofs=0; track=-1; setting=false; }

};


void AnimationKeyEditor::_menu_track(int p_type) {

	ERR_FAIL_COND(!animation.is_valid());


	last_menu_track_opt=p_type;
	switch(p_type) {

		case TRACK_MENU_ADD_VALUE_TRACK:
		case TRACK_MENU_ADD_TRANSFORM_TRACK:
		case TRACK_MENU_ADD_CALL_TRACK: {

			undo_redo->create_action("Anim Add Track");
			undo_redo->add_do_method(animation.ptr(),"add_track",p_type);			
			undo_redo->add_do_method(animation.ptr(),"track_set_path",animation->get_track_count(),".");
			undo_redo->add_undo_method(animation.ptr(),"remove_track",animation->get_track_count());
			undo_redo->commit_action();


		} break;
		case TRACK_MENU_SCALE:
		case TRACK_MENU_SCALE_PIVOT: {

			scale_dialog->popup_centered(Size2(200,100));
		} break;
		case TRACK_MENU_MOVE_UP: {

			int idx=selected_track;
			if (idx>0 && idx<animation->get_track_count()) {
				undo_redo->create_action("Move Anim Track Up");
				undo_redo->add_do_method(animation.ptr(),"track_move_down",idx);
				undo_redo->add_undo_method(animation.ptr(),"track_move_up",idx-1);
				undo_redo->commit_action();
				selected_track=idx-1;
			}

		} break;
		case TRACK_MENU_MOVE_DOWN: {


			int idx=selected_track;
			if (idx>=0 && idx<animation->get_track_count()-1) {
				undo_redo->create_action("Move Anim Track Down");
				undo_redo->add_do_method(animation.ptr(),"track_move_up",idx);
				undo_redo->add_undo_method(animation.ptr(),"track_move_down",idx+1);
				undo_redo->commit_action();
				selected_track=idx+1;
			}

		} break;
		case TRACK_MENU_REMOVE: {

			int idx=selected_track;
			if (idx>=0 && idx<animation->get_track_count()) {
				undo_redo->create_action("Remove Anim Track");
				undo_redo->add_do_method(animation.ptr(),"remove_track",idx);
				undo_redo->add_undo_method(animation.ptr(),"add_track",animation->track_get_type(idx),idx);
				undo_redo->add_undo_method(animation.ptr(),"track_set_path",idx,animation->track_get_path(idx));
				//todo interpolation
				for(int i=0;i<animation->track_get_key_count(idx);i++) {

					Variant v = animation->track_get_key_value(idx,i);
					float time =  animation->track_get_key_time(idx,i);
					float trans =  animation->track_get_key_transition(idx,i);

					undo_redo->add_undo_method(animation.ptr(),"track_insert_key",idx,time,v);
					undo_redo->add_undo_method(animation.ptr(),"track_set_key_transition",idx,i,trans);

				}

				undo_redo->add_undo_method(animation.ptr(),"track_set_interpolation_type",idx,animation->track_get_interpolation_type(idx));
				if (animation->track_get_type(idx)==Animation::TYPE_VALUE) {
					undo_redo->add_undo_method(animation.ptr(),"value_track_set_continuous",idx,animation->value_track_is_continuous(idx));

				}

				undo_redo->commit_action();
			}


		} break;
		case TRACK_MENU_DUPLICATE:
		case TRACK_MENU_DUPLICATE_TRANSPOSE: {


			//duplicait!
			if (selection.size() && animation.is_valid() && selected_track>=0 && selected_track<animation->get_track_count()) {


				int top_track=0x7FFFFFFF;
				float top_time = 1e10;
				for(Map<SelectedKey,KeyInfo>::Element *E=selection.back();E;E=E->prev()) {

					const SelectedKey &sk = E->key();

					float t = animation->track_get_key_time(sk.track,sk.key);
					if (t<top_time)
						top_time=t;
					if (sk.track<top_track)
						top_track=sk.track;


				}
				ERR_FAIL_COND( top_track == 0x7FFFFFFF || top_time==1e10 );

				//

				int start_track = p_type==TRACK_MENU_DUPLICATE_TRANSPOSE ? selected_track : top_track;


				undo_redo->create_action("Anim Duplicate Keys");

				List<Pair<int,float> > new_selection_values;

				for(Map<SelectedKey,KeyInfo>::Element *E=selection.back();E;E=E->prev()) {

					const SelectedKey &sk = E->key();

					float t = animation->track_get_key_time(sk.track,sk.key);

					float dst_time = t+(timeline_pos - top_time);
					int dst_track = sk.track + (start_track - top_track);

					if (dst_track < 0 || dst_track>= animation->get_track_count())
						continue;

					if (animation->track_get_type(dst_track) != animation->track_get_type(sk.track))
						continue;

					int existing_idx = animation->track_find_key(dst_track,dst_time,true);

					undo_redo->add_do_method(animation.ptr(),"track_insert_key",dst_track,dst_time,animation->track_get_key_value(E->key().track,E->key().key),animation->track_get_key_transition(E->key().track,E->key().key));
					undo_redo->add_undo_method(animation.ptr(),"track_remove_key_at_pos",dst_track,dst_time);

					Pair<int,float> p;
					p.first=dst_track;
					p.second=dst_time;
					new_selection_values.push_back( p );

					if (existing_idx!=-1) {

						undo_redo->add_undo_method(animation.ptr(),"track_insert_key",dst_track,dst_time,animation->track_get_key_value(dst_track,existing_idx),animation->track_get_key_transition(dst_track,existing_idx));

					}



				}

				undo_redo->commit_action();

				//reselect duplicated

				Map<SelectedKey,KeyInfo> new_selection;
				for (List<Pair<int,float> >::Element *E=new_selection_values.front();E;E=E->next()) {


					int track=E->get().first;
					float time = E->get().second;

					int existing_idx = animation->track_find_key(track,time,true);

					if (existing_idx==-1)
						continue;
					SelectedKey sk2;
					sk2.track=track;
					sk2.key=existing_idx;

					KeyInfo ki;
					ki.pos=time;


					new_selection[sk2]=ki;


				}


				selection=new_selection;
				track_editor->update();

			}


		} break;
		case TRACK_MENU_SET_ALL_TRANS_LINEAR:
		case TRACK_MENU_SET_ALL_TRANS_CONSTANT:
		case TRACK_MENU_SET_ALL_TRANS_OUT:
		case TRACK_MENU_SET_ALL_TRANS_IN:
		case TRACK_MENU_SET_ALL_TRANS_INOUT:
		case TRACK_MENU_SET_ALL_TRANS_OUTIN: {

			if (!selection.size() || !animation.is_valid())
				break;

			float t=0;
			switch(p_type) {
				case TRACK_MENU_SET_ALL_TRANS_LINEAR: t=1.0; break;
				case TRACK_MENU_SET_ALL_TRANS_CONSTANT: t=0.0; break;
				case TRACK_MENU_SET_ALL_TRANS_OUT: t=0.5; break;
				case TRACK_MENU_SET_ALL_TRANS_IN: t=2.0; break;
				case TRACK_MENU_SET_ALL_TRANS_INOUT: t=-0.5; break;
				case TRACK_MENU_SET_ALL_TRANS_OUTIN: t=-2.0; break;
			}

			undo_redo->create_action("Set Transitions to: "+rtos(t));

			for(Map<SelectedKey,KeyInfo>::Element *E=selection.back();E;E=E->prev()) {

				const SelectedKey &sk = E->key();

				undo_redo->add_do_method(animation.ptr(),"track_set_key_transition",sk.track,sk.key,t);
				undo_redo->add_undo_method(animation.ptr(),"track_set_key_transition",sk.track,sk.key,animation->track_get_key_transition(sk.track,sk.key));

			}

			undo_redo->commit_action();

		} break;
		case TRACK_MENU_NEXT_STEP: {

			if (animation.is_null())
				break;
			float step = animation->get_step();
			if (step==0)
				step=1;

			float pos=timeline_pos;

			pos=Math::stepify(pos+step,step);
			if (pos>animation->get_length())
				pos=animation->get_length();
			timeline_pos=pos;
			track_pos->update();
			emit_signal("timeline_changed",pos);

		} break;
		case TRACK_MENU_PREV_STEP: {
			if (animation.is_null())
				break;
			float step = animation->get_step();
			if (step==0)
				step=1;

			float pos=timeline_pos;
			pos=Math::stepify(pos-step,step);
			if (pos<0)
				pos=0;
			timeline_pos=pos;
			track_pos->update();
			emit_signal("timeline_changed",pos);


		} break;

		case TRACK_MENU_OPTIMIZE: {

			optimize_dialog->popup_centered(Size2(250,180));
		} break;



	}

}


void AnimationKeyEditor::_animation_optimize()  {


	print_line("OPTIMIZE!");
	animation->optimize(optimize_linear_error->get_val(),optimize_angular_error->get_val(),optimize_max_angle->get_val());
	track_editor->update();
	undo_redo->clear_history();

}


float AnimationKeyEditor::_get_zoom_scale() const {

	float zv = zoom->get_val();
	if (zv<1) {
		zv = 1.0-zv;
		return Math::pow(1.0+zv,8.0)*100;
	} else {
		return 1.0/Math::pow(zv,8.0)*100;
	}
}

void AnimationKeyEditor::_track_pos_draw() {

	if (!animation.is_valid()) {
		return;
	}


	Ref<StyleBox> style = get_stylebox("normal","TextEdit");
	Size2 size= track_editor->get_size() - style->get_minimum_size();
	Size2 ofs = style->get_offset();

	int settings_limit = size.width - right_data_size_cache;
	int name_limit = settings_limit * name_column_ratio;

	float keys_from= h_scroll->get_val();
	float zoom_scale = _get_zoom_scale();
	float keys_to=keys_from+(settings_limit-name_limit) / zoom_scale;



	//will move to separate control! (for speedup)
	if (timeline_pos >= keys_from && timeline_pos<keys_to) {
		//draw position
		int pixel = (timeline_pos - h_scroll->get_val()) * zoom_scale;
		pixel+=name_limit;
		track_pos->draw_line(ofs+Point2(pixel,0),ofs+Point2(pixel,size.height),Color(1,0.3,0.3,0.8));

	}
}

void AnimationKeyEditor::_track_editor_draw() {

	VisualServer::get_singleton()->canvas_item_set_clip(track_editor->get_canvas_item(),true);

	if (animation.is_valid() && animation->get_track_count()) {
		if (selected_track < 0)
			selected_track=0;
		else if (selected_track>=animation->get_track_count())
			selected_track=animation->get_track_count()-1;
	}

	track_pos->update();
	Control *te=track_editor;
	Ref<StyleBox> style = get_stylebox("normal","TextEdit");
	te->draw_style_box(style,Rect2(Point2(),track_editor->get_size()));

	if (te->has_focus()) {
		te->draw_style_box(get_stylebox("bg_focus","Tree"),Rect2(Point2(),track_editor->get_size()));
	}

	if (!animation.is_valid()) {
		v_scroll->hide();
		h_scroll->hide();
		menu_track->set_disabled(true);
		edit_button->set_disabled(true);
		move_up_button->set_disabled(true);
		move_down_button->set_disabled(true);
		remove_button->set_disabled(true);
		return;
	}

	menu_track->set_disabled(false);
	edit_button->set_disabled(false);
	move_up_button->set_disabled(false);
	move_down_button->set_disabled(false);
	remove_button->set_disabled(false);

	te_drawing=true;

	Size2 size= te->get_size() - style->get_minimum_size();
	Size2 ofs = style->get_offset();

	Ref<Font> font = te->get_font("font","Tree");
	int sep = get_constant("vseparation","Tree");
	int hsep = get_constant("hseparation","Tree");
	Color color = get_color("font_color","Tree");
	Color sepcolor = get_color("guide_color","Tree");
	Color timecolor = get_color("prop_subsection","Editor");
	timecolor = Color::html("ff4a414f");
	Color hover_color = Color(1,1,1,0.05);
	Color select_color = Color(1,1,1,0.1);
	Color track_select_color =Color::html("ffbd8e8e");

	Ref<Texture> remove_icon = get_icon("Remove","EditorIcons");
	Ref<Texture> move_up_icon = get_icon("MoveUp","EditorIcons");
	Ref<Texture> move_down_icon = get_icon("MoveDown","EditorIcons");
	Ref<Texture> remove_icon_hl = get_icon("RemoveHl","EditorIcons");
	Ref<Texture> move_up_icon_hl = get_icon("MoveUpHl","EditorIcons");
	Ref<Texture> move_down_icon_hl = get_icon("MoveDownHl","EditorIcons");
	Ref<Texture> add_key_icon = get_icon("TrackAddKey","EditorIcons");
	Ref<Texture> add_key_icon_hl = get_icon("TrackAddKeyHl","EditorIcons");
	Ref<Texture> down_icon = get_icon("select_arrow","Tree");
	Ref<Texture> interp_icon[3]={
		get_icon("InterpRaw","EditorIcons"),
		get_icon("InterpLinear","EditorIcons"),
		get_icon("InterpCubic","EditorIcons")
	};
	Ref<Texture> cont_icon[3]={
		get_icon("TrackDiscrete","EditorIcons"),
		get_icon("TrackContinuous","EditorIcons")
	};
	Ref<Texture> type_icon[3]={
		get_icon("KeyValue","EditorIcons"),
		get_icon("KeyXform","EditorIcons"),
		get_icon("KeyCall","EditorIcons")
	};

	Ref<Texture> hsize_icon = get_icon("Hsize","EditorIcons");

	Ref<Texture> type_hover=get_icon("KeyHover","EditorIcons");
	Ref<Texture> type_selected=get_icon("KeySelected","EditorIcons");

	int right_separator_ofs = down_icon->get_width() *2 + add_key_icon->get_width() + interp_icon[0]->get_width() + cont_icon[0]->get_width() + hsep*7;

	int h = font->get_height()+sep;


	int fit = (size.height / h)-1;
	int total = animation->get_track_count();
	if (total < fit) {
		v_scroll->hide();
		v_scroll->set_max( total );
		v_scroll->set_page( fit );
	} else {
		v_scroll->show();
		v_scroll->set_max( total );
		v_scroll->set_page( fit );
	}


	int settings_limit = size.width - right_separator_ofs;
	int name_limit = settings_limit * name_column_ratio;

	te->draw_line(ofs+Point2(name_limit,0),ofs+Point2(name_limit,size.height),color);
	te->draw_line(ofs+Point2(settings_limit,0),ofs+Point2(settings_limit,size.height),color);
	te->draw_texture(hsize_icon,ofs+Point2(name_limit-hsize_icon->get_width()-hsep,(h-hsize_icon->get_height())/2));

	te->draw_line(ofs+Point2(0,h),ofs+Point2(size.width,h),color);
	// draw time

	float keys_from;
	float keys_to;
	float zoom_scale;


	{

		int zoomw = settings_limit-name_limit;


		float scale = _get_zoom_scale();
		zoom_scale=scale;


		float l = animation->get_length();
		if (l<=0)
			l=0.001; //avoid crashor

		int end_px = (l - h_scroll->get_val()) * scale;
		int begin_px = -h_scroll->get_val() * scale;
		Color notimecol;
		notimecol.r=timecolor.gray();
		notimecol.g=notimecol.r;
		notimecol.b=notimecol.r;
		notimecol.a=timecolor.a;

		{

			te->draw_rect(Rect2( ofs + Point2(name_limit,0), Point2(zoomw-1,h)), notimecol);


			if (begin_px < zoomw && end_px >0) {

				if (begin_px<0)
					begin_px=0;
				if (end_px>zoomw)
					end_px=zoomw;

				te->draw_rect(Rect2( ofs + Point2(name_limit+begin_px,0), Point2(end_px-begin_px-1,h)), timecolor);
			}

		}



		keys_from= h_scroll->get_val();
		keys_to=keys_from+zoomw / scale;

		{
			float time_min=0;
			float time_max=animation->get_length();
			for(int i=0;i<animation->get_track_count();i++) {

				if (animation->track_get_key_count(i)>0) {

					float beg = animation->track_get_key_time(i,0);
					if (beg<time_min)
						time_min=beg;
					float end = animation->track_get_key_time(i,animation->track_get_key_count(i)-1);
					if (end>time_max)
						time_max=end;
				}
			}





			float extra = (zoomw / scale)*0.5;

			if (time_min<-0.001)
				time_min-=extra;
			time_max+=extra;
			h_scroll->set_min(time_min);
			h_scroll->set_max(time_max);

			if (zoomw / scale < (time_max-time_min)) {
				h_scroll->show();

			} else {

				h_scroll->hide();
			}


		}

		h_scroll->set_page(zoomw / scale);

		Color color_time_sec = color;
		Color color_time_dec = color;
		color_time_dec.a*=0.5;
#define SC_ADJ 100
		int min=30;
		int dec=1;
		int step=1;
		int decimals=2;
		bool step_found=false;

		while(!step_found) {

			static const int _multp[3]={1,2,5};
			for(int i=0;i<3;i++) {

				step = (_multp[i] * dec);
				if (step*scale/SC_ADJ> min) {
					step_found=true;
					break;
				}

			}
			if (step_found)
				break;
			dec*=10;
			decimals--;
			if (decimals<0)
				decimals=0;
		}


		for(int i=0;i<zoomw;i++) {

			float pos = h_scroll->get_val() + double(i)/scale;
			float prev = h_scroll->get_val() + (double(i)-1.0)/scale;


			int sc = int(Math::floor(pos*SC_ADJ));
			int prev_sc = int(Math::floor(prev*SC_ADJ));
			bool sub = (sc % SC_ADJ);

			if ((sc/step)!=(prev_sc/step) || (prev_sc<0 && sc>=0)) {

				int scd = sc < 0 ? prev_sc : sc;
				te->draw_line( ofs + Point2(name_limit+i,0), ofs+Point2(name_limit+i,h), color);
				te->draw_string( font,ofs + Point2(name_limit+i+3,(h-font->get_height())/2+font->get_ascent()).floor(),String::num((scd-(scd%step))/double(SC_ADJ),decimals),sub?color_time_dec:color_time_sec,zoomw-i);
			}


		}
	}

	for(int i=0;i<fit;i++) {

		int idx = v_scroll->get_val() + i;
		if (idx>=animation->get_track_count())
			break;
		int y = h+i*h+sep;


		if (/*mouse_over.over!=MouseOver::OVER_NONE &&*/ idx==mouse_over.track) {
			Color sepc=hover_color;
			te->draw_rect(Rect2(ofs+Point2(0,y),Size2(size.width,h-1)),sepc);
		}

		if (selected_track==idx) {
			Color tc = select_color;
			//tc.a*=0.7;
			te->draw_rect(Rect2(ofs+Point2(0,y),Size2(size.width-1,h-1)),tc);
		}

		te->draw_texture(type_icon[animation->track_get_type(idx)],ofs+Point2(0,y+(h-type_icon[0]->get_height())/2).floor());
		NodePath np = animation->track_get_path(idx);
		Node *n = root->get_node(np);
		Color ncol = color;
		if (n && editor_selection->is_selected(n))
			ncol=track_select_color;
		te->draw_string(font,Point2(ofs+Point2(type_icon[0]->get_width()+sep,y+font->get_ascent()+(sep/2))).floor(),np,ncol,name_limit-(type_icon[0]->get_width()+sep)-5);


		te->draw_line(ofs+Point2(0,y+h),ofs+Point2(size.width,y+h),sepcolor);

		Point2 icon_ofs = ofs + Point2( size.width,  y + (h - remove_icon->get_height() )/2).floor();
		icon_ofs.y+=4;


/*		icon_ofs.x-=remove_icon->get_width();

		te->draw_texture((mouse_over.over==MouseOver::OVER_REMOVE && mouse_over.track==idx)?remove_icon_hl:remove_icon,icon_ofs);
		icon_ofs.x-=hsep;
		icon_ofs.x-=move_down_icon->get_width();
		te->draw_texture((mouse_over.over==MouseOver::OVER_DOWN && mouse_over.track==idx)?move_down_icon_hl:move_down_icon,icon_ofs);
		icon_ofs.x-=hsep;
		icon_ofs.x-=move_up_icon->get_width();
		te->draw_texture((mouse_over.over==MouseOver::OVER_UP && mouse_over.track==idx)?move_up_icon_hl:move_up_icon,icon_ofs);
		icon_ofs.x-=hsep;
		te->draw_line(Point2(icon_ofs.x,ofs.y+y),Point2(icon_ofs.x,ofs.y+y+h),sepcolor);

		icon_ofs.x-=hsep;
		*/
		icon_ofs.x-=down_icon->get_width();
		te->draw_texture(down_icon,icon_ofs);

		int interp_type = animation->track_get_interpolation_type(idx);
		ERR_CONTINUE(interp_type<0 || interp_type>=3);
		icon_ofs.x-=hsep;
		icon_ofs.x-=interp_icon[interp_type]->get_width();
		te->draw_texture(interp_icon[interp_type],icon_ofs);

		icon_ofs.x-=hsep;
		te->draw_line(Point2(icon_ofs.x,ofs.y+y),Point2(icon_ofs.x,ofs.y+y+h),sepcolor);

		if (animation->track_get_type(idx)==Animation::TYPE_VALUE) {


			int continuous = animation->value_track_is_continuous(idx)?1:0;

			icon_ofs.x-=hsep;
			icon_ofs.x-=down_icon->get_width();
			te->draw_texture(down_icon,icon_ofs);

			icon_ofs.x-=hsep;
			icon_ofs.x-=cont_icon[continuous]->get_width();
			te->draw_texture(cont_icon[continuous],icon_ofs);
		} else {

			icon_ofs.x -= hsep*2 + cont_icon[0]->get_width() + down_icon->get_width();
		}

		icon_ofs.x-=hsep;
		te->draw_line(Point2(icon_ofs.x,ofs.y+y),Point2(icon_ofs.x,ofs.y+y+h),sepcolor);

		icon_ofs.x-=hsep;
		icon_ofs.x-=add_key_icon->get_width();
		te->draw_texture((mouse_over.over==MouseOver::OVER_ADD_KEY && mouse_over.track==idx)?add_key_icon_hl:add_key_icon,icon_ofs);

		//draw the keys;
		int tt = animation->track_get_type(idx);
		float key_vofs = Math::floor((h - type_icon[tt]->get_height())/2);
		float key_hofs = -Math::floor(type_icon[tt]->get_height()/2);

		int kc=animation->track_get_key_count(idx);

		for(int i=0;i<kc;i++) {


			float time = animation->track_get_key_time(idx,i);
			if (time<keys_from)
				continue;
			if (time>keys_to)
				break;
			float x =  key_hofs + name_limit + (time-keys_from)*zoom_scale;

			Ref<Texture> tex = type_icon[tt];

			SelectedKey sk;
			sk.key=i;
			sk.track=idx;
			if (selection.has(sk)) {

				if (click.click==ClickOver::CLICK_MOVE_KEYS)
					continue;
				tex=type_selected;
			}

			if (mouse_over.over==MouseOver::OVER_KEY && mouse_over.track==idx && mouse_over.over_key==i)
				tex=type_hover;

			te->draw_texture(tex,ofs+Point2(x,y+key_vofs).floor());
		}

	}

	switch(click.click) {
		case ClickOver::CLICK_SELECT_KEYS: {

			te->draw_rect(Rect2(click.at,click.to-click.at),Color(0.7,0.7,1.0,0.5));

		} break;
		case ClickOver::CLICK_MOVE_KEYS: {

			float from_t = 1e20;

			for(Map<SelectedKey,KeyInfo>::Element *E=selection.front();E;E=E->next()) {
				float t = animation->track_get_key_time(E->key().track,E->key().key);
				if (t<from_t)
					from_t=t;

			}

			float motion = from_t+(click.to.x - click.at.x)/zoom_scale;
			if (step->get_val())
				motion = Math::stepify(motion,step->get_val());

			for(Map<SelectedKey,KeyInfo>::Element *E=selection.front();E;E=E->next()) {


				int idx = E->key().track;
				int i = idx-v_scroll->get_val();
				if (i<0 || i>=fit)
					continue;
				int y = h+i*h+sep;

				float key_vofs = Math::floor((h - type_selected->get_height())/2);
				float key_hofs = -Math::floor(type_selected->get_height()/2);

				float time = animation->track_get_key_time(idx,E->key().key);
				float diff = time-from_t;

				float t = motion + diff;

				float x =  (t-keys_from)*zoom_scale;
				//x+=click.to.x - click.at.x;
				if (x<0 || x>=(settings_limit-name_limit))
					continue;

				x+=name_limit;

				te->draw_texture(type_selected,ofs+Point2(x+key_hofs,y+key_vofs).floor());

			}
		} break;
		default: {};
	}


	te_drawing=false;
}

void AnimationKeyEditor::_track_name_changed(const String& p_name) {

	ERR_FAIL_COND(!animation.is_valid());
	undo_redo->create_action("Anim Track Rename");
	undo_redo->add_do_method(animation.ptr(),"track_set_path",track_name_editing,p_name);
	undo_redo->add_undo_method(animation.ptr(),"track_set_path",track_name_editing,animation->track_get_path(track_name_editing));
	undo_redo->commit_action();
	track_name->hide();

}

void AnimationKeyEditor::_track_menu_selected(int p_idx) {


	ERR_FAIL_COND(!animation.is_valid());

	if (interp_editing!=-1) {

		ERR_FAIL_INDEX(interp_editing,animation->get_track_count());
		undo_redo->create_action("Anim Track Change Interpolation");
		undo_redo->add_do_method(animation.ptr(),"track_set_interpolation_type",interp_editing,p_idx);
		undo_redo->add_undo_method(animation.ptr(),"track_set_interpolation_type",interp_editing,animation->track_get_interpolation_type(interp_editing));
		undo_redo->commit_action();
	}

	if (cont_editing!=-1) {

		ERR_FAIL_INDEX(cont_editing,animation->get_track_count());

		undo_redo->create_action("Anim Track Change Value Mode");
		undo_redo->add_do_method(animation.ptr(),"value_track_set_continuous",cont_editing,p_idx);
		undo_redo->add_undo_method(animation.ptr(),"value_track_set_continuous",cont_editing,animation->value_track_is_continuous(cont_editing));
		undo_redo->commit_action();
	}

}

struct _AnimMoveRestore {

	int track;
	float time;
	Variant key;
	float transition;
};

void AnimationKeyEditor::_clear_selection_for_anim(const Ref<Animation>& p_anim) {

	if (!(animation==p_anim))
		return;
	selection.clear();
}

void AnimationKeyEditor::_select_at_anim(const Ref<Animation>& p_anim,int p_track,float p_pos){

	if (!(animation==p_anim))
		return;

	int idx = animation->track_find_key(p_track,p_pos,true);
	ERR_FAIL_COND(idx<0);

	SelectedKey sk;
	sk.track=p_track;
	sk.key=idx;
	KeyInfo ki;
	ki.pos=p_pos;

	selection.insert(sk,ki);
}


PropertyInfo AnimationKeyEditor::_find_hint_for_track(int p_idx) {

	ERR_FAIL_COND_V(!animation.is_valid(),PropertyInfo());
	ERR_FAIL_INDEX_V(p_idx,animation->get_track_count(),PropertyInfo());

	if (!root)
		return PropertyInfo();

	NodePath path = animation->track_get_path(p_idx);

	String property = path.get_property();
	if (property=="")
		return PropertyInfo();

	if (!root->has_node_and_resource(path))
		return PropertyInfo();

	RES res;
	Node *node = root->get_node_and_resource(path,res);

	List<PropertyInfo> pinfo;
	if (res.is_valid())
		res->get_property_list(&pinfo);
	else
		node->get_property_list(&pinfo);

	for(List<PropertyInfo>::Element *E=pinfo.front();E;E=E->next()) {

		if (E->get().name==property)
			return E->get();
	}

	return PropertyInfo();
}



void AnimationKeyEditor::_track_editor_input_event(const InputEvent& p_input) {

	Control *te=track_editor;
	Ref<StyleBox> style = get_stylebox("normal","TextEdit");

	if (!animation.is_valid()) {
		return;
	}

	Size2 size= te->get_size() - style->get_minimum_size();
	Size2 ofs = style->get_offset();

	Ref<Font> font = te->get_font("font","Tree");
	int sep = get_constant("vseparation","Tree");
	int hsep = get_constant("hseparation","Tree");
	Color color = get_color("font_color","Tree");
	Color sepcolor = get_color("guide_color","Tree");
	Ref<Texture> remove_icon = get_icon("Remove","EditorIcons");
	Ref<Texture> move_up_icon = get_icon("MoveUp","EditorIcons");
	Ref<Texture> move_down_icon = get_icon("MoveDown","EditorIcons");
	Ref<Texture> down_icon = get_icon("select_arrow","Tree");
	Ref<Texture> hsize_icon = get_icon("Hsize","EditorIcons");
	Ref<Texture> add_key_icon = get_icon("TrackAddKey","EditorIcons");

	Ref<Texture> interp_icon[3]={
		get_icon("InterpRaw","EditorIcons"),
		get_icon("InterpLinear","EditorIcons"),
		get_icon("InterpCubic","EditorIcons")
	};
	Ref<Texture> cont_icon[3]={
		get_icon("TrackDiscrete","EditorIcons"),
		get_icon("TrackContinuous","EditorIcons")
	};
	Ref<Texture> type_icon[3]={
		get_icon("KeyValue","EditorIcons"),
		get_icon("KeyXform","EditorIcons"),
		get_icon("KeyCall","EditorIcons")
	};
	int right_separator_ofs = down_icon->get_width() *2 + add_key_icon->get_width() + interp_icon[0]->get_width() + cont_icon[0]->get_width() + hsep*7;

	int h = font->get_height()+sep;

	int fit = (size.height / h)-1;
	int total = animation->get_track_count();
	if (total < fit) {
		v_scroll->hide();
	} else {
		v_scroll->show();
		v_scroll->set_max( total );
		v_scroll->set_page( fit );
	}

	int settings_limit = size.width - right_separator_ofs;
	int name_limit = settings_limit * name_column_ratio;


	switch(p_input.type) {

		case InputEvent::KEY: {

			if (p_input.key.scancode==KEY_D && p_input.key.pressed && p_input.key.mod.command) {

				if (p_input.key.mod.shift)
					_menu_track(TRACK_MENU_DUPLICATE_TRANSPOSE);
				else
					_menu_track(TRACK_MENU_DUPLICATE);

				accept_event();;

			} else if (p_input.key.scancode==KEY_DELETE && p_input.key.pressed && click.click==ClickOver::CLICK_NONE) {

				if (selection.size()) {
					undo_redo->create_action("Anim Delete Keys");

					for(Map<SelectedKey,KeyInfo>::Element *E=selection.back();E;E=E->prev()) {

						undo_redo->add_do_method(animation.ptr(),"track_remove_key",E->key().track,E->key().key);
						undo_redo->add_undo_method(animation.ptr(),"track_insert_key",E->key().track,E->get().pos,animation->track_get_key_value(E->key().track,E->key().key),animation->track_get_key_transition(E->key().track,E->key().key));

					}
					undo_redo->commit_action();
					selection.clear();
					accept_event();
				}
			} else if (animation.is_valid() && animation->get_track_count()>0) {

				if (p_input.is_pressed() && (p_input.is_action("ui_up") || p_input.is_action("ui_page_up"))) {

					if (p_input.is_action("ui_up"))
						selected_track--;
					if (v_scroll->is_visible() && p_input.is_action("ui_page_up"))
						selected_track=selected_track--;;

					if (selected_track<0)
						selected_track=0;


					if (v_scroll->is_visible()) {
						if (v_scroll->get_val() > selected_track)
							v_scroll->set_val(selected_track);

					}

					track_editor->update();
					accept_event();

				}

				if (p_input.is_pressed() && (p_input.is_action("ui_down") || p_input.is_action("ui_page_down"))) {

					if (p_input.is_action("ui_down"))
						selected_track++;
					else if (v_scroll->is_visible() && p_input.is_action("ui_page_down"))
						selected_track+=v_scroll->get_page();

					if (selected_track >= animation->get_track_count())
						selected_track=animation->get_track_count()-1;

					if (v_scroll->is_visible() && v_scroll->get_page()+v_scroll->get_val() < selected_track+1) {
						v_scroll->set_val(selected_track-v_scroll->get_page()+1);
					}

					track_editor->update();
					accept_event();
				}
			}


		} break;
		case InputEvent::MOUSE_BUTTON: {

			const InputEventMouseButton &mb = p_input.mouse_button;

			if (mb.button_index==BUTTON_WHEEL_UP && mb.pressed) {

				v_scroll->set_val( v_scroll->get_val() - v_scroll->get_page() / 8 );
			}

			if (mb.button_index==BUTTON_WHEEL_DOWN && mb.pressed) {

				v_scroll->set_val( v_scroll->get_val() + v_scroll->get_page() / 8 );
			}

			if (mb.button_index==BUTTON_LEFT && !(mb.button_mask&~BUTTON_MASK_LEFT)) {


				if (mb.pressed) {

					Point2 mpos = Point2(mb.x,mb.y)-ofs;

					if (mpos.y < h ) {


						if (mpos.x<name_limit && mpos.x > (name_limit - hsep - hsize_icon->get_width())) {


							click.click=ClickOver::CLICK_RESIZE_NAMES;
							click.at=Point2(mb.x,mb.y);
							click.to=click.at;
							click.at.y=name_limit;

						}

						if (mpos.x>=name_limit && mpos.x<settings_limit) {
							//seek
							//int zoomw = settings_limit-name_limit;
							float scale = _get_zoom_scale();
							float pos = h_scroll->get_val() + (mpos.x-name_limit) / scale;
							if (animation->get_step())
								pos=Math::stepify(pos,animation->get_step());

							if (pos<0 )
								pos=0;
							if (pos>=animation->get_length())
								pos=animation->get_length();
							timeline_pos=pos;
							click.click=ClickOver::CLICK_DRAG_TIMELINE;
							click.at=Point2(mb.x,mb.y);
							click.to=click.at;
							emit_signal("timeline_changed",pos);

						}

						return;
					}

					mpos.y -= h;

					int idx = mpos.y / h;
					idx+=v_scroll->get_val();
					if (idx <0)
						break;

					if (idx>=animation->get_track_count()) {

						if (mpos.x >= name_limit && mpos.x<settings_limit) {

							click.click=ClickOver::CLICK_SELECT_KEYS;
							click.at=Point2(mb.x,mb.y);
							click.to=click.at;
							//drag select region
						}

						break;
					}

					if (mpos.x < name_limit) {
						//name column

						// area
						if (idx!=selected_track) {

							selected_track=idx;
							track_editor->update();
							break;
						}

						Rect2 area(ofs.x,ofs.y+((int(mpos.y)/h)+1)*h,name_limit, h );
						track_name->set_text(animation->track_get_path(idx));
						track_name->set_pos( te->get_global_pos() + area.pos);
						track_name->set_size(area.size);
						track_name->show_modal();
						track_name->grab_focus();
						track_name->select_all();
						track_name_editing=idx;

					} else if (mpos.x < settings_limit) {

						float pos = mpos.x - name_limit;
						pos/=_get_zoom_scale();
						pos+=h_scroll->get_val();
						float w_time = (type_icon[0]->get_width() / _get_zoom_scale())/2.0;

						int kidx = animation->track_find_key(idx,pos);
						int kidx_n = kidx+1;
						int key=-1;

						if (kidx>=0 && kidx<animation->track_get_key_count(idx)) {

							float kpos = animation->track_get_key_time(idx,kidx);
							if (ABS(pos-kpos)<=w_time) {

								key=kidx;
							}
						}

						if (key==-1 && kidx_n>=0 && kidx_n<animation->track_get_key_count(idx)) {

							float kpos = animation->track_get_key_time(idx,kidx_n);
							if (ABS(pos-kpos)<=w_time) {

								key=kidx_n;
							}
						}

						if (key==-1) {

							click.click=ClickOver::CLICK_SELECT_KEYS;
							click.at=Point2(mb.x,mb.y);
							click.to=click.at;
							click.shift=mb.mod.shift;
							selected_track=idx;
							track_editor->update();
							//drag select region
							return;

						}

						if (mb.mod.command || edit_button->is_pressed()) {

							key_edit->animation=animation;
							key_edit->track=idx;
							key_edit->key_ofs=animation->track_get_key_time(idx,key);
							key_edit->hint=_find_hint_for_track(idx);

							key_edit->notify_change();							

							key_edit_dialog->set_size( Size2( 200,200) );
							key_edit_dialog->set_pos(  track_editor->get_global_pos() + ofs + mpos +Point2(-100,20));
							key_edit_dialog->popup();

						}

						SelectedKey sk;
						sk.track=idx;
						sk.key=key;
						KeyInfo ki;
						ki.pos= animation->track_get_key_time(idx,key);
						click.shift=mb.mod.shift;
						click.selk=sk;


						if (!mb.mod.shift && !selection.has(sk))
							selection.clear();

						selection.insert(sk,ki);

						click.click=ClickOver::CLICK_MOVE_KEYS;
						click.at=Point2(mb.x,mb.y);
						click.to=click.at;
						update();
						selected_track=idx;
						track_editor->update();


					} else {
						//button column
						int ofsx = size.width - mpos.x;
						if (ofsx < 0)
							break;
/*
						if (ofsx < remove_icon->get_width()) {

							undo_redo->create_action("Remove Anim Track");
							undo_redo->add_do_method(animation.ptr(),"remove_track",idx);
							undo_redo->add_undo_method(animation.ptr(),"add_track",animation->track_get_type(idx),idx);
							undo_redo->add_undo_method(animation.ptr(),"track_set_path",idx,animation->track_get_path(idx));
							//todo interpolation
							for(int i=0;i<animation->track_get_key_count(idx);i++) {

								Variant v = animation->track_get_key_value(idx,i);
								float time =  animation->track_get_key_time(idx,i);
								float trans =  animation->track_get_key_transition(idx,i);

								undo_redo->add_undo_method(animation.ptr(),"track_insert_key",idx,time,v);
								undo_redo->add_undo_method(animation.ptr(),"track_set_key_transition",idx,i,trans);

							}

							undo_redo->add_undo_method(animation.ptr(),"track_set_interpolation_type",idx,animation->track_get_interpolation_type(idx));
							if (animation->track_get_type(idx)==Animation::TYPE_VALUE) {
								undo_redo->add_undo_method(animation.ptr(),"value_track_set_continuous",idx,animation->value_track_is_continuous(idx));

							}

							undo_redo->commit_action();


							return;
						}

						ofsx-=hsep+remove_icon->get_width();

						if (ofsx < move_down_icon->get_width()) {

							if (idx < animation->get_track_count() -1) {
								undo_redo->create_action("Move Anim Track Down");
								undo_redo->add_do_method(animation.ptr(),"track_move_up",idx);
								undo_redo->add_undo_method(animation.ptr(),"track_move_down",idx+1);
								undo_redo->commit_action();
							}
							return;
						}

						ofsx-=hsep+move_down_icon->get_width();

						if (ofsx < move_up_icon->get_width()) {

							if (idx >0) {
								undo_redo->create_action("Move Anim Track Up");
								undo_redo->add_do_method(animation.ptr(),"track_move_down",idx);
								undo_redo->add_undo_method(animation.ptr(),"track_move_up",idx-1);
								undo_redo->commit_action();
							}
							return;
						}


						ofsx-=hsep*3+move_up_icon->get_width();
						*/

						if (ofsx < down_icon->get_width() + interp_icon[0]->get_width() + hsep*2) {

							track_menu->clear();
							track_menu->set_size(Point2(1,1));
							static const char *interp_name[3]={"Nearest","Linear","Cubic"};
							for(int i=0;i<3;i++) {
								track_menu->add_icon_item(interp_icon[i],interp_name[i]);
							}

							int lofs = remove_icon->get_width() + move_up_icon->get_width() + move_down_icon->get_width() + down_icon->get_width() *2 + hsep*7;//interp_icon[0]->get_width() + cont_icon[0]->get_width() ;
							int popup_y = ofs.y+((int(mpos.y)/h)+2)*h;
							int popup_x = ofs.x+size.width-lofs;

							track_menu->set_pos(te->get_global_pos()+Point2(popup_x,popup_y));


							interp_editing=idx;
							cont_editing=-1;

							track_menu->popup();

							return;
						}

						ofsx-=hsep*2+interp_icon[0]->get_width()+down_icon->get_width();

						if (ofsx < down_icon->get_width() + cont_icon[0]->get_width()) {

							track_menu->clear();
							track_menu->set_size(Point2(1,1));
							static const char *cont_name[3]={"Discrete","Continuous"};
							for(int i=0;i<2;i++) {
								track_menu->add_icon_item(cont_icon[i],cont_name[i]);
							}

							int lofs = settings_limit;
							int popup_y = ofs.y+((int(mpos.y)/h)+2)*h;
							int popup_x = ofs.x+lofs;

							track_menu->set_pos(te->get_global_pos()+Point2(popup_x,popup_y));

							interp_editing=-1;
							cont_editing=idx;

							track_menu->popup();

							return;
						}

						ofsx-=hsep*3+cont_icon[0]->get_width()+down_icon->get_width();

						if (ofsx < add_key_icon->get_width()) {

							Animation::TrackType tt = animation->track_get_type(idx);						

							float pos = timeline_pos;
							int existing = animation->track_find_key(idx,pos,true);



							Variant newval;

							if (tt==Animation::TYPE_TRANSFORM) {
								Dictionary d;
								d["loc"]=Vector3();
								d["rot"]=Quat();
								d["scale"]=Vector3();
								newval=d;

							} else if (tt==Animation::TYPE_METHOD) {

								Dictionary d;
								d["method"]="";
								d["args"]=Vector<Variant>();


								newval=d;
							} else if (tt==Animation::TYPE_VALUE) {

								PropertyInfo inf = _find_hint_for_track(idx);
								if (inf.type!=Variant::NIL) {

									Variant::CallError err;
									newval=Variant::construct(inf.type,NULL,0,err);

								}

								if (newval.get_type()==Variant::NIL) {
									//popup a new type
									cvi_track=idx;
									cvi_pos=pos;

									type_menu->set_pos( get_global_pos() + mpos +ofs );
									type_menu->popup();
									return;
								}

							}

							undo_redo->create_action("Anim Add Key");

							undo_redo->add_do_method(animation.ptr(),"track_insert_key",idx,pos,newval,1);
							undo_redo->add_undo_method(animation.ptr(),"track_remove_key_at_pos",idx,pos);

							if (existing!=-1) {
								Variant v = animation->track_get_key_value(idx,existing);
								float trans = animation->track_get_key_transition(idx,existing);
								undo_redo->add_undo_method(animation.ptr(),"track_insert_key",idx,pos,v,trans);
							}

							undo_redo->commit_action();


							return;
						}

					}

				} else {

					switch(click.click) {
						case ClickOver::CLICK_SELECT_KEYS: {


							float zoom_scale=_get_zoom_scale();
							float keys_from = h_scroll->get_val();
							float keys_to = keys_from + (settings_limit-name_limit) / zoom_scale;

							float from_time = keys_from + ( click.at.x - (name_limit+ofs.x)) / zoom_scale;
							float to_time = keys_from + (click.to.x - (name_limit+ofs.x)) / zoom_scale;


							if (to_time < from_time)
								SWAP(from_time,to_time);

							if (from_time > keys_to || to_time < keys_from)
								break;

							if (from_time < keys_from)
								from_time = keys_from;

							if (to_time >= keys_to)
								to_time = keys_to;


							int from_track = int(click.at.y-ofs.y-h-sep) / h + v_scroll->get_val();
							int to_track  = int(click.to.y-ofs.y-h-sep) / h + v_scroll->get_val();
							int from_mod = int(click.at.y-ofs.y-sep) % h;
							int to_mod = int(click.to.y-ofs.y-sep) % h;

							if (to_track < from_track) {

								SWAP(from_track,to_track);
								SWAP(from_mod,to_mod);
							}




							if ((from_mod > (h/2)) && ((click.at.y-ofs.y)>=(h+sep))) {
								from_track++;
							}

							if (to_mod < h/2) {
								to_track--;
							}

							if (from_track>to_track) {
								if (!click.shift)
									selection.clear();
								break;
							}

							int tracks_from = v_scroll->get_val();
							int tracks_to = v_scroll->get_val()+fit-1;
							if (tracks_to>=animation->get_track_count())
								tracks_to=animation->get_track_count()-1;

							tracks_from=0;
							tracks_to=animation->get_track_count()-1;
							if (to_track >tracks_to)
								to_track = tracks_to;
							if (from_track<tracks_from)
								from_track=tracks_from;

							if (from_track > tracks_to  || to_track < tracks_from) {
								if (!click.shift)
									selection.clear();
								break;
							}

							if (!click.shift)
								selection.clear();


							int higher_track=0x7FFFFFFF;
							for(int i=from_track;i<=to_track;i++) {

								int kc=animation->track_get_key_count(i);
								for(int j=0;j<kc;j++) {

									float t = animation->track_get_key_time(i,j);
									if (t<from_time)
										continue;
									if (t>to_time)
										break;

									if (i<higher_track)
										higher_track=i;

									SelectedKey sk;
									sk.track=i;
									sk.key=j;
									KeyInfo ki;
									ki.pos=t;
									selection[sk]=ki;
								}
							}

							if (higher_track!=0x7FFFFFFF) {
								selected_track=higher_track;
								track_editor->update();
							}



						} break;
						case ClickOver::CLICK_MOVE_KEYS: {

							if (selection.empty())
								break;
							if (click.at==click.to) {

								if (!click.shift) {

									KeyInfo ki=selection[click.selk];
									selection.clear();
									selection[click.selk]=ki;
								}

								break;
							}

							float from_t = 1e20;

							for(Map<SelectedKey,KeyInfo>::Element *E=selection.front();E;E=E->next()) {
								float t = animation->track_get_key_time(E->key().track,E->key().key);
								if (t<from_t)
									from_t=t;

							}

							float motion = from_t+(click.to.x - click.at.x)/_get_zoom_scale();
							if (step->get_val())
								motion = Math::stepify(motion,step->get_val());




							undo_redo->create_action("Anim Move Keys");

							List<_AnimMoveRestore> to_restore;

							// 1-remove the keys
							for(Map<SelectedKey,KeyInfo>::Element *E=selection.back();E;E=E->prev()) {

								undo_redo->add_do_method(animation.ptr(),"track_remove_key",E->key().track,E->key().key);
							}
							// 2- remove overlapped keys
							for(Map<SelectedKey,KeyInfo>::Element *E=selection.back();E;E=E->prev()) {

								float newtime = E->get().pos-from_t+motion;
								int idx = animation->track_find_key(E->key().track,newtime,true);
								if (idx==-1)
									continue;
								SelectedKey sk;
								sk.key=idx;
								sk.track=E->key().track;
								if (selection.has(sk))
									continue; //already in selection, don't save

								undo_redo->add_do_method(animation.ptr(),"track_remove_key_at_pos",E->key().track,newtime);
								_AnimMoveRestore amr;

								amr.key=animation->track_get_key_value(E->key().track,idx);
								amr.track=E->key().track;
								amr.time=newtime;
								amr.transition=animation->track_get_key_transition(E->key().track,idx);

								to_restore.push_back(amr);

							}

							// 3-move the keys (re insert them)
							for(Map<SelectedKey,KeyInfo>::Element *E=selection.back();E;E=E->prev()) {

								float newpos=E->get().pos-from_t+motion;
								//if (newpos<0)
								//	continue; //no add at the begining
								undo_redo->add_do_method(animation.ptr(),"track_insert_key",E->key().track,newpos,animation->track_get_key_value(E->key().track,E->key().key),animation->track_get_key_transition(E->key().track,E->key().key));

							}

							// 4-(undo) remove inserted keys
							for(Map<SelectedKey,KeyInfo>::Element *E=selection.back();E;E=E->prev()) {

								float newpos=E->get().pos+-from_t+motion;
								//if (newpos<0)
								//	continue; //no remove what no inserted
								undo_redo->add_undo_method(animation.ptr(),"track_remove_key_at_pos",E->key().track,newpos);

							}

							// 5-(undo) reinsert keys
							for(Map<SelectedKey,KeyInfo>::Element *E=selection.back();E;E=E->prev()) {

								undo_redo->add_undo_method(animation.ptr(),"track_insert_key",E->key().track,E->get().pos,animation->track_get_key_value(E->key().track,E->key().key),animation->track_get_key_transition(E->key().track,E->key().key));

							}

							// 6-(undo) reinsert overlapped keys
							for(List<_AnimMoveRestore>::Element *E=to_restore.front();E;E=E->next()) {

								_AnimMoveRestore &amr = E->get();
								undo_redo->add_undo_method(animation.ptr(),"track_insert_key",amr.track,amr.time,amr.key,amr.transition);

							}

							// 6-(undo) reinsert overlapped keys
							for(List<_AnimMoveRestore>::Element *E=to_restore.front();E;E=E->next()) {

								_AnimMoveRestore &amr = E->get();
								undo_redo->add_undo_method(animation.ptr(),"track_insert_key",amr.track,amr.time,amr.key,amr.transition);

							}

							undo_redo->add_do_method(this,"_clear_selection_for_anim",animation);
							undo_redo->add_undo_method(this,"_clear_selection_for_anim",animation);

							// 7-reselect

							for(Map<SelectedKey,KeyInfo>::Element *E=selection.back();E;E=E->prev()) {

								float oldpos=E->get().pos;
								float newpos=oldpos-from_t+motion;
								//if (newpos>=0)
									undo_redo->add_do_method(this,"_select_at_anim",animation,E->key().track,newpos);
								undo_redo->add_undo_method(this,"_select_at_anim",animation,E->key().track,oldpos);

							}

							undo_redo->commit_action();

						} break;
						default: {}
					}

					//button released
					click.click=ClickOver::CLICK_NONE;
					track_editor->update();


				}
			}

		} break;

		case InputEvent::MOUSE_MOTION: {

			const InputEventMouseMotion &mb = p_input.mouse_motion;

			mouse_over.over=MouseOver::OVER_NONE;
			mouse_over.track=-1;
			te->update();
			track_editor->set_tooltip("");

			if (!track_editor->has_focus() && (!get_focus_owner() || !get_focus_owner()->cast_to<LineEdit>()))
				track_editor->call_deferred("grab_focus");


			if (click.click!=ClickOver::CLICK_NONE) {

				switch(click.click) {
					case ClickOver::CLICK_RESIZE_NAMES: {


						float base = click.at.y;
						float clickp = click.at.x-ofs.x;
						float dif = base - clickp;

						float target = mb.x+dif-ofs.x;

						float ratio = target / settings_limit;

						if (ratio>0.9)
							ratio=0.9;
						else if (ratio<0.2)
							ratio=0.2;

						name_column_ratio=ratio;


					} break;
					case ClickOver::CLICK_DRAG_TIMELINE: {

						Point2 mpos = Point2(mb.x,mb.y)-ofs;
						/*
						if (mpos.x<name_limit)
							mpos.x=name_limit;
						if (mpos.x>settings_limit)
							mpos.x=settings_limit;
							*/


						//int zoomw = settings_limit-name_limit;
						float scale = _get_zoom_scale();
						float pos = h_scroll->get_val() + (mpos.x-name_limit) / scale;
						if (animation->get_step()) {
							pos=Math::stepify(pos,animation->get_step());

						}
						if (pos<0)
							pos=0;
						if (pos>=animation->get_length())
							pos=animation->get_length();

						if (pos < h_scroll->get_val()) {
							h_scroll->set_val(pos);
						} else if (pos > h_scroll->get_val() + (settings_limit - name_limit) / scale) {
							h_scroll->set_val( pos - (settings_limit - name_limit) / scale );
						}

						timeline_pos=pos;
						emit_signal("timeline_changed",pos);



					} break;
					case ClickOver::CLICK_SELECT_KEYS: {

						click.to=Point2(mb.x,mb.y);
						if (click.to.y<h && click.at.y>h && mb.relative_y<0) {

							float prev = v_scroll->get_val();
							v_scroll->set_val( v_scroll->get_val() -1 );
							if (prev!=v_scroll->get_val())
								click.at.y+=h;


						}
						if (click.to.y>size.height && click.at.y<size.height && mb.relative_y>0) {

							float prev = v_scroll->get_val();
							v_scroll->set_val( v_scroll->get_val() +1 );
							if (prev!=v_scroll->get_val())
								click.at.y-=h;
						}

					} break;
					case ClickOver::CLICK_MOVE_KEYS: {

						click.to=Point2(mb.x,mb.y);
					} break;
					default: {}
				}

				return;
			} else if (mb.button_mask&BUTTON_MASK_MIDDLE) {

				int rel = mb.relative_x;
				float relf = rel / _get_zoom_scale();
				h_scroll->set_val( h_scroll->get_val() - relf );
			}

			if (mb.button_mask==0) {


				Point2 mpos = Point2(mb.x,mb.y)-ofs;

				if (mpos.y < h ) {
#if 0
					//seek
					//int zoomw = settings_limit-name_limit;
					float scale = _get_zoom_scale();
					float pos = h_scroll->get_val() + (mpos.y-name_limit) / scale;
					if (pos<0 )
						pos=0;
					if (pos>=animation->get_length())
						pos=animation->get_length();
					timeline->set_val(pos);
#endif
					return;
				}

				mpos.y -= h;

				int idx = mpos.y / h;
				idx+=v_scroll->get_val();
				if (idx <0 || idx>=animation->get_track_count())
					break;

				mouse_over.track=idx;

				if (mpos.x < name_limit) {
					//name column


					mouse_over.over=MouseOver::OVER_NAME;

				} else if (mpos.x < settings_limit) {

					float pos = mpos.x - name_limit;
					pos/=_get_zoom_scale();
					pos+=h_scroll->get_val();
					float w_time = (type_icon[0]->get_width() / _get_zoom_scale())/2.0;

					int kidx = animation->track_find_key(idx,pos);
					int kidx_n = kidx+1;

					bool found = false;

					if (kidx>=0 && kidx<animation->track_get_key_count(idx)) {

						float kpos = animation->track_get_key_time(idx,kidx);
						if (ABS(pos-kpos)<=w_time) {

							mouse_over.over=MouseOver::OVER_KEY;
							mouse_over.track=idx;
							mouse_over.over_key=kidx;
							found=true;
						}
					}

					if (!found && kidx_n>=0 && kidx_n<animation->track_get_key_count(idx)) {

						float kpos = animation->track_get_key_time(idx,kidx_n);
						if (ABS(pos-kpos)<=w_time) {

							mouse_over.over=MouseOver::OVER_KEY;
							mouse_over.track=idx;
							mouse_over.over_key=kidx_n;
							found=true;
						}
					}


					if (found) {

						String text;
						text="time: "+rtos(animation->track_get_key_time(idx,mouse_over.over_key))+"\n";
						switch(animation->track_get_type(idx)) {

							case Animation::TYPE_TRANSFORM: {

								Dictionary d = animation->track_get_key_value(idx,mouse_over.over_key);
								if (d.has("loc"))
									text+="loc: "+String(d["loc"])+"\n";
								if (d.has("rot"))
									text+="rot: "+String(d["rot"])+"\n";
								if (d.has("scale"))
									text+="scale: "+String(d["scale"])+"\n";
							} break;
							case Animation::TYPE_VALUE: {

								Variant v = animation->track_get_key_value(idx,mouse_over.over_key);
								text+="value: "+String(v)+"\n";
							} break;
							case Animation::TYPE_METHOD: {


								Dictionary d = animation->track_get_key_value(idx,mouse_over.over_key);
								if (d.has("method"))
									text+=String(d["method"]);
								text+="(";
								Vector<Variant> args;
								if (d.has("args"))
									args=d["args"];
								for(int i=0;i<args.size();i++) {

									if (i>0)
										text+=", ";
									text+=String(args[i]);
								}
								text+=")\n";

							} break;
						}
						text+="easing: "+rtos(animation->track_get_key_transition(idx,mouse_over.over_key));
						track_editor->set_tooltip(text);
						return;

					}

				} else {
					//button column
					int ofsx = size.width - mpos.x;
					if (ofsx < 0)
						break;
/*
					if (ofsx < remove_icon->get_width()) {

						mouse_over.over=MouseOver::OVER_REMOVE;

						return;
					}

					ofsx-=hsep+remove_icon->get_width();

					if (ofsx < move_down_icon->get_width()) {

						mouse_over.over=MouseOver::OVER_DOWN;
						return;
					}

					ofsx-=hsep+move_down_icon->get_width();

					if (ofsx < move_up_icon->get_width()) {

						mouse_over.over=MouseOver::OVER_UP;
						return;
					}

					ofsx-=hsep*3+move_up_icon->get_width();

	*/

					if (ofsx < down_icon->get_width() + interp_icon[0]->get_width() + hsep*2) {

						mouse_over.over=MouseOver::OVER_INTERP;
						return;
					}


					ofsx-=hsep*2+interp_icon[0]->get_width() + down_icon->get_width();

					if (ofsx < down_icon->get_width() + cont_icon[0]->get_width() +hsep*3) {

						mouse_over.over=MouseOver::OVER_VALUE;
						return;
					}

					ofsx-=hsep*3+cont_icon[0]->get_width() + down_icon->get_width();

					if (ofsx < add_key_icon->get_width()) {

						mouse_over.over=MouseOver::OVER_ADD_KEY;
						return;
					}


				}

			}

		} break;

	}
}

void AnimationKeyEditor::_notification(int p_what) {


	switch(p_what) {

		case NOTIFICATION_ENTER_TREE: {

				zoomicon->set_texture( get_icon("Zoom","EditorIcons") );				
				//menu_track->set_icon(get_icon("AddTrack","EditorIcons"));
				menu_track->get_popup()->add_icon_item(get_icon("KeyValue","EditorIcons"),"Add Normal Track",TRACK_MENU_ADD_VALUE_TRACK);
				menu_track->get_popup()->add_icon_item(get_icon("KeyXform","EditorIcons"),"Add Transform3D Track",TRACK_MENU_ADD_TRANSFORM_TRACK);
				menu_track->get_popup()->add_icon_item(get_icon("KeyCall","EditorIcons"),"Add Call Func Track",TRACK_MENU_ADD_CALL_TRACK);
				menu_track->get_popup()->add_separator();
				menu_track->get_popup()->add_item("Scale Selection",TRACK_MENU_SCALE);
				menu_track->get_popup()->add_item("Scale From Cursor",TRACK_MENU_SCALE_PIVOT);
				menu_track->get_popup()->add_separator();
				menu_track->get_popup()->add_item("Duplicate Selection",TRACK_MENU_DUPLICATE);
				menu_track->get_popup()->add_item("Duplicate Transposed",TRACK_MENU_DUPLICATE_TRANSPOSE);
				menu_track->get_popup()->add_separator();
				menu_track->get_popup()->add_item("Goto Next Step",TRACK_MENU_NEXT_STEP,KEY_MASK_CMD|KEY_RIGHT);
				menu_track->get_popup()->add_item("Goto Prev Step",TRACK_MENU_PREV_STEP,KEY_MASK_CMD|KEY_LEFT);
				menu_track->get_popup()->add_separator();
				PopupMenu *tpp = memnew( PopupMenu );
				tpp->add_item("Linear",TRACK_MENU_SET_ALL_TRANS_LINEAR);
				tpp->add_item("Constant",TRACK_MENU_SET_ALL_TRANS_CONSTANT);
				tpp->add_item("In",TRACK_MENU_SET_ALL_TRANS_IN);
				tpp->add_item("Out",TRACK_MENU_SET_ALL_TRANS_OUT);
				tpp->add_item("In-Out",TRACK_MENU_SET_ALL_TRANS_INOUT);
				tpp->add_item("Out-In",TRACK_MENU_SET_ALL_TRANS_OUTIN);
				tpp->set_name("Transitions");
				tpp->connect("item_pressed",this,"_menu_track");
				optimize_dialog->connect("confirmed",this,"_animation_optimize");

				menu_track->get_popup()->add_child(tpp);
				menu_track->get_popup()->add_submenu_item("Set Transitions..","Transitions");
				menu_track->get_popup()->add_separator();
				menu_track->get_popup()->add_item("Optimize Animation",TRACK_MENU_OPTIMIZE);





				move_up_button->set_icon(get_icon("MoveUp","EditorIcons"));
				move_down_button->set_icon(get_icon("MoveDown","EditorIcons"));
				remove_button->set_icon(get_icon("Remove","EditorIcons"));
				edit_button->set_icon(get_icon("EditKey","EditorIcons"));

				loop->set_icon(get_icon("Loop","EditorIcons"));

				//edit_button->add_color_override("font_color",get_color("font_color","Tree"));
				//edit_button->add_color_override("font_color_hover",get_color("font_color","Tree"));

				{

					right_data_size_cache=0;
					int hsep = get_constant("hseparation","Tree");
					Ref<Texture> remove_icon = get_icon("Remove","EditorIcons");
					Ref<Texture> move_up_icon = get_icon("MoveUp","EditorIcons");
					Ref<Texture> move_down_icon = get_icon("MoveDown","EditorIcons");
					Ref<Texture> down_icon = get_icon("select_arrow","Tree");
					Ref<Texture> add_key_icon = get_icon("TrackAddKey","EditorIcons");
					Ref<Texture> interp_icon[3]={
						get_icon("InterpRaw","EditorIcons"),
						get_icon("InterpLinear","EditorIcons"),
						get_icon("InterpCubic","EditorIcons")
					};
					Ref<Texture> cont_icon[3]={
						get_icon("TrackDiscrete","EditorIcons"),
						get_icon("TrackContinuous","EditorIcons")
					};

					//right_data_size_cache = remove_icon->get_width() + move_up_icon->get_width() + move_down_icon->get_width() + down_icon->get_width() *2 + interp_icon[0]->get_width() + cont_icon[0]->get_width() + add_key_icon->get_width() + hsep*11;
					right_data_size_cache = down_icon->get_width() *2 + add_key_icon->get_width() + interp_icon[0]->get_width() + cont_icon[0]->get_width() + hsep*7;


				}
//				rename_anim->set_icon( get_icon("Rename","EditorIcons") );
/*
				edit_anim->set_icon( get_icon("Edit","EditorIcons") );
				blend_anim->set_icon( get_icon("Blend","EditorIcons") );
				play->set_icon( get_icon("Play","EditorIcons") );
				stop->set_icon( get_icon("Stop","EditorIcons") );
				pause->set_icon( get_icon("Pause","EditorIcons") );
*/
//			menu->set_icon(get_icon("Animation","EditorIcons"));
//			play->set_icon(get_icon("AnimationPlay","EditorIcons"));
			//menu->set_icon(get_icon("Animation","EditorIcons"));
			_update_menu();

		} break;


	}

}



void AnimationKeyEditor::_scroll_changed(double) {

	if (te_drawing)
		return;

	track_editor->update();
}



void AnimationKeyEditor::_update_paths() {

	if (animation.is_valid()) {
		//timeline->set_max(animation->get_length());
		//timeline->set_step(0.01);
		track_editor->update();
		length->set_val(animation->get_length());
		step->set_val(animation->get_step());
	}
}


void AnimationKeyEditor::_root_removed() {

	root=NULL;
}

void AnimationKeyEditor::_update_menu() {


	updating=true;

	bool empty= !animation.is_valid();
	if (animation.is_valid()) {

		length->set_val(animation->get_length());
		loop->set_pressed(animation->has_loop());
		step->set_val(animation->get_step());
	}

	track_editor->update();
	updating=false;

}

void AnimationKeyEditor::set_animation(const Ref<Animation>& p_anim) {

	if (animation.is_valid())
		animation->disconnect("changed",this,"_update_paths");
	animation=p_anim;
	if (animation.is_valid())
		animation->connect("changed",this,"_update_paths");

	timeline_pos=0;
	selection.clear();
	_update_paths();

	_update_menu();
	selected_track=-1;
}

void AnimationKeyEditor::set_root(Node *p_root) {

	if (root)
		root->disconnect("exit_tree",this,"_root_removed");

	root=p_root;

	if (root)
		root->connect("exit_tree",this,"_root_removed",make_binds(),CONNECT_ONESHOT);


}

Node *AnimationKeyEditor::get_root() const {

	return root;
}






void AnimationKeyEditor::set_keying(bool p_enabled) {

	keying=p_enabled;
	_update_menu();
	emit_signal("keying_changed",p_enabled);

}

bool AnimationKeyEditor::has_keying() const {

	return keying;
}

void AnimationKeyEditor::_query_insert(const InsertData& p_id) {


	if (insert_frame!=OS::get_singleton()->get_frames_drawn()) {
		//clear insert list for the frame if frame changed
		if (insert_confirm->is_visible())
			return; //do nothing
		insert_data.clear();
		insert_query=false;
	}
	insert_frame=OS::get_singleton()->get_frames_drawn();

	for (List<InsertData>::Element *E=insert_data.front();E;E=E->next()) {
		//prevent insertion of multiple tracks
		if (E->get().path==p_id.path)
			return; //already inserted a track for this on this frame
	}

	insert_data.push_back(p_id);

	if (p_id.track_idx==-1) {
		if (bool(EDITOR_DEF("animation/confirm_insert_track",true))) {
			//potential new key, does not exist		
			if (insert_data.size()==1)
				insert_confirm->set_text("Create NEW track for "+p_id.query+" and insert key?");
			else
				insert_confirm->set_text("Create "+itos(insert_data.size())+" NEW tracks and insert keys?");

			insert_confirm->get_ok()->set_text("Create");
			insert_confirm->popup_centered(Size2(300,100));
			insert_query=true;
		} else {
			call_deferred("_insert_delay");
			insert_queue=true;
		}

	} else {
		if (!insert_query && !insert_queue) {
			call_deferred("_insert_delay");
			insert_queue=true;
		}
	}

}

void AnimationKeyEditor::insert_transform_key(Node3D *p_node,const String& p_sub,const Transform3D& p_xform) {

	if (!keying)
		return;
	if (!animation.is_valid())
		return;


	ERR_FAIL_COND(!root);
	//let's build a node path
	String path = root->get_path_to(p_node);
	if (p_sub!="")
		path+=":"+p_sub;

	NodePath np=path;

	int track_idx=-1;

	for(int i=0;i<animation->get_track_count();i++) {

		if (animation->track_get_type(i)!=Animation::TYPE_TRANSFORM)
			continue;
		if (animation->track_get_path(i)!=np)
			continue;

		track_idx=i;
		break;
	}

	InsertData id;
	Dictionary val;

	id.path=np;
	id.track_idx=track_idx;
	id.value=p_xform;
	id.type=Animation::TYPE_TRANSFORM;
	id.query="node '"+p_node->get_name()+"'";

	//dialog insert

	_query_insert(id);

}


void AnimationKeyEditor::insert_node_value_key(Node* p_node, const String& p_property,const Variant& p_value,bool p_only_if_exists) {

	ERR_FAIL_COND(!root);
	//let's build a node path

	Node *node = p_node;

	String path = root->get_path_to(node);

	for(int i=1;i<history->get_path_size();i++) {

		String prop = history->get_path_property(i);
		ERR_FAIL_COND(prop=="");
		path+=":"+prop;
	}


	path+=":"+p_property;

	NodePath np = path;

	//locate track

	int track_idx=-1;

	for(int i=0;i<animation->get_track_count();i++) {

		if (animation->track_get_type(i)!=Animation::TYPE_VALUE)
			continue;
		if (animation->track_get_path(i)!=np)
			continue;

		track_idx=i;
		break;
	}

	if (p_only_if_exists && track_idx==-1)
		return;
	InsertData id;
	id.path=np;
	id.track_idx=track_idx;
	id.value=p_value;
	id.type=Animation::TYPE_VALUE;
	id.query="property '"+p_property+"'";
	//dialog insert
	_query_insert(id);



}

void AnimationKeyEditor::insert_value_key(const String& p_property,const Variant& p_value) {

	ERR_FAIL_COND(!root);
	//let's build a node path
	ERR_FAIL_COND(history->get_path_size()==0);
	Object *obj = ObjectDB::get_instance(history->get_path_object(0));
	ERR_FAIL_COND(!obj || !obj->cast_to<Node>());

	Node *node = obj->cast_to<Node>();

	String path = root->get_path_to(node);

	for(int i=1;i<history->get_path_size();i++) {

		String prop = history->get_path_property(i);
		ERR_FAIL_COND(prop=="");
		path+=":"+prop;
	}



	path+=":"+p_property;

	NodePath np = path;

	//locate track

	int track_idx=-1;

	for(int i=0;i<animation->get_track_count();i++) {

		if (animation->track_get_type(i)!=Animation::TYPE_VALUE)
			continue;
		if (animation->track_get_path(i)!=np)
			continue;

		track_idx=i;
		break;
	}

	InsertData id;
	id.path=np;
	id.track_idx=track_idx;
	id.value=p_value;
	id.type=Animation::TYPE_VALUE;
	id.query="property '"+p_property+"'";
	//dialog insert
	_query_insert(id);



}

void AnimationKeyEditor::_confirm_insert_list() {


	undo_redo->create_action("Anim Create & Insert");

	int last_track = animation->get_track_count();
	while(insert_data.size()) {

		last_track=_confirm_insert(insert_data.front()->get(),last_track);
		insert_data.pop_front();
	}

	undo_redo->commit_action();
}

int AnimationKeyEditor::_confirm_insert(InsertData p_id,int p_last_track) {

	if (p_last_track==-1)
		p_last_track=animation->get_track_count();

	bool created=false;
	if (p_id.track_idx<0) {

		created=true;
		undo_redo->create_action("Anim Insert Track & Key");
		bool continuous=false;

		if (p_id.type==Animation::TYPE_VALUE) {
			//wants a new tack

			{
				//shitty hack
				animation->add_track(p_id.type);
				animation->track_set_path(animation->get_track_count()-1,p_id.path);
				PropertyInfo h = _find_hint_for_track(animation->get_track_count()-1);
				animation->remove_track(animation->get_track_count()-1); //hack


				continuous =
					h.type==Variant::REAL ||
					h.type==Variant::VECTOR2 ||
					h.type==Variant::RECT2 ||
					h.type==Variant::VECTOR3 ||
					h.type==Variant::_AABB ||
					h.type==Variant::QUAT ||
					h.type==Variant::COLOR ||
					h.type==Variant::TRANSFORM ;
			}
		}

		p_id.track_idx=p_last_track;

		undo_redo->add_do_method(animation.ptr(),"add_track",p_id.type);
		undo_redo->add_do_method(animation.ptr(),"track_set_path",p_id.track_idx,p_id.path);
		if (p_id.type==Animation::TYPE_VALUE)
			undo_redo->add_do_method(animation.ptr(),"value_track_set_continuous",p_id.track_idx,continuous);

	} else {
		undo_redo->create_action("Anim Insert Key");
	}

	float time = timeline_pos;
	Variant value;


	switch(p_id.type) {

		case Animation::TYPE_VALUE: {

			value = p_id.value;


		} break;
		case Animation::TYPE_TRANSFORM: {


			Transform3D tr = p_id.value;
			Dictionary d;
			d["loc"]=tr.origin;
			d["scale"]=tr.basis.get_scale();
			d["rot"]=Quat(tr.basis);//.orthonormalized();
			value=d;
		} break;
		default:{}
	}



	undo_redo->add_do_method(animation.ptr(),"track_insert_key",p_id.track_idx,time,value);

	if (created) {

		//just remove the track
		undo_redo->add_undo_method(animation.ptr(),"remove_track",p_last_track);
		p_last_track++;
	} else {

		undo_redo->add_undo_method(animation.ptr(),"track_remove_key_at_pos",p_id.track_idx,time);
		int existing = animation->track_find_key(p_id.track_idx,time,true);
		if (existing!=-1) {
			Variant v = animation->track_get_key_value(p_id.track_idx,existing);
			float trans = animation->track_get_key_transition(p_id.track_idx,existing);
			undo_redo->add_undo_method(animation.ptr(),"track_insert_key",p_id.track_idx,time,v,trans);
		}
	}

	undo_redo->add_do_method(this,"update");
	undo_redo->add_undo_method(this,"update");
	undo_redo->add_do_method(track_editor,"update");
	undo_redo->add_undo_method(track_editor,"update");
	undo_redo->add_do_method(track_pos,"update");
	undo_redo->add_undo_method(track_pos,"update");

	undo_redo->commit_action();

	return p_last_track;

}




Ref<Animation> AnimationKeyEditor::get_current_animation() const {

	return animation;
}

void AnimationKeyEditor::_animation_len_changed(float p_len) {

	if (updating)
		return;

	if (!animation.is_null()) {

		undo_redo->create_action("Change Anim Len");
		undo_redo->add_do_method(animation.ptr(),"set_length",p_len);
		undo_redo->add_undo_method(animation.ptr(),"set_length",animation->get_length());
		undo_redo->add_do_method(this,"_animation_len_update");
		undo_redo->add_undo_method(this,"_animation_len_update");
		undo_redo->commit_action();
	}
}

void AnimationKeyEditor::_animation_len_update() {

	if  (!animation.is_null())
		emit_signal(alc,animation->get_length());
}

void AnimationKeyEditor::_animation_changed() {
	if (updating)
		return;
	_update_menu();

}

void AnimationKeyEditor::_animation_loop_changed() {

	if (updating)
		return;

	if (!animation.is_null()) {

		undo_redo->create_action("Change Anim Loop");
		undo_redo->add_do_method(animation.ptr(),"set_loop",loop->is_pressed());
		undo_redo->add_undo_method(animation.ptr(),"set_loop",!loop->is_pressed());
		undo_redo->commit_action();
	}

}


void AnimationKeyEditor::_create_value_item(int p_type) {

	undo_redo->create_action("Anim Create Typed Value Key");

	Variant::CallError ce;
	Variant v = Variant::construct(Variant::Type(p_type),NULL,0,ce);
	undo_redo->add_do_method(animation.ptr(),"track_insert_key",cvi_track,cvi_pos,v);
	undo_redo->add_undo_method(animation.ptr(),"track_remove_key_at_pos",cvi_track,cvi_pos);

	int existing = animation->track_find_key(cvi_track,cvi_pos,true);

	if (existing!=-1) {
		Variant v = animation->track_get_key_value(cvi_track,existing);
		float trans = animation->track_get_key_transition(cvi_track,existing);
		undo_redo->add_undo_method(animation.ptr(),"track_insert_key",cvi_track,cvi_pos,v,trans);
	}

	undo_redo->commit_action();

}


void AnimationKeyEditor::set_anim_pos(float p_pos) {

	if (animation.is_null())
		return;
	timeline_pos=p_pos;
	update();
	track_pos->update();
	track_editor->update();
}

void AnimationKeyEditor::_pane_drag(const Point2& p_delta) {

	Size2 ecs = ec->get_minsize();
	ecs.y-=p_delta.y;
	if (ecs.y<100)
		ecs.y=100;
	ec->set_minsize(ecs);;

}

void AnimationKeyEditor::_insert_delay() {

	if (insert_query) {
		//discard since it's entered into query mode
		insert_queue=false;
		return;
	}

	undo_redo->create_action("Anim  Insert");

	int last_track = animation->get_track_count();
	while(insert_data.size()) {

		last_track=_confirm_insert(insert_data.front()->get(),last_track);
		insert_data.pop_front();
	}

	undo_redo->commit_action();
	insert_queue=false;
}

void AnimationKeyEditor::_step_changed(float p_len) {

	updating=true;
	animation->set_step(p_len);
	updating=false;
}

void AnimationKeyEditor::_scale() {


	if (selection.empty())
		return;


	float from_t = 1e20;
	float to_t = -1e20;
	float len = -1e20;
	float pivot=0;

	for(Map<SelectedKey,KeyInfo>::Element *E=selection.front();E;E=E->next()) {
		float t = animation->track_get_key_time(E->key().track,E->key().key);
		if (t<from_t)
			from_t=t;
		if (t>to_t)
			to_t=t;

	}

	len=to_t-from_t;
	if (last_menu_track_opt==TRACK_MENU_SCALE_PIVOT) {
		pivot = timeline_pos;

	} else {

		pivot=from_t;

	}

	float s = scale->get_val();
	if (s==0) {
		ERR_PRINT("Can't scale to 0");
	}



	undo_redo->create_action("Anim Scale Keys");

	List<_AnimMoveRestore> to_restore;

	// 1-remove the keys
	for(Map<SelectedKey,KeyInfo>::Element *E=selection.back();E;E=E->prev()) {

		undo_redo->add_do_method(animation.ptr(),"track_remove_key",E->key().track,E->key().key);
	}
	// 2- remove overlapped keys
	for(Map<SelectedKey,KeyInfo>::Element *E=selection.back();E;E=E->prev()) {

		float newtime = (E->get().pos-from_t)*s+from_t;
		int idx = animation->track_find_key(E->key().track,newtime,true);
		if (idx==-1)
			continue;
		SelectedKey sk;
		sk.key=idx;
		sk.track=E->key().track;
		if (selection.has(sk))
			continue; //already in selection, don't save

		undo_redo->add_do_method(animation.ptr(),"track_remove_key_at_pos",E->key().track,newtime);
		_AnimMoveRestore amr;

		amr.key=animation->track_get_key_value(E->key().track,idx);
		amr.track=E->key().track;
		amr.time=newtime;
		amr.transition=animation->track_get_key_transition(E->key().track,idx);

		to_restore.push_back(amr);

	}

#define _NEW_POS(m_ofs) (((s>0)?m_ofs:from_t+(len-(m_ofs-from_t)))-pivot)*ABS(s)+from_t
	// 3-move the keys (re insert them)
	for(Map<SelectedKey,KeyInfo>::Element *E=selection.back();E;E=E->prev()) {

		float newpos=_NEW_POS(E->get().pos);
		undo_redo->add_do_method(animation.ptr(),"track_insert_key",E->key().track,newpos,animation->track_get_key_value(E->key().track,E->key().key),animation->track_get_key_transition(E->key().track,E->key().key));

	}

	// 4-(undo) remove inserted keys
	for(Map<SelectedKey,KeyInfo>::Element *E=selection.back();E;E=E->prev()) {

		float newpos=_NEW_POS(E->get().pos);
		undo_redo->add_undo_method(animation.ptr(),"track_remove_key_at_pos",E->key().track,newpos);

	}

	// 5-(undo) reinsert keys
	for(Map<SelectedKey,KeyInfo>::Element *E=selection.back();E;E=E->prev()) {

		undo_redo->add_undo_method(animation.ptr(),"track_insert_key",E->key().track,E->get().pos,animation->track_get_key_value(E->key().track,E->key().key),animation->track_get_key_transition(E->key().track,E->key().key));

	}

	// 6-(undo) reinsert overlapped keys
	for(List<_AnimMoveRestore>::Element *E=to_restore.front();E;E=E->next()) {

		_AnimMoveRestore &amr = E->get();
		undo_redo->add_undo_method(animation.ptr(),"track_insert_key",amr.track,amr.time,amr.key,amr.transition);

	}

	// 6-(undo) reinsert overlapped keys
	for(List<_AnimMoveRestore>::Element *E=to_restore.front();E;E=E->next()) {

		_AnimMoveRestore &amr = E->get();
		undo_redo->add_undo_method(animation.ptr(),"track_insert_key",amr.track,amr.time,amr.key,amr.transition);

	}

	undo_redo->add_do_method(this,"_clear_selection_for_anim",animation);
	undo_redo->add_undo_method(this,"_clear_selection_for_anim",animation);

	// 7-reselect

	for(Map<SelectedKey,KeyInfo>::Element *E=selection.back();E;E=E->prev()) {

		float oldpos=E->get().pos;
		float newpos=_NEW_POS(oldpos);
		if (newpos>=0)
			undo_redo->add_do_method(this,"_select_at_anim",animation,E->key().track,newpos);
		undo_redo->add_undo_method(this,"_select_at_anim",animation,E->key().track,oldpos);

	}
#undef _NEW_POS
	undo_redo->commit_action();

}


void AnimationKeyEditor::cleanup() {

	set_animation(Ref<Animation>());
}

void AnimationKeyEditor::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("_root_removed"),&AnimationKeyEditor::_root_removed);
	ObjectTypeDB::bind_method(_MD("_scale"),&AnimationKeyEditor::_scale);
	ObjectTypeDB::bind_method(_MD("set_root"),&AnimationKeyEditor::set_root);


//	ObjectTypeDB::bind_method(_MD("_confirm_insert"),&AnimationKeyEditor::_confirm_insert);
	ObjectTypeDB::bind_method(_MD("_confirm_insert_list"),&AnimationKeyEditor::_confirm_insert_list);



	ObjectTypeDB::bind_method(_MD("_update_paths"),&AnimationKeyEditor::_update_paths);
	ObjectTypeDB::bind_method(_MD("_track_editor_draw"),&AnimationKeyEditor::_track_editor_draw);




	ObjectTypeDB::bind_method(_MD("_animation_changed"),&AnimationKeyEditor::_animation_changed);
	ObjectTypeDB::bind_method(_MD("_scroll_changed"),&AnimationKeyEditor::_scroll_changed);
	ObjectTypeDB::bind_method(_MD("_track_editor_input_event"),&AnimationKeyEditor::_track_editor_input_event);
	ObjectTypeDB::bind_method(_MD("_track_name_changed"),&AnimationKeyEditor::_track_name_changed);
	ObjectTypeDB::bind_method(_MD("_track_menu_selected"),&AnimationKeyEditor::_track_menu_selected);
	ObjectTypeDB::bind_method(_MD("_menu_track"),&AnimationKeyEditor::_menu_track);
	ObjectTypeDB::bind_method(_MD("_clear_selection_for_anim"),&AnimationKeyEditor::_clear_selection_for_anim);
	ObjectTypeDB::bind_method(_MD("_select_at_anim"),&AnimationKeyEditor::_select_at_anim);
	ObjectTypeDB::bind_method(_MD("_track_pos_draw"),&AnimationKeyEditor::_track_pos_draw);
	ObjectTypeDB::bind_method(_MD("_insert_delay"),&AnimationKeyEditor::_insert_delay);
	ObjectTypeDB::bind_method(_MD("_step_changed"),&AnimationKeyEditor::_step_changed);


	ObjectTypeDB::bind_method(_MD("_animation_loop_changed"),&AnimationKeyEditor::_animation_loop_changed);
	ObjectTypeDB::bind_method(_MD("_animation_len_changed"),&AnimationKeyEditor::_animation_len_changed);	
	ObjectTypeDB::bind_method(_MD("_create_value_item"),&AnimationKeyEditor::_create_value_item);
	ObjectTypeDB::bind_method(_MD("_pane_drag"),&AnimationKeyEditor::_pane_drag);

	ObjectTypeDB::bind_method(_MD("_animation_len_update"),&AnimationKeyEditor::_animation_len_update);

	ObjectTypeDB::bind_method(_MD("set_animation"),&AnimationKeyEditor::set_animation);
	ObjectTypeDB::bind_method(_MD("_animation_optimize"),&AnimationKeyEditor::_animation_optimize);


	ADD_SIGNAL( MethodInfo("resource_selected", PropertyInfo( Variant::OBJECT, "res"),PropertyInfo( Variant::STRING, "prop") ) );
	ADD_SIGNAL( MethodInfo("keying_changed" ) );
	ADD_SIGNAL( MethodInfo("timeline_changed", PropertyInfo(Variant::REAL,"pos") ) );
	ADD_SIGNAL( MethodInfo("animation_len_changed", PropertyInfo(Variant::REAL,"len") ) );

}


AnimationKeyEditor::AnimationKeyEditor(UndoRedo *p_undo_redo, EditorHistory *p_history,EditorSelection *p_selection) {

	alc="animation_len_changed";
	editor_selection=p_selection;

	selected_track=-1;
	updating=false;
	te_drawing=false;
	undo_redo=p_undo_redo;
	history=p_history;

	HBoxContainer *hb = memnew( HBoxContainer );
	add_child(hb);


	root=NULL;
	//menu = memnew( MenuButton );
	//menu->set_flat(true);
	//menu->set_pos(Point2());
	//add_child(menu);

	menu_track = memnew( MenuButton );
	menu_track->set_text("Tracks");
	hb->add_child(menu_track);
	menu_track->get_popup()->connect("item_pressed",this,"_menu_track");



	hb->add_child( memnew( VSeparator ) );

	zoomicon = memnew( TextureFrame );
	hb->add_child(zoomicon);
	zoomicon->set_tooltip("Animation zoom.");

	zoom = memnew( HSlider );
	//hb->add_child(zoom);
	zoom->set_step(0.01);
	zoom->set_min(0.0);
	zoom->set_max(2.0);
	zoom->set_val(1.0);
	zoom->set_h_size_flags(SIZE_EXPAND_FILL);
	zoom->set_stretch_ratio(2);
	hb->add_child(zoom);
	zoom->connect("value_changed",this,"_scroll_changed");
	zoom->set_tooltip("Animation zoom.");

	hb->add_child( memnew( VSeparator ) );

	Label *l = memnew( Label );
	l->set_text("Len(s):");
	hb->add_child(l);

	length = memnew( SpinBox );
	length->set_min(0.01);
	length->set_max(10000);
	length->set_step(0.01);
	length->set_h_size_flags(SIZE_EXPAND_FILL);
	length->set_stretch_ratio(1);
	length->set_tooltip("Animation length (in seconds).");

	hb->add_child(length);
	length->connect("value_changed",this,"_animation_len_changed");

	l = memnew( Label );
	l->set_text("Step(s):");
	hb->add_child(l);

	step = memnew( SpinBox );
	step->set_min(0.00);
	step->set_max(128);
	step->set_step(0.01);
	step->set_val(0.0);
	step->set_h_size_flags(SIZE_EXPAND_FILL);
	step->set_stretch_ratio(1);
	step->set_tooltip("Cursor step snap (in seconds).");

	hb->add_child(step);
	step->connect("value_changed",this,"_step_changed");

	loop = memnew( ToolButton );
	loop->set_toggle_mode(true);
	loop->connect("pressed",this,"_animation_loop_changed");
	hb->add_child(loop);
	loop->set_tooltip("Enable/Disable looping in animation.");

	hb->add_child( memnew( VSeparator ) );

	edit_button = memnew( ToolButton );
	edit_button->set_toggle_mode(true);
	edit_button->set_focus_mode(FOCUS_NONE);
	edit_button->set_disabled(true);
	hb->add_child(edit_button);
	edit_button->set_tooltip("Enable editing of individual keys by clicking them.");

	move_up_button = memnew( ToolButton );
	hb->add_child(move_up_button);
	move_up_button->connect("pressed",this,"_menu_track",make_binds(TRACK_MENU_MOVE_UP));
	move_up_button->set_focus_mode(FOCUS_NONE);
	move_up_button->set_disabled(true);
	move_up_button->set_tooltip("Move current track up.");

	move_down_button = memnew( ToolButton );
	hb->add_child(move_down_button);
	move_down_button->connect("pressed",this,"_menu_track",make_binds(TRACK_MENU_MOVE_DOWN));
	move_down_button->set_focus_mode(FOCUS_NONE);
	move_down_button->set_disabled(true);
	move_down_button->set_tooltip("Move current track dosn.");

	remove_button = memnew( ToolButton );
	hb->add_child(remove_button);
	remove_button->connect("pressed",this,"_menu_track",make_binds(TRACK_MENU_REMOVE));
	remove_button->set_focus_mode(FOCUS_NONE);
	remove_button->set_disabled(true);
	remove_button->set_tooltip("Remove selected track.");


	optimize_dialog = memnew( ConfirmationDialog );
	add_child(optimize_dialog);
	optimize_dialog->set_title("Anim. Optimizer");
	VBoxContainer *optimize_vb = memnew( VBoxContainer );
	optimize_dialog->add_child(optimize_vb);
	optimize_dialog->set_child_rect(optimize_vb);
	optimize_linear_error = memnew( SpinBox );
	optimize_linear_error->set_max(1.0);
	optimize_linear_error->set_min(0.001);
	optimize_linear_error->set_step(0.001);
	optimize_linear_error->set_val(0.05);
	optimize_vb->add_margin_child("Max. Linear Error:",optimize_linear_error);
	optimize_angular_error = memnew( SpinBox );
	optimize_angular_error->set_max(1.0);
	optimize_angular_error->set_min(0.001);
	optimize_angular_error->set_step(0.001);
	optimize_angular_error->set_val(0.01);

	optimize_vb->add_margin_child("Max. Angular Error:",optimize_angular_error);
	optimize_max_angle = memnew( SpinBox );
	optimize_vb->add_margin_child("Max Optimizable Angle:",optimize_max_angle);
	optimize_max_angle->set_max(360.0);
	optimize_max_angle->set_min(0.0);
	optimize_max_angle->set_step(0.1);
	optimize_max_angle->set_val(22);

	optimize_dialog->get_ok()->set_text("Optimize");



	/*keying = memnew( Button );
	keying->set_toggle_mode(true);
	//keying->set_text("Keys");
	keying->set_anchor_and_margin(MARGIN_LEFT,ANCHOR_END,60);
	keying->set_anchor_and_margin(MARGIN_RIGHT,ANCHOR_END,10);
	keying->set_anchor_and_margin(MARGIN_BOTTOM,ANCHOR_BEGIN,55);
	keying->set_anchor_and_margin(MARGIN_TOP,ANCHOR_BEGIN,10);
	//add_child(keying);
	keying->connect("pressed",this,"_keying_toggled");
	*/

	l = memnew( Label );
	l->set_text("Base: ");
	l->set_pos(Point2(0,3));
//	dr_panel->add_child(l);

//	menu->get_popup()->connect("item_pressed",this,"_menu_callback");

	ec = memnew (EmptyControl);
	ec->set_minsize(Size2(0,50));
	add_child(ec);
	ec->set_v_size_flags(SIZE_EXPAND_FILL);

	hb = memnew( HBoxContainer);
	hb->set_area_as_parent_rect();
	ec->add_child(hb);
	hb->set_v_size_flags(SIZE_EXPAND_FILL);

	track_editor = memnew( Control );
	track_editor->connect("draw",this,"_track_editor_draw");
	hb->add_child(track_editor);
	track_editor->connect("input_event",this,"_track_editor_input_event");
	track_editor->set_focus_mode(Control::FOCUS_ALL);
	track_editor->set_h_size_flags(SIZE_EXPAND_FILL);

	track_pos = memnew( Control );
	track_pos->set_area_as_parent_rect();
	track_pos->set_ignore_mouse(true);
	track_editor->add_child(track_pos);
	track_pos->connect("draw",this,"_track_pos_draw");


	v_scroll = memnew( VScrollBar );
	hb->add_child(v_scroll);
	v_scroll->connect("value_changed",this,"_scroll_changed");
	v_scroll->set_val(0);

	h_scroll = memnew( HScrollBar );
	h_scroll->connect("value_changed",this,"_scroll_changed");
	add_child(h_scroll);
	h_scroll->set_val(0);


	track_name = memnew( LineEdit );
	track_name->set_as_toplevel(true);
	track_name->hide();
	add_child(track_name);
	track_name->connect("text_entered",this,"_track_name_changed");
	track_menu = memnew( PopupMenu );
	add_child(track_menu);
	track_menu->connect("item_pressed",this,"_track_menu_selected");



	last_idx =1;

	_update_menu();



	insert_confirm = memnew( ConfirmationDialog );
	add_child(insert_confirm);
	insert_confirm->connect("confirmed",this,"_confirm_insert_list");

	click.click=ClickOver::CLICK_NONE;


	name_column_ratio=0.3;
	timeline_pos=0;


	key_edit_dialog = memnew( PopupDialog );
	key_editor = memnew( PropertyEditor );
	add_child(key_edit_dialog);
	key_editor->set_area_as_parent_rect();
	key_editor->hide_top_label();
	for(int i=0;i<4;i++)
		key_editor->set_margin(Margin(i),5);
	key_edit_dialog->add_child(key_editor);

	key_edit = memnew( AnimationKeyEdit );
	key_edit->undo_redo=undo_redo;
	key_edit->ke_dialog=key_edit_dialog;
	key_editor->edit(key_edit);
	type_menu = memnew( PopupMenu );
	add_child(type_menu);
	for(int i=0;i<Variant::VARIANT_MAX;i++)
		type_menu->add_item(Variant::get_type_name(Variant::Type(i)),i);
	type_menu->connect("item_pressed",this,"_create_value_item");
	keying=false;
	insert_frame=0;
	insert_query=false;
	insert_queue=false;

	editor_selection->connect("selection_changed",track_editor,"update");


	scale_dialog = memnew( ConfirmationDialog );
	VBoxContainer *vbc = memnew( VBoxContainer );
	scale_dialog->add_child(vbc);
	scale_dialog->set_child_rect(vbc);
	scale = memnew( SpinBox );
	scale->set_min(-99999);
	scale->set_max(99999);
	scale->set_step(0.001);
	vbc->add_margin_child("Scale Ratio:",scale);
	scale_dialog->connect("confirmed",this,"_scale");
	add_child(scale_dialog);



}

AnimationKeyEditor::~AnimationKeyEditor() {



	memdelete( key_edit );

}
