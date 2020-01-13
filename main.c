#include <stddef.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <sys/time.h>

#include "piescan.h"
#include "imsave.h"



/*****************************************************************************\
| Macros                                                                      |
\*****************************************************************************/



/*****************************************************************************\
| Function declarations                                                       |
\*****************************************************************************/

static int uniform_int(int min, int max);



/*****************************************************************************\
| Function implementations                                                    |
\*****************************************************************************/

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

int main()
{
    piescan_open();

    ScanSettings settings = get_default_settings();
    settings.resolution = 7200;

    Image* im = new_image();

    char filename_base[128];
    char filename[128];
    struct timeval tv1, tv2;



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
                                            settings.threshold,
                                            secs);

    sprintf(filename, "%s_R.png", filename_base);
    imsave16(im->r, im->width, im->height, 1, filename);
    sprintf(filename, "%s_G.png", filename_base);
    imsave16(im->g, im->width, im->height, 1, filename);
    sprintf(filename, "%s_B.png", filename_base);
    imsave16(im->b, im->width, im->height, 1, filename);
    sprintf(filename, "%s_I.png", filename_base);
    imsave16(im->i, im->width, im->height, 1, filename);

    /*
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
    */

    free_image(im);
    piescan_close();
}

