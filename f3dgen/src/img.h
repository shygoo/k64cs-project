#include <stdint.h>

#include "vecdef.h"
#include "stb_image.h"

typedef struct
{
    uint8_t r, g, b, a;
} img_pixel_t;

VECTOR_TYPEDEF(img_pixel_t, img_pixel_vec_t, img_pixel_vec_)

typedef struct
{
    int width, height, num_pixels;
    img_pixel_t* pixels;
} img_t;

typedef struct
{
    img_pixel_t* colors;
    int* indeces;
    int width, height, num_indeces, num_colors;
} img_ci_t;

/*
typedef struct
{
    img_pixel_t pixel;
    int count;
} img_color_count_t;
*/

int img_load(img_t* img, const char* path);
int img_unload(img_t* img);
int img_ci_init(img_ci_t* img_ci, img_t* img);
int img_ci_free(img_ci_t* img_ci);

int img_pixel_vec_push_unique(img_pixel_vec_t* pv, img_pixel_t p);


uint8_t channel8to5(uint8_t channel_value);
int img_ci_print(img_ci_t* img_ci);
int img_is_grayscale(img_t* img);