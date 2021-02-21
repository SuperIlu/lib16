/**
 * @file rawdisk.h
 * @author SuperIlu (superilu@yahoo.com)
 * @brief raw disk access functions
 *
 * @copyright SuperIlu
 */
#ifndef __RAWDISK_H_
#define __RAWDISK_H_

#include <stdbool.h>
#include <stdint.h>

/* ======================================================================
** defines
** ====================================================================== */
#define RD_BLOCKSIZE 514      //!< block size is hardcoded to 512+2 byte for now, see http://www.ctyme.com/intr/rb-0607.htm
#define RD_HDD_FLAG (1 << 7)  //!< HDDs start at 0x80, FDDs at 0

/* ======================================================================
** typedefs
** ====================================================================== */
//! rawdisk info struct
typedef struct __rawdisk {
    uint8_t num_fdd;  //!< number of floppy drives
    uint8_t num_hdd;  //!< number of harddrives
} rawdisk_t;

/* ======================================================================
** prototypes
** ====================================================================== */
extern rawdisk_t *rd_init();
extern int rd_disk_status(uint8_t drive);
extern bool rd_extensions_check(uint8_t drive);
extern uint32_t rd_drive_parameters(uint8_t drive);
extern uint32_t rd_extended_drive_parameters(uint8_t drive);
extern bool rd_read_sector(uint8_t drive, uint32_t lba, char *buff, uint8_t num_sectors);
extern bool rd_write_sector(uint8_t drive, uint32_t lba, char *buff, uint8_t num_sectors);
extern bool rd_extended_read(uint8_t drive, uint64_t lba, char *buff, uint8_t num_sectors);
extern bool rd_extended_write(uint8_t drive, uint64_t lba, char *buff, uint8_t num_sectors);

#endif  // __RAWDISK_H_
