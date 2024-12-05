#include "ltm.h"

#define U16MAX 65535
U8 ltm_process(U16* raw, IMG_CONTEXT context, G_CONFIG cfg)
{
    if (cfg.ltm_on == 0) {
        return OK;
    }

    U16 height = context.height;
    U16 width = context.width;
    U8 vblk = cfg.ltm_vblk;
    U8 hblk = cfg.ltm_hblk;

    if (vblk == 0 || hblk == 0) {
        return OK; // 参数无效，直接返回
    }

    // 每个块的尺寸
    U16 blk_h = height / vblk;
    U16 blk_w = width / hblk;

    // 初始化直方图和亮度映射表
    U32* histograms[4]; // 4个通道分别统计直方图
    float* ltm_curves[4]; // 亮度映射曲线

    for (int i = 0; i < 4; ++i) {
        histograms[i] = (U32*)calloc(U16MAX + 1, sizeof(U32));
        ltm_curves[i] = (float*)malloc((U16MAX + 1) * sizeof(float));
    }

    // 分块处理图像
    for (U8 v = 0; v < vblk; ++v) {
        for (U8 h = 0; h < hblk; ++h) {
            // 当前块的起始坐标
            U16 blk_start_y = v * blk_h;
            U16 blk_start_x = h * blk_w;

            // 当前块内直方图统计
            memset(histograms[0], 0, (U16MAX + 1) * sizeof(U32));
            memset(histograms[1], 0, (U16MAX + 1) * sizeof(U32));
            memset(histograms[2], 0, (U16MAX + 1) * sizeof(U32));
            memset(histograms[3], 0, (U16MAX + 1) * sizeof(U32));

            U32 sum[4] = { 0 }; // 各通道亮度总和
            U16 max_brightness[4] = { 0 }; // 各通道最大亮度

            for (U16 y = blk_start_y; y < blk_start_y + blk_h; ++y) {
                for (U16 x = blk_start_x; x < blk_start_x + blk_w; ++x) {
                    // 根据 Bayer 格式选择通道
                    int channel;
                    if (cfg.pattern == RGGB) {
                        channel = ((y % 2) << 1) + (x % 2);
                    }
                    else if (cfg.pattern == GRBG) {
                        channel = ((y % 2) << 1) + !(x % 2);
                    }
                    else if (cfg.pattern == GBRG) {
                        channel = (!(y % 2) << 1) + (x % 2);
                    }
                    else { // BGGR
                        channel = (!(y % 2) << 1) + !(x % 2);
                    }

                    U16 pixel_value = raw[y * width + x];
                    histograms[channel][pixel_value]++;
                    sum[channel] += pixel_value;
                    if (pixel_value > max_brightness[channel]) {
                        max_brightness[channel] = pixel_value;
                    }
                }
            }

            // 计算各通道的平均亮度和对比度限制阈值
            for (int c = 0; c < 4; ++c) {
                float avg_brightness = (float)sum[c] / (blk_h * blk_w / 4);
                float contrast_threshold = ((max_brightness[c] - avg_brightness) * cfg.ltm_cst_thdr + avg_brightness);

                // 对比度限制：调整直方图
                U32 excess = 0; // 超出部分总和
                for (int i = 0; i <= U16MAX; ++i) {
                    if (histograms[c][i] > contrast_threshold) {
                        excess += histograms[c][i] - contrast_threshold;
                        histograms[c][i] = contrast_threshold;
                    }
                }

                // 将超出的部分均匀分配
                U32 increment = excess / (U16MAX + 1);
                for (int i = 0; i <= U16MAX; ++i) {
                    histograms[c][i] += increment;
                }

                // 生成亮度映射曲线
                U32 cumulative = 0;
                for (int i = 0; i <= U16MAX; ++i) {
                    cumulative += histograms[c][i];
                    ltm_curves[c][i] = (float)cumulative / (blk_h * blk_w / 4) * U16MAX;
                }
            }
        }
    }

    // 对图像每个像素应用亮度映射（双线性插值）
    for (U16 y = 0; y < height; ++y) {
        for (U16 x = 0; x < width; ++x) {
            int channel;
            if (cfg.pattern == RGGB) {
                channel = ((y % 2) << 1) + (x % 2);
            }
            else if (cfg.pattern == GRBG) {
                channel = ((y % 2) << 1) + !(x % 2);
            }
            else if (cfg.pattern == GBRG) {
                channel = (!(y % 2) << 1) + (x % 2);
            }
            else { // BGGR
                channel = (!(y % 2) << 1) + !(x % 2);
            }

            // 当前像素所在块的位置
            float fy = (float)y / blk_h - 0.5f;
            float fx = (float)x / blk_w - 0.5f;

            int blk_y = (int)floor(fy);
            int blk_x = (int)floor(fx);
            float dy = fy - blk_y;
            float dx = fx - blk_x;

            blk_y = blk_y < 0 ? 0 : (blk_y >= vblk - 1 ? vblk - 1 : blk_y);
            blk_x = blk_x < 0 ? 0 : (blk_x >= hblk - 1 ? hblk - 1 : blk_x);

            // 双线性插值四个块的映射值
            int idx = raw[y * width + x];
            float mapped = (1 - dy) * (1 - dx) * ltm_curves[channel][idx] +
                dy * (1 - dx) * ltm_curves[channel][idx] +
                (1 - dy) * dx * ltm_curves[channel][idx] +
                dy * dx * ltm_curves[channel][idx];

            // 应用 LTM 映射结果
            raw[y * width + x] = (U16)(cfg.ltm_strength * mapped + (1 - cfg.ltm_strength) * raw[y * width + x]);
        }
    }

    // 释放内存
    for (int i = 0; i < 4; ++i) {
        free(histograms[i]);
        free(ltm_curves[i]);
    }

    LOG("done.");
    return OK;
}