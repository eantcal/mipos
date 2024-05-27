/*-----------------------------------------------------------------------*/
/* Low level disk I/O module (c) Antonino Calderone                      */
/* based on skeleton for FatFs (C)ChaN, 2016                             */
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/

#include "diskio.h" /* FatFs lower layer API */
#include "mipos_diskio.h"

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(BYTE pdrv)
{
    return mipos_disk_status((mipos_pdrv_t)pdrv);
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(BYTE pdrv)
{
    return mipos_disk_init((mipos_pdrv_t)pdrv);
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(BYTE pdrv,  /* Physical drive nmuber to identify the drive */
                  BYTE* buff, /* Data buffer to store read data */
                  DWORD sector, /* Start sector in LBA */
                  UINT count    /* Number of sectors to read */
)
{
    return mipos_disk_read(
      (mipos_pdrv_t)pdrv, buff, (mipos_sec_t)sector, (mipos_sec_t)count);
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write(BYTE pdrv, /* Physical drive nmuber to identify the drive */
                   const BYTE* buff, /* Data to be written */
                   DWORD sector,     /* Start sector in LBA */
                   UINT count        /* Number of sectors to write */
)
{
    return mipos_disk_write(
      (mipos_pdrv_t)pdrv, buff, (mipos_sec_t)sector, (mipos_sec_t)count);
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(BYTE pdrv, /* Physical drive nmuber (0..) */
                   BYTE cmd,  /* Control code */
                   void* buff /* Buffer to send/receive control data */
)
{
    return mipos_disk_ioctl((mipos_pdrv_t)pdrv, (mipos_ioctl_cmd_t)cmd, buff);
}
