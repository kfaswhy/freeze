#include "ccm.h"
//#include "raw_process.h"
U8 ccm_process(RGB* rgb, IMG_CONTEXT context, G_CONFIG cfg)
{
    if (cfg.ccm_on == 0)
    {
        return OK;
    }
    
    RGB* p_rgb = &rgb[0];
    float tmp = 0.0;
    for (int i = 0; i < context.full_size; i++)
    {

        // 应用色彩校正矩阵
        tmp = (float)cfg.ccm[0] * p_rgb->r + cfg.ccm[1] * p_rgb->g + cfg.ccm[2] * p_rgb->b;
        p_rgb->r = clp_range(0, tmp, 255);

        tmp = (float)cfg.ccm[3] * p_rgb->r + cfg.ccm[4] * p_rgb->g + cfg.ccm[5] * p_rgb->b;
        p_rgb->g = clp_range(0, tmp, 255);

        tmp = (float)cfg.ccm[6] * p_rgb->r + cfg.ccm[7] * p_rgb->g + cfg.ccm[8] * p_rgb->b;
        p_rgb->b = clp_range(0, tmp, 255);

        p_rgb++;
    }




    LOG("done.");

    return OK;
}