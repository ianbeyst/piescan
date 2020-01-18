#ifndef MMAPARRAY_H
#define MMAPARRAY_H

#ifdef __cplusplus
extern "C" {
#endif



/*****************************************************************************\
| Type definitions                                                            |
\*****************************************************************************/

typedef struct {
    void* data;
    size_t size;
} MmapArray;



/*****************************************************************************\
| Function declarations                                                       |
\*****************************************************************************/

void free_mmap_array(MmapArray* arr);
MmapArray* get_mmap_writer(const char* filename, const size_t size);
MmapArray* get_mmap_reader(const char* filename);



#ifdef __cplusplus
}  // extern "C"
#endif


#endif  // MMAPARRAY_H
