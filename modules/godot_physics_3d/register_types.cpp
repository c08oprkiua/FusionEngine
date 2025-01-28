#include "register_types.h"

#include "physics_server_sw.h"

void register_godot_physics_3d_types(){
    PhysicsServer3D::set_singleton(memnew(PhysicsServer3DSW));
}

void unregister_godot_physics_3d_types(){
    //Server is cleaned up by unregister_server_types
}
