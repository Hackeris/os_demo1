#ifndef _H_FAT12
#define _H_FAT12
#include "ctype.h"
#include "fs.h"

#define SECTOR_SIZE			512

typedef struct _DIRECTORY {

	uint8_t   Filename[8];
	uint8_t   Ext[3];
	uint8_t   Attrib;
	uint8_t   Reserved;
	uint8_t   TimeCreatedMs;
	uint16_t  TimeCreated;
	uint16_t  DateCreated;
	uint16_t  DateLastAccessed;
	uint16_t  FirstClusterHiBytes;
	uint16_t  LastModTime;
	uint16_t  LastModDate;
	uint16_t  FirstCluster;
	uint32_t  FileSize;

}DIRECTORY;

typedef struct _MOUNT_INFO {

	uint32_t numSectors;
	uint32_t fatOffset;
	uint32_t numRootEntries;
	uint32_t rootOffset;
	uint32_t rootSize;
	uint32_t fatSize;
	uint32_t fatEntrySize;

}MOUNT_INFO;

FILE fsys_fat_directory (const char* dir_name);
void fsys_fat_read(FILE* file, unsigned char* buffer, unsigned int len);
FILE fsys_fat_open (const char* filename);
void fsys_fat_initialize ();
void fsys_fat_mount();


#endif