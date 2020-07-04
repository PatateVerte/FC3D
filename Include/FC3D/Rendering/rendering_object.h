#ifndef FC3D_RENDERING_OBJECT_H_INCLUDED
#define FC3D_RENDERING_OBJECT_H_INCLUDED

#include <OWL/q32.h>
#include <OWL/v3f32.h>

#include <FC3D/Rendering/wolf_object_interface.h>

typedef struct
{
    void* wolf_obj;
    fc3d_wolf_object_interface* wolf_obj_interface;

    owl_v3f32 v_pos;
    owl_q32 q_rot;

} fc3d_rendering_object;

#endif // FC3D_RENDERING_OBJECT_H_INCLUDED
