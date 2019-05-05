/*
 * operations on IDE disk.
 */

#include "fs.h"
#include "lib.h"
#include <mmu.h>

// Overview:
// 	read data from IDE disk. First issue a read request through
// 	disk register and then copy data from disk buffer
// 	(512 bytes, a sector) to destination array.
//
// Parameters:
//	diskno: disk number.
// 	secno: start sector number.
// 	dst: destination for data read from IDE disk.
// 	nsecs: the number of sectors to read.
//
// Post-Condition:
// 	If error occurred during read the IDE disk, panic.
//
// Hint: use syscalls to access device registers and buffers
void ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs)
{
	// 0x200: the size of a sector: 512 bytes.
	int offset_begin = secno * 0x200;
	int offset_end = offset_begin + nsecs * 0x200;
	int offset = 0;
	u_int ide_base = 0x13000000;
	syscall_write_dev((u_int)&diskno, ide_base + 0x0010, 4);
	while (offset_begin + offset < offset_end)
	{
		u_int tmp;
		tmp = offset_begin + offset;
		syscall_write_dev((u_int)&tmp, ide_base + 0x0000, 4);
		u_char uch = 0;
		syscall_write_dev((u_int)&uch, ide_base + 0x0020, 1);
		syscall_read_dev((u_int)&tmp, ide_base + 0x0030, 4);
		if (tmp == 0)
		{
			user_panic("ide_read");
		}
		syscall_read_dev((u_int)dst + offset, ide_base + 0x4000, 512);
		offset+=512;
	}
}

// Overview:
// 	write data to IDE disk.
//
// Parameters:
//	diskno: disk number.
//	secno: start sector number.
// 	src: the source data to write into IDE disk.
//	nsecs: the number of sectors to write.
//
// Post-Condition:
//	If error occurred during read the IDE disk, panic.
//
// Hint: use syscalls to access device registers and buffers
void ide_write(u_int diskno, u_int secno, void *src, u_int nsecs)
{
	// Your code here
	int offset_begin = secno * 0x200;
	int offset_end = offset_begin + nsecs * 0x200;
	int offset = 0;
	writef("diskno: %d\n", diskno);
	u_int ide_base = 0x13000000;
	syscall_write_dev((u_int)&diskno, ide_base + 0x0010, 4);
	while (offset_begin + offset < offset_end)
	{
		u_int tmp;
		// copy data from source array to disk buffer.
		syscall_write_dev((u_int)src + offset, ide_base + 0x4000, 512);
		// if error occur, then panic.
		tmp = offset_begin + offset;
		syscall_write_dev((u_int)&tmp, ide_base + 0x0000, 4);
		u_char uch = 1;
		syscall_write_dev((u_int)&uch, ide_base + 0x0020, 1);
		syscall_read_dev((u_int)&tmp, ide_base + 0x0030, 4);
		if (tmp == 0)
		{
			user_panic("ide_write");
		}
		offset+=512;
	}
}
