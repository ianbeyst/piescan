#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <sane/sane.h>

#include "imsave.h"



typedef struct {
    char* mode;
    char* calibration;
    char* gain_adjust;
    char* crop;

    SANE_Fixed resolution;
    SANE_Fixed threshold;

    SANE_Bool sharpen;
    SANE_Bool shading_analysis;
    SANE_Bool fast_infrared;
    SANE_Bool auto_advance;
    SANE_Bool correct_shading;
    SANE_Bool correct_infrared;
    SANE_Bool clean_image;
    SANE_Bool preview;
    SANE_Bool save_shading;
    SANE_Bool save_ccdmask;

    SANE_Int depth;
    SANE_Int smooth;
    SANE_Int light;
    SANE_Int double_times;
    SANE_Int exposure_r;
    SANE_Int exposure_g;
    SANE_Int exposure_b;
    SANE_Int exposure_i;
    SANE_Int gain_r;
    SANE_Int gain_g;
    SANE_Int gain_b;
    SANE_Int gain_i;
    SANE_Int offset_r;
    SANE_Int offset_g;
    SANE_Int offset_b;
    SANE_Int offset_i;
} ScanSettings;



static SANE_Handle device;



static void
auth_callback(SANE_String_Const resource,
               SANE_Char * username, SANE_Char * password)
{
    fprintf(stderr, "Entered auth_callback procedure!\n");
}

static void
piescan_exit(int status)
{
    if (device) {
        fprintf(stderr, "Closing device\n");
        sane_close(device);
    }
    fprintf(stderr, "Calling sane_exit\n");
    sane_exit();

    fprintf(stderr, "finished\n");
    exit(status);
}

const SANE_Option_Descriptor*
get_option_descriptor_safe(SANE_Int i)
{
    const SANE_Option_Descriptor* opt = sane_get_option_descriptor(device, i);
    if (!opt) {
        fprintf(stderr, "unable to get option descriptor\n");
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
        fprintf(stderr, "Unable to determine option value\n");
        piescan_exit(1);
    }
}

SANE_Int
set_option_value_safe(SANE_Int i, void* v)
{
    SANE_Int result = 0;
    SANE_Status status = sane_control_option(device, i, SANE_ACTION_SET_VALUE,
                                             v, &result);
    if (status != SANE_STATUS_GOOD) {
        fprintf(stderr, "Unable to determine option value\n");
        piescan_exit(1);
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

    settings.resolution = SANE_FIX(300.0);
    settings.threshold = SANE_FIX(50.0);

    settings.sharpen = SANE_FALSE;
    settings.shading_analysis = SANE_FALSE;
    settings.fast_infrared = SANE_FALSE;
    settings.auto_advance = SANE_FALSE;
    settings.correct_shading = SANE_FALSE;
    settings.correct_infrared = SANE_FALSE;
    settings.clean_image = SANE_FALSE;
    settings.preview = SANE_FALSE;
    settings.save_shading = SANE_FALSE;
    settings.save_ccdmask = SANE_FALSE;

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
        fprintf(stderr, "%-20s: %s\n", opt->name, strval);
    }

    fprintf(stderr, "\n");

    for (int i = 0; i < 6; ++i) {
        SANE_Int optnum = options_fix[i];
        opt = get_option_descriptor_safe(optnum);
        get_option_value_safe(optnum, &fixval);
        fprintf(stderr, "%-20s: %.0f\n", opt->name, SANE_UNFIX(fixval));
    }

    fprintf(stderr, "\n");

    for (int i = 0; i < 26; ++i) {
        SANE_Int optnum = options_int[i];
        opt = get_option_descriptor_safe(optnum);
        get_option_value_safe(optnum, &intval);
        fprintf(stderr, "%-20s: %d\n", opt->name, intval);
    }
}

void
set_options(ScanSettings settings)
{
    SANE_Int optnum;
    SANE_Int result;

    // Option 2: mode
    optnum = 2;
    result = set_option_value_safe(optnum, settings.mode);

    // Option 3: depth
    optnum = 3;
    result = set_option_value_safe(optnum, &settings.depth);

    // Option 4: resolution
    optnum = 4;
    result = set_option_value_safe(optnum, &settings.resolution);

    // Option 6: threshold
    optnum = 6;
    result = set_option_value_safe(optnum, &settings.threshold);

    // Option 7: sharpen
    optnum = 7;
    result = set_option_value_safe(optnum, &settings.sharpen);

    // Option 8: shading-analysis
    optnum = 8;
    result = set_option_value_safe(optnum, &settings.shading_analysis);

    // Option 9: fast-infrared
    optnum = 9;
    result = set_option_value_safe(optnum, &settings.fast_infrared);

    // Option 10: advcane
    optnum = 10;
    result = set_option_value_safe(optnum, &settings.auto_advance);

    // Option 11: calibration
    optnum = 11;
    result = set_option_value_safe(optnum, settings.calibration);

    // Option 18: correct-shading
    optnum = 18;
    result = set_option_value_safe(optnum, &settings.correct_shading);

    // Option 19: correct-infrared
    optnum = 19;
    result = set_option_value_safe(optnum, &settings.correct_infrared);

    // Option 20: clean-image
    optnum = 20;
    result = set_option_value_safe(optnum, &settings.clean_image);

    // Option 21: gain-adjust
    optnum = 21;
    result = set_option_value_safe(optnum, settings.gain_adjust);

    // Option 22: crop
    optnum = 22;
    result = set_option_value_safe(optnum, settings.crop);

    // Option 23: smooth
    optnum = 23;
    result = set_option_value_safe(optnum, &settings.smooth);

    // Option 27: preview
    optnum = 27;
    result = set_option_value_safe(optnum, &settings.preview);

    // Option 28: save-shading-data
    optnum = 28;
    result = set_option_value_safe(optnum, &settings.save_shading);

    // Option 29: save-ccdmask
    optnum = 29;
    result = set_option_value_safe(optnum, &settings.save_ccdmask);

    // Option 30: light
    optnum = 30;
    result = set_option_value_safe(optnum, &settings.light);

    // Option 31: double-times
    optnum = 31;
    result = set_option_value_safe(optnum, &settings.double_times);

    // Option 32: exposure-time-r
    optnum = 32;
    result = set_option_value_safe(optnum, &settings.exposure_r);

    // Option 33: exposure-time-g
    optnum = 33;
    result = set_option_value_safe(optnum, &settings.exposure_g);

    // Option 34: exposure-time-b
    optnum = 34;
    result = set_option_value_safe(optnum, &settings.exposure_b);

    // Option 35: exposure-time-i
    optnum = 35;
    result = set_option_value_safe(optnum, &settings.exposure_i);

    // Option 36: gain-r
    optnum = 36;
    result = set_option_value_safe(optnum, &settings.gain_r);

    // Option 37: gain-g
    optnum = 37;
    result = set_option_value_safe(optnum, &settings.gain_g);

    // Option 38: gain-b
    optnum = 38;
    result = set_option_value_safe(optnum, &settings.gain_b);

    // Option 39: gain-i
    optnum = 39;
    result = set_option_value_safe(optnum, &settings.gain_i);

    // Option 40: offset-r
    optnum = 40;
    result = set_option_value_safe(optnum, &settings.offset_r);

    // Option 41: offset-g
    optnum = 41;
    result = set_option_value_safe(optnum, &settings.offset_g);

    // Option 42: offset-b
    optnum = 42;
    result = set_option_value_safe(optnum, &settings.offset_b);

    // Option 43: offset-i
    optnum = 43;
    result = set_option_value_safe(optnum, &settings.offset_i);
}

void
open_device()
{
    SANE_Status status;
    const SANE_Device **device_list;
    const char* devname = 0;

    status = sane_get_devices (&device_list, SANE_FALSE);
    if (status != SANE_STATUS_GOOD) {
        fprintf (stderr, "sane_get_devices() failed: %s\n", sane_strstatus(status));
        piescan_exit(1);
    }
    if (!device_list[0]) {
        fprintf (stderr, "no SANE devices found\n");
        piescan_exit(1);
    }

    devname = device_list[0]->name;
    printf("Device found: %s\n", devname);

    status = sane_open(devname, &device);

    if (status != SANE_STATUS_GOOD) {
        fprintf (stderr, "Open of device %s failed: %s\n",
           devname, sane_strstatus(status));
        piescan_exit(1);
    }
}

int main()
{
    SANE_Int version_code;
    SANE_Status status = SANE_STATUS_GOOD;

    sane_init(&version_code, auth_callback);

    open_device();

    ScanSettings settings = get_default_settings();
    settings.resolution = SANE_FIX(7200.0);
    settings.gain_r = 23;
    settings.gain_g = 23;
    settings.gain_b = 23;
    settings.gain_i = 23;
    settings.exposure_r = 10000;
    settings.exposure_g = 10000;
    settings.exposure_b = 10000;
    settings.exposure_i = 10000;
    set_options(settings);
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
        fprintf (stderr, "Sane_start: %s\n", sane_strstatus(status));
        sane_cancel(device);
        piescan_exit(1);
    }

    SANE_Parameters parm;
    status = sane_get_parameters (device, &parm);

    SANE_Byte* buffer = (SANE_Byte*) malloc(parm.bytes_per_line);

    uint16_t* im_r;
    uint16_t* im_g;
    uint16_t* im_b;
    uint16_t* im_i;
    im_r = (uint16_t*) malloc(parm.pixels_per_line*parm.lines*sizeof(uint16_t));
    im_g = (uint16_t*) malloc(parm.pixels_per_line*parm.lines*sizeof(uint16_t));
    im_b = (uint16_t*) malloc(parm.pixels_per_line*parm.lines*sizeof(uint16_t));
    im_i = (uint16_t*) malloc(parm.pixels_per_line*parm.lines*sizeof(uint16_t));
    int len;
    int line = 0;

    while (1) {
        status = sane_read(device, buffer, parm.bytes_per_line, &len);

        if (status == SANE_STATUS_EOF) {
            status = SANE_STATUS_GOOD;
            break;
        }

        if (status != SANE_STATUS_GOOD) {
            fprintf(stderr, "error: %s\n", sane_strstatus(status));
            piescan_exit(status);
        }

        if (len == 0)
            continue;

        fprintf(stderr, "Line %d of %d\n", line + 1, parm.lines);

        uint16_t* tmpbuf = (uint16_t*) buffer;
        for (int i = 0; i < parm.pixels_per_line; ++i) {
            im_r[i + line*parm.pixels_per_line] = tmpbuf[4*i];
            im_g[i + line*parm.pixels_per_line] = tmpbuf[4*i + 1];
            im_b[i + line*parm.pixels_per_line] = tmpbuf[4*i + 2];
            im_i[i + line*parm.pixels_per_line] = tmpbuf[4*i + 3];
        }
        ++line;
    }

    set_filename("r.png");
    imsave16(im_r, parm.pixels_per_line, parm.lines, 1);
    set_filename("g.png");
    imsave16(im_g, parm.pixels_per_line, parm.lines, 1);
    set_filename("b.png");
    imsave16(im_b, parm.pixels_per_line, parm.lines, 1);
    set_filename("i.png");
    imsave16(im_i, parm.pixels_per_line, parm.lines, 1);

    sane_cancel(device);

    piescan_exit(status);
}
