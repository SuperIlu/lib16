/**
 * @file rawdisk.c
 * @author SuperIlu (superilu@yahoo.com)
 * @brief raw disk access functions
 *
 * @copyright SuperIlu
 */
#include <dos.h>
#include <stdio.h>

#include "rawdisk.h"
#include "error.h"

/************
** defines **
************/
#define LL_INT13_42_SIZE 0x10  //!< size of INT13,42 data structure
#define LL_INT13_48_SIZE 0x1A  //!< size of INT13,48 data structure

#define INT_EQUIPMENT 0x11  //!< BIOS equipment INT

#define INT_DISK 0x13                       //!< BIOS disk access INT
#define INT_DISK_STATUS 0x01                //!< get drive parameters FUNC
#define INT_DISK_READ_SECTORS 0x02          //!< read sectors FUNC
#define INT_DISK_WRITE_SECTORS 0x02         //!< read sectors FUNC
#define INT_DISK_DRIVE_PARAMETERS 0x08      //!< get drive parameters FUNC
#define INT_DISK_EXTENSION_CHECK 0x41       //!< extended functions FUNC
#define INT_DISK_READ_SECTORS_EXT 0x42      //!< read sectors extended FUNC
#define INT_DISK_WRITE_SECTORS_EXT 0x42     //!< read sectors extended FUNC
#define INT_DISK_DRIVE_PARAMETERS_EXT 0x48  //!< get extended drive parameters FUNC
#define INT_DISK_INSTALLED 0x41             //!< extension installed FUNC

#define BDA_NUM_HDD 0x75  //!< offset in BIOS DATA AREA: num hdd

/************
** structs **
************/
#pragma pack(__push, 1)  // make sure no padding is used
//!< see https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH=42h:_Extended_Read_Sectors_From_Drive
typedef struct {
    uint8_t size;
    uint8_t unused;
    uint16_t num_sectors;
    uint16_t offset;
    uint16_t segment;
    uint64_t start_sector;
} ll_int13_42_t;

//!< https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH=48h:_Extended_Read_Drive_Parameters
typedef struct {
    uint16_t size;
    uint16_t flags;
    uint32_t cylinders;
    uint32_t heads;
    uint32_t sectors_track;
    uint64_t total_sectors;
    uint16_t bytes_sector;
} ll_int13_48_t;
#pragma pack(__pop)  // end pack pragms

/* ======================================================================
** private variables
** ====================================================================== */
//! static buffer for disk data
static rawdisk_t rawdisk_data;

/* ======================================================================
** public functions
** ====================================================================== */

/**
 * @brief initialize raw disk access, determine number of disks.
 *
 * @return rawdisk_t* infostructure with the number of HDD/FDD
 */
rawdisk_t *rd_init() {
    union REGS regs;
    uint8_t *bda = (uint8_t *)0x0400;

    // get number of FDD, https://stanislavs.org/helppc/int_11.html
    int86(INT_EQUIPMENT, &regs, &regs);

    rawdisk_data.num_fdd = 0;
    if (0x01 & regs.x.ax) {
        rawdisk_data.num_fdd = 0x03 & ((regs.x.ax >> 6) + 1);
    }

    // get number of HDD, https://stanislavs.org/helppc/bios_data_area.html
    rawdisk_data.num_hdd = bda[BDA_NUM_HDD];

    return &rawdisk_data;
}

/**
 * @brief query disk status
 * @see http://www.ctyme.com/intr/rb-0606.htm#Table234
 *
 * @param drive drive number
 * @return status as returned by BIOS
 */
int rd_disk_status(uint8_t drive) {
    union REGS regs;

    regs.h.dl = drive;

    regs.h.ah = INT_DISK_STATUS;
    int86(INT_DISK, &regs, &regs);

    return 0xFF & regs.h.ah;
}

/**
 * @brief call int13,41 to check for 4x extensions
 *
 * @param drive drive number
 * @return true if the extension is detected, else false
 */
bool rd_extensions_check(uint8_t drive) {
    union REGS regs;

    regs.h.dl = drive;

    regs.h.ah = INT_DISK_EXTENSION_CHECK;  // http://www.ctyme.com/intr/rb-0706.htm
    int86(INT_DISK, &regs, &regs);

    if (regs.x.cflag & 1) { /* is carry flag set?  */
        ERR_DRIVR();
        return false;
    }

    ERR_OK();
    return regs.x.bx == 0xAA55 && regs.h.ah >= 0x01;  // check if installed and version >= 1.x
}

/**
 * @brief determine number of sectors for drives without int13,4x extension
 * Please use always the extended versions when available. This function calculates C/H/S from the given blocknumber and disk geometry which might be wrong for larger disks.
 *
 * @param drive drive number
 * @return uint32_t number of sectors or 0 for error
 */
uint32_t rd_drive_parameters(uint8_t drive) {
    union REGS regs;
    struct SREGS sregs;
    uint16_t heads;
    uint16_t sectors;
    uint32_t cylinders;

    regs.h.dl = drive;
    sregs.es = 0x00;
    regs.x.di = 0x00;

    regs.h.ah = INT_DISK_DRIVE_PARAMETERS;  // http://www.ctyme.com/intr/rb-0621.htm
    int86x(INT_DISK, &regs, &regs, &sregs);

    if (regs.x.cflag & 1) { /* is carry flag set?  */
        ERR_IOERR();
        return 0;
    }
    heads = regs.h.dh + 1;
    sectors = regs.h.cl & 0x3F;
    cylinders = ((uint32_t)regs.h.ch | ((uint32_t)(regs.h.cl & 0xC0) << 2)) + 1;

    ERR_OK();
    return cylinders * sectors * heads;
}

/**
 * @brief determine number of sectors for drives with int13,4x extension
 *
 * @param drive drive number
 * @return uint32_t number of sectors or 0 for error
 */
uint32_t rd_extended_drive_parameters(uint8_t drive) {
    char buff[LL_INT13_48_SIZE];
    union REGS regs;
    struct SREGS sregs;
    ll_int13_48_t *data = (ll_int13_48_t *)buff;

    regs.h.dl = drive;
    regs.x.si = FP_OFF((char far *)buff);  // buffer for result
    sregs.ds = FP_SEG((char far *)buff);

    regs.h.ah = INT_DISK_DRIVE_PARAMETERS_EXT;  // http://www.ctyme.com/intr/rb-0715.htm
    int86x(INT_DISK, &regs, &regs, &sregs);

    if (regs.x.cflag & 1) { /* is carry flag set?  */
        ERR_IOERR();
        return 0;
    }

    ERR_OK();
    return data->total_sectors;
}

/**
 * @brief read a sector for drives without in13,4x extension.
 * Please use always the extended versions when available. This function calculates C/H/S from the given blocknumber and disk geometry which might be wrong for larger disks.
 *
 * @param drive drive number
 * @param lba block number
 * @param buff destination buffer, must be able to hold RD_BLOCKSIZE*num_sectors bytes.
 * @param num_sectors number of sectors to read (muts be >0).
 * @return true if reading is successfull, else false
 */
bool rd_read_sector(uint8_t drive, uint32_t lba, char *buff, uint8_t num_sectors) {
    union REGS regs;
    struct SREGS sregs;
    uint16_t heads;
    uint16_t sectors;
    uint16_t cylinder;
    uint16_t temp;
    uint16_t head;
    uint16_t sector;

    // get drive info
    regs.h.dl = drive;
    sregs.es = 0x00;
    regs.x.di = 0x00;

    regs.h.ah = INT_DISK_DRIVE_PARAMETERS;  // http://www.ctyme.com/intr/rb-0621.htm
    int86x(INT_DISK, &regs, &regs, &sregs);

    if (regs.x.cflag & 1) { /* is carry flag set?  */
        ERR_IOERR();
        return false;
    }
    heads = regs.h.dh + 1;
    sectors = regs.h.cl & 0x3F;

    // calculate CHS
    cylinder = lba / (heads * sectors);
    temp = lba % (heads * sectors);
    head = temp / sectors;
    sector = temp % sectors + 1;

    // read sector
    regs.h.al = num_sectors;
    regs.h.dl = drive;
    regs.h.ch = cylinder & 0xff;
    regs.h.cl = sector | ((cylinder >> 2) & 0xc0);
    regs.h.dh = head;
    regs.x.bx = FP_OFF((char far *)buff);  // buffer for result
    sregs.es = FP_SEG((char far *)buff);

    regs.h.ah = INT_DISK_READ_SECTORS;  // http://www.ctyme.com/intr/rb-0607.htm
    int86x(INT_DISK, &regs, &regs, &sregs);

    if (regs.x.cflag & 1) { /* is carry flag set?  */
        ERR_IOERR();
        return false;
    }

    ERR_OK();
    return true;
}

/**
 * @brief write a sector for drives without in13,4x extension.
 * Please use always the extended versions when available. This function calculates C/H/S from the given blocknumber and disk geometry which might be wrong for larger disks.
 *
 * @param drive drive number
 * @param lba block number
 * @param buff source buffer, must hold at least LL_BLOCKSIZE*num_sectors bytes.
 * @param num_sectors number of sectors to read (muts be >0).
 * @return true if writing is successfull, else false
 */
bool rd_write_sector(uint8_t drive, uint32_t lba, char *buff, uint8_t num_sectors) {
    union REGS regs;
    struct SREGS sregs;
    uint16_t heads;
    uint16_t sectors;
    uint16_t cylinder;
    uint16_t temp;
    uint16_t head;
    uint16_t sector;

    // get drive info
    regs.h.dl = drive;
    sregs.es = 0x00;
    regs.x.di = 0x00;

    regs.h.ah = INT_DISK_DRIVE_PARAMETERS;  // http://www.ctyme.com/intr/rb-0621.htm
    int86x(INT_DISK, &regs, &regs, &sregs);

    if (regs.x.cflag & 1) { /* is carry flag set?  */
        ERR_IOERR();
        return false;
    }
    heads = regs.h.dh + 1;
    sectors = regs.h.cl & 0x3F;

    // calculate CHS
    cylinder = lba / (heads * sectors);
    temp = lba % (heads * sectors);
    head = temp / sectors;
    sector = temp % sectors + 1;

    // write sector
    regs.h.al = num_sectors;
    regs.h.dl = drive;
    regs.h.ch = cylinder & 0xff;
    regs.h.cl = sector | ((cylinder >> 2) & 0xc0);
    regs.h.dh = head;
    regs.x.bx = FP_OFF((char far *)buff);  // buffer with data
    sregs.es = FP_SEG((char far *)buff);

    regs.h.ah = INT_DISK_WRITE_SECTORS;  // http://www.ctyme.com/intr/rb-0608.htm
    int86x(INT_DISK, &regs, &regs, &sregs);

    if (regs.x.cflag & 1) { /* is carry flag set?  */
        ERR_IOERR();
        return false;
    }

    ERR_OK();
    return true;
}

/**
 * @brief read a sector for drives with in13,4x extension
 *
 * @param drive drive number
 * @param lba block number
 * @param buff destination buffer, must be able to hold RD_BLOCKSIZE bytes.
 * @param num_sectors number of sectors to read (muts be >0).
 * @return true if reading is successfull, else false
 */
bool rd_extended_read(uint8_t drive, uint64_t lba, char *buff, uint8_t num_sectors) {
    union REGS regs;
    struct SREGS sregs;
    ll_int13_42_t data;

    // fill in disk address packet
    data.size = LL_INT13_42_SIZE;
    data.offset = FP_OFF((char far *)buff);
    data.segment = FP_SEG((char far *)buff);
    data.num_sectors = num_sectors;
    data.start_sector = lba;

    // fill in registers
    regs.h.dl = drive;
    regs.x.si = FP_OFF((char far *)&data);
    sregs.ds = FP_SEG((char far *)&data);

    regs.h.ah = INT_DISK_READ_SECTORS_EXT;  // http://www.ctyme.com/intr/rb-0708.htm
    int86x(INT_DISK, &regs, &regs, &sregs);

    if (regs.x.cflag & 1) { /* is carry flag set?  */
        ERR_IOERR();
        return false;
    }

    ERR_OK();
    return true;
}

/**
 * @brief write a sector for drives with in13,4x extension
 *
 * @param drive drive number
 * @param lba block number
 * @param buff data buffer, must be at least RD_BLOCKSIZE bytes.
 * @param num_sectors number of sectors to read (muts be >0).
 *
 * @return true if writing is successfull, else false
 */
bool rd_extended_write(uint8_t drive, uint64_t lba, char *buff, uint8_t num_sectors) {
    union REGS regs;
    struct SREGS sregs;
    ll_int13_42_t data;

    // fill in disk address packet
    data.size = LL_INT13_42_SIZE;  // same struct as for reading
    data.offset = FP_OFF((char far *)buff);
    data.segment = FP_SEG((char far *)buff);
    data.num_sectors = num_sectors;
    data.start_sector = lba;

    regs.h.dl = drive;
    regs.x.si = FP_OFF((char far *)&data);
    sregs.ds = FP_SEG((char far *)&data);
    regs.h.al = 0;

    regs.h.ah = INT_DISK_WRITE_SECTORS_EXT;  // http://www.ctyme.com/intr/rb-0710.htm
    int86x(INT_DISK, &regs, &regs, &sregs);

    if (regs.x.cflag & 1) { /* is carry flag set?  */
        return false;
    }
    return true;
}
