#include <stdbool.h>
#include <inttypes.h>



/*****************************************************************************\
| Type definitions                                                            |
\*****************************************************************************/

typedef struct {
    // String options
    char* mode;
    char* calibration;
    char* gain_adjust;
    char* crop;

    // Fixed point options
    int resolution;
    int threshold;
    double tl_x;
    double tl_y;
    double br_x;
    double br_y;

    // Boolean options
    bool sharpen;
    bool shading_analysis;
    bool fast_infrared;
    bool auto_advance;
    bool correct_shading;
    bool correct_infrared;
    bool clean_image;
    bool preview;
    bool save_shading;
    bool save_ccdmask;

    // Integer options
    int depth;
    int smooth;
    int light;
    int double_times;
    int exposure_r;
    int exposure_g;
    int exposure_b;
    int exposure_i;
    int gain_r;
    int gain_g;
    int gain_b;
    int gain_i;
    int offset_r;
    int offset_g;
    int offset_b;
    int offset_i;
} ScanSettings;

typedef struct {
    uint32_t width;
    uint32_t height;

    uint16_t* r;
    uint16_t* g;
    uint16_t* b;
    uint16_t* i;
} Image;



/*****************************************************************************\
| Function declarations                                                       |
\*****************************************************************************/

void piescan_open(void);
void piescan_close(void);
ScanSettings get_default_settings();
void print_options();
void scan_image(Image* im, ScanSettings settings);
Image* new_image();
void free_image(Image* im);
