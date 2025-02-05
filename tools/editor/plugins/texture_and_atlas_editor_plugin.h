#ifndef TEXTURE_AND_ATLASTEX_EDITOR_PLUGIN_H
#define TEXTURE_AND_ATLASTEX_EDITOR_PLUGIN_H

#include "tools/editor/editor_plugin.h"
#include "tools/editor/editor_node.h"
#include "scene/gui/box_container.h"
#include "scene/gui/split_container.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/dialogs.h"

class AtlasTexPreview : public PanelContainer {
    OBJ_TYPE(AtlasTexPreview, PanelContainer);

    AtlasTexture *tex;
    Button *select;
    Button *del_button;

public:
    AtlasTexPreview(AtlasTexture *p_tex);
};


class TextureViewerEditor : public PanelContainer {
    OBJ_TYPE(TextureViewerEditor, PanelContainer);
public:
    enum ViewMode {
        VIEW_IMAGE,
        VIEW_ATLASTEX_CREATOR,
        VIEW_PALETTE,
    };
private:
    TextureFrame *texture_preview;
    ScrollContainer *texture_scroll;
    VBoxContainer *atlastex_list;
    PanelContainer *info_base;
    UndoRedo *undo_redo;

    Vector2 corner_a;
    Vector2 corner_b;

    ViewMode mode;
    bool creating_atlas;
    bool panning;

    void load_atlas_menu();
    //void load_palette_menu();

    void load_atlas_textures(EditorFileSystemDirectory *efsd);

    void add_atlas_button(AtlasTexture *p_atlas);

protected:
    void _notification(int p_what);
    static void _bind_methods();

public:
    void _input_event(const InputEvent p_event);

    Texture *get_texture();
    void load_texture(Texture *p_texture);

    void set_undo_redo(UndoRedo *p_undo_redo) {undo_redo=p_undo_redo; }

    void switch_mode(ViewMode mode);

    TextureViewerEditor();
};

VARIANT_ENUM_CAST(TextureViewerEditor::ViewMode);

class TextureViewEditorPlugin : public EditorPlugin {
    OBJ_TYPE(TextureViewEditorPlugin, EditorPlugin);

    EditorNode *editor;
    TextureViewerEditor *tex_view;

public:

	virtual String get_name() const { return "Texture"; }
	bool has_main_screen() const { return true; }
	virtual void edit(Object *p_node);
	virtual bool handles(Object *p_node) const;
	virtual void make_visible(bool p_visible);

    TextureViewEditorPlugin(EditorNode *p_editor_node);
};

#endif
