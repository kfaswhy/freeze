#include "sharp.h"
// Simple sharpen kernel (example: a 3x3 filter)

#define K_SIZE 5

int sharp_kernel[K_SIZE][K_SIZE] = {
    {0,0,0,0,0},
    {0,0,-1,0,0},
    {0,-1,4,-1,0},
    {0,0,-1,0,0},
    {0,0,0,0,0}
};

int sharp_kernel2[K_SIZE][K_SIZE] = {
    {0,0,0,0,0},
    {0,-1,-1,-1,0},
    {0,-1,8,-1,0},
    {0,-1,-1,-1,0},
    {0,0,0,0,0}
};

int sharp_kernel1[K_SIZE][K_SIZE] = {
    {0, 0, -1,  0, 0 },
    {0, -1, -2,  -1, 0 },
    {-1, -2,  16, -2, -1},
    {0, -1, -2,  -1, 0 },
    {0, 0, -1,  0, 0 },
};

int sharp_kernel8[K_SIZE][K_SIZE] = {
    {0, 0, -2,  0, 0 },
    {0, -2, 2,  -2, 0 },
    {-2, 2,  8, 2, -2},
    {0, -2, 2,  -2, 0 },
    {0, 0, -2,  0, 0 },
};

int sharp_kernel4[K_SIZE][K_SIZE] = {
    {0,0,0,0,0},
    {-1,-1,-1,-1,-1},
    {2,2,2,2,2},
    {-1,-1,-1,-1,-1},
    {0,0,0,0,0}
};

int sharp_kernel5[K_SIZE][K_SIZE] = {
    {0,-1,2,-1,0},
    {0,-1,2,-1,0},
    {0,-1,2,-1,0},
    {0,-1,2,-1,0},
    {0,-1,2,-1,0}
};

int sharp_kernel9[K_SIZE][K_SIZE] = {
    {-1,0,2,-1,0},
    {-1,0,2,-1,0},
    {-1,0,2,-1,0},
    {-1,0,2,-1,0},
    {-1,0,2,-1,0}
    

};

U8 sharp_process(YUV* yuv, IMG_CONTEXT context, G_CONFIG cfg)
{
    if (cfg.sharp_on == 0)
    {
        return OK;
    }

    S16* y_sharp = (S16*)calloc(context.height * context.width, sizeof(S16));

    // Apply sharpen filter
    for (U16 y = 2; y < context.height; ++y) {
        for (U16 x = 2; x < context.width; ++x) {
            S32 new_y = 0;
            
            for (int ky = -K_SIZE / 2; ky <= K_SIZE / 2; ++ky) {
                for (int kx = -K_SIZE / 2; kx <= K_SIZE / 2; ++kx) {
                    // Calculate the pixel index considering boundary conditions
                    int ny = y + ky;
                    int nx = x + kx;

                    ny = clp_range(0, ny, (context.height - 1));
                    nx = clp_range(0, nx, (context.width - 1));

                    int pixel_idx = ny * context.width + nx;


                    new_y += sharp_kernel[ky + K_SIZE / 2][kx + K_SIZE / 2] * yuv[pixel_idx].y;
                }
            }
            //new_y += 128;
            y_sharp[y * context.width + x] = new_y;
        }
    }

    for (int i = 0; i < context.full_size; i++)
    {
        U16 tmp = yuv[i].y + 0.5*y_sharp[i];
       //tmp = y_sharp[i]+128;
        yuv[i].y = clp_range(0, tmp, U8MAX);
    }

    LOG("done.");
    return OK;
}