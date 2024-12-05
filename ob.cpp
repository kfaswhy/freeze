#include "ob.h"

U8 ob_process(U16* raw, IMG_CONTEXT context, G_CONFIG cfg)
{
    if (cfg.ob_on == 0)
    {
        return OK;
    }

    float ob_gain = (float)U16MAX / (U16MAX - cfg.ob);
    for (int i = 0; i < context.full_size; i++)
    {
        raw[i] = safe_sub(raw[i], cfg.ob);
        raw[i] = clp_range(0, (U32)raw[i] * ob_gain, U16MAX);
    }
    LOG("done.");

    return OK;
}

U8 ob_process2(U16* raw, IMG_CONTEXT context, G_CONFIG cfg)
{
    if (cfg.ob_on == 0)
    {
        return OK;
    }
    U64 ob00 = 0;
    U64 ob01 = 0;
    U64 ob10 = 0;
    U64 ob11 = 0;
    U32 cnt = 0;


    for (U16 y = 0; y < context.height; y++)
    {
        for (U16 x = 0; x < context.width; x++)
        {
            if ((y % 2 == 0) && (x % 2 == 0))
            {
                ob00 += raw[y * context.width + x];
                cnt++;
            }
            if ((y % 2 == 0) && (x % 2 == 1))
            {
                ob01 += raw[y * context.width + x];
            }
            if ((y % 2 == 1) && (x % 2 == 0))
            {
                ob10 += raw[y * context.width + x];
            }

            if ((y % 2 == 1) && (x % 2 == 1))
            {
                ob11 += raw[y * context.width + x];
            }
        }
    }

    U16 bit_tranz = cfg.bit - cfg.used_bit - 2;

    ob00 = (ob00 >> bit_tranz) / cnt;
    ob01 = (ob01 >> bit_tranz) / cnt;
    ob10 = (ob10 >> bit_tranz) / cnt;
    ob11 = (ob11 >> bit_tranz) / cnt;

    LOG("OB = [%u, %u, %u, %u].", ob00, ob01, ob10, ob11);
    LOG("done.");

    return OK;
}