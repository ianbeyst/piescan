#include <stddef.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include <sys/time.h>

#include "piescan.h"
#include "imsave.h"
#include "mmaparray.h"



/*****************************************************************************\
| Macros                                                                      |
\*****************************************************************************/



/*****************************************************************************\
| Type definitions                                                            |
\*****************************************************************************/



/*****************************************************************************\
| Function declarations                                                       |
\*****************************************************************************/

static int uniform_int(int min, int max);



/*****************************************************************************\
| Function implementations                                                    |
\*****************************************************************************/

/*
// Taken from https://stackoverflow.com/questions/11641629/generating-a-uniform-distribution-of-integers-in-c
int
uniform_int(int min, int max)
{
    int range = max - min + 1;
    int range_max = RAND_MAX - RAND_MAX % range;
    int x;
    do x = rand(); while (x >= range_max);
    return min + x % range;
}
*/

void
normalize_image(Image* im)
{
    uint16_t minval_r = 65535;
    uint16_t minval_g = 65535;
    uint16_t minval_b = 65535;
    uint16_t minval_i = 65535;

    uint16_t maxval_r = 0;
    uint16_t maxval_g = 0;
    uint16_t maxval_b = 0;
    uint16_t maxval_i = 0;

    for (size_t i = 0; i < im->width*im->height; ++i) {
        if (im->r[i] < minval_r) minval_r = im->r[i];
        if (im->g[i] < minval_g) minval_g = im->g[i];
        if (im->b[i] < minval_b) minval_b = im->b[i];
        if (im->i[i] < minval_i) minval_i = im->i[i];

        if (im->r[i] > maxval_r) maxval_r = im->r[i];
        if (im->g[i] > maxval_g) maxval_g = im->g[i];
        if (im->b[i] > maxval_b) maxval_b = im->b[i];
        if (im->i[i] > maxval_i) maxval_i = im->i[i];
    }

    double range_r = maxval_r - minval_r;
    double range_g = maxval_g - minval_g;
    double range_b = maxval_b - minval_b;
    double range_i = maxval_i - minval_i;

    for (size_t i = 0; i < im->width*im->height; ++i) {
        im->r[i] = (uint16_t) round(65535.0 * ((double) (im->r[i] - minval_r)) / range_r);
        im->g[i] = (uint16_t) round(65535.0 * ((double) (im->g[i] - minval_g)) / range_g);
        im->b[i] = (uint16_t) round(65535.0 * ((double) (im->b[i] - minval_b)) / range_b);
        im->i[i] = (uint16_t) round(65535.0 * ((double) (im->i[i] - minval_i)) / range_i);
    }
}

int main()
{
    piescan_open();

    ScanSettings settings = get_default_settings();
    settings.resolution = 7200;
    settings.gain_r = 0;
    settings.gain_g = 0;
    settings.gain_b = 0;
    settings.gain_i = 0;

    Image* im = new_image();

    struct timeval tv1, tv2;
    char filename[128];

    for (int light = 1; light >= 0; --light) {
        settings.light = 4*light;

        for (size_t i = 0; i < 26; ++i) {
            gettimeofday(&tv1, NULL);

            settings.exposure_r = 3000 + 280 * i;
            settings.exposure_g = 3000 + 280 * i;
            settings.exposure_b = 3000 + 280 * i;
            settings.exposure_i =  700 + 372 * i;

            scan_image(im, settings);


            sprintf(filename, "raw/test_r_%d_%05lu.mmarr", light, i);
            MmapArray* arr_r = get_mmap_writer(filename, im->width*im->height*sizeof(uint16_t));
            sprintf(filename, "raw/test_g_%d_%05lu.mmarr", light, i);
            MmapArray* arr_g = get_mmap_writer(filename, im->width*im->height*sizeof(uint16_t));
            sprintf(filename, "raw/test_b_%d_%05lu.mmarr", light, i);
            MmapArray* arr_b = get_mmap_writer(filename, im->width*im->height*sizeof(uint16_t));
            sprintf(filename, "raw/test_i_%d_%05lu.mmarr", light, i);
            MmapArray* arr_i = get_mmap_writer(filename, im->width*im->height*sizeof(uint16_t));

            memcpy(arr_r->data, im->r, arr_r->size);
            memcpy(arr_g->data, im->g, arr_g->size);
            memcpy(arr_b->data, im->b, arr_b->size);
            memcpy(arr_i->data, im->i, arr_i->size);

            free_mmap_array(arr_r);
            free_mmap_array(arr_g);
            free_mmap_array(arr_b);
            free_mmap_array(arr_i);

            normalize_image(im);

            sprintf(filename, "png/test_r_%d_%05lu.png", light, i);
            imsave16(im->r, im->width, im->height, 1, filename);
            sprintf(filename, "png/test_g_%d_%05lu.png", light, i);
            imsave16(im->g, im->width, im->height, 1, filename);
            sprintf(filename, "png/test_b_%d_%05lu.png", light, i);
            imsave16(im->b, im->width, im->height, 1, filename);
            sprintf(filename, "png/test_i_%d_%05lu.png", light, i);
            imsave16(im->i, im->width, im->height, 1, filename);

            gettimeofday(&tv2, NULL);

            int secs = (int) round((double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
                 (double) (tv2.tv_sec - tv1.tv_sec));

            printf("Processing scan %lu took %d seconds\n", i, secs);
        }
    }
    free_image(im);


    /*
    ScanSettings settings = get_default_settings();
    settings.resolution = 7200;

    Image* im = new_image();

    char filename_base[128];
    char filename[128];
    struct timeval tv1, tv2;

    for (int i = 0; i < 150; ++i) {
        settings.light = uniform_int(0, 1)*uniform_int(1, 9);
        settings.gain_r = uniform_int(0, 63);
        settings.gain_g = settings.gain_r;
        settings.gain_b = settings.gain_r;
        settings.gain_i = settings.gain_r;
        settings.exposure_r = uniform_int(100, 10000);
        settings.exposure_g = settings.exposure_r;
        settings.exposure_b = settings.exposure_r;
        settings.exposure_i = settings.exposure_r;
        settings.offset_r = uniform_int(0, 255);
        settings.offset_g = settings.offset_r;
        settings.offset_b = settings.offset_r;
        settings.offset_i = settings.offset_r;
        settings.sharpen = uniform_int(0, 1);
        settings.threshold = SANE_FIX((double)uniform_int(0, 100));

        gettimeofday(&tv1, NULL);

        scan_image(im, settings);

        gettimeofday(&tv2, NULL);

        int secs = (int) round((double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec));

        sprintf(filename_base, "calib/%d_%02d_%05d_%03d_%d_%03d_%03d",
                                                settings.light,
                                                settings.gain_r,
                                                settings.exposure_r,
                                                settings.offset_r,
                                                settings.sharpen,
                                                (int)round(SANE_UNFIX(settings.threshold)),
                                                secs);

        sprintf(filename, "%s_R.png", filename_base);
        imsave16(im->r, im->width, im->height, 1, filename);
        sprintf(filename, "%s_G.png", filename_base);
        imsave16(im->g, im->width, im->height, 1, filename);
        sprintf(filename, "%s_B.png", filename_base);
        imsave16(im->b, im->width, im->height, 1, filename);
        sprintf(filename, "%s_I.png", filename_base);
        imsave16(im->i, im->width, im->height, 1, filename);
    }
    free_image(im);
    */

    piescan_close();
}

