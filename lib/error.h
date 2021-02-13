/**
 * @file error.h
 * @author SuperIlu (superilu@yahoo.com)
 * @brief errno-clone
 *
 * @copyright SuperIlu
 */
#ifndef __ERROR_H_
#define __ERROR_H_

#ifndef NO_ERRORS
/* ======================================================================
** defines
** ====================================================================== */
//!< set err_no and err_str
#define ERR_ERROR(x)              \
    {                             \
        err_no = x;               \
        err_str = err_strings[x]; \
    }

#define ERR_OK() ERR_ERROR(ERR_OK)        //!< see error_t enum
#define ERR_NOENT() ERR_ERROR(ERR_NOENT)  //!< see error_t enum
#define ERR_NOMEM() ERR_ERROR(ERR_NOMEM)  //!< see error_t enum
#define ERR_PARAM() ERR_ERROR(ERR_PARAM)  //!< see error_t enum
#define ERR_DRIVR() ERR_ERROR(ERR_DRIVR)  //!< see error_t enum
#define ERR_AVAIL() ERR_ERROR(ERR_AVAIL)  //!< see error_t enum
#define ERR_IOERR() ERR_ERROR(ERR_IOERR)  //!< see error_t enum
#define ERR_CREAT() ERR_ERROR(ERR_CREAT)  //!< see error_t enum

/* ======================================================================
** typedefs
** ====================================================================== */
typedef enum __error {
    ERR_OK = 0,     // all ok
    ERR_NOMEM = 1,  // no memory
    ERR_NOENT = 2,  // file not found
    ERR_PARAM = 3,  // illegal parameters
    ERR_DRIVR = 4,  // driver not found
    ERR_AVAIL = 5,  // function not available
    ERR_IOERR = 6,  // io error
    ERR_CREAT = 7,  // can't create file
    ERR_SIZE = 8
} error_t;

/* ======================================================================
** global variables
** ====================================================================== */
extern const char *err_strings[ERR_SIZE];
extern error_t err_no;
extern const char *err_str;

#else  // NO_ERRORS
#define ERR_OK()
#define ERR_NOENT()
#define ERR_NOMEM()
#define ERR_PARAM()
#define ERR_DRIVR()
#define ERR_AVAIL()
#define ERR_IOERR()
#define ERR_CREAT()
#endif  // NO_ERRORS

#endif  // __ERROR_H_
