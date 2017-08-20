/*------------------------------------------------------------------------*/
/* Sample code of OS dependent controls for FatFs                         */
/* (C)ChaN, 2017                                                          */
/*------------------------------------------------------------------------*/


#include "ff.h"


#if FF_USE_LFN == 3	/* Dynamic memory allocation */

/*------------------------------------------------------------------------*/
/* Allocate a memory block                                                */
/*------------------------------------------------------------------------*/

void* ff_memalloc(	/* Returns pointer to the allocated memory block (null on not enough core) */
    UINT msize		/* Number of bytes to allocate */
)
{
    return mipos_malloc(msize);	/* Allocate a new memory block with POSIX API */
}


/*------------------------------------------------------------------------*/
/* Free a memory block                                                    */
/*------------------------------------------------------------------------*/

void ff_memfree(
    void* mblock	/* Pointer to the memory block to free */
)
{
    mipos_free(mblock);	/* Free the memory block with POSIX API */
}

#endif


#ifdef FF_FS_REENTRANT	/* Mutal exclusion */

#include "mipos.h"

/*------------------------------------------------------------------------*/
/* Create a Synchronization Object
/*------------------------------------------------------------------------*/
/* This function is called in f_mount() function to create a new
/  synchronization object for the volume, such as semaphore and mutex.
/  When a 0 is returned, the f_mount() function fails with FR_INT_ERR.
*/

#ifndef MIPOS_FF_VOLUMES
#define MIPOS_FF_VOLUMES 1
#endif

static mipos_mtx_t mtx[MIPOS_FF_VOLUMES] = { 0 };

int ff_cre_syncobj(	/* 1:Function succeeded, 0:Could not create the sync object */
    BYTE vol,			/* Corresponding volume (logical drive number) */
    FF_SYNC_t *sobj		/* Pointer to return the created sync object */
)
{
    if (vol >= MIPOS_FF_VOLUMES) {
        return 0;
    }

    mipos_mu_init(&mtx[vol]);
    *sobj = (FF_SYNC_t)&mtx[vol];

    return 1;
}


/*------------------------------------------------------------------------*/
/* Delete a Synchronization Object                                        */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount() function to delete a synchronization
/  object that created with ff_cre_syncobj() function. When a 0 is returned,
/  the f_mount() function fails with FR_INT_ERR.
*/

int ff_del_syncobj(	/* 1:Function succeeded, 0:Could not delete due to an error */
    FF_SYNC_t sobj		/* Sync object tied to the logical drive to be deleted */
)
{
    mipos_mu_init((mipos_mtx_t*)sobj);
    return 0;
}


/*------------------------------------------------------------------------*/
/* Request Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on entering file functions to lock the volume.
/  When a 0 is returned, the file function fails with FR_TIMEOUT.
*/

int ff_req_grant(	/* 1:Got a grant to access the volume, 0:Could not get a grant */
    FF_SYNC_t sobj	/* Sync object to wait */
)
{
    mipos_mu_lock((mipos_mtx_t*)sobj);
    return 1;
}


/*------------------------------------------------------------------------*/
/* Release Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on leaving file functions to unlock the volume.
*/

void ff_rel_grant(
    FF_SYNC_t sobj	/* Sync object to be signaled */
)
{
    mipos_mu_unlock((mipos_mtx_t*)sobj);
}

#endif

