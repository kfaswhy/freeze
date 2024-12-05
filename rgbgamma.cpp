#include "rgbgamma.h"

U8 rgbgamma_process(RGB* rgb, IMG_CONTEXT context, G_CONFIG cfg)
{
    if (cfg.rgbgamma_on == 0)
    {
        return OK;
    } 
    
    RGB* p_rgb = &rgb[0];
    for (int i = 0; i < context.full_size; i++)
    {
        U32 tmp = cfg.gamma[p_rgb->r] >> 2;
        p_rgb->r = clp_range(0, tmp, U8MAX);

        tmp = cfg.gamma[p_rgb->g] >> 2;
        p_rgb->g = clp_range(0, tmp, U8MAX);

        tmp = cfg.gamma[p_rgb->b] >> 2;
        p_rgb->b = clp_range(0, tmp, U8MAX);

        p_rgb++;
    }

    LOG("done.");
    return OK;
}