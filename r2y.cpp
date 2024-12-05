#include "r2y.h"

YUV* r2y_process(RGB* rgb, IMG_CONTEXT context, G_CONFIG cfg)
{
    YUV *yuv = (YUV*)calloc(context.height * context.width, sizeof(YUV));

    if (!yuv) return NULL;

    // 遍历每个像素
    for (U32 i = 0; i < context.full_size; i++)
    {      
        // 获取当前 RGB 值
        S32 r = rgb[i].r;
        S32 g = rgb[i].g;
        S32 b = rgb[i].b;

        // 转换为 YUV，使用整数计算
        S32 y_val = ((S32)77 * r + 150 * g + 29 * b) >> 8;  // Y = (0.299*256 * R + 0.587*256 * G + 0.114*256 * B) / 256
        S32 u_val = (((S32)-43 * r - 85 * g + 128 * b) >> 8) + 128; // U = (−0.147*256 * R − 0.289*256 * G + 0.436*256 * B) / 256 + 128
        S32 v_val = (((S32)128 * r - 107 * g - 21 * b) >> 8) + 128; // V = (0.615*256 * R − 0.515*256 * G − 0.100*256 * B) / 256 + 128

        // 限制值在 [0, 255] 范围
        yuv[i].y = clp_range(0, y_val, U8MAX);
        yuv[i].u = clp_range(0, u_val, U8MAX);
        yuv[i].v = clp_range(0, v_val, U8MAX);

    }
    LOG("done.");
    return yuv;
}