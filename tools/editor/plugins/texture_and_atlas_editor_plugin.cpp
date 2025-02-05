#include "texture_and_atlas_editor_plugin.h"

Rect2i create_rect_from_points(Point2i p_point_a, Point2i p_point_b){
    Rect2i rect;

    Point2i top_left;
    Point2i bottom_right;

    bool a_is_leftward = p_point_a.x < p_point_b.x;
    bool a_is_upward = p_point_a.y < p_point_b.y;

    if (a_is_leftward){
            top_left.x = p_point_a.x;
            bottom_right.x = p_point_b.x;
    } else {
            top_left.x = p_point_b.x;
            bottom_right.x = p_point_a.x;
    }
    if (a_is_upward){
            top_left.y = p_point_a.y;
            bottom_right.y = p_point_b.y;
    } else {
            top_left.y = p_point_b.y;
            bottom_right.y = p_point_a.y;
    }

    rect.pos = top_left;
    rect.size = bottom_right - top_left;

    return rect;
}


AtlasTexPreview::AtlasTexPreview(AtlasTexture *p_tex){
    tex = p_tex;

    HBoxContainer *hbox = memnew(HBoxContainer);

    select = memnew(Button);
    select->set_icon(tex);

    del_button = memnew(Button);
    del_button->set_text("Delete");
    del_button->set_h_size_flags(SIZE_EXPAND_FILL);
    del_button->set_v_size_flags(SIZE_EXPAND_FILL);

    hbox->add_child(select);
    hbox->add_child(del_button);

    add_child(hbox);
}

void TextureViewerEditor::_notification(int p_what){
    if (p_what == NOTIFICATION_DRAW){
        if (creating_atlas and corner_b != Vector2(0,0)){
            Color rect_color = Color::hex(0x00'FF'00'FF);
            Color corn_a_color = Color::hex(0xFF'00'00'FF);
            Color corn_b_color = Color::hex(0x00'00'FF'FF);

            Rect2 corner_rect;
            corner_rect.size = Vector2(10.0f, 10.0f);

            draw_rect(create_rect_from_points(corner_a, corner_b), rect_color);

            corner_rect.pos = corner_a - Vector2(5.0, 5.0);
            draw_rect(corner_rect, corn_a_color);

            corner_rect.pos = corner_b - Vector2(5.0, 5.0);
            draw_rect(corner_rect, corn_b_color);
        }
    }
}

void TextureViewerEditor::_input_event(const InputEvent p_event){
    if (p_event.type == InputEvent::MOUSE_BUTTON){
        int button = p_event.mouse_button.button_index;

        if (p_event.is_pressed()){
            switch (button){
                case BUTTON_LEFT: //left click
                    if (mode == VIEW_ATLASTEX_CREATOR){
                        if (!texture_scroll){ break;}
                        Point2i mouse_pos = Point2i(p_event.mouse_button.global_x, p_event.mouse_button.global_y);

                        corner_a = mouse_pos - get_global_transform().get_origin();
                        creating_atlas = true;
                    }
                    break;
                case BUTTON_RIGHT: //right click
                    //TODO: Have this create an atlas texture by scanning outwards from the click point until clear space is hit on all sides
                    break;

                case BUTTON_WHEEL_UP: //zoom in / pan up
                    texture_scroll->set_v_scroll(texture_scroll->get_v_scroll() - 1);
                    break;
                case BUTTON_WHEEL_DOWN: //zoom out / pan down
                    texture_scroll->set_v_scroll(texture_scroll->get_v_scroll() + 1);
                    break;
                case 6: //pan left
                    texture_scroll->set_h_scroll(texture_scroll->get_h_scroll() - 1);
                    break;
                case 7: //pan right
                    texture_scroll->set_h_scroll(texture_scroll->get_h_scroll() + 1);
                    break;

                default: break;
            }
        } else {
            creating_atlas = false;
        }

        if (not creating_atlas and corner_a != Point2i(0,0) and corner_b != Point2i(0,0)){
            //generate_atlas

            AtlasTexture *new_tex = memnew(AtlasTexture);

            new_tex->set_atlas(texture_preview->get_texture());

            Rect2i tex_region = create_rect_from_points(corner_a, corner_b);

            new_tex->set_region(tex_region);

            add_atlas_button(new_tex);

            corner_a = Vector2(0,0);
            corner_b = Vector2(0,0);
        }
    } else if (p_event.type == InputEvent::MOUSE_MOTION and creating_atlas){
        Point2i mouse_pos = Point2i(p_event.mouse_motion.global_x, p_event.mouse_motion.global_y);
        corner_b = mouse_pos - get_global_transform().get_origin();
    }
    update();
}

void TextureViewerEditor::_bind_methods(){
    ObjectTypeDB::bind_method(_MD("_input_event"),&TextureViewerEditor::_input_event);
    ObjectTypeDB::bind_method(_MD("switch_mode", "mode"),&TextureViewerEditor::switch_mode);


}

void TextureViewerEditor::load_texture(Texture *p_texture){
    if (!p_texture){ hide(); return;}

    texture_preview->set_texture(p_texture);

    switch_mode(mode);
}

void TextureViewerEditor::switch_mode(ViewMode p_mode){

    if (p_mode != mode){ //clear out info panel
        for (int i = 0; i < info_base->get_child_count();){
            Node * child =info_base->get_child(i);
            memdelete(child);
        }
    }

    if (p_mode == VIEW_IMAGE){
        info_base->hide();
    }

    else if (p_mode == VIEW_ATLASTEX_CREATOR){
        if (p_mode != mode){
            load_atlas_menu();
        }

        load_atlas_textures(EditorFileSystem::get_singleton()->get_filesystem());
    }

    else if (p_mode == VIEW_PALETTE){
        ImageTexture *img_text = texture_preview->get_texture()->cast_to<ImageTexture>();

        if (img_text and (img_text->get_format() == Image::FORMAT_INDEXED or img_text->get_format() == Image::FORMAT_INDEXED_ALPHA)){
            //we can actually load the palette info
        }
        else {
            //smth smth tell the user this is not an indexed image
        }
    }

    mode = p_mode;
}

void TextureViewerEditor::load_atlas_menu(){
    info_base->show();
    info_base->set_tooltip("All the AtlasTextures using this texture as an atlas.");

    VBoxContainer *vbox = memnew(VBoxContainer);
    vbox->set_h_size_flags(SIZE_FILL);
    vbox->set_v_size_flags(SIZE_FILL);

    info_base->add_child(vbox);

    Label *label = memnew(Label); label->set_text("AtlasTextures");

    vbox->add_child(label);

    ScrollContainer *scroll = memnew(ScrollContainer);

    vbox->add_child(scroll);

    atlastex_list = memnew(VBoxContainer);
    atlastex_list->set_h_size_flags(SIZE_EXPAND_FILL);
    atlastex_list->set_v_size_flags(SIZE_EXPAND_FILL);

    scroll->add_child(atlastex_list);
}

//based on QuickOpen
void TextureViewerEditor::load_atlas_textures(EditorFileSystemDirectory *efsd){

	for(int i = 0; i < efsd->get_subdir_count(); i++) {
		load_atlas_textures(efsd->get_subdir(i));
	}

	for(int i = 0; i < efsd->get_file_count(); i++) {
		if (ObjectTypeDB::is_type(efsd->get_file_type(i),"AtlasTexture")) {
            String atlas_path = efsd->get_file(i);

            Ref<AtlasTexture> tex = ResourceLoader::load(atlas_path);
            //This kinda just assumes the file loads successfully, might be an issue but idk
            add_atlas_button(tex.ptr());
		}
	}
}

void TextureViewerEditor::add_atlas_button(AtlasTexture *p_atlas){
    if (!atlastex_list){return;}

    AtlasTexPreview *preview = memnew(AtlasTexPreview(p_atlas));

    atlastex_list->add_child(preview);
}

TextureViewerEditor::TextureViewerEditor(){
    set_h_size_flags(SIZE_EXPAND_FILL);
    set_v_size_flags(SIZE_EXPAND_FILL);

    VBoxContainer *base = memnew(VBoxContainer);

    add_child(base);

    //Header bar with options

    OptionButton *opt = memnew(OptionButton);

    opt->set_tooltip("Change the image viewer mode");
    opt->add_item("Texture viewer");
    opt->add_item("AtlasTexture Creator");
    //opt->add_item("PaletteTexture viewer");

    opt->connect("item_selected", this, "switch_mode");

    HBoxContainer *hbox = memnew(HBoxContainer);

    hbox->add_child(opt);

    base->add_child(hbox);

    //Base GUI

    HSplitContainer *hsplit = memnew(HSplitContainer);

    hsplit->set_anchor(MARGIN_RIGHT, ANCHOR_END);
    hsplit->set_anchor(MARGIN_BOTTOM, ANCHOR_END);
    hsplit->set_stop_mouse(false);
    hsplit->set_h_size_flags(SIZE_FILL);
    hsplit->set_v_size_flags(SIZE_EXPAND_FILL);

    base->add_child(hsplit);

    //Info base

    info_base = memnew(PanelContainer);
    info_base->set_h_size_flags(SIZE_FILL);
    info_base->set_v_size_flags(SIZE_FILL);

    hsplit->add_child(info_base);

    info_base->hide();

    //Texture view


    texture_preview = memnew(TextureFrame);
    texture_preview->set_expand(false);
    texture_preview->set_h_size_flags(SIZE_FILL);
    texture_preview->set_v_size_flags(SIZE_FILL);
    texture_preview->set_stop_mouse(false);
    texture_preview->set_ignore_mouse(false);


    texture_scroll = memnew(ScrollContainer);
    texture_scroll->set_h_size_flags(SIZE_EXPAND_FILL);
    texture_scroll->set_v_size_flags(SIZE_EXPAND_FILL);
    texture_scroll->set_stop_mouse(false);

    texture_scroll->add_child(texture_preview);

    hsplit->add_child(texture_scroll);

}



void TextureViewEditorPlugin::edit(Object *p_node){

    tex_view->set_undo_redo(&get_undo_redo());

    Texture *tex = p_node->cast_to<Texture>();

    if (!tex) return;

     tex_view->load_texture(tex);
}

bool TextureViewEditorPlugin::handles(Object *p_node) const {
    return p_node->is_type("Texture");
}

void TextureViewEditorPlugin::make_visible(bool p_visible){
    if (p_visible){
        tex_view->show();
        tex_view->set_process(true);

    } else {
        tex_view->hide();
        tex_view->set_process(false);
    }
}

TextureViewEditorPlugin::TextureViewEditorPlugin(EditorNode *p_node){
	editor=p_node;
	tex_view = memnew( TextureViewerEditor );
	editor->get_viewport()->add_child(tex_view);
	tex_view->set_area_as_parent_rect();
    tex_view->hide();
}
