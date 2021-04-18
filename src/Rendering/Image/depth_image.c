#include <FC3D/Rendering/Image/depth_image.h>

#include <math.h>

//Create DepthImage
//
//
fc3d_DepthImage* fc3d_DepthImage_Create(int width, int height)
{
    fc3d_DepthImage* img = malloc(sizeof(*img));

    if(img != NULL)
    {
        img->width = width;
        img->height = height;

        size_t size = (size_t)width * (size_t)height;
        img->depth = malloc(size * sizeof(*img->depth));

        if(size == 0 || img->depth != NULL)
        {
            fc3d_DepthImage_Clear(img);
        }
        else
        {
            fc3d_DepthImage_Destroy(img);
            img = NULL;
        }
    }

    return img;
}

//Destroy DepthImage
//
//
void fc3d_DepthImage_Destroy(fc3d_DepthImage* img)
{
    if(img != NULL)
    {
        free(img->depth);

        free(img);
    }
}

//Clear DepthImage
//
//
fc3d_DepthImage* fc3d_DepthImage_Clear(fc3d_DepthImage* img)
{
    size_t size = (size_t)img->width * (size_t)img->height;
    for(size_t i = 0 ; i < size ; i++)
    {
        img->depth[i] = INFINITY;
    }

    return img;
}

//
//
//
fc3d_DepthImage* fc3d_DepthImage_PartialClear(fc3d_DepthImage* img, wf3d_rasterization_rectangle const* clear_rect)
{
    for(int y = clear_rect->y_min ; y < clear_rect->y_max ; y++)
    {
        for(int x = clear_rect->x_min ; x < clear_rect->x_max ; x++)
        {
            img->depth[fc3d_DepthImage_pixel_index(img, x, y)] = INFINITY;
        }
    }

    return img;
}

//
//
//
void OWL_VECTORCALL fc3d_DepthImage_rasterization_callback(wf3d_rasterization_rectangle const* rect, int x, int y, void const* callback_arg, owl_v3f32 v_intersection, owl_v3f32 normal)
{
    fc3d_DepthImage const* depth_img = callback_arg;

    (void)rect;
    (void)normal;

    float depth = -owl_v3f32_unsafe_get_component(v_intersection, 2);

    if(depth < fc3d_DepthImage_unsafe_Depth(depth_img, x, y))
    {
        fc3d_DepthImage_unsafe_SetPixel(depth_img, x, y, depth);
    }
}

