#include "isp_gain.h"
//#include "raw_process.h"
U8 isp_gain_process(U16* raw, IMG_CONTEXT context, G_CONFIG cfg)
{
    if (cfg.isp_gain_on == 0)
    {
        return OK;
    }
    

    for (int i = 0; i < context.full_size; i++)
    {
        raw[i] = clp_range(0, ((U32)raw[i] * cfg.isp_gain) >> 10, U16MAX);
    }

    LOG("done.");
    return OK;
}