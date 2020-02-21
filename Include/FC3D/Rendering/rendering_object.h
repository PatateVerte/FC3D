#ifndef FC3D_RENDERING_OBJECT_H_INCLUDED
#define FC3D_RENDERING_OBJECT_H_INCLUDED

#include <WF3D/Geometry/quat.h>
#include <WF3D/Geometry/vect3d.h>

#include <FC3D/Rendering/wolf_object_interface.h>

typedef struct
{
    void* wolf_obj;
    fc3d_wolf_object_interface* wolf_obj_interface;

    wf3d_vect3d v_pos;
    wf3d_quat q_rot;

} fc3d_rendering_object;

#endif // FC3D_RENDERING_OBJECT_H_INCLUDED
