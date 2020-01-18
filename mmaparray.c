#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <sys/stat.h>

#include "mmaparray.h"



/*****************************************************************************\
| Function declarations                                                       |
\*****************************************************************************/

void
free_mmap_array(MmapArray* arr)
{
    munmap(arr->data, arr->size);
    free(arr);
}

// TODO: sane filesize handling
MmapArray*
get_mmap_writer(const char* filename, const size_t size)
{
    MmapArray* result = (MmapArray*) malloc(sizeof(MmapArray));
    result->size = size;

    int fd = open(filename,
                  O_RDWR|O_CREAT|O_TRUNC,
                  (mode_t)0600);
    if(fd < 0) {
        printf("Error: %d: unable to open file %s\n", errno, filename);
    }

    // TODO: only perform this (making file big enough) if it's not already so!
    if(lseek(fd, size - 1, SEEK_SET) == -1) {
        close(fd);
        printf("Error: %d: unable to create file %s\n", errno, filename);
    }
    if(write(fd, "", 1) == -1) {
        close(fd);
        printf("Error: %d: unable to create file %s\n", errno, filename);
    }

    result->data = mmap(0,
                        size,
                        PROT_READ|PROT_WRITE,
                        MAP_FILE|MAP_SHARED,
                        fd,
                        0);
    if(result == MAP_FAILED) {
        printf("Error %d: unable to memory map file %s\n", errno, filename);
    }

    // TODO: Effect of madvise here?

    close(fd);
    return result;
}

MmapArray*
get_mmap_reader(const char* filename)
{
    MmapArray* result = (MmapArray*) malloc(sizeof(MmapArray));
    struct stat buf;

    int fd = open(filename,
                  O_RDONLY);
    if(fd < 0) {
        printf("Error %d: unable to open file %s\n", errno, filename);
    }

    if (fstat(fd,&buf) < 0) {
        printf("Error %d: unable to determine file size of file %s\n", errno, filename);
    }
    result->size = buf.st_size;

    result->data = mmap(0,
                        result->size,
                        PROT_READ,
                        MAP_FILE|MAP_SHARED,
                        fd,
                        0);

    if(result == MAP_FAILED) {
        printf("Error %d: unable to memory map file %s\n", errno, filename);
    }

    if(madvise(result->data, result->size, MADV_SEQUENTIAL|MADV_WILLNEED) != 0) {
        printf("Error %d: failed to set madvise for file %s\n", errno, filename);
    }

    close(fd);
    return result;
}

