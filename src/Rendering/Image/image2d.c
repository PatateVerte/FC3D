#include <FC3D/Rendering/Image/image2d.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

//
//
//
FC3D_DLL_EXPORT fc3d_Image2d* fc3d_Image2d_Create(int width, int height)
{
    fc3d_Image2d* img = malloc(sizeof(*img));
    size_t nb_pixels = (size_t)width * (size_t)height;

    if(img != NULL)
    {
        img->width = width;
        img->height = height;

        img->color = malloc(nb_pixels * sizeof(*(img->color)));

        if(nb_pixels > 0 && (img->color == NULL))
        {
            fc3d_Image2d_Destroy(img);
            img = NULL;
        }
        else
        {
            wf3d_color background_color = (wf3d_color){.rgb = {0.0, 0.0, 0.0, 0.0}};
			fc3d_Image2d_Clear(img, &background_color);
        }
    }

    return img;
}

//
//
//
FC3D_DLL_EXPORT void fc3d_Image2d_Destroy(fc3d_Image2d* img)
{
    if(img != NULL)
    {
        free(img->color);

        free(img);
    }
}

//
//
//
FC3D_DLL_EXPORT wf3d_error fc3d_Image2d_SetPixel(fc3d_Image2d* img, int x, int y, wf3d_color const* color)
{
    if(0 <= x && x < img->width && 0 <= y && y < img->height)
    {
        size_t pixel_index = fc3d_Image2d_pixel_index(img, x, y);
        wf3d_color_uint8_from_color(img->color + pixel_index, color);
        return WF3D_SUCCESS;
    }
    else
    {
        return WF3D_IMAGE_ACCESS_ERROR;
    }
}

//
//
//
FC3D_DLL_EXPORT wf3d_error fc3d_Image2d_GetPixel(fc3d_Image2d* img, int x, int y, wf3d_color* ret_color)
{
    if(0 <= x && x < img->width && 0 <= y && y < img->height)
    {
        size_t pixel_index = fc3d_Image2d_pixel_index(img, x, y);
        wf3d_color_from_color_uint8(ret_color, img->color + pixel_index);
        return WF3D_SUCCESS;
    }
    else
    {
        return WF3D_IMAGE_ACCESS_ERROR;
    }
}

//
//
//
FC3D_DLL_EXPORT fc3d_Image2d* fc3d_Image2d_Clear(fc3d_Image2d* img, wf3d_color const* background_color)
{
    size_t nb_pixels = (size_t)img->width *  (size_t)img->height;

    wf3d_color_uint8 background_color8;
    wf3d_color_uint8_from_color(&background_color8, background_color);

    for(size_t k = 0 ; k < nb_pixels ; k++)
    {
        img->color[k] = background_color8;
    }

    return img;
}

//
//
//
FC3D_DLL_EXPORT int fc3d_Image2d_WriteInBMPFile(fc3d_Image2d const* img, FILE* bmp_file)
{
    #define DIB_HEADER_SIZE 40

    int error = 0;
    rewind(bmp_file);

    uint32_t file_total_size = 14 + DIB_HEADER_SIZE + 4 * (uint32_t)img->width * (uint32_t)img->height;
    uint16_t zero32[2] = {0};
    uint32_t starting_address = 14 + DIB_HEADER_SIZE;

    size_t nb_bloc_written = 0;

    //Write header
    nb_bloc_written += fwrite("BM", sizeof(char), 2, bmp_file);
    nb_bloc_written += fwrite(&file_total_size, sizeof(uint32_t), 1, bmp_file);
    nb_bloc_written += fwrite(zero32, 2, 2, bmp_file);
    nb_bloc_written += fwrite(&starting_address, sizeof(uint32_t), 1, bmp_file);

    if(nb_bloc_written == 6)
    {
        uint32_t DIB_header_size = DIB_HEADER_SIZE;
        uint32_t file_width = (uint32_t)img->width;
        uint32_t file_height = (uint32_t)img->height;
        uint16_t nb_planes = 1;
        uint16_t bpp = 32;
        uint32_t compression = 0;
        uint32_t img_size = 4 * file_width * file_height;
        int32_t h_resol = 5000;
        int32_t v_resol = 5000;
        uint32_t nb_colors = 0;
        uint32_t nb_imp_colors = 0;

        nb_bloc_written = 0;
        nb_bloc_written += fwrite(&DIB_header_size, sizeof(uint32_t), 1, bmp_file);
        nb_bloc_written += fwrite(&file_width, sizeof(file_width), 1, bmp_file);
        nb_bloc_written += fwrite(&file_height, sizeof(file_height), 1, bmp_file);
        nb_bloc_written += fwrite(&nb_planes, sizeof(nb_planes), 1, bmp_file);
        nb_bloc_written += fwrite(&bpp, sizeof(bpp), 1, bmp_file);
        nb_bloc_written += fwrite(&compression, sizeof(compression), 1, bmp_file);
        nb_bloc_written += fwrite(&img_size, sizeof(img_size), 1, bmp_file);
        nb_bloc_written += fwrite(&h_resol, sizeof(h_resol), 1, bmp_file);
        nb_bloc_written += fwrite(&v_resol, sizeof(v_resol), 1, bmp_file);
        nb_bloc_written += fwrite(&nb_colors, sizeof(nb_colors), 1, bmp_file);
        nb_bloc_written += fwrite(&nb_imp_colors, sizeof(nb_imp_colors), 1, bmp_file);

        if(nb_bloc_written == 11)
        {
            //Write data
            size_t nb_pixels = (size_t)img->width * (size_t)img->height;

            for(size_t k = 0 ; k < nb_pixels && error == 0 ; k++)
            {
                uint8_t buff[4];
                for(int c = 0 ; c < 3 ; c++)
                {
                    buff[c] = img->color[k].rgb[2 - c];
                }
                buff[3] = 0;

                nb_bloc_written = fwrite(buff, sizeof(uint8_t), 4, bmp_file);
                if(nb_bloc_written != 4)
                {
                    error = -1;
                }
            }
        }
        else
        {
            error = -1;
        }
    }
    else
    {
        error = -1;
    }

    return error;

    #undef DIB_HEADER_SIZE
}

#define FXAA_EDGE_THRESHOLD (3.0 / 50.0)
#define FXAA_EDGE_THRESHOLD_MIN (3.0 / 200.0)
//#define FXAA_SUBPIX 1.0
#define FXAA_SUBPIX_TRIM 0.25
#define FXAA_SUBPIX_TRIM_SCALE (1.0/(1.0 - FXAA_SUBPIX_TRIM))
#define FXAA_SUBPIX_CAP 0.75

//
FC3D_DLL_EXPORT wf3d_error fc3d_Image2d_FXAA(fc3d_Image2d* img_out, fc3d_Image2d const* img_src)
{
    wf3d_error error = WF3D_SUCCESS;

    int width = img_out->width;
    int height = img_out->height;

    float mix_coeff9[9];
    for(unsigned int k = 0 ; k < 9 ; k++)
    {
        mix_coeff9[k] = 1.0f / 9.0f;
    }

    if(width == img_src->width && height == img_src->height)
    {
        for(int y = 1 ; y < height - 1 ; y++)
        {
            for(int x = 1 ; x < width - 1 ; x++)
            {
                size_t pixel_index = fc3d_Image2d_pixel_index(img_src, x, y);
                float lumaM = wf3d_color_uint8_luminance(img_src->color + pixel_index);
                float lumaN = wf3d_color_uint8_luminance(img_src->color + fc3d_Image2d_pixel_index(img_src, x, y + 1));
                float lumaS = wf3d_color_uint8_luminance(img_src->color + fc3d_Image2d_pixel_index(img_src, x, y - 1));
                float lumaE = wf3d_color_uint8_luminance(img_src->color + fc3d_Image2d_pixel_index(img_src, x + 1, y));
                float lumaW = wf3d_color_uint8_luminance(img_src->color + fc3d_Image2d_pixel_index(img_src, x - 1, y));
                float luma_min = fminf(lumaM, fminf(fminf(lumaN, lumaW), fminf(lumaS, lumaE)));
                float luma_max = fmaxf(lumaM, fmaxf(fmaxf(lumaN, lumaW), fmaxf(lumaS, lumaE)));
                float range  = luma_max - luma_min;

                if(range > fmaxf((float)FXAA_EDGE_THRESHOLD_MIN, luma_max * (float)FXAA_EDGE_THRESHOLD))
                {
                    float lumaL = 0.25f * (lumaN + lumaS + lumaE + lumaW);
                    float rangeL = fabsf(lumaL - lumaM);
                    float blendL = fmaxf(0.0f, (rangeL / range) -(float) FXAA_SUBPIX_TRIM) * (float)FXAA_SUBPIX_TRIM_SCALE;
                    blendL = fminf(FXAA_SUBPIX_CAP, blendL);

                    wf3d_color pixel_list[9];
                    for(int j = - 1 ; j <= 1 ; j++)
                    {
                        for(int i = - 1 ; i <= 1 ; i++)
                        {
                            wf3d_color_from_color_uint8(pixel_list + 3 * (j + 1) + (i + 1), img_src->color + fc3d_Image2d_pixel_index(img_src, x + i, y + j));
                        }
                    }

                    wf3d_color colorM, colorL;
                    wf3d_color_from_color_uint8(&colorM, img_src->color + pixel_index);
                    colorL = wf3d_color_mix(pixel_list, mix_coeff9, 9);

                    float final_mix_coeff[2];
                    final_mix_coeff[0] = 1.0f - blendL;
                    final_mix_coeff[1] = blendL;

                    wf3d_color final_mix_color[2];
                    final_mix_color[0] = colorM;
                    final_mix_color[1] = colorL;

                    wf3d_color final_color = wf3d_color_mix(final_mix_color, final_mix_coeff, 2);

                    wf3d_color_uint8_from_color(img_out->color + pixel_index, &final_color);
                }
                else
                {
                    img_out->color[pixel_index] = img_src->color[pixel_index];
                }
            }
        }
    }
    else
    {
        error = WF3D_IMAGE_ACCESS_ERROR;
    }

    return error;
}
