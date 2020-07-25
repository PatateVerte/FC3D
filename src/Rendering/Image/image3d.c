#include <FC3D/Rendering/Image/image3d.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

//
//
//
fc3d_Image3d* fc3d_Image3d_Create(int width, int height)
{
    fc3d_Image3d* img = malloc(sizeof(*img));
    size_t nb_pixels = (size_t)width * (size_t)height;

    if(img != NULL)
    {
        img->width = width;
        img->height = height;

        img->surface = malloc(nb_pixels * sizeof(*img->surface));
        img->depth_buffer = malloc(nb_pixels * sizeof(*img->depth_buffer));
        img->M = _aligned_malloc(nb_pixels * sizeof(*img->M), 16);
        img->normal = _aligned_malloc(nb_pixels * sizeof(*img->normal), 16);

        if(nb_pixels > 0 && (img->surface == NULL || img->depth_buffer == NULL || img->M == NULL || img->normal == NULL))
        {
            fc3d_Image3d_Destroy(img);
            img = NULL;
        }
        else
        {
            fc3d_Image3d_Clear(img);
        }
    }

    return img;
}

//
//
//
void fc3d_Image3d_Destroy(fc3d_Image3d* img)
{
    if(img != NULL)
    {
        free(img->surface);
        free(img->depth_buffer);
        _aligned_free(img->M);
        _aligned_free(img->normal);

        free(img);
    }
}

//
//
//
fc3d_Image3d* fc3d_Image3d_Clear(fc3d_Image3d* img)
{
    size_t nb_pixels = (size_t)img->width *  (size_t)img->height;

    for(size_t k = 0 ; k < nb_pixels ; k++)
    {
        img->depth_buffer[k] = INFINITY;
    }

    return img;
}

