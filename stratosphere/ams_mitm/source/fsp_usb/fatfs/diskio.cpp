/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include <stdio.h>	
#include <string.h>
#include "ff.h"
#include "diskio.h"
#include "../impl/fspusb_usb_manager.hpp"

/* Reference for needed FATFS impl functions: http://irtos.sourceforge.net/FAT32_ChaN/doc/en/appnote.html#port */

namespace {

	u8 GetDriveStatus(u32 mounted_idx) {
		u8 status = STA_NOINIT;

		ams::mitm::fspusb::impl::DoWithDriveMountedIndex(mounted_idx, [&](ams::mitm::fspusb::impl::DrivePointer &drive_ptr) {
			if (drive_ptr->IsSCSIOk()) {
				status = 0;
			}
		});

		return status;
	}

}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

extern "C" DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return GetDriveStatus((u32)pdrv);
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

extern "C" DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	return GetDriveStatus((u32)pdrv);
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

extern "C" DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	auto res = RES_PARERR;

	ams::mitm::fspusb::impl::DoWithDriveMountedIndex((u32)pdrv, [&](ams::mitm::fspusb::impl::DrivePointer &drive_ptr) {
		res = drive_ptr->DoReadSectors(buff, sector, count);
	});

	return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

extern "C" DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	auto res = RES_PARERR;

	ams::mitm::fspusb::impl::DoWithDriveMountedIndex((u32)pdrv, [&](ams::mitm::fspusb::impl::DrivePointer &drive_ptr) {
		res = drive_ptr->DoWriteSectors(buff, sector, count);
	});
	
	return res;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

extern "C" DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	/* Shall we implement any ioctls here? */

	return RES_OK;
}
