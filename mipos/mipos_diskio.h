/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */


/* ------------------------------------------------------------------------- */

#ifndef __MIPOS_DISKIO_H__
#define __MIPOS_DISKIO_H__


/* ------------------------------------------------------------------------- */

typedef unsigned int mipos_pdrv_t;
typedef unsigned int mipos_sec_t;
typedef int mipos_ioctl_cmd_t;

typedef int (*mipos_disk_status_t)(mipos_pdrv_t pdrv);
typedef int (*mipos_disk_init_t)(mipos_pdrv_t pdrv);
typedef int (*mipos_disk_read_t)(mipos_pdrv_t pdrv,
                                 char* buf,
                                 mipos_sec_t sec,
                                 mipos_sec_t count);
typedef int (*mipos_disk_write_t)(mipos_pdrv_t pdrv,
                                  const char* buf,
                                  mipos_sec_t sec,
                                  mipos_sec_t count);
typedef int (*mipos_disk_ioctl_t)(mipos_pdrv_t pdrv,
                                  mipos_ioctl_cmd_t cmd,
                                  void* buf);


/* ------------------------------------------------------------------------- */

typedef struct
{
    mipos_pdrv_t _pdrv; //!< Physical driver number

    mipos_disk_init_t _disk_init;
    mipos_disk_status_t _disk_status;

    mipos_disk_read_t _disk_read;
    mipos_disk_write_t _disk_write;
    mipos_disk_ioctl_t _disk_ioctl;
} mipos_disk_t;


/* ------------------------------------------------------------------------- */

#ifndef MIPOS_PH_DISK_DRVS
#define MIPOS_PH_DISK_DRVS 1
#endif

/* ------------------------------------------------------------------------- */

#define MIPOS_DISK_STS_NOINIT 1  //!< Drive not initialized
#define MIPOS_DISK_STS_NODISK 2  //!< No medium in the drive */
#define MIPOS_DISK_STS_PROTECT 4 //!< Write protected

#define MIPOS_DISK_RES_OK 0     //!< Successful
#define MIPOS_DISK_RES_ERROR 1  //!< R/W Error
#define MIPOS_DISK_RES_WRTPR 2  //!< Write Protected
#define MIPOS_DISK_RES_NOTRDY 3 //!< Not Ready
#define MIPOS_DISK_RES_INVPAR 4 //!< Invalid Parameter


/* ------------------------------------------------------------------------- */

/** Register a new disk driver
 *
 *  @param drv_ptr pointer at mipos disk driver
 *
 *  @return 0 on success, -1 otherwise
 */
int mipos_add_disk_driver(const mipos_disk_t* drv_ptr);


/* ------------------------------------------------------------------------- */


/* ------------------------------------------------------------------------- */

/** Get drive status
 *
 * @param pdrv Physical drive nmuber to identify the drive
 *
 * @return 1: drive not initialized, 2: no medium in the drive, 3: write
 * protected, 0: Ok
 */
int mipos_disk_status(mipos_pdrv_t pdrv);


/* ------------------------------------------------------------------------- */

/** Inidialize a drive
 *
 * @param pdrv Physical drive nmuber to identify the drive
 *
 * @return 1: drive not initialized, 2: no medium in the drive, 3: write
 * protected, 0: Ok
 */
int mipos_disk_init(mipos_pdrv_t pdrv);


/* ------------------------------------------------------------------------- */

/** Read sectors
 *
 * @param pdrv Physical drive nmuber to identify the drive
 * @param char *buff Data buffer to store read data
 * @param sector Start sector in LBA
 * @param uint32_t count  Number of sectors to read
 *
 * @return  0: Successful, 1: R/W Error, 2: Write Protected, 3: Not Ready, 4
 * Invalid Parameter
 */
int mipos_disk_read(mipos_pdrv_t pdrv,
                    char* buf,
                    mipos_sec_t sector,
                    mipos_sec_t count);


/* ------------------------------------------------------------------------- */

/** Write sectors
 *
 * @param pdrv Physical drive nmuber to identify the drive
 * @param char *buff Data buffer to store read data
 * @param sector Start sector in LBA
 * @param uint32_t count  Number of sectors to read
 *
 * @return  0: Successful, 1: R/W Error, 2: Write Protected, 3: Not Ready, 4
 * Invalid Parameter
 */
int mipos_disk_write(mipos_pdrv_t pdrv,
                     const char* buf,
                     mipos_sec_t sector,
                     mipos_sec_t count);


/* ------------------------------------------------------------------------- */

/** Control device
 *
 * @param pdrv Physical drive nmuber to identify the drive
 * @param cmd Control code
 * @param buf Buffer to send/receive control data
 *
 * @return  0: Successful, 1: R/W Error, 2: Write Protected, 3: Not Ready, 4
 * Invalid Parameter
 */
int mipos_disk_ioctl(mipos_pdrv_t pdrv, mipos_ioctl_cmd_t cmd, void* buf);


#endif // __mipos_DISKIO_H__
