#include "y2r.h"

RGB* y2r_process(YUV* yuv, IMG_CONTEXT context, G_CONFIG cfg)
{
    if (!yuv) {
        std::cerr << "Invalid input parameters!" << std::endl;
        return NULL; // 错误代码
    }
    
    RGB* rgb = (RGB*)calloc(context.height * context.width, sizeof(RGB));

    for (U32 i = 0; i < context.full_size; ++i) {
        // 根据 G_CONFIG::order 决定 U 和 V 的读取顺序
        U8 y = yuv[i].y;
        U8 u = yuv[i].u;
        U8 v = yuv[i].v;
        
        // 转换为 RGB，使用整数计算公式
        S32 r = y + ((359 * (v - 128)) >> 8);
        S32 g = y - ((88 * (u - 128) + 183 * (v - 128)) >> 8);
        S32 b = y + ((454 * (u - 128)) >> 8);

        // 限制 RGB 值在 [0, 255] 范围
        rgb[i].r = clp_range(0, r, 255);
        rgb[i].g = clp_range(0, g, 255);
        rgb[i].b = clp_range(0, b, 255);
    }
    //LOG("done.");

    return rgb; 
}