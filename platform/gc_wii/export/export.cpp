#include "export.h"
#include "platform/gc_wii/logo.h"
#include "platform/gc_wii/gc-logo.h"
#include "tools/editor/editor_import_export.h"
#include "tools/editor/editor_settings.h"
#include "scene/resources/texture.h"


class EditorExportPlatformWii : public EditorExportPlatform {
    OBJ_TYPE(EditorExportPlatformWii, EditorExportPlatform);

public:

	virtual String get_name() const;
	virtual ImageCompression get_image_compression() const;
	virtual Ref<Texture> get_logo() const;

	virtual bool poll_devices() {return false;}
	virtual int get_device_count() const {return 0;}
	virtual String get_device_name(int p_device) const {return "";}
	virtual String get_device_info(int p_device) const {return "";}
	virtual Error run(int p_device,bool p_dumb=false);

	virtual bool can_export(String *r_error=NULL) const;

	virtual bool requires_password(bool p_debug) const {return false;}
	virtual String get_binary_extension() const;
	virtual Error export_project(const String& p_path,bool p_debug,bool p_dumb=false);
};

class EditorExportPlatformGameCube : public EditorExportPlatform {
    OBJ_TYPE(EditorExportPlatformGameCube, EditorExportPlatform);
public:

	virtual String get_name() const;
	virtual ImageCompression get_image_compression() const;
	virtual Ref<Texture> get_logo() const;

	virtual bool poll_devices() {return false;}
	virtual int get_device_count() const {return 0;}
	virtual String get_device_name(int p_device) const {return "";}
	virtual String get_device_info(int p_device) const {return "";}
	virtual Error run(int p_device,bool p_dumb=false);

	virtual bool can_export(String *r_error=NULL) const;

	virtual bool requires_password(bool p_debug) const {return false;}
	virtual String get_binary_extension() const;
	virtual Error export_project(const String& p_path,bool p_debug,bool p_dumb=false);
};


String EditorExportPlatformWii::get_name() const {
	return "Nintendo Wii";
};

EditorExportPlatform::ImageCompression EditorExportPlatformWii::get_image_compression() const{
    return IMAGE_COMPRESSION_NONE;
};

Ref<Texture> EditorExportPlatformWii::get_logo() const {
	Image img(_gc_wii_logo);
	Ref<ImageTexture> logo = memnew(ImageTexture);
	logo->create_from_image(img);
	return logo;
}

Error EditorExportPlatformWii::run(int p_device,bool p_dumb){
    return OK;
}

bool EditorExportPlatformWii::can_export(String *r_error) const {
    return false;
};

String EditorExportPlatformWii::get_binary_extension() const {
    return "dol";
};

Error EditorExportPlatformWii::export_project(const String& p_path,bool p_debug,bool p_dumb){
    return OK;
};

//////////////////////////////////

String EditorExportPlatformGameCube::get_name() const {
	return "Nintendo GameCube";
};

EditorExportPlatform::ImageCompression EditorExportPlatformGameCube::get_image_compression() const{
    return IMAGE_COMPRESSION_NONE;
};

Ref<Texture> EditorExportPlatformGameCube::get_logo() const {
	Image img(_gc_logo);
	Ref<ImageTexture> logo = memnew(ImageTexture);
	logo->create_from_image(img);
	return logo;
}

Error EditorExportPlatformGameCube::run(int p_device,bool p_dumb){
    return OK;
}

bool EditorExportPlatformGameCube::can_export(String *r_error) const {
    return false;
};

String EditorExportPlatformGameCube::get_binary_extension() const {
    return "gcm";
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
    Ref<EditorExportPlatformWii> exporter = Ref<EditorExportPlatformWii>(memnew(EditorExportPlatformWii));
	EditorImportExport::get_singleton()->add_export_platform(exporter);

	Ref<EditorExportPlatformGameCube> exporter_gc = Ref<EditorExportPlatformGameCube>(memnew(EditorExportPlatformGameCube));
	EditorImportExport::get_singleton()->add_export_platform(exporter_gc);
} 
