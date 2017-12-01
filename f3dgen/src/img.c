#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "img.h"
#include "vecdef.h"
#include "vecprim.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

VECTOR_IMPLDEF(img_pixel_t, img_pixel_vec_t, img_pixel_vec_)

int img_load(img_t* img, const char* path)
{
    int nchannels;
    img->pixels = (img_pixel_t*)stbi_load(path, &img->width, &img->height, &nchannels, 4);

    if(img->pixels == NULL)
    {
        return 0;
    }

    img->num_pixels = img->width * img->height;

    return 1;
}

int img_unload(img_t* img)
{
    stbi_image_free(img->pixels);
    return 1;
}

int img_is_grayscale(img_t* img)
{
    for(int i = 0; i < img->num_pixels; i++)
    {
        img_pixel_t p = img->pixels[i];
        if(p.r != p.g && p.g != p.b)
        {
            return 0;
        }
    }
    return 1;
}

/////////////////////////////

int img_ci_init(img_ci_t* img_ci, img_t* img)
{
    img_pixel_vec_t pv;
    img_pixel_vec_init(&pv);

    img_ci->num_indeces = img->num_pixels;
    img_ci->width = img->width;
    img_ci->height = img->height;
    
    img_ci->indeces = (int*)malloc(img->num_pixels * sizeof(int));

    for(int i = 0; i < img->num_pixels; i++)
    {
        img_pixel_t p = img->pixels[i];
        int ci = img_pixel_vec_push_unique(&pv, p);
        img_ci->indeces[i] = ci;
    }

    img_ci->num_colors = pv.length;
    img_ci->colors = pv.array;

    return 1;
}

int img_ci_free(img_ci_t* img_ci)
{
    free(img_ci->colors);
    free(img_ci->indeces);
    return 1;
}

int img_pixel_vec_push_unique(img_pixel_vec_t* pv, img_pixel_t p)
{
    for(int i = 0; i < pv->length; i++)
    {
        if((memcmp(&pv->array[i], &p, sizeof(img_pixel_t)) == 0))
        {
            return i;
        }
    }
    return img_pixel_vec_push(pv, p) - 1;
}

uint8_t channel8to5(uint8_t channel_value)
{
    float fc = (float)channel_value / UINT8_MAX;
    uint8_t val5 = round(fc * 0x1F);
    return val5;
}

int img_ci_print(img_ci_t* img_ci)
{
    printf("num colors: %d, px: %d\n", img_ci->num_colors, img_ci->num_indeces);

    for(int i = 0; i < img_ci->num_colors; i++)
    {
        img_pixel_t p = img_ci->colors[i];
        printf("%02X: %02X%02X%02X%02X\n", i, p.r, p.g, p.b, p.a);
    }

    printf("\n");

    for(int i = 0; i < img_ci->num_indeces; i++)
    {
        printf("%02X", img_ci->indeces[i]);
        
        if((i + 1) % img_ci->width == 0)
        {
            printf("\n");
        }
    }
    return 1;
}

/*

int main(int argc, const char* argv[])
{
    img_t img;
    int res = img_load(&img, "test.png");

    img_ci_t img_ci;
    img_ci_init(&img_ci, &img);

    img_ci_print(&img_ci);

    //printf("num colors: %d\n", ncolors);

    //for(int i = 0; i < img.num_pixels; i++)
    //{
    //    uint8_t r = channel8to5(img.pixels[i].r);
    //    uint8_t g = channel8to5(img.pixels[i].g);
    //    uint8_t b = channel8to5(img.pixels[i].b);
    //    uint8_t a = img.pixels[i].a > 0 ? 1 : 0;
//
    //    uint8_t pxhi = (r << 3) | ((g & 0b11100) >> 2);
    //    uint8_t pxlo = ((g & 0b11000) << 3) | (b << 1) | a;
    //    
    //    img_out[i / 2] = pxhi;
    //    img_out[i / 2 + 1] = pxlo;
//
    //    printf("%02X %02X %02X %02X -> %02X%02X (%d %d %d)\n", img[i + 0], img[i + 1], img[i + 2], img[i + 3], pxhi, pxlo, r, g, b);
    //}

    //printf("%d %d\n", width, height);
}

*/