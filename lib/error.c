/**
 * @file error.c
 * @author SuperIlu (superilu@yahoo.com)
 * @brief errno-clone
 *
 * @copyright SuperIlu
 */
#include "error.h"

#ifndef NO_ERRORS
/* ======================================================================
** global variables
** ====================================================================== */
//! stringf for the error_t errors
const char *err_strings[ERR_SIZE] = {
    "",                   // all ok
    "Out of memory",      // no memory
    "File not found",     // file not found
    "Illegal parameter",  // illegal parameters
    "Driver not found",   // driver not found
    "Not available",      // function not available
    "IO error",           // io error
    "Can't create",       // Can't create file
};

//! global error variable
error_t err_no = ERR_OK;

//! global error string variable
const char *err_str = "";
#endif  // NO_ERRORS
