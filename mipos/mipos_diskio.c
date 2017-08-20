/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

/* -------------------------------------------------------------------------- */

#include "mipos.h"
#include "mipos_diskio.h"


/* -------------------------------------------------------------------------- */

static mipos_disk_t _mipos_disk_drv[ MIPOS_PH_DISK_DRVS ] = {0}; 
static int _mipos_disk_drv_cnt = 0; 


/* -------------------------------------------------------------------------- */

int mipos_add_disk_driver( const mipos_disk_t * drv_ptr ) 
{
    mipos_init_cs();
    mipos_enter_cs();

    if (_mipos_disk_drv_cnt<MIPOS_PH_DISK_DRVS) { 
        ++_mipos_disk_drv_cnt;
    }
    else {
        mipos_leave_cs();
        return -1;
    }

    memcpy( 
        &_mipos_disk_drv[ _mipos_disk_drv_cnt-1 ], 
        drv_ptr, 
        sizeof(mipos_disk_t) );

    mipos_leave_cs();

    return 0;
}


/* -------------------------------------------------------------------------- */

static
mipos_disk_t * mipos_find_disk_driver( mipos_pdrv_t pdrv ) 
{
    mipos_disk_t * drv_ptr = 0;
    int cnt = 0, i = 0;

    mipos_init_cs();
    mipos_enter_cs();
    cnt = _mipos_disk_drv_cnt;
    mipos_leave_cs();

    if (cnt<1)
        return 0;

    drv_ptr = &_mipos_disk_drv[ 0 ];

    for (; i<cnt; ++i) {

        if (drv_ptr->_pdrv == pdrv ) {
           return drv_ptr; 
        }

        ++drv_ptr;
    }

    return 0;
}


/* -------------------------------------------------------------------------- */

int mipos_disk_status ( mipos_pdrv_t pdrv )
{
    mipos_disk_t * drv_ptr = mipos_find_disk_driver( pdrv );

    return drv_ptr ? 
        drv_ptr->_disk_status( pdrv ) :
        MIPOS_DISK_STS_NOINIT;
}


/* -------------------------------------------------------------------------- */

int mipos_disk_init( mipos_pdrv_t pdrv )
{
    mipos_disk_t * drv_ptr = mipos_find_disk_driver( pdrv );

    return drv_ptr ? 
        drv_ptr->_disk_init( pdrv ) :
        MIPOS_DISK_STS_NOINIT;
}


/* -------------------------------------------------------------------------- */

int mipos_disk_read ( 
    mipos_pdrv_t pdrv, 
    char *buf,
    mipos_sec_t sec, 
    mipos_sec_t count )
{
    mipos_disk_t * drv_ptr = mipos_find_disk_driver( pdrv );

    return drv_ptr ? 
        drv_ptr->_disk_read( pdrv, buf, sec, count ) :
        MIPOS_DISK_RES_NOTRDY;
}


/* -------------------------------------------------------------------------- */

int mipos_disk_write( 
    mipos_pdrv_t pdrv, 
    const char *buf, 
    mipos_sec_t sec, 
    mipos_sec_t count )
{
    mipos_disk_t * drv_ptr = mipos_find_disk_driver( pdrv );

    return drv_ptr ? 
        drv_ptr->_disk_write( pdrv, buf, sec, count ) :
        MIPOS_DISK_RES_NOTRDY;
}


/* -------------------------------------------------------------------------- */

int mipos_disk_ioctl(mipos_pdrv_t pdrv, mipos_ioctl_cmd_t cmd, void *buf)
{
    mipos_disk_t * drv_ptr = mipos_find_disk_driver( pdrv );

    return drv_ptr ? 
        drv_ptr->_disk_ioctl( pdrv, cmd, buf ) :
        MIPOS_DISK_RES_INVPAR;
}


/* -------------------------------------------------------------------------- */

