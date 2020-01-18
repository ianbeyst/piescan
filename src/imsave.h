#ifndef IMSAVE_H
#define IMSAVE_H

#include <inttypes.h>



int imsave8(uint8_t* buf, const size_t width, const size_t height,
            const size_t n_channels, const char* fname);
int imsave16(uint16_t* buf, const size_t width, const size_t height,
             const size_t n_channels, const char* fname);



#endif  // IMSAVE_H
