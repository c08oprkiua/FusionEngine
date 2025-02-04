#ifndef TEXTURE_AND_ATLASTEX_EDITOR_PLUGIN_H
#define TEXTURE_AND_ATLASTEX_EDITOR_PLUGIN_H

#include "tools/editor/editor_plugin.h"
#include "tools/editor/editor_node.h"
#include "scene/gui/box_container.h"
#include "scene/gui/split_container.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/dialogs.h"

class TextureViewerAtlasTexEditor : public PanelContainer {
    OBJ_TYPE(TextureViewerAtlasTexEditor, PanelContainer);
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

    void load_atlas_textures();

protected:
    void _notification(int p_what);

    static void _bind_methods();

public:
    Texture *get_texture();
    void load_texture(Texture *p_texture);

    void set_undo_redo(UndoRedo *p_undo_redo) {undo_redo=p_undo_redo; }

    void input_process(const InputEvent p_event);

    void switch_mode(ViewMode mode);

    TextureViewerAtlasTexEditor();
};

VARIANT_ENUM_CAST(TextureViewerAtlasTexEditor::ViewMode);

class TexViewAtlasEditEditorPlugin : public EditorPlugin {
    OBJ_TYPE(TexViewAtlasEditEditorPlugin, EditorPlugin);

    EditorNode *editor;
    TextureViewerAtlasTexEditor *tex_view;

public:

	virtual String get_name() const { return "ImageTexture"; }
	bool has_main_screen() const { return false; }
	virtual void edit(Object *p_node);
	virtual bool handles(Object *p_node) const;
	virtual void make_visible(bool p_visible);

    virtual bool forward_input_event(const InputEvent& p_event);

    TexViewAtlasEditEditorPlugin(EditorNode *p_editor_node);
};

#endif
