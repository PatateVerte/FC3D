#ifndef WF3D_IMAGE2D_H_INCLUDED
#define WF3D_IMAGE2D_H_INCLUDED

#include <WF3D/error.h>
#include <WF3D/Rendering/Design/color.h>

#include <stdio.h>
#include <stdbool.h>

typedef struct
{
    int width;
    int height;

    wf3d_color_uint8* color;

} fc3d_Image2d;

//
fc3d_Image2d* fc3d_Image2d_Create(int width, int height);

//
void fc3d_Image2d_Destroy(fc3d_Image2d* img);

//
fc3d_Image2d* fc3d_Image2d_Clear(fc3d_Image2d* img, wf3d_color const* background_color);

static inline size_t fc3d_Image2d_pixel_index(fc3d_Image2d const* img, int x, int y)
{
    return (size_t)y * (size_t)img->width + (size_t)x;
}

//
static inline void fc3d_Image2d_unsafe_SetPixel(fc3d_Image2d* img, int x, int y, wf3d_color const* color)
{
	size_t pixel_index = fc3d_Image2d_pixel_index(img, x, y);
	wf3d_color_uint8_from_color(img->color + pixel_index, color);
}

//
wf3d_error fc3d_Image2d_SetPixel(fc3d_Image2d* img, int x, int y, wf3d_color const* color);

//
wf3d_error fc3d_Image2d_GetPixel(fc3d_Image2d* img, int x, int y, wf3d_color* ret_color);

//Write Image2d in a bitmap file
int fc3d_Image2d_WriteInBMPFile(fc3d_Image2d const* img, FILE* bmp_file);

//Perform FXAA on an image
wf3d_error fc3d_Image2d_FXAA(fc3d_Image2d* img_out, fc3d_Image2d const* img_src);

typedef struct
{
    fc3d_Image2d* img2d;
    int x_min;
    int x_max;
    int y_min;
    int y_max;

} fc3d_image2d_rectangle;

#endif // WF3D_IMAGE2D_H_INCLUDED
