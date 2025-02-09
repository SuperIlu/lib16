/**
 * @file bitmap.c
 * @author SuperIlu (superilu@yahoo.com)
 * @brief utility functions
 *
 * @copyright SuperIlu
 */
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "error.h"
#include "util.h"

/**
 * @brief get size of file.
 *
 * @param fp file object.
 * @return size_t size in bytes.
 */
long int util_filesize(FILE *fp) {
    long int old_pos, size;

    old_pos = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    fseek(fp, old_pos, SEEK_SET);

    return size;
}

/**
 * @brief read a file into memory. The read file is always null terminated, but 'size' always represents the original file size.
 *
 * @param fname a filename.
 * @param buf pointer to the read data. must be freed by caller.
 * @param size the real file size (without terminating null byte)
 *
 * @return true if the file could be read, buf and size will return valid values
 * @return false if an error occured, buf and size will be 0.
 */
bool util_read_file(const char *fname, void **buf, size_t *size) {
    FILE *f;
    char *s;
    long int n, t;

    *buf = NULL;
    *size = 0;

    f = fopen(fname, "rb");
    if (!f) {
        ERR_NOENT();
        return false;
    }

    n = util_filesize(f);

    s = malloc(n + 1);
    if (!s) {
        fclose(f);
        ERR_NOMEM();
        return false;
    }

    t = fread(s, 1, n, f);
    if (t != n) {
        free(s);
        fclose(f);
        ERR_IOERR();
        return false;
    }
    s[n] = 0;
    fclose(f);

    *buf = s;
    *size = n;
    return true;
}
