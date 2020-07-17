#include <FC3D/Rendering/Octree/octree.h>

#include <pthread.h>

//Create an RenderingOctree
//
//
fc3d_RenderingOctree* fc3d_RenderingOctree_Create(owl_v3f32 center, float half_size, int max_depth, size_t children_data_pool_block_len, size_t auxiliary_data_pool_block_len)
{
    fc3d_RenderingOctree* octree = malloc(sizeof(*octree));

    if(octree != NULL)
    {
        octree->center = center;
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
void fc3d_RenderingOctree_Destroy(fc3d_RenderingOctree* octree)
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
fc3d_rendering_octree_node* fc3d_RenderingOctree_Rewind(fc3d_RenderingOctree* octree)
{
    octree->node_0 = fc3d_DataPool_Rewind(octree->octree_children_data_pool);
    fc3d_DataPool_Rewind(octree->octree_auxiliary_data_pool);

    return fc3d_rendering_octree_node_Set(octree->node_0, octree->center, octree->half_size);
}

//Add an object into the octree
//
//
fc3d_rendering_octree_node* fc3d_RenderingOctree_AddObject(fc3d_RenderingOctree* octree, fc3d_rendering_object* obj, bool spatial_extension)
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
    S
    T
    E
    R
    I
    Z
    A
    T
    I
    O
    N
*/

//Rasterization
//
//
wf3d_error fc3d_RenderingOctree_Rasterization(fc3d_RenderingOctree* octree, wf3d_Image2d* img_out, wf3d_lightsource const* lightsource_list, unsigned int nb_lightsources, owl_v3f32 cam_v_pos, owl_q32 cam_q_rot, wf3d_camera3d const* cam)
{
    wf3d_error error = WF3D_SUCCESS;

    wf3d_image2d_rectangle img_rect;
    img_rect.img2d = img_out;
    img_rect.x_min = 0;
    img_rect.x_max = img_out->width;
    img_rect.y_min = 0;
    img_rect.y_max = img_out->height;

    owl_q32 cam_q_rot_conj = owl_q32_conj(cam_q_rot);
    owl_v3f32 opp_cam_v_pos = owl_v3f32_sub(owl_v3f32_zero(), cam_v_pos);

    wf3d_lightsource* cam_lightsource_list = malloc(nb_lightsources * sizeof(*cam_lightsource_list));
    if(nb_lightsources == 0 || cam_lightsource_list != NULL)
    {
        for(unsigned int k = 0 ; k < nb_lightsources ; k++)
        {
            wf3d_lightsource_transform(cam_lightsource_list + k, lightsource_list + k, opp_cam_v_pos, cam_q_rot_conj);
        }

        error = fc3d_rendering_octree_node_Rasterization(octree->node_0, &img_rect, cam_lightsource_list, nb_lightsources, cam_v_pos, cam_q_rot, cam);
    }
    else
    {
        error = WF3D_MEMORY_ERROR;
    }

    free(cam_lightsource_list);

    return error;
}

//Multithread rasterization data structures
typedef struct
{
    fc3d_RenderingOctree const* octree;

    wf3d_lightsource const* cam_lightsource_list;
    unsigned int nb_lightsources;
    owl_v3f32 cam_v_pos;
    owl_q32 cam_q_rot;
    wf3d_camera3d const* cam;

    wf3d_Image2d* img_out;

    int x_block_len;
    int y_block_len;

    pthread_mutex_t shared_mutex;
    int next_x;
    int next_y;

} wf3d_octree_rasterization_thread_shared_arg;

typedef struct
{
    wf3d_octree_rasterization_thread_shared_arg* shared_arg;
    wf3d_error error_ret;

} wf3d_octree_rasterization_thread_arg;

//Multithread rasterization function
static void* wf3d_octree_multithread_rasterization(void* ptr)
{
    wf3d_error error = WF3D_SUCCESS;

    wf3d_octree_rasterization_thread_arg* arg = ptr;
    wf3d_octree_rasterization_thread_shared_arg* shared_arg = arg->shared_arg;

    wf3d_image2d_rectangle img_rect;
    img_rect.img2d = shared_arg->img_out;

    bool rasterization_over = false;

    while(error == WF3D_SUCCESS && !rasterization_over)
    {
        int thread_error = pthread_mutex_lock(&shared_arg->shared_mutex);

        if(thread_error == 0)
        {
            if(shared_arg->next_x < shared_arg->img_out->width && shared_arg->next_y < shared_arg->img_out->height)
            {
                img_rect.y_min = shared_arg->next_y;
                img_rect.x_min = shared_arg->next_x;

                img_rect.y_max = shared_arg->next_y + shared_arg->y_block_len;
                if(img_rect.y_max > shared_arg->img_out->height)
                {
                    img_rect.y_max = shared_arg->img_out->height;
                }

                img_rect.x_max = img_rect.x_min + shared_arg->x_block_len;

                if(img_rect.x_max >= shared_arg->img_out->width)
                {
                    img_rect.x_max = shared_arg->img_out->width;
                    shared_arg->next_x = 0;
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

        pthread_mutex_unlock(&shared_arg->shared_mutex);

        if(error == WF3D_SUCCESS && !rasterization_over)
        {
            error = fc3d_rendering_octree_node_Rasterization(shared_arg->octree->node_0, &img_rect, shared_arg->cam_lightsource_list, shared_arg->nb_lightsources, shared_arg->cam_v_pos, shared_arg->cam_q_rot, shared_arg->cam);
        }
    }

    arg->error_ret = error;

    return NULL;
}

//Rasterization with multithreading
//
//
wf3d_error fc3d_RenderingOctree_MultiThreadRasterization(fc3d_RenderingOctree* octree, wf3d_Image2d* img_out, unsigned short nb_threads, wf3d_lightsource const* lightsource_list, unsigned int nb_lightsources, owl_v3f32 cam_v_pos, owl_q32 cam_q_rot, wf3d_camera3d const* cam)
{
    wf3d_error error = WF3D_SUCCESS;

    if(nb_threads > 0)
    {
        wf3d_lightsource* cam_lightsource_list = malloc(nb_lightsources * sizeof(*cam_lightsource_list));

        pthread_t* thread_list = malloc((size_t)nb_threads * sizeof(*thread_list));

        wf3d_octree_rasterization_thread_shared_arg shared_arg;
        wf3d_octree_rasterization_thread_arg* thread_arg_list = malloc((size_t)nb_threads * sizeof(*thread_arg_list));

        if((nb_lightsources == 0 || cam_lightsource_list != NULL) && thread_list != NULL && thread_arg_list != NULL)
        {
            owl_q32 cam_q_rot_conj = owl_q32_conj(cam_q_rot);
            owl_v3f32 opp_cam_v_pos = owl_v3f32_sub(owl_v3f32_zero(), cam_v_pos);

            for(unsigned int k = 0 ; k < nb_lightsources ; k++)
            {
                wf3d_lightsource_transform(cam_lightsource_list + k, lightsource_list + k, opp_cam_v_pos, cam_q_rot_conj);
            }

            shared_arg.octree = octree;
            shared_arg.cam_lightsource_list = cam_lightsource_list;
            shared_arg.nb_lightsources = nb_lightsources;
            shared_arg.cam_v_pos = cam_v_pos;
            shared_arg.cam_q_rot = cam_q_rot;
            shared_arg.cam = cam;
            shared_arg.img_out = img_out;

            unsigned int nb_div = 2;
            while(nb_div * nb_div < 4 * (unsigned int)nb_threads)
            {
                nb_div *= 2;
            }

            shared_arg.x_block_len = img_out->width / nb_div;
            shared_arg.y_block_len = img_out->height / nb_div;

            shared_arg.shared_mutex = PTHREAD_MUTEX_INITIALIZER;
            shared_arg.next_x = 0;
            shared_arg.next_y = 0;

            unsigned short nb_thread_launched = 0;
            for(unsigned short k = 0 ; k < nb_threads && error == WF3D_SUCCESS ; k++)
            {
                thread_arg_list[k].shared_arg = &shared_arg;
                thread_arg_list[k].error_ret = WF3D_SUCCESS;
                int thread_error = pthread_create(thread_list + k, NULL, wf3d_octree_multithread_rasterization, thread_arg_list + k);

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

        free(cam_lightsource_list);
        free(thread_list);
        free(thread_arg_list);
    }
    else
    {
        error = WF3D_THREAD_ERROR;
    }

    return error;
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

//
//
//
wf3d_color* fc3d_RenderingOctree_PointColor(fc3d_RenderingOctree const* octree, wf3d_color* final_color, wf3d_lightsource const* lightsource_list, unsigned int nb_lightsources, owl_v3f32 cam_ray_dir, owl_v3f32 v_pos, owl_v3f32 normal, wf3d_surface const* surface, int max_nb_reflections, float near_clipping_distance, float far_clipping_distance)
{
    wf3d_lightsource_enlight_surface(lightsource_list, nb_lightsources, final_color, surface, v_pos, normal);

    if(max_nb_reflections > 0)
    {
        bool reflection_exists = false;
        for(unsigned int k = 0 ; k < 3 ; k++)
        {
            reflection_exists = reflection_exists || (surface->reflection_filter[k] != 0.0);
        }

        if(reflection_exists)
        {
            owl_v3f32 reflection_ray_dir = owl_v3f32_add_scalar_mul(
                                                                        cam_ray_dir,
                                                                        normal,
                                                                        -2.0 * owl_v3f32_dot(cam_ray_dir, normal)
                                                                    );
            reflection_ray_dir = owl_v3f32_normalize(reflection_ray_dir);

            float t;
            owl_v3f32 reflected_point_normal;
            wf3d_surface reflected_point_surface;
            if(fc3d_rendering_octree_node_NearestIntersectionWithRay(octree->node_0, v_pos, reflection_ray_dir, near_clipping_distance, far_clipping_distance, &t, &reflected_point_normal, &reflected_point_surface))
            {
                owl_v3f32 reflected_point_v_pos = owl_v3f32_add_scalar_mul(v_pos, reflection_ray_dir, t);
                wf3d_color reflection_color;
                fc3d_RenderingOctree_PointColor(
                                                    octree, &reflection_color, lightsource_list, nb_lightsources,
                                                    reflection_ray_dir, reflected_point_v_pos, reflected_point_normal, &reflected_point_surface,
                                                    max_nb_reflections - 1, near_clipping_distance, far_clipping_distance
                                               );
                for(unsigned int k = 0 ; k < 3 ; k++)
                {
                    final_color->rgba[k] += surface->reflection_filter[k] * reflection_color.rgba[k];
                }
            }
        }

        bool refraction_exists = false;
        for(unsigned int k = 0 ; k < 3 ; k++)
        {
            refraction_exists = refraction_exists || (surface->refraction_filter[k] != 0.0);
        }

        if(refraction_exists)
        {
            owl_v3f32 refraction_ray_dir = cam_ray_dir;

            float t;
            owl_v3f32 refracted_point_normal;
            wf3d_surface refracted_point_surface;
            if(fc3d_rendering_octree_node_NearestIntersectionWithRay(octree->node_0, v_pos, refraction_ray_dir, near_clipping_distance, far_clipping_distance, &t, &refracted_point_normal, &refracted_point_surface))
            {
                owl_v3f32 refracted_point_v_pos = owl_v3f32_add_scalar_mul(v_pos, refraction_ray_dir, t);
                wf3d_color refraction_color;
                fc3d_RenderingOctree_PointColor(
                                                    octree, &refraction_color, lightsource_list, nb_lightsources,
                                                    refraction_ray_dir, refracted_point_v_pos, refracted_point_normal, &refracted_point_surface,
                                                    max_nb_reflections - 1, near_clipping_distance, far_clipping_distance
                                               );
                for(unsigned int k = 0 ; k < 3 ; k++)
                {
                    final_color->rgba[k] += surface->refraction_filter[k] * refraction_color.rgba[k];
                }
            }
        }
    }

    return final_color;
}

typedef struct
{
    fc3d_RenderingOctree const* octree;

    wf3d_lightsource const* lightsource_list;
    unsigned int nb_lightsources;
    owl_v3f32 cam_v_pos;
    owl_q32 cam_q_rot;
    wf3d_camera3d const* cam;

    wf3d_Image2d* img_out;

    int x_block_len;
    int y_block_len;

    pthread_mutex_t shared_mutex;
    int next_x;
    int next_y;

} wf3d_octree_ray_tracing_thread_shared_arg;

typedef struct
{
    wf3d_octree_ray_tracing_thread_shared_arg* shared_arg;
    wf3d_Image3d* img3d_buffer;
    wf3d_error error_ret;

} wf3d_octree_ray_tracing_thread_arg;

//
static void* wf3d_octree_multithreaded_ray_tracing(void* ptr)
{
    wf3d_error error = WF3D_SUCCESS;

    wf3d_octree_ray_tracing_thread_arg* arg = ptr;
    wf3d_octree_ray_tracing_thread_shared_arg* shared_arg = arg->shared_arg;

    wf3d_image3d_image_piece img_piece;
    img_piece.img3d = arg->img3d_buffer;
    img_piece.full_img_width = shared_arg->img_out->width;
    img_piece.full_img_height = shared_arg->img_out->height;

    bool rasterization_over = false;

    while(error == WF3D_SUCCESS && !rasterization_over)
    {
        int thread_error = pthread_mutex_lock(&shared_arg->shared_mutex);

        if(thread_error == 0)
        {
            if(shared_arg->next_x < shared_arg->img_out->width && shared_arg->next_y < shared_arg->img_out->height)
            {
                img_piece.y_min = shared_arg->next_y;
                img_piece.x_min = shared_arg->next_x;

                img_piece.y_max = shared_arg->next_y + shared_arg->y_block_len;
                if(img_piece.y_max > shared_arg->img_out->height)
                {
                    img_piece.y_max = shared_arg->img_out->height;
                }

                img_piece.x_max = shared_arg->next_x + shared_arg->x_block_len;
                if(img_piece.x_max >= shared_arg->img_out->width)
                {
                    img_piece.x_max = shared_arg->img_out->width;
                    shared_arg->next_x = 0;
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

        pthread_mutex_unlock(&shared_arg->shared_mutex);

        if(error == WF3D_SUCCESS && !rasterization_over)
        {
            wf3d_Image3d_Clear(img_piece.img3d);
            error = fc3d_rendering_octree_node_Rasterization2(shared_arg->octree->node_0, &img_piece, shared_arg->cam_v_pos, shared_arg->cam_q_rot, shared_arg->cam);
            if(error == WF3D_SUCCESS)
            {
                for(int y = img_piece.y_min ; y < img_piece.y_max ; y++)
                {
                    int y3d = y - img_piece.y_min;
                    for(int x = img_piece.x_min ; x < img_piece.x_max ; x++)
                    {
                        int x3d = x - img_piece.x_min;
                        float depth = wf3d_Image3d_unsafe_Depth(img_piece.img3d, x3d, y3d);
                        if(isfinite(depth) != 0)
                        {
                            owl_v3f32 u = owl_q32_transform_v3f32(shared_arg->cam_q_rot, wf3d_Image3d_unsafe_M(img_piece.img3d, x3d, y3d));
                            owl_v3f32 obj_v_pos = owl_v3f32_add(
                                                                    u,
                                                                    shared_arg->cam_v_pos
                                                                );
                            owl_v3f32 ray_dir = owl_v3f32_normalize(u);
                            owl_v3f32 obj_normal = owl_q32_transform_v3f32(shared_arg->cam_q_rot, wf3d_Image3d_unsafe_Normal(img_piece.img3d, x3d, y3d));
                            wf3d_color pixel_color;
                            fc3d_RenderingOctree_PointColor(
                                                                shared_arg->octree, &pixel_color, shared_arg->lightsource_list, shared_arg->nb_lightsources,
                                                                ray_dir, obj_v_pos, obj_normal, wf3d_Image3d_unsafe_Surface(img_piece.img3d, x3d, y3d),
                                                                shared_arg->cam->max_nb_reflections, shared_arg->cam->near_clipping_distance, shared_arg->cam->far_clipping_distance
                                                            );
                            error = wf3d_Image2d_SetPixel(shared_arg->img_out, x, y, &pixel_color, depth);
                        }
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
wf3d_error fc3d_RenderingOctree_MultiThreadRayTracing(fc3d_RenderingOctree* octree, wf3d_Image2d* img_out, wf3d_Image3d** img3d_thread_buffer_list, unsigned short nb_threads, wf3d_lightsource const* lightsource_list, unsigned int nb_lightsources, owl_v3f32 cam_v_pos, owl_q32 cam_q_rot, wf3d_camera3d const* cam)
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
            shared_arg.cam_v_pos = cam_v_pos;
            shared_arg.cam_q_rot = cam_q_rot;
            shared_arg.cam = cam;
            shared_arg.img_out = img_out;

            shared_arg.x_block_len = img3d_thread_buffer_list[0]->width;
            shared_arg.y_block_len = img3d_thread_buffer_list[0]->height;

            shared_arg.shared_mutex = PTHREAD_MUTEX_INITIALIZER;
            shared_arg.next_x = 0;
            shared_arg.next_y = 0;

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

