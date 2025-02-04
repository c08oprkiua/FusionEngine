/*************************************************************************/
/*  bone_attachment.cpp                                                  */
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
#include "bone_attachment.h"

bool BoneAttachment3D::_get(const StringName& p_name,Variant &r_ret) const {

	if (String(p_name)=="bone_name") {

		r_ret=get_bone_name();
		return true;
	}

	return false;
}
bool BoneAttachment3D::_set(const StringName& p_name, const Variant& p_value){

	if (String(p_name)=="bone_name") {

		set_bone_name(p_value);
		return true;
	}

	return false;
}
void BoneAttachment3D::_get_property_list( List<PropertyInfo>* p_list ) const{

	Skeleton3D *parent=NULL;
	if(get_parent())
		parent=get_parent()->cast_to<Skeleton3D>();

	if (parent) {

		String names;
		for(int i=0;i<parent->get_bone_count();i++) {
			if(i>0)
				names+=",";
			names+=parent->get_bone_name(i);
		}

		p_list->push_back(PropertyInfo(Variant::STRING,"bone_name",PROPERTY_HINT_ENUM,names));
	} else {

		p_list->push_back(PropertyInfo(Variant::STRING,"bone_name"));

	}

}


void BoneAttachment3D::_check_bind() {

	if (get_parent() && get_parent()->cast_to<Skeleton3D>()) {
		Skeleton3D *sk = get_parent()->cast_to<Skeleton3D>();
		int idx = sk->find_bone(bone_name);
		if (idx!=-1) {
			sk->bind_child_node_to_bone(idx,this);;
			bound=true;
		}
	}
}

void BoneAttachment3D::_check_unbind() {

	if (bound) {

		if (get_parent() && get_parent()->cast_to<Skeleton3D>()) {
			Skeleton3D *sk = get_parent()->cast_to<Skeleton3D>();
			int idx = sk->find_bone(bone_name);
			if (idx!=-1) {
				sk->unbind_child_node_from_bone(idx,this);;
			}
		}
		bound=false;
	}
}

void BoneAttachment3D::set_bone_name(const String& p_name) {

	if (is_inside_tree())
		_check_unbind();

	bone_name=p_name;

	if (is_inside_tree())
		_check_bind();
}

String BoneAttachment3D::get_bone_name() const{

	return bone_name;
}

void BoneAttachment3D::_notification(int p_what) {

	switch(p_what) {

		case NOTIFICATION_ENTER_TREE: {

			_check_bind();
		} break;
		case NOTIFICATION_EXIT_TREE: {

			_check_unbind();
		} break;
	}
}

BoneAttachment3D::BoneAttachment3D()
{
	bound=false;

}
