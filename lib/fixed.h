/**
 * @file fixed.h
 * @author SuperIlu (superilu@yahoo.com)
 * @brief simple fixed math
 *
 * @copyright SuperIlu
 */
#ifndef __FIXED_H_
#define __FIXED_H_

/* ======================================================================
** typedefs
** ====================================================================== */
typedef long fixed16_16;  //!< used for fixed-point math

/* ======================================================================
** defines
** ====================================================================== */
#define FIXED_POINT_FACTOR 0x10000L  //!< factor for the fixed point implementation

#define TO_FIXED(x) ((x)*FIXED_POINT_FACTOR)
#define FROM_FIXED_F(x) (((float)x) / FIXED_POINT_FACTOR)
#define FROM_FIXED_I(x) ((x) / FIXED_POINT_FACTOR)

#endif  // __FIXED_H_
