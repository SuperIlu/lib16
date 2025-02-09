/**
 * @file bitmap.c
 * @author SuperIlu (superilu@yahoo.com)
 * @brief utility functions
 *
 * @copyright SuperIlu
 */
#ifndef __UTIL_H_
#define __UTIL_H_

#include <stdio.h>
#include <stdbool.h>

#define UTIL_KILOBYTE(x) (1024U * (unsigned int)x)
#define UTIL_MEGABYTE(x) UTIL_KILOBYTE(1024U * (unsigned int)x)

extern bool util_read_file(const char *fname, void **buf, size_t *size);
extern long int util_filesize(FILE *fp);

#endif  // __UTIL_H_
