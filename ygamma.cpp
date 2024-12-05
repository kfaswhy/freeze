#include "ygamma.h"

U8 ygamma_process(YUV* yuv, IMG_CONTEXT context, G_CONFIG cfg)
{
    if (cfg.ygamma_on == 0)
    {
        return OK;
    }

    for (int i = 0; i < context.full_size; i++)
    {
        U32 tmp = cfg.gamma[yuv[i].y] >> 2;
        yuv[i].y = clp_range(0, tmp, U8MAX);
    }
    LOG("done.");
    return OK;
}