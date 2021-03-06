#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//Controller stuff
#include <dc/maple.h>
#include <dc/maple/controller.h>

//For mounting the sd dir
#include <dc/sd.h>
#include <kos/blockdev.h>
#include <fat/fs_fat.h>

#include <kos/fs.h>	//Might need it for fs_load()
#include <arch/exec.h>	//arch_exec()
#include <assert.h>	//assert()

#include "debug.h"	//For error_freeze

#define MNT_MODE FS_FAT_MOUNT_READONLY

static void unmount_fat_sd(){
	fs_fat_unmount("/sd");
	fs_fat_shutdown();
	sd_shutdown();
}

static int mount_fat_sd(){
	kos_blockdev_t sd_dev;
	uint8 partition_type;

	// Initialize the sd card if its present
	if(sd_init()){
		return 1;
	}

	// Grab the block device for the first partition on the SD card. Note that
	// you must have the SD card formatted with an MBR partitioning scheme
	if(sd_blockdev_for_partition(0, &sd_dev, &partition_type)){
		return 2;
	}

	// Check to see if the MBR says that we have a valid partition
	// if(partition_type != 0x83){
		//I don't know what value I should be comparing against, hence this check is disabled for now
		// This: https://en.wikipedia.org/wiki/Partition_type
			//Suggests there's multiple types for FAT...not sure how to handle this
		// return 3;
	// }

	// Initialize fs_fat and attempt to mount the device
	if(fs_fat_init()){
		return 4;
	}

	//Mount the SD card to the sd dir in the VFS
	if(fs_fat_mount("/sd", &sd_dev, MNT_MODE)){
		return 5;
	}
	return 0;
}

int main(){
	int sdRes = mount_fat_sd();	//This function should be able to mount an ext2 formatted sd card to the /sd dir	
	if(sdRes != 0){
		error_freeze("\nsdRes = %d\n", sdRes);	//Hopefully the \n will push it one line down...
	}

	void *prog;
	ssize_t length = fs_load("/sd/Prog.bin", &prog);
	unmount_fat_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore
	assert(length > 0);
	arch_exec(prog, length);
}
