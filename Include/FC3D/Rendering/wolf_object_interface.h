#ifndef FC3D_OBJECT_INTERFACE_H_INCLUDED
#define FC3D_OBJECT_INTERFACE_H_INCLUDED

#include <WF3D/error.h>
#include <WF3D/Geometry/vect3d.h>
#include <WF3D/Geometry/quat.h>
#include <WF3D/Rendering/camera3d.h>
#include <WF3D/Rendering/lightsource.h>
#include <WF3D/Rendering/Design/image_gen_interface.h>

typedef struct
{
    //Destroy() parameter
    void (*Destroy)(void);

    //wf3d_error wf3d_PolygonMesh_Rasterization(wf3d_PolygonMesh const* obj, wf3d_img_gen_interface* img_out, float* depth_buffer, wf3d_vect3d v_pos, wf3d_quat q_rot, wf3d_camera3d const* cam)
    wf3d_error (*Rasterization)(void const*, wf3d_img_gen_interface*, float*, wf3d_vect3d, wf3d_quat, wf3d_camera3d const*);

    //float Radius(void* obj)
    float (*Radius)(void*);

    //float wf3d_PolygonMesh_InfRadius(void* obj, wf3d_vect3d v_pos)
    float (*InfRadius)(void*, wf3d_vect3d);

    //float wf3d_PolygonMesh_InfRadiusWithRot(void* obj, wf3d_vect3d v_pos, wf3d_quat q_rot)
    float (*InfRadiusWithRot)(void*, wf3d_vect3d, wf3d_quat);

} fc3d_wolf_object_interface;

typedef struct
{
    void* obj;
    fc3d_wolf_object_interface const* obj_interface;

} fc3d_wolf_object;

#endif // FC3D_OBJECT_INTERFACE_H_INCLUDED
