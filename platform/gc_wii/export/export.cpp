#include "export.h"
#include "platform/gc_wii/logo.h"
#include "platform/gc_wii/gc-logo.h"
#include "tools/editor/editor_import_export.h"
#include "tools/editor/editor_settings.h"
#include "scene/resources/texture.h"


class EditorExportPlatformWii : public EditorExportPlatformConsole {
    OBJ_TYPE(EditorExportPlatformWii, EditorExportPlatformConsole);
public:
	virtual ImageCompression get_image_compression() const;

	virtual bool poll_devices() {return false;}
	virtual int get_device_count() const {return 0;}
	virtual String get_device_name(int p_device) const {return "";}
	virtual String get_device_info(int p_device) const {return "";}
	virtual Error run(int p_device,bool p_dumb=false);

	virtual bool can_export(String *r_error=NULL) const;

	virtual bool requires_password(bool p_debug) const {return false;}
	virtual Error export_project(const String& p_path,bool p_debug,bool p_dumb=false);
};

class EditorExportPlatformGameCube : public EditorExportPlatformConsole {
    OBJ_TYPE(EditorExportPlatformGameCube, EditorExportPlatformConsole);
public:
	virtual ImageCompression get_image_compression() const;

	virtual bool poll_devices() {return false;}
	virtual int get_device_count() const {return 0;}
	virtual String get_device_name(int p_device) const {return "";}
	virtual String get_device_info(int p_device) const {return "";}
	virtual Error run(int p_device,bool p_dumb=false);

	virtual bool can_export(String *r_error=NULL) const;

	virtual bool requires_password(bool p_debug) const {return false;}
	virtual Error export_project(const String& p_path,bool p_debug,bool p_dumb=false);
};

EditorExportPlatform::ImageCompression EditorExportPlatformWii::get_image_compression() const{
    return IMAGE_COMPRESSION_NONE;
};

Error EditorExportPlatformWii::run(int p_device,bool p_dumb){
    return OK;
}

bool EditorExportPlatformWii::can_export(String *r_error) const {
    return false;
};

Error EditorExportPlatformWii::export_project(const String& p_path,bool p_debug,bool p_dumb){
    return OK;
};

//////////////////////////////////

EditorExportPlatform::ImageCompression EditorExportPlatformGameCube::get_image_compression() const{
    return IMAGE_COMPRESSION_NONE;
};

Error EditorExportPlatformGameCube::run(int p_device,bool p_dumb){
    return OK;
}

bool EditorExportPlatformGameCube::can_export(String *r_error) const {
    return false;
};

/*
	GCM is the disc container for GameCube games, similar in equivalency to formats like ISO.
	Loaders like Swiss can support up to ~4GiB-sized containers, and the theoretical max is 16GiB,
	according to Extrems. Of course, that is much larger than the 1.5GB of real estate on a GC disc,
	so if you want your game to be burnable onto a disc you'll have to make it fit into that!

	A GCM that exceeds 4GiB will have to use the "Wii variant" of FST.
*/
Error EditorExportPlatformGameCube::export_project(const String& p_path,bool p_debug,bool p_dumb){
    return OK;
};

void register_gc_wii_exporter(){
	Image img(_gc_wii_logo);

    Ref<EditorExportPlatformWii> exporter_wii = Ref<EditorExportPlatformWii>(memnew(EditorExportPlatformWii));

	exporter_wii->set_name("Nintendo Wii");
	exporter_wii->set_binary_extension("dol");

	Ref<ImageTexture> wii_logo = memnew(ImageTexture);
	wii_logo->create_from_image(img);
	exporter_wii->set_logo(wii_logo);

	EditorImportExport::get_singleton()->add_export_platform(exporter_wii);

	Ref<EditorExportPlatformGameCube> exporter_gc = Ref<EditorExportPlatformGameCube>(memnew(EditorExportPlatformGameCube));

	exporter_gc->set_name("Nintendo GameCube");
	exporter_gc->set_binary_extension("gcm");

	img = Image(_gc_logo);

	Ref<ImageTexture> gc_logo = memnew(ImageTexture);
	gc_logo->create_from_image(img);
	exporter_gc->set_logo(gc_logo);

	EditorImportExport::get_singleton()->add_export_platform(exporter_gc);
} 
