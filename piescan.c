#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <signal.h>
#include <unistd.h>

#include <sane/sane.h>

#include "piescan.h"



/*****************************************************************************\
| Macros                                                                      |
\*****************************************************************************/



/*****************************************************************************\
| Type definitions                                                            |
\*****************************************************************************/



/*****************************************************************************\
| Function declarations                                                       |
\*****************************************************************************/

static void piescan_exit(int status);
static void sighandler(int signum);
static const SANE_Option_Descriptor* get_option_descriptor_safe(SANE_Int i);
static void get_option_value_safe(SANE_Int i, void* v);
static SANE_Int set_option_value_safe(SANE_Int i, void* v);
static void set_options(ScanSettings settings);
static void open_device();
static void resize_image(Image* im, uint32_t width, uint32_t height);



/*****************************************************************************\
| Global variables                                                            |
\*****************************************************************************/

static SANE_Handle device;



/*****************************************************************************\
| Function implementations                                                    |
\*****************************************************************************/

void
piescan_open()
{
    SANE_Int version_code;
    sane_init(&version_code, NULL);

#ifdef SIGHUP
    signal(SIGHUP, sighandler);
#endif
#ifdef SIGPIPE
    signal(SIGPIPE, sighandler);
#endif
    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);

    open_device();
}

void
piescan_close()
{
    piescan_exit(SANE_STATUS_GOOD);
}

void
piescan_exit(int status)
{
    if (device) {
        fprintf(stderr, "Closing device\n");
        sane_close(device);
    }
    fprintf(stderr, "Exiting SANE\n");
    sane_exit();

    fprintf(stderr, "finished\n");
    exit(status);
}

void
sighandler(int signum)
{
    static SANE_Bool first_time = SANE_TRUE;

    if (device) {
        fprintf (stderr, "Received signal %d\n", signum);
        if (first_time) {
            first_time = SANE_FALSE;
            fprintf(stderr, "Trying to stop scanner...\n");
            sane_cancel(device);
        } else {
            fprintf(stderr, "Aborting\n");
            _exit(1);
        }
    }
}

const SANE_Option_Descriptor*
get_option_descriptor_safe(SANE_Int i)
{
    const SANE_Option_Descriptor* opt = sane_get_option_descriptor(device, i);
    if (!opt) {
        fprintf(stderr, "Error: unable to get option descriptor\n");
        piescan_exit(1);
    }
    return opt;
}

void
get_option_value_safe(SANE_Int i, void* v)
{
    SANE_Status status = sane_control_option(device, i, SANE_ACTION_GET_VALUE,
                                             v, 0);
    if (status != SANE_STATUS_GOOD) {
        fprintf(stderr, "Error: %s\n", sane_strstatus(status));
        piescan_exit(status);
    }
}

SANE_Int
set_option_value_safe(SANE_Int i, void* v)
{
    SANE_Int result = 0;
    SANE_Status status = sane_control_option(device, i, SANE_ACTION_SET_VALUE,
                                             v, &result);
    if (status != SANE_STATUS_GOOD) {
        fprintf(stderr, "Error: %s\n", sane_strstatus(status));
        piescan_exit(status);
    }
    return result;
}

ScanSettings
get_default_settings()
{
    ScanSettings settings;

    settings.mode = "RGBI";
    settings.calibration = "from options";
    settings.gain_adjust = "* 1.0";
    settings.crop = "None";

    settings.resolution = 300;
    settings.threshold = 50;
    SANE_Fixed fixval;
    get_option_value_safe(13, &fixval);
    settings.tl_x = SANE_UNFIX(fixval);
    get_option_value_safe(14, &fixval);
    settings.tl_y = SANE_UNFIX(fixval);
    get_option_value_safe(15, &fixval);
    settings.br_x = SANE_UNFIX(fixval);
    get_option_value_safe(16, &fixval);
    settings.br_y = SANE_UNFIX(fixval);

    settings.sharpen = false;
    settings.shading_analysis = false;
    settings.fast_infrared = false;
    settings.auto_advance = false;
    settings.correct_shading = false;
    settings.correct_infrared = false;
    settings.clean_image = false;
    settings.preview = false;
    settings.save_shading = false;
    settings.save_ccdmask = false;

    settings.depth = 16;
    settings.smooth = 0;
    settings.light = 4;
    settings.double_times = 0;
    settings.exposure_r = 2937;
    settings.exposure_g = 2937;
    settings.exposure_b = 2937;
    settings.exposure_i = 2937;
    settings.gain_r = 19;
    settings.gain_g = 19;
    settings.gain_b = 19;
    settings.gain_i = 19;
    settings.offset_r = 0;
    settings.offset_g = 0;
    settings.offset_b = 0;
    settings.offset_i = 0;

    return settings;
}

void
print_options()
{
    const SANE_Option_Descriptor* opt;
    char strval[64];
    SANE_Int intval;
    SANE_Fixed fixval;

    SANE_Int options_str[] = {2, 11, 21, 22};
    SANE_Int options_fix[] = {4, 6, 13, 14, 15, 16};
    SANE_Int options_int[] = {3, 7, 8, 9, 10, 18, 19, 20, 23, 27, 28, 29, 30,
                              31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
                              43};

    for (int i = 0; i < 4; ++i) {
        SANE_Int optnum = options_str[i];
        opt = get_option_descriptor_safe(optnum);
        get_option_value_safe(optnum, strval);
        fprintf(stderr, "\t%-20s: %s\n", opt->name, strval);
    }

    fprintf(stderr, "\n");

    for (int i = 0; i < 6; ++i) {
        SANE_Int optnum = options_fix[i];
        opt = get_option_descriptor_safe(optnum);
        get_option_value_safe(optnum, &fixval);
        fprintf(stderr, "\t%-20s: %.17g\n", opt->name, SANE_UNFIX(fixval));
    }

    fprintf(stderr, "\n");

    for (int i = 0; i < 26; ++i) {
        SANE_Int optnum = options_int[i];
        opt = get_option_descriptor_safe(optnum);
        get_option_value_safe(optnum, &intval);
        fprintf(stderr, "\t%-20s: %d\n", opt->name, intval);
    }
}

void
set_options(ScanSettings settings)
{
    SANE_Int optnum;
    SANE_Int result;

    SANE_Int   intval;
    SANE_Fixed fixval;
    SANE_Bool  boolval;

    // Option 2: mode
    optnum = 2;
    result = set_option_value_safe(optnum, settings.mode);

    // Option 3: depth
    optnum = 3;
    intval = settings.depth;
    result = set_option_value_safe(optnum, &intval);

    // Option 4: resolution
    optnum = 4;
    fixval = SANE_FIX((double) settings.resolution);
    result = set_option_value_safe(optnum, &fixval);

    // Option 6: threshold
    optnum = 6;
    fixval = SANE_FIX((double) settings.threshold);
    result = set_option_value_safe(optnum, &fixval);

    // Option 7: sharpen
    optnum = 7;
    boolval = settings.sharpen;
    result = set_option_value_safe(optnum, &boolval);

    // Option 8: shading-analysis
    optnum = 8;
    boolval = settings.shading_analysis;
    result = set_option_value_safe(optnum, &boolval);

    // Option 9: fast-infrared
    optnum = 9;
    boolval = settings.fast_infrared;
    result = set_option_value_safe(optnum, &boolval);

    // Option 10: advcane
    optnum = 10;
    boolval = settings.auto_advance;
    result = set_option_value_safe(optnum, &boolval);

    // Option 11: calibration
    optnum = 11;
    result = set_option_value_safe(optnum, settings.calibration);

    // Option 13: top-left x
    optnum = 13;
    fixval = SANE_FIX(settings.tl_x);
    result = set_option_value_safe(optnum, &fixval);

    // Option 14: top-left y
    optnum = 14;
    fixval = SANE_FIX(settings.tl_y);
    result = set_option_value_safe(optnum, &fixval);

    // Option 15: bottom-right x
    optnum = 15;
    fixval = SANE_FIX(settings.br_x);
    result = set_option_value_safe(optnum, &fixval);

    // Option 16: bottom-right y
    optnum = 16;
    fixval = SANE_FIX(settings.br_y);
    result = set_option_value_safe(optnum, &fixval);

    // Option 18: correct-shading
    optnum = 18;
    boolval = settings.correct_shading;
    result = set_option_value_safe(optnum, &boolval);

    // Option 19: correct-infrared
    optnum = 19;
    boolval = settings.correct_infrared;
    result = set_option_value_safe(optnum, &boolval);

    // Option 20: clean-image
    optnum = 20;
    boolval = settings.clean_image;
    result = set_option_value_safe(optnum, &boolval);

    // Option 21: gain-adjust
    optnum = 21;
    result = set_option_value_safe(optnum, settings.gain_adjust);

    // Option 22: crop
    optnum = 22;
    result = set_option_value_safe(optnum, settings.crop);

    // Option 23: smooth
    optnum = 23;
    intval = settings.smooth;
    result = set_option_value_safe(optnum, &intval);

    // Option 27: preview
    optnum = 27;
    boolval = settings.preview;
    result = set_option_value_safe(optnum, &boolval);

    // Option 28: save-shading-data
    optnum = 28;
    boolval = settings.save_shading;
    result = set_option_value_safe(optnum, &boolval);

    // Option 29: save-ccdmask
    optnum = 29;
    boolval = settings.save_ccdmask;
    result = set_option_value_safe(optnum, &boolval);

    // Option 30: light
    optnum = 30;
    intval = settings.light;
    result = set_option_value_safe(optnum, &intval);

    // Option 31: double-times
    optnum = 31;
    intval = settings.double_times;
    result = set_option_value_safe(optnum, &intval);

    // Option 32: exposure-time-r
    optnum = 32;
    intval = settings.exposure_r;
    result = set_option_value_safe(optnum, &intval);

    // Option 33: exposure-time-g
    optnum = 33;
    intval = settings.exposure_g;
    result = set_option_value_safe(optnum, &intval);

    // Option 34: exposure-time-b
    optnum = 34;
    intval = settings.exposure_b;
    result = set_option_value_safe(optnum, &intval);

    // Option 35: exposure-time-i
    optnum = 35;
    intval = settings.exposure_i;
    result = set_option_value_safe(optnum, &intval);

    // Option 36: gain-r
    optnum = 36;
    intval = settings.gain_r;
    result = set_option_value_safe(optnum, &intval);

    // Option 37: gain-g
    optnum = 37;
    intval = settings.gain_g;
    result = set_option_value_safe(optnum, &intval);

    // Option 38: gain-b
    optnum = 38;
    intval = settings.gain_b;
    result = set_option_value_safe(optnum, &intval);

    // Option 39: gain-i
    optnum = 39;
    intval = settings.gain_i;
    result = set_option_value_safe(optnum, &intval);

    // Option 40: offset-r
    optnum = 40;
    intval = settings.offset_r;
    result = set_option_value_safe(optnum, &intval);

    // Option 41: offset-g
    optnum = 41;
    intval = settings.offset_g;
    result = set_option_value_safe(optnum, &intval);

    // Option 42: offset-b
    optnum = 42;
    intval = settings.offset_b;
    result = set_option_value_safe(optnum, &intval);

    // Option 43: offset-i
    optnum = 43;
    intval = settings.offset_i;
    result = set_option_value_safe(optnum, &intval);
}

void
open_device()
{
    SANE_Status status;
    const SANE_Device **device_list;
    const char* devname = 0;

    status = sane_get_devices (&device_list, SANE_FALSE);
    if (status != SANE_STATUS_GOOD) {
        fprintf(stderr, "Error: %s\n", sane_strstatus(status));
        piescan_exit(status);
    }
    if (!device_list[0]) {
        fprintf (stderr, "no SANE devices found\n");
        piescan_exit(status);
    }

    devname = device_list[0]->name;
    fprintf(stderr, "Device found: %s\n", devname);

    status = sane_open(devname, &device);

    if (status != SANE_STATUS_GOOD) {
        fprintf (stderr, "Failed to open device %s with error: %s\n",
           devname, sane_strstatus(status));
        piescan_exit(status);
    }
}

void
scan_image(Image* im, ScanSettings settings)
{
    SANE_Status status = SANE_STATUS_GOOD;
    SANE_Parameters parm;
    SANE_Byte* buffer;

    set_options(settings);
    fprintf(stderr, "Scanning image with settings: \n");
    print_options();

#ifdef SANE_STATUS_WARMING_UP
    do {
        fprintf(stderr, "Warming up...\n");
        status = sane_start(device);
    } while (status == SANE_STATUS_WARMING_UP);
#else
    fprintf(stderr, "Starting device\n");
    status = sane_start(device);
#endif

    if (status != SANE_STATUS_GOOD) {
        fprintf(stderr, "Error: %s\n", sane_strstatus(status));
        sane_cancel(device);
        piescan_exit(status);
    }

    status = sane_get_parameters(device, &parm);
    if (status != SANE_STATUS_GOOD) {
        fprintf(stderr, "Error: %s\n", sane_strstatus(status));
        sane_cancel(device);
        piescan_exit(status);
    }


    buffer = (SANE_Byte*) malloc(parm.bytes_per_line);
    resize_image(im, parm.pixels_per_line, parm.lines);

    int len;
    int line = 0;

    while (1) {
        status = sane_read(device, buffer, parm.bytes_per_line, &len);

        if (status == SANE_STATUS_EOF) break;

        if (status != SANE_STATUS_GOOD) {
            fprintf(stderr, "Error: %s\n", sane_strstatus(status));
            sane_cancel(device);
            free(buffer);
            piescan_exit(status);
        }

        if (len == 0) continue;

        fprintf(stderr, "Line %d of %d\t", line + 1, parm.lines);

        uint16_t* tmpbuf = (uint16_t*) buffer;
        for (int i = 0; i < parm.pixels_per_line; ++i) {
            im->r[i + line*parm.pixels_per_line] = tmpbuf[4*i];
            im->g[i + line*parm.pixels_per_line] = tmpbuf[4*i + 1];
            im->b[i + line*parm.pixels_per_line] = tmpbuf[4*i + 2];
            im->i[i + line*parm.pixels_per_line] = tmpbuf[4*i + 3];
        }

        fprintf(stderr, "finished\n");
        ++line;
    }

    sane_cancel(device);
    free(buffer);
}

Image*
new_image()
{
    Image* im = (Image*) malloc(sizeof(Image));
    im->width = 0;
    im->height = 0;
    im->r = (uint16_t*) malloc(sizeof(uint16_t));
    im->g = (uint16_t*) malloc(sizeof(uint16_t));
    im->b = (uint16_t*) malloc(sizeof(uint16_t));
    im->i = (uint16_t*) malloc(sizeof(uint16_t));

    return im;
}

void
resize_image(Image* im, uint32_t width, uint32_t height)
{
    im->width = width;
    im->height = height;
    im->r = (uint16_t*) realloc(im->r, im->width*im->height*sizeof(uint16_t));
    im->g = (uint16_t*) realloc(im->g, im->width*im->height*sizeof(uint16_t));
    im->b = (uint16_t*) realloc(im->b, im->width*im->height*sizeof(uint16_t));
    im->i = (uint16_t*) realloc(im->i, im->width*im->height*sizeof(uint16_t));
}

void
free_image(Image* im)
{
    free(im->r);
    free(im->g);
    free(im->b);
    free(im->i);
    free(im);
}

