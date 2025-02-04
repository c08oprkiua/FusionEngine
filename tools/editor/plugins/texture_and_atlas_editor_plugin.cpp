#include "texture_and_atlas_editor_plugin.h"


void TextureViewerAtlasTexEditor::_notification(int p_what){
    if (p_what == NOTIFICATION_DRAW and creating_atlas){

        Rect2 rect;
        rect.pos = corner_a;

        draw_rect(Rect2(corner_a, corner_b), Color(1.0, 1.0, 1.0));
    }
}

void TextureViewerAtlasTexEditor::_bind_methods(){
    ObjectTypeDB::bind_method(_MD("_input_event"),&TextureViewerAtlasTexEditor::input_process);
    ObjectTypeDB::bind_method(_MD("switch_mode", "mode"),&TextureViewerAtlasTexEditor::switch_mode);


}

void TextureViewerAtlasTexEditor::load_texture(Texture *p_texture){
    if (!p_texture){ hide(); return;}

    texture_preview->set_texture(p_texture);

    switch_mode(mode);
}

void TextureViewerAtlasTexEditor::input_process(const InputEvent p_event){
    if (p_event.type == InputEvent::MOUSE_BUTTON){
        int button = p_event.mouse_button.button_index;

        if (p_event.is_pressed()){
            switch (button){
                case BUTTON_LEFT: //left click
                    creating_atlas = true;
                    break;
                case BUTTON_RIGHT: //right click
                    //TODO: Have this create an atlas texture by scanning outwards from the click point until clear space is hit on all sides
                    break;

                case BUTTON_WHEEL_UP: //zoom in / pan up
                    //texture_scroll->set_v_scroll(texture_scroll->get_v_scroll() - 1);
                    break;
                case BUTTON_WHEEL_DOWN: //zoom out / pan down
                    //texture_scroll->set_v_scroll(texture_scroll->get_v_scroll() + 1);

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

        if (creating_atlas){
            corner_a = Vector2(0, 0);
        } else {
            //generate_atlas();
        }
    } else if (p_event.type == InputEvent::MOUSE_MOTION and creating_atlas){


        Vector2 pos = Vector2(p_event.mouse_motion.global_x, p_event.mouse_motion.global_y);
        if (corner_a == Vector2(0,0)){
            corner_a = pos;
        } else {
            corner_b = pos;
        }

        //emit_signal("draw");
    }
}

void TextureViewerAtlasTexEditor::switch_mode(ViewMode p_mode){

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

        //TODO: Load atlases
        load_atlas_textures();
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

void TextureViewerAtlasTexEditor::load_atlas_menu(){
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

    if (atlastex_list){memdelete(atlastex_list);}

    atlastex_list = memnew(VBoxContainer);
    atlastex_list->set_h_size_flags(SIZE_EXPAND_FILL);
    atlastex_list->set_v_size_flags(SIZE_EXPAND_FILL);

    scroll->add_child(atlastex_list);
}

void TextureViewerAtlasTexEditor::load_atlas_textures(){

}

TextureViewerAtlasTexEditor::TextureViewerAtlasTexEditor(){
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
    hsplit->set_ignore_mouse(false);
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


    ScrollContainer *scroll = memnew(ScrollContainer);
    scroll->set_h_size_flags(SIZE_EXPAND_FILL);
    scroll->set_v_size_flags(SIZE_EXPAND_FILL);

    CenterContainer *centbox = memnew(CenterContainer);
    centbox->set_h_size_flags(SIZE_EXPAND_FILL);
    centbox->set_v_size_flags(SIZE_EXPAND_FILL);
    centbox->add_child(texture_preview);
    scroll->add_child(centbox);

    hsplit->add_child(scroll);

}



void TexViewAtlasEditEditorPlugin::edit(Object *p_node){

    tex_view->set_undo_redo(&get_undo_redo());

    Texture *tex = p_node->cast_to<Texture>();

    if (!tex) return;

     tex_view->load_texture(tex);
}

bool TexViewAtlasEditEditorPlugin::handles(Object *p_node) const {
    return p_node->is_type("Texture");
}

void TexViewAtlasEditEditorPlugin::make_visible(bool p_visible){
    p_visible ? tex_view->show() : tex_view->hide();
}

bool TexViewAtlasEditEditorPlugin::forward_input_event(const InputEvent& p_event){
    tex_view->input_process(p_event);
    return false;
}

TexViewAtlasEditEditorPlugin::TexViewAtlasEditEditorPlugin(EditorNode *p_node){
	editor=p_node;
	tex_view = memnew( TextureViewerAtlasTexEditor );
	editor->get_viewport()->add_child(tex_view);
	tex_view->set_area_as_parent_rect();
    tex_view->hide();
}
