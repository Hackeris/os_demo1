#include "fat12.h"
#include "flpy.h"
#include "bpb.h"
#include "string.h"
#include "ipc.h"
#include "config.h"

#include "dbg.h"

FILE_SYSTEM	_fs_fat;

MOUNT_INFO	_mnt_info;

uint8_t		FAT[SECTOR_SIZE * 2];

void to_dos_filename(const char* filename,char* fname,uint32_t len)
{
	uint32_t	i = 0;

	if(len > 11){
		return;
	}
	if(!fname || !filename){
		return ;
	}

	memset(fname,' ',len);

	//	8.3
	for(i = 0; i < strlen(filename)-1 && i < len ; i++){
		if(filename[i] == '.' || i == 8){
			break;
		}
		fname[i] = filename[i] & ~(1 << 5);
	}
	// add extension if needed
	if (filename[i]=='.') {

		// note: cant just copy over-extension might not be 3 chars
		for (int k=0; k<3; k++) {

			++i;
			if ( filename[i] )
				fname[8+k] = filename[i];
		}
	}
	// extension must be uppercase (we dont handle LFNs)
	for (i = 0; i < 3; i++)
		fname[8+i] = (fname[8+i]) & ~(1 << 5);
}

FILE fsys_fat_directory(const char* dir_name)
{
	FILE file;
	unsigned char* buf;
	DIRECTORY*	directory;

	//	get 8.3 dir name
	char dos_name[11];
	to_dos_filename(dir_name,dos_name,11);
	dos_name[11]=0;

	//	14 sectors per dir
	for(int sector = 0;sector < 14; sector++){
		//	read in sector of root directory
		buf = (unsigned char* )flpy_do_read_sector(_mnt_info.rootOffset + sector);

		//DbgPrintf("%d  ",_mnt_info.rootOffset + sector);

		//	get directory info
		directory = (DIRECTORY*)buf;

		//	16 entries per sector
		for(int i = 0; i< 16 ; i++){
			char name[11];
			memcpy(name,directory->Filename,11);
			name[11] = 0;

			//	find a match?
			if(strcmp(dos_name,name) == 0){
				strcpy(file.name,dir_name);
				file.id = 0;
				file.current_cluster = directory->FirstCluster;
				file.file_len = directory->FileSize;
				file.eof = 0;
				
				if(directory->Attrib == 0x10){
					file.flags = FS_DIRECTORY;
				}
				else{
					file.flags = FS_FILE;
				}

				return file;
			}
			directory ++;
		}
	}
	file.flags = FS_INVALID;
	return file;
}

void fsys_fat_read(FILE* file,unsigned char* buffer,uint32_t len)
{
	if(file){
		unsigned int phys_sector = 32 + (file->current_cluster - 1);

		unsigned char* sector = (unsigned char*)flpy_do_read_sector(phys_sector);
		memcpy(buffer,sector,512);

		//	locate FAT sector
		unsigned int FAT_Offset = file->current_cluster + (file->current_cluster / 2);
		unsigned int FAT_Sector = 1 + (FAT_Offset / SECTOR_SIZE);
		unsigned int enrtyOffset = FAT_Offset % SECTOR_SIZE;

		//	read 1st FAT sector 
		sector = (unsigned char*)flpy_do_read_sector(FAT_Sector);
		memcpy(FAT,sector,512);

		sector = (unsigned char*)flpy_do_read_sector(FAT_Sector + 1);
		memcpy(FAT + 512,sector,512);

		uint16_t nextCluster = *(uint16_t*)&FAT[enrtyOffset];

		//	test if entry is ODD or EVEN
		if(file->current_cluster & 0x0001){
			nextCluster >> 4;
		}
		else{
			nextCluster &= 0x0fff;
		}

		//	test for end of file
		if(nextCluster >= 0xff8){
			file->eof = 1;
			return ;
		}

		//	set next cluster
		file->current_cluster = nextCluster;
	}
}

void fsys_fat_close(FILE* file)
{
	if(file)
		file->flags = FS_INVALID;
}

FILE fsys_fat_open_subdir(FILE kFile,const char* filename)
{
	FILE file;

	char dos_name[11];
	to_dos_filename(filename,dos_name,11);
	dos_name[11] = 0;
	if(kFile.flags != FS_INVALID){
		while(! kFile.eof ){
			unsigned char buf[512];
			fsys_fat_read(&file,buf,512);

			DIRECTORY* pkDir = (DIRECTORY*)buf;

			//	16 entries in buffer
			for(unsigned int i = 0;i < 16; i++){
				char name[11];
				memcpy(name,pkDir->Filename,11);
				name[11] = 0;

				//	match?
				if(strcmp(name,dos_name) == 0){
					strcpy(file.name,filename);
					file.id = 0;
					file.current_cluster = pkDir->FirstCluster;
					file.file_len = pkDir->FileSize;
					file.eof = 0;
				}

				//	set file type
				if(pkDir->Attrib == 0x10){
					file.flags = FS_DIRECTORY;
				}
				else{
					file.flags = FS_FILE;
				}

				return file;
			}

			pkDir ++;
		}
	}
	file.flags = FS_INVALID;
	return file;
}

FILE fsys_fat_open(const char * filename)
{
	FILE curDirectory;
	char* p = 0;
	bool rootDir=true;
	char* path = (char*) filename;

	//! ANY_TASK '\'s in path?
	p = strchr (path, '\\');
	if (!p) {
		//! nope, must be in root directory, search it
		curDirectory = fsys_fat_directory (path);

		//! found file?
		if (curDirectory.flags == FS_FILE)
			return curDirectory;

		//! unable to find
		FILE ret;
		ret.flags = FS_INVALID;
		return ret;
	}

	//! go to next character after first '\'
	p++;

	while ( p ) {

		//! get pathname
		char pathname[16];
		int i=0;
		for (i=0; i<16; i++) {

			//! if another '\' or end of line is reached, we are done
			if (p[i]=='\\' || p[i]=='\0')
				break;

			//! copy character
			pathname[i]=p[i];
		}
		pathname[i]=0; //null terminate

		//! open subdirectory or file
		if (rootDir) {
			//! search root directory - open pathname
			curDirectory = fsys_fat_directory (pathname);
			rootDir=false;
		}
		else {
			//! search a subdirectory instead for pathname
			curDirectory = fsys_fat_open_subdir (curDirectory, pathname);
		}

		//! found directory or file?
		if (curDirectory.flags == FS_INVALID)
			break;

		//! found file?
		if (curDirectory.flags == FS_FILE)
			return curDirectory;

		//! find next '\'
		p=strchr (p+1, '\\');
		if (p)
			p++;
	}

	//! unable to find
	FILE ret;
	ret.flags = FS_INVALID;
	return ret;
}

void fsys_fat_mount()
{
	BOOTSECTOR*	bootsector;

	bootsector = (BOOTSECTOR*)flpy_do_read_sector(0);//flpydsk_read_sector(0);

	//	store mount info
	_mnt_info.numSectors		= bootsector->Bpb.NumSectors;
	_mnt_info.fatOffset			= 1;
	_mnt_info.fatSize			= bootsector->Bpb.SectorsPerFat;
	_mnt_info.fatEntrySize		= 8;
	_mnt_info.numRootEntries	= bootsector->Bpb.NumDirEntries;
	_mnt_info.rootOffset		= (bootsector->Bpb.NumberOfFats * bootsector->Bpb.SectorsPerFat) + 1;
	_mnt_info.rootSize			= ( bootsector->Bpb.NumDirEntries * 32 ) / bootsector->Bpb.BytesPerSector;
}

void fsys_fat_initialize()
{
	strcpy(_fs_fat.name,"FAT12");
	_fs_fat.directory = fsys_fat_directory;
	_fs_fat.mount     = fsys_fat_mount;
	_fs_fat.open      = fsys_fat_open;
	_fs_fat.read      = fsys_fat_read;
	_fs_fat.close     = fsys_fat_close;

	//! register ourself to volume manager
	volRegisterFileSystem ( &_fs_fat, 0 );

	//! mounr filesystem
	fsys_fat_mount();
}