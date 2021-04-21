#ifndef FC3D_DEPTH_IMAGE_H_INCLUDED
#define FC3D_DEPTH_IMAGE_H_INCLUDED

#include <FC3D/fc3d.h>

#include <OWL/Optimized3d/vector/v3f32.h>

#include <WF3D/Rendering/Shapes/rasterization_attr.h>

typedef struct
{
    int width;
    int height;

    float* depth;

} fc3d_DepthImage;

//Create DepthImage
FC3D_DLL_EXPORT fc3d_DepthImage* fc3d_DepthImage_Create(int width, int height);

//Destroy DepthImage
FC3D_DLL_EXPORT void fc3d_DepthImage_Destroy(fc3d_DepthImage* img);

//Clear DepthImage
FC3D_DLL_EXPORT fc3d_DepthImage* fc3d_DepthImage_Clear(fc3d_DepthImage* img);

//Partial Clear DepthImage
FC3D_DLL_EXPORT fc3d_DepthImage* fc3d_DepthImage_PartialClear(fc3d_DepthImage* img, wf3d_rasterization_rectangle const* clear_rect);

//
static inline size_t fc3d_DepthImage_pixel_index(fc3d_DepthImage const* img, int x, int y)
{
    return (size_t)y * (size_t)img->width + (size_t)x;
}

//
static inline float fc3d_DepthImage_unsafe_Depth(fc3d_DepthImage const* img, int x, int y)
{
    return img->depth[fc3d_DepthImage_pixel_index(img, x, y)];
}

//
static inline void fc3d_DepthImage_unsafe_SetPixel(fc3d_DepthImage const* img, int x, int y, float depth)
{
    img->depth[fc3d_DepthImage_pixel_index(img, x, y)] = depth;
}

//callback_arg is a pointer to the depth_img
FC3D_DLL_EXPORT void OWL_VECTORCALL fc3d_DepthImage_rasterization_callback(wf3d_rasterization_rectangle const* rect, int x, int y, void const* callback_arg, owl_v3f32 v_intersection, owl_v3f32 normal);

#endif // FC3D_DEPTH_IMAGE_H_INCLUDED
