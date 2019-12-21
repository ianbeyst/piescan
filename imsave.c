#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include "imsave.h"

static char filename[128];

void
set_filename(const char* fname) {
    sprintf(filename, "%s", fname);
}

int
imsave16(uint16_t* buf,
         const size_t width,
         const size_t height,
         const size_t n_channels)
{
    size_t bit_depth = 16;
    FILE * fp;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_byte** row_pointers = NULL;
    size_t x, y;

    int status = -1;

    fp = fopen (filename, "wb");
    if (! fp) {
        goto fopen_failed;
    }

    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        goto png_create_write_struct_failed;
    }

    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL) {
        goto png_create_info_struct_failed;
    }

    if (setjmp (png_jmpbuf (png_ptr))) {
        goto png_failure;
    }

    switch (n_channels) {
        case 1:
            png_set_IHDR (png_ptr,
                          info_ptr,
                          width,
                          height,
                          bit_depth,
                          PNG_COLOR_TYPE_GRAY,
                          PNG_INTERLACE_NONE,
                          PNG_COMPRESSION_TYPE_DEFAULT,
                          PNG_FILTER_TYPE_DEFAULT);
            break;
        case 3:
            png_set_IHDR (png_ptr,
                          info_ptr,
                          width,
                          height,
                          bit_depth,
                          PNG_COLOR_TYPE_RGB,
                          PNG_INTERLACE_NONE,
                          PNG_COMPRESSION_TYPE_DEFAULT,
                          PNG_FILTER_TYPE_DEFAULT);
            break;
        default:
            goto png_failure;
    }

    row_pointers = (png_byte**) png_malloc(png_ptr, height * sizeof (png_byte *));
    for (y = 0; y < height; y++) {
        png_byte *row = (png_byte*) png_malloc(png_ptr,
                                            sizeof (int) * width * n_channels);
        row_pointers[y] = row;
        for (x = 0; x < width; x++) {
            for (size_t c = 0; c < n_channels; ++c) {
                uint16_t color = buf[c + n_channels * (x + width * y)];
                *row++ = (png_byte)(color >> 8);
                *row++ = (png_byte)(color & 0xFF);
            }
        }
    }

    png_init_io (png_ptr, fp);
    png_set_rows (png_ptr, info_ptr, row_pointers);
    png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    status = 0;

    for (y = 0; y < height; y++) {
        png_free (png_ptr, row_pointers[y]);
    }
    png_free (png_ptr, row_pointers);

 png_failure:
 png_create_info_struct_failed:
    png_destroy_write_struct (&png_ptr, &info_ptr);
 png_create_write_struct_failed:
    fclose (fp);
 fopen_failed:
    return status;

}

int
imsave8(uint8_t* buf,
        const size_t width,
        const size_t height,
        const size_t n_channels)
{
    size_t bit_depth = 8;
    FILE * fp;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_byte** row_pointers = NULL;
    size_t x, y;

    int status = -1;

    fp = fopen (filename, "wb");
    if (! fp) {
        goto fopen_failed;
    }

    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        goto png_create_write_struct_failed;
    }

    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL) {
        goto png_create_info_struct_failed;
    }

    if (setjmp (png_jmpbuf (png_ptr))) {
        goto png_failure;
    }

    switch (n_channels) {
        case 1:
            png_set_IHDR (png_ptr,
                          info_ptr,
                          width,
                          height,
                          bit_depth,
                          PNG_COLOR_TYPE_GRAY,
                          PNG_INTERLACE_NONE,
                          PNG_COMPRESSION_TYPE_DEFAULT,
                          PNG_FILTER_TYPE_DEFAULT);
            break;
        case 3:
            png_set_IHDR (png_ptr,
                          info_ptr,
                          width,
                          height,
                          bit_depth,
                          PNG_COLOR_TYPE_RGB,
                          PNG_INTERLACE_NONE,
                          PNG_COMPRESSION_TYPE_DEFAULT,
                          PNG_FILTER_TYPE_DEFAULT);
            break;
        default:
            goto png_failure;
    }

    row_pointers = (png_byte**) png_malloc(png_ptr, height * sizeof (png_byte *));
    for (y = 0; y < height; y++) {
        png_byte *row = (png_byte*) png_malloc(png_ptr,
                                            sizeof (int) * width * n_channels);
        row_pointers[y] = row;
        for (x = 0; x < width; x++) {
            for (size_t c = 0; c < n_channels; ++c) {
                *row++ = buf[c + n_channels * (x + width * y)];
            }
        }
    }

    png_init_io (png_ptr, fp);
    png_set_rows (png_ptr, info_ptr, row_pointers);
    png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    status = 0;

    for (y = 0; y < height; y++) {
        png_free (png_ptr, row_pointers[y]);
    }
    png_free (png_ptr, row_pointers);

 png_failure:
 png_create_info_struct_failed:
    png_destroy_write_struct (&png_ptr, &info_ptr);
 png_create_write_struct_failed:
    fclose (fp);
 fopen_failed:
    return status;
}

