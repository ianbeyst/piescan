/* write simple bitmap buffer to PNG file.
 * Inspired by https://www.lemoda.net/c/write-png/
 */


#include <inttypes.h>

void set_filename(const char* fname);
int imsave(uint8_t* buf, const size_t width, const size_t height,
           const size_t n_channels);
int imsave16(uint16_t* buf, const size_t width, const size_t height,
             const size_t n_channels);

