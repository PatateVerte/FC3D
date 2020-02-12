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

    //wf3d_error Rasterization(wf3d_PolygonMesh const* obj, wf3d_img_gen_interface* img_out, float* depth_buffer, wf3d_camera3d const* cam, wf3d_lightsource const* ls_list, int nb_ls)
    wf3d_error (*Rasterization)(void const*, wf3d_img_gen_interface*, float*, wf3d_camera3d const*, wf3d_lightsource const*, int);

    //void* Move(void* obj, wf3d_vect3d v)
    void* (*Move)(void*, wf3d_vect3d);

    //void* Transform(void* obj, wf3d_quat q_rot, wf3d_vect3d v)
    void* (*Transform)(void*, wf3d_quat, wf3d_vect3d);

    //wf3d_vect3d Center(void* obj)
    wf3d_vect3d (*Center)(void* obj);

    //float Radius(void* obj)
    float (*Radius)(void*);

} fc3d_wolf_object_interface;

typedef struct
{
    void* obj;
    fc3d_wolf_object_interface const* obj_interface;

} fc3d_wolf_object;

#endif // FC3D_OBJECT_INTERFACE_H_INCLUDED
