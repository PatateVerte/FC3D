#ifndef FC3D_RENDERING_OBJECT_H_INCLUDED
#define FC3D_RENDERING_OBJECT_H_INCLUDED

#include <OWL/Optimized3d/quaternion/q32.h>
#include <OWL/Optimized3d/vector/v3f32.h>

#include <FC3D/Rendering/wolf_object_interface.h>

typedef struct
{
    owl_v3f32 v_pos;
    owl_q32 q_rot;

    void* wolf_obj;
    fc3d_wolf_object_interface* wolf_obj_interface;

} fc3d_rendering_object;

#endif // FC3D_RENDERING_OBJECT_H_INCLUDED
