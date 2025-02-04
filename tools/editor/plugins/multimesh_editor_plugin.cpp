/*************************************************************************/
/*  multimesh_editor_plugin.cpp                                          */
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
#include "multimesh_editor_plugin.h"
#include "scene/gui/box_container.h"
#include "scene/3d/mesh_instance.h"



void MultiMeshEditor::_node_removed(Node *p_node) {

	if(p_node==node) {
		node=NULL;
		hide();
	}

}


void MultiMeshEditor::_populate() {

	if(!node)
		return;


	Ref<Mesh> mesh;

	if (mesh_source->get_text()=="") {

		Ref<MultiMesh> multimesh;
		multimesh = node->get_multimesh();
		if (multimesh.is_null()) {

			err_dialog->set_text("No mesh source specified (and no MultiMesh set in node).");
			err_dialog->popup_centered(Size2(300,100));
			return;
		}
		if (multimesh->get_mesh().is_null()) {

			err_dialog->set_text("No mesh source specified (and MultiMesh contains no Mesh).");
			err_dialog->popup_centered(Size2(300,100));
			return;
		}

		mesh = multimesh->get_mesh();
	} else {

		Node *ms_node = node->get_node(mesh_source->get_text());

		if (!ms_node) {

			err_dialog->set_text("Mesh source is invalid (Invalid Path).");
			err_dialog->popup_centered(Size2(300,100));
			return;
		}

		MeshInstance3D *ms_instance = ms_node->cast_to<MeshInstance3D>();

		if (!ms_instance) {

			err_dialog->set_text("Mesh source is invalid (Not a MeshInstance3D).");
			err_dialog->popup_centered(Size2(300,100));
			return;
		}

		mesh=ms_instance->get_mesh();

		if (mesh.is_null()) {

			err_dialog->set_text("Mesh source is invalid (Contains no Mesh resource).");
			err_dialog->popup_centered(Size2(300,100));
			return;
		}

	}

	if (surface_source->get_text()=="") {

		err_dialog->set_text("No surface source specified.");
		err_dialog->popup_centered(Size2(300,100));
		return;
	}

	Node *ss_node = node->get_node(surface_source->get_text());

	if (!ss_node) {

		err_dialog->set_text("Surface source is invalid (Invalid Path).");
		err_dialog->popup_centered(Size2(300,100));
		return;
	}

	GeometryInstance3D *ss_instance = ss_node->cast_to<MeshInstance3D>();

	if (!ss_instance) {

		err_dialog->set_text("Surface source is invalid (Not Geometry).");
		err_dialog->popup_centered(Size2(300,100));
		return;
	}

	Transform3D geom_xform = node->get_global_transform().affine_inverse() * ss_instance->get_global_transform();

	DVector<Face3> geometry = ss_instance->get_faces(VisualInstance3D::FACES_SOLID);

	if (geometry.size()==0) {

		err_dialog->set_text("Surface source is invalid (No Faces).");
		err_dialog->popup_centered(Size2(300,100));
		return;
	}

	//make all faces local

	int gc = geometry.size();
	DVector<Face3>::Write w = geometry.write();

	for(int i=0;i<gc;i++) {
		for(int j=0;j<3;j++) {
			w[i].vertex[j] = geom_xform.xform( w[i].vertex[j] );
		}
	}



	w = DVector<Face3>::Write();
#if 0
	node->get_multimesh()->set_instance_count(populate_amount->get_val());
	node->populate_parent(populate_rotate_random->get_val(),populate_tilt_random->get_val(),populate_scale_random->get_val(),populate_scale->get_val());


	ERR_EXPLAIN("Parent is not of type VisualInstance3D.");
	ERR_FAIL_COND(!get_parent() || !get_parent()->is_type("VisualInstance3D"));

	ERR_EXPLAIN("Multimesh not present");
	ERR_FAIL_COND(multimesh.is_null());

	VisualInstance3D *vi = get_parent()->cast_to<VisualInstance3D>();
	ERR_EXPLAIN("Parent is not of type VisualInstance3D, can't be populated.");
	ERR_FAIL_COND(!vi);

#endif
	DVector<Face3> faces = geometry;
	ERR_EXPLAIN("Parent has no solid faces to populate.");
	int facecount=faces.size();
	ERR_FAIL_COND(!facecount);

	DVector<Face3>::Read r = faces.read();



	float area_accum=0;
	Map<float,int> triangle_area_map;
	for(int i=0;i<facecount;i++) {

		float area = r[i].get_area();;
		if (area<CMP_EPSILON)
			continue;
		triangle_area_map[area_accum]=i;
		area_accum+=area;
	}

	ERR_EXPLAIN("Couldn't map area");
	ERR_FAIL_COND(triangle_area_map.size()==0);
	ERR_EXPLAIN("Couldn't map area");
	ERR_FAIL_COND(area_accum==0);


	Ref<MultiMesh> multimesh = memnew( MultiMesh );
	multimesh->set_mesh(mesh);

	int instance_count=populate_amount->get_val();

	multimesh->set_instance_count(instance_count);

	float _tilt_random = populate_tilt_random->get_val();
	float _rotate_random = populate_rotate_random->get_val();
	float _scale_random = populate_scale_random->get_val();
	float _scale = populate_scale->get_val();
	int axis = populate_axis->get_selected();

	Transform3D axis_xform;
	if (axis==Vector3::AXIS_Z) {
		axis_xform.rotate(Vector3(1,0,0),Math_PI*0.5);
	}
	if (axis==Vector3::AXIS_X) {
		axis_xform.rotate(Vector3(0,0,1),Math_PI*0.5);
	}

	for(int i=0;i<instance_count;i++) {

		float areapos = Math::random(0,area_accum);

		Map<float,int>::Element *E = triangle_area_map.find_closest(areapos);
		ERR_FAIL_COND(!E)
		int index = E->get();
		ERR_FAIL_INDEX(index,facecount);

		// ok FINALLY get face
		Face3 face = r[index];
		//now compute some position inside the face...

		Vector3 pos = face.get_random_point_inside();
		Vector3 normal = face.get_plane().normal;
		Vector3 op_axis = (face.vertex[0]-face.vertex[1]).normalized();

		Transform3D xform;

		xform.set_look_at(pos, pos+op_axis,normal);
		xform = xform * axis_xform;


		Basis post_xform;

		post_xform.rotate(xform.basis.get_axis(0),Math::random(-_tilt_random,_tilt_random)*Math_PI);
		post_xform.rotate(xform.basis.get_axis(2),Math::random(-_tilt_random,_tilt_random)*Math_PI);
		post_xform.rotate(xform.basis.get_axis(1),Math::random(-_rotate_random,_rotate_random)*Math_PI);
		xform.basis = post_xform * xform.basis;
		//xform.basis.orthonormalize();


		xform.basis.scale(Vector3(1,1,1)*(_scale+Math::random(-_scale_random,_scale_random)));


		multimesh->set_instance_transform(i,xform);
		multimesh->set_instance_color(i,Color(1,1,1,1));
	}

	multimesh->generate_aabb();

	node->set_multimesh(multimesh);

}

void MultiMeshEditor::_browsed(const NodePath& p_path) {


	NodePath path = node->get_path_to( get_node(p_path) );

	if (browsing_source)
		mesh_source->set_text(path);
	else
		surface_source->set_text(path);
}

void MultiMeshEditor::_menu_option(int p_option) {


	switch(p_option) {


		case MENU_OPTION_POPULATE: {

			if (_last_pp_node!=node) {


				surface_source->set_text("..");
				mesh_source->set_text("..");
				populate_axis->select(1);
				populate_rotate_random->set_val(0);
				populate_tilt_random->set_val(0);
				populate_scale_random->set_val(0);
				populate_scale->set_val(1);
				populate_amount->set_val(128);

				_last_pp_node=node;
			}
			populate_dialog->popup_centered(Size2(250,395));

		} break;
	}
}


void MultiMeshEditor::edit(MultiMeshInstance3D *p_multimesh) {

	node=p_multimesh;	

}

void MultiMeshEditor::_browse(bool p_source) {

	browsing_source=p_source;
	std->get_tree()->set_marked(node,false);
	std->popup_centered_ratio();
	if (p_source)
		std->set_title("Select a Source Mesh:");
	else
		std->set_title("Select a Target Surface:");
}

void MultiMeshEditor::_bind_methods() {

	ObjectTypeDB::bind_method("_menu_option",&MultiMeshEditor::_menu_option);
	ObjectTypeDB::bind_method("_populate",&MultiMeshEditor::_populate);
	ObjectTypeDB::bind_method("_browsed",&MultiMeshEditor::_browsed);
	ObjectTypeDB::bind_method("_browse",&MultiMeshEditor::_browse);
}

MultiMeshEditor::MultiMeshEditor() {


	options = memnew( MenuButton );
	add_child(options);
	options->set_area_as_parent_rect();

	options->set_text("MultiMesh");
	options->get_popup()->add_item("Populate Surface");
	options->get_popup()->connect("item_pressed", this,"_menu_option");

	populate_dialog = memnew( ConfirmationDialog );
	populate_dialog->set_title("Populate MultiMesh");
	add_child(populate_dialog);

	VBoxContainer *vbc = memnew( VBoxContainer );
	populate_dialog->add_child(vbc);
	populate_dialog->set_child_rect(vbc);


	HBoxContainer *hbc = memnew( HBoxContainer );

	surface_source = memnew( LineEdit );
	hbc->add_child(surface_source);
	surface_source->set_h_size_flags(SIZE_EXPAND_FILL);
	Button *b = memnew( Button );
	hbc->add_child(b);
	b->set_text("..");
	b->connect("pressed",this,"_browse",make_binds(false));

	vbc->add_margin_child("Target Surface:",hbc);

	hbc = memnew( HBoxContainer );
	mesh_source = memnew( LineEdit );
	hbc->add_child(mesh_source);
	mesh_source->set_h_size_flags(SIZE_EXPAND_FILL);
	b = memnew( Button );
	hbc->add_child(b);
	b->set_text("..");
	vbc->add_margin_child("Source Mesh:",hbc);
	b->connect("pressed",this,"_browse",make_binds(true));


	populate_axis = memnew( OptionButton );
	populate_axis->add_item("X-Axis");
	populate_axis->add_item("Y-Axis");
	populate_axis->add_item("Z-Axis");
	populate_axis->select(2);
	vbc->add_margin_child("Mesh Up Axis:",populate_axis);

	populate_rotate_random = memnew( HScrollBar );
	populate_rotate_random->set_max(1);
	populate_rotate_random->set_step(0.01);
	vbc->add_margin_child("Random Rotation:",populate_rotate_random);

	populate_tilt_random = memnew( HScrollBar );
	populate_tilt_random->set_max(1);
	populate_tilt_random->set_step(0.01);
	vbc->add_margin_child("Random Tilt:",populate_tilt_random);


	populate_scale_random = memnew( SpinBox );
	populate_scale_random->set_min(0);
	populate_scale_random->set_max(1);
	populate_scale_random->set_val(0);

	vbc->add_margin_child("Random Scale:",populate_scale_random);

	populate_scale = memnew( SpinBox );
	populate_scale->set_min(0.001);
	populate_scale->set_max(4096);
	populate_scale->set_val(1);

	vbc->add_margin_child("Scale:",populate_scale);


	populate_amount = memnew( SpinBox );
	populate_amount->set_anchor(MARGIN_RIGHT,ANCHOR_END);
	populate_amount->set_begin( Point2(20,232));
	populate_amount->set_end( Point2(5,237));
	populate_amount->set_min(1);
	populate_amount->set_max(65536);
	populate_amount->set_val(128);
	vbc->add_margin_child("Amount:",populate_amount);

	populate_dialog->get_ok()->set_text("Populate");

	populate_dialog->get_ok()->connect("pressed", this,"_populate");
	std = memnew( SceneTreeDialog );
	populate_dialog->add_child(std);
	std->connect("selected",this,"_browsed");

	_last_pp_node=NULL;
	//options->set_anchor(MARGIN_LEFT,Control::ANCHOR_END);
	//options->set_anchor(MARGIN_RIGHT,Control::ANCHOR_END);
	err_dialog = memnew( AcceptDialog );
	add_child(err_dialog);
}


void MultiMeshEditorPlugin::edit(Object *p_object) {

	multimesh_editor->edit(p_object->cast_to<MultiMeshInstance3D>());
}

bool MultiMeshEditorPlugin::handles(Object *p_object) const {

	return p_object->is_type("MultiMeshInstance3D");
}

void MultiMeshEditorPlugin::make_visible(bool p_visible) {

	if (p_visible) {
		multimesh_editor->show();
	} else {

		multimesh_editor->hide();
		multimesh_editor->edit(NULL);
	}

}

MultiMeshEditorPlugin::MultiMeshEditorPlugin(EditorNode *p_node) {

	editor=p_node;
	multimesh_editor = memnew( MultiMeshEditor );
	editor->get_viewport()->add_child(multimesh_editor);

//	multimesh_editor->set_anchor(MARGIN_LEFT,Control::ANCHOR_END);
//	multimesh_editor->set_anchor(MARGIN_RIGHT,Control::ANCHOR_END);
	multimesh_editor->set_margin(MARGIN_LEFT,253);
	multimesh_editor->set_margin(MARGIN_RIGHT,310);
	multimesh_editor->set_margin(MARGIN_TOP,0);
	multimesh_editor->set_margin(MARGIN_BOTTOM,10);



	multimesh_editor->hide();
}


MultiMeshEditorPlugin::~MultiMeshEditorPlugin()
{
}


