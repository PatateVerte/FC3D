#ifndef FC3D_IMAGE3D_H_INCLUDED
#define FC3D_IMAGE3D_H_INCLUDED

#include <stdio.h>
#include <stdbool.h>

#include <FC3D/fc3d.h>

#include <OWL/Optimized3d/vector/v3f32.h>

#include <WF3D/error.h>
#include <WF3D/Rendering/Shapes/rasterization_attr.h>
#include <WF3D/Rendering/Design/surface.h>
#include <WF3D/Rendering/Design/color.h>

typedef struct
{
    int width;
    int height;

    float* depth_buffer;
    wf3d_surface const** surface;
    wf3d_color* diffusion_color;
    owl_v3f32* normal;

} fc3d_Image3d;

//
FC3D_DLL_EXPORT fc3d_Image3d* fc3d_Image3d_Create(int width, int height);

//
FC3D_DLL_EXPORT void fc3d_Image3d_Destroy(fc3d_Image3d* img);

//
FC3D_DLL_EXPORT fc3d_Image3d* fc3d_Image3d_Clear(fc3d_Image3d* img);

static inline size_t fc3d_Image3d_pixel_index(fc3d_Image3d const* img, int x, int y)
{
    return (size_t)y * (size_t)img->width + (size_t)x;
}

//
static inline float fc3d_Image3d_unsafe_Depth(fc3d_Image3d const* img, int x, int y)
{
    size_t pixel_index = fc3d_Image3d_pixel_index(img, x, y);
    return img->depth_buffer[pixel_index];
}

//
static inline wf3d_surface const* fc3d_Image3d_unsafe_Surface(fc3d_Image3d const* img, int x, int y)
{
    size_t pixel_index = fc3d_Image3d_pixel_index(img, x, y);
    return img->surface[pixel_index];
}

//
static inline wf3d_color fc3d_Image3d_unsafe_DiffColor(fc3d_Image3d const* img, int x, int y)
{
    size_t pixel_index = fc3d_Image3d_pixel_index(img, x, y);
    return img->diffusion_color[pixel_index];
}

//
static inline owl_v3f32 fc3d_Image3d_unsafe_Normal(fc3d_Image3d const* img, int x, int y)
{
    size_t pixel_index = fc3d_Image3d_pixel_index(img, x, y);
    return img->normal[pixel_index];
}

//
static inline void fc3d_Image3d_unsafe_SetPixel(fc3d_Image3d* img, int x, int y, wf3d_surface const* surface, wf3d_color const* diffusion_color, float depth, owl_v3f32 normal)
{
    size_t pixel_index = fc3d_Image3d_pixel_index(img, x, y);

    img->surface[pixel_index] = surface;
    img->diffusion_color[pixel_index] = *diffusion_color;
    img->depth_buffer[pixel_index] = depth;
    img->normal[pixel_index] = normal;
}

#endif // FC3D_IMAGE3D_H_INCLUDED
