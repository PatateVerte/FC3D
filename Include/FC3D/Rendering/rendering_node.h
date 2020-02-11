#ifndef FC3D_RENDERING_NODE_H_INCLUDED
#define FC3D_RENDERING_NODE_H_INCLUDED

#include <WF3D/error.h>

#include <FC3D/Rendering/object_interface.h>

typedef struct
{
    void const* obj;
    fc3d_object_interface const* obj_interface;

} fc3d_RenderingNode;

#endif // FC3D_RENDERING_NODE_H_INCLUDED
