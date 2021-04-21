#include <FC3D/Rendering/Octree/octree.h>

#include <pthread.h>

//Create an RenderingOctree
//
//
FC3D_DLL_EXPORT fc3d_RenderingOctree* fc3d_RenderingOctree_Create(float half_size, int max_depth, size_t children_data_pool_block_len, size_t auxiliary_data_pool_block_len)
{
    fc3d_RenderingOctree* octree = malloc(sizeof(*octree));

    if(octree != NULL)
    {
        octree->half_size = half_size;

        octree->max_depth = max_depth;

        octree->octree_children_data_pool = fc3d_DataPool_Create(children_data_pool_block_len, sizeof(fc3d_rendering_octree_node));
        octree->octree_auxiliary_data_pool = fc3d_DataPool_Create(auxiliary_data_pool_block_len, sizeof(fc3d_rendering_octree_node));

        if(octree->octree_children_data_pool == NULL || octree->octree_children_data_pool == NULL)
        {
            fc3d_RenderingOctree_Destroy(octree);
            octree = NULL;
        }
        else
        {
            fc3d_RenderingOctree_Rewind(octree);
        }
    }

    return octree;
}

//Destroy an RenderingOctree
//
//
FC3D_DLL_EXPORT void fc3d_RenderingOctree_Destroy(fc3d_RenderingOctree* octree)
{
    if(octree != NULL)
    {
        fc3d_DataPool_Destroy(octree->octree_children_data_pool);
        fc3d_DataPool_Destroy(octree->octree_auxiliary_data_pool);

        free(octree);
    }
}

//Rewind an octree
//Cannot fail
//
FC3D_DLL_EXPORT fc3d_rendering_octree_node* fc3d_RenderingOctree_Rewind(fc3d_RenderingOctree* octree)
{
    octree->node_0 = fc3d_DataPool_Rewind(octree->octree_children_data_pool);
    fc3d_DataPool_Rewind(octree->octree_auxiliary_data_pool);

    return fc3d_rendering_octree_node_Set(octree->node_0, owl_v3f32_zero(), octree->half_size);
}

//Add an object into the octree
//
//
FC3D_DLL_EXPORT fc3d_rendering_octree_node* fc3d_RenderingOctree_AddObject(fc3d_RenderingOctree* octree, fc3d_rendering_object* obj, bool spatial_extension)
{
    if(octree == NULL)
    {
        return NULL;
    }

    return fc3d_rendering_octree_node_AddObject(octree->node_0, obj, octree->max_depth, spatial_extension, octree->octree_children_data_pool, octree->octree_auxiliary_data_pool);
}


/*
    R
    A
    Y

    T
    R
    A
    C
    I
    N
    G
*/

//||ray_dir|| = 1
//
//
FC3D_DLL_EXPORT wf3d_color OWL_VECTORCALL fc3d_RenderingOctree_PointColor(fc3d_RenderingOctree const* octree, fc3d_octree_point_color_attr const* attr, owl_v3f32 cam_ray_dir, owl_v3f32 v_pos, owl_v3f32 normal, wf3d_surface const* surface, wf3d_color diffusion_color, int max_nb_reflections)
{
    wf3d_color final_color = wf3d_color_black();

    for(unsigned int k = 0 ; k < attr->nb_lightsources ; k++)
    {
        fc3d_lightsource const* lightsource = attr->lightsource_list[k];

        owl_q32 lightsource_q_rot_conj = owl_q32_conj(lightsource->q_rot);
        owl_v3f32 rel_v_pos = owl_q32_transform_v3f32(
                                                        lightsource_q_rot_conj,
                                                        owl_v3f32_sub(v_pos, lightsource->v_pos)
                                                      );
        owl_v3f32 rel_normal = owl_q32_transform_v3f32(lightsource_q_rot_conj, normal);
        owl_v3f32 rel_cam_ray_dir = owl_q32_transform_v3f32(lightsource_q_rot_conj, cam_ray_dir);

        wf3d_color local_color = lightsource->lightsource_interface->EnlightSurfacePoint(lightsource->lightsource_obj, surface, diffusion_color, rel_v_pos, rel_normal, rel_cam_ray_dir);
        final_color = wf3d_color_add(final_color, local_color);
    }

    if(max_nb_reflections > 0)
    {
        if(!wf3d_color_isblack(surface->reflection_color))
        {
            owl_v3f32 reflection_ray_dir = owl_v3f32_add_scalar_mul(
                                                                        cam_ray_dir,
                                                                        normal,
                                                                        -2.0f * owl_v3f32_dot(cam_ray_dir, normal)
                                                                    );
            reflection_ray_dir = owl_v3f32_normalize(reflection_ray_dir);

            float t;
            owl_v3f32 reflected_point_normal;
            wf3d_surface const* reflected_point_surface;
            wf3d_color reflected_point_diffusion_color;
            if(fc3d_rendering_octree_node_NearestIntersectionWithRay(octree->node_0, attr->octree_v_pos, attr->octree_q_rot, v_pos, reflection_ray_dir, attr->near_clipping_distance, INFINITY, &t, &reflected_point_normal, &reflected_point_surface, &reflected_point_diffusion_color))
            {
                owl_v3f32 reflected_point_v_pos = owl_v3f32_add_scalar_mul(v_pos, reflection_ray_dir, t);
                wf3d_color reflection_color = fc3d_RenderingOctree_PointColor(
                                                                                octree, attr,
                                                                                reflection_ray_dir, reflected_point_v_pos, reflected_point_normal, reflected_point_surface, reflected_point_diffusion_color,
                                                                                max_nb_reflections - 1
                                                                             );

                final_color = wf3d_color_add(final_color, wf3d_color_mul(reflection_color, surface->reflection_color));
            }
        }

        if(!wf3d_color_isblack(surface->refraction_color))
        {
            float n_int_sign = (owl_v3f32_dot(cam_ray_dir, normal) < 0.0f) ? -1.0f : 1.0f;
            float rel_refractive_index = (n_int_sign < 0.0f) ? surface->rel_refractive_index : 1.0f / surface->rel_refractive_index;
            owl_v3f32 n_int = owl_v3f32_scalar_mul(normal, n_int_sign);

            owl_v3f32 rot_refraction_vec = owl_v3f32_cross(n_int, cam_ray_dir);
            float sin_inc_angle = owl_v3f32_norm(rot_refraction_vec);
            float sin_ref_angle = rel_refractive_index * sin_inc_angle;

            if(0.0f <= fabsf(sin_ref_angle) && fabsf(sin_ref_angle) <= 1.0f)
            {
                float inc_angle = asinf(sin_inc_angle);
                float ref_angle = asinf(sin_ref_angle);
                owl_q32 q_ref_rot = owl_q32_from_rotation(
                                                            (sin_inc_angle > 0.0f) ? owl_v3f32_scalar_div(rot_refraction_vec, sin_inc_angle) : owl_v3f32_zero(),
                                                            ref_angle - inc_angle
                                                          );
                owl_v3f32 refraction_ray_dir = owl_q32_transform_v3f32(q_ref_rot, cam_ray_dir);

                float t;
                owl_v3f32 refracted_point_normal;
                wf3d_surface const* refracted_point_surface;
                wf3d_color refracted_point_diffusion_color;
                if(fc3d_rendering_octree_node_NearestIntersectionWithRay(octree->node_0, attr->octree_v_pos, attr->octree_q_rot, v_pos, refraction_ray_dir, attr->near_clipping_distance, INFINITY, &t, &refracted_point_normal, &refracted_point_surface, &refracted_point_diffusion_color))
                {
                    owl_v3f32 refracted_point_v_pos = owl_v3f32_add_scalar_mul(v_pos, refraction_ray_dir, t);
                    wf3d_color refraction_color = fc3d_RenderingOctree_PointColor(
                                                                                    octree, attr,
                                                                                    refraction_ray_dir, refracted_point_v_pos, refracted_point_normal, refracted_point_surface, refracted_point_diffusion_color,
                                                                                    max_nb_reflections - 1
                                                                                 );
                    final_color = wf3d_color_add(final_color, wf3d_color_mul(refraction_color, surface->refraction_color));
                }
            }
        }
    }

    return final_color;
}

typedef struct
{
    fc3d_RenderingOctree const* octree;

    fc3d_lightsource const* const* lightsource_list;
    unsigned int nb_lightsources;
    owl_v3f32 octree_v_pos;
    owl_q32 octree_q_rot;
    wf3d_camera3d const* cam;

    fc3d_image2d_rectangle* img_out;

    int x_block_len;
    int y_block_len;

    pthread_mutex_t shared_mutex;
    int next_x;
    int next_y;

} wf3d_octree_ray_tracing_thread_shared_arg;

typedef struct
{
    wf3d_octree_ray_tracing_thread_shared_arg* shared_arg;
    fc3d_Image3d* img3d_buffer;
    wf3d_error error_ret;

} wf3d_octree_ray_tracing_thread_arg;

//
static void* wf3d_octree_multithreaded_ray_tracing(void* ptr)
{
    wf3d_error error = WF3D_SUCCESS;

    wf3d_octree_ray_tracing_thread_arg* arg = ptr;
    wf3d_octree_ray_tracing_thread_shared_arg* shared_arg = arg->shared_arg;
    fc3d_image2d_rectangle* img_out = shared_arg->img_out;
    fc3d_Image3d* img3d_buff = arg->img3d_buffer;

    wf3d_rasterization_rectangle rect;
    rect.width = img_out->img2d->width;
    rect.height = img_out->img2d->height;

    bool rasterization_over = false;

    while(error == WF3D_SUCCESS && !rasterization_over)
    {
        int thread_error = pthread_mutex_lock(&shared_arg->shared_mutex);

        if(thread_error == 0)
        {
            if(shared_arg->next_x < img_out->x_max && shared_arg->next_y < img_out->y_max)
            {
                rect.y_min = shared_arg->next_y;
                rect.x_min = shared_arg->next_x;

                rect.y_max = shared_arg->next_y + shared_arg->y_block_len;
                if(rect.y_max > img_out->y_max)
                {
                    rect.y_max = img_out->y_max;
                }

                rect.x_max = shared_arg->next_x + shared_arg->x_block_len;
                if(rect.x_max >= img_out->x_max)
                {
                    rect.x_max = img_out->x_max;
                    shared_arg->next_x = img_out->x_min;
                    shared_arg->next_y += shared_arg->y_block_len;
                }
                else
                {
                    shared_arg->next_x += shared_arg->x_block_len;
                }
            }
            else
            {
                rasterization_over = true;
            }
        }
        else
        {
            error = WF3D_THREAD_ERROR;
        }

        thread_error |= pthread_mutex_unlock(&shared_arg->shared_mutex);
        if(thread_error != 0)
        {
            error = WF3D_THREAD_ERROR;
        }

        if(error == WF3D_SUCCESS && !rasterization_over)
        {
            fc3d_Image3d_Clear(img3d_buff);
            fc3d_rendering_octree_node_Rasterization(shared_arg->octree->node_0, img3d_buff, &rect, shared_arg->octree_v_pos, shared_arg->octree_q_rot, shared_arg->cam);

            fc3d_octree_point_color_attr attr;
            attr.octree_v_pos = shared_arg->octree_v_pos;
            attr.octree_q_rot = shared_arg->octree_q_rot;
            attr.lightsource_list = shared_arg->lightsource_list;
            attr.nb_lightsources = shared_arg->nb_lightsources;
            attr.near_clipping_distance = shared_arg->cam->near_clipping_distance;

            float half_width = 0.5f * (float)img_out->img2d->width;
            float half_height = 0.5f * (float)img_out->img2d->height;
            float x_to_xf_scale = shared_arg->cam->tan_h_half_opening_angle / half_width;
            float y_to_yf_scale = shared_arg->cam->tan_v_half_opening_angle / half_height;

            for(int y = rect.y_min ; y < rect.y_max ; y++)
            {
                int y3d = y - rect.y_min;
                for(int x = rect.x_min ; x < rect.x_max ; x++)
                {
                    int x3d = x - rect.x_min;
                    float depth = fc3d_Image3d_unsafe_Depth(img3d_buff, x3d, y3d);

                    //Ray tracing for every pixel that has hit something
                    if(isfinite(depth) != 0)
                    {
                        float xf = ((float)x - half_width + 0.5f) * x_to_xf_scale;
                        float yf = ((float)y - half_height + 0.5f) * y_to_yf_scale;
                        owl_v3f32 cam_ray = owl_v3f32_set(xf, yf, -1.0f);
                        owl_v3f32 obj_v_pos = owl_v3f32_scalar_mul(cam_ray, depth);

                        owl_v3f32 ray_dir = owl_v3f32_normalize(cam_ray);
                        owl_v3f32 obj_normal = fc3d_Image3d_unsafe_Normal(img3d_buff, x3d, y3d);
                        wf3d_color pixel_color = fc3d_RenderingOctree_PointColor(
                                                                                    shared_arg->octree, &attr,
                                                                                    ray_dir, obj_v_pos, obj_normal, fc3d_Image3d_unsafe_Surface(img3d_buff, x3d, y3d), fc3d_Image3d_unsafe_DiffColor(img3d_buff, x3d, y3d),
                                                                                    shared_arg->cam->max_nb_reflections
                                                                                );
                        fc3d_Image2d_unsafe_SetPixel(img_out->img2d, x, y, &pixel_color);
                    }
                }
            }
        }
    }

    arg->error_ret = error;

    return NULL;
}

//Multithread ray tracing
//img3d_thread_buffer_list must contain nb_threads elements of the same size (width and height)
//
FC3D_DLL_EXPORT wf3d_error fc3d_RenderingOctree_MultiThreadRayTracing(fc3d_RenderingOctree* octree, fc3d_image2d_rectangle* img_out, fc3d_Image3d** img3d_thread_buffer_list, unsigned short nb_threads, fc3d_lightsource const* const* lightsource_list, unsigned int nb_lightsources, owl_v3f32 octree_v_pos, owl_q32 octree_q_rot, wf3d_camera3d const* cam)
{
    wf3d_error error = WF3D_SUCCESS;

    if(nb_threads > 0)
    {
        pthread_t* thread_list = malloc((size_t)nb_threads * sizeof(*thread_list));

        wf3d_octree_ray_tracing_thread_shared_arg shared_arg;
        wf3d_octree_ray_tracing_thread_arg* thread_arg_list = malloc((size_t)nb_threads * sizeof(*thread_arg_list));

        if(thread_list != NULL && thread_arg_list != NULL)
        {
            shared_arg.octree = octree;
            shared_arg.lightsource_list = lightsource_list;
            shared_arg.nb_lightsources = nb_lightsources;
            shared_arg.octree_v_pos = octree_v_pos;
            shared_arg.octree_q_rot = octree_q_rot;
            shared_arg.cam = cam;
            shared_arg.img_out = img_out;

            shared_arg.x_block_len = img3d_thread_buffer_list[0]->width;
            shared_arg.y_block_len = img3d_thread_buffer_list[0]->height;

            shared_arg.shared_mutex = PTHREAD_MUTEX_INITIALIZER;
            shared_arg.next_x = img_out->x_min;
            shared_arg.next_y = img_out->y_min;

            unsigned short nb_thread_launched = 0;
            for(unsigned short k = 0 ; k < nb_threads && error == WF3D_SUCCESS ; k++)
            {
                thread_arg_list[k].shared_arg = &shared_arg;
                thread_arg_list[k].error_ret = WF3D_SUCCESS;
                thread_arg_list[k].img3d_buffer = img3d_thread_buffer_list[k];

                int thread_error = pthread_create(thread_list + k, NULL, wf3d_octree_multithreaded_ray_tracing, thread_arg_list + k);

                if(thread_error == 0)
                {
                    nb_thread_launched++;
                }
                else
                {
                    error = WF3D_THREAD_ERROR;
                }
            }

            for(unsigned int k = 0 ; k < nb_thread_launched ; k++)
            {
                int thread_error = pthread_join(thread_list[k], NULL);
                if(thread_error != 0)
                {
                    error = WF3D_THREAD_ERROR;
                }
            }

            if(error == WF3D_SUCCESS)
            {
                for(unsigned short k = 0 ; k < nb_threads && error == WF3D_SUCCESS ; k++)
                {
                    error = thread_arg_list[k].error_ret;
                }
            }
        }
        else
        {
            error = WF3D_MEMORY_ERROR;
        }

        free(thread_list);
        free(thread_arg_list);
    }
    else
    {
        error = WF3D_THREAD_ERROR;
    }

    return error;
}

//
//
//
FC3D_DLL_EXPORT void fc3d_RenderingOctree_DepthRasterization(void const* obj, fc3d_DepthImage* depth_img, wf3d_rasterization_rectangle const* rect, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam)
{
    fc3d_RenderingOctree const* octree = obj;
    fc3d_rendering_octree_node_DepthRasterization(octree->node_0, depth_img, rect, v_pos, q_rot, cam);
}
