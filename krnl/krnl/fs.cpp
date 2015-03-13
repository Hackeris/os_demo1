#include "fs.h"
#include "proc.h"
#include "ipc.h"
#include "fat12.h"
#include "pmm.h"
#include "vmm.h"
#include "config.h"
#include "string.h"

#include "dbg.h"

FILE_SYSTEM* _fsys[DEVICE_MAX];

FILE*	file_table =(FILE*) FS_FILE_TABLE;

MESSAGE fsmsg;

void task_fs()
{
	init_fsys();

	DbgPrintf("FS initialized.\n");
	while(1){
		send_recv(RECEIVE,ANY_TASK,&fsmsg);
		int src = fsmsg.source;
		switch(fsmsg.type){
			case OPEN:
				{
					//DbgPrintf("FS OPEN\n");
					fsmsg.FD = fsys_do_open((char*)fsmsg.PATHNAME);
					//DbgPrintf("FS OPEN OK\n");
				}
				break;
			case READ:
				{
					//DbgPrintf("fd :%d buf: 0x%x cnt: %d\n",fsmsg.FD,(unsigned char*)fsmsg.BUF,fsmsg.CNT);
					fsmsg.RETVAL = fsys_do_read(fsmsg.FD,(unsigned char*)fsmsg.BUF,fsmsg.CNT);
					//DbgPrintf("end at fd :%d buf: 0x%x cnt: %d\n",fsmsg.FD,(unsigned char*)fsmsg.BUF,fsmsg.CNT);
				}
				break;
			case WRITE:
				break;
			case CLOSE:
				{
					fsmsg.RETVAL = fsys_do_close(fsmsg.FD);
				}
				break;
			case SIZE:
				{
					fsmsg.RETVAL = fsys_get_file_len(fsmsg.FD);
				}
				break;
			default:
				break;
		}
		send_recv(SEND,src,&fsmsg);
	}
}

void init_fsys()
{
	fsys_fat_initialize();

	int i ;
	for(i = 0;i < MAX_OPEN_FILE; i++){
		file_table[i].flags = FS_NOTHING;
	}
}

/**
*	Opens a file
*/
FILE volOpenFile(const char* fname)
{
	if (fname)
	{
		//! default to device 'a'
		unsigned char device = 'a';

		//! filename
		char* filename = (char*) fname;

		//! in all cases, if fname[1]==':' then the first character must be device letter
		//! FIXME: Using fname[2] do to BUG 2. Please see main.cpp for info
		if (fname[1]==':') {
			device = fname[0];
			filename += 2; //strip it from pathname
		}

		//! call filesystem
		if (_fsys [device - 'a']) {
			//! set volume specific information and return file
			FILE file = _fsys[device - 'a']->open (filename);
			file.device_id = device;
			return file;
		}
	}

	//! return invalid file
	FILE file;
	file.flags = FS_INVALID;
	return file;
}

/**
*	Reads file
*/
void volReadFile (FILE* file, unsigned char* Buffer, unsigned int Length)
{
	if (file)
		if (_fsys [file->device_id - 'a'])
			_fsys[file->device_id - 'a']->read (file,Buffer,Length);
}

/**
*	Close file
*/
void volCloseFile (FILE* file)
{
	if (file)
		if (_fsys [file->device_id - 'a'])
			_fsys[file->device_id - 'a']->close (file);
}


/**
*	Registers a filesystem
*/
void volRegisterFileSystem (FILE_SYSTEM* fsys, unsigned int deviceID)
{
	static int i=0;

	if (i < DEVICE_MAX)
		if (fsys){
			_fsys[ deviceID ] = fsys;
			i++;
		}
}

/**
*	Unregister file system
*/
void volUnregisterFileSystem (FILE_SYSTEM* fsys)
{
	for (int i=0;i < DEVICE_MAX; i++)
		if (_fsys[i]==fsys)
			_fsys[i]=0;
}

/**
*	Unregister file system
*/
void volUnregisterFileSystemByID (unsigned int deviceID)
{
	if (deviceID < DEVICE_MAX)
		_fsys [deviceID] = 0;
}

int fsys_do_open(const char* filename)
{
	int fd = -1;
	int i;
	if(strlen(filename) >= MAX_PATH){
		return -1;
	}
	char name[MAX_PATH];
	strcpy(name,(char*)va2la(fsmsg.source,(void*)filename));
	for(i = 0;i< MAX_OPEN_FILE; i++){
		if(file_table[i].flags == FS_NOTHING){
			fd = i;
			break;
		}
	}
	if(fd == -1){
		return -1;
	}
	file_table[i] = volOpenFile(filename);
	if(file_table[i].flags == FS_INVALID){
		return -1;
	}
	return fd;
}

int fsys_do_read(int fd,unsigned char* buffer,unsigned int len)
{
	if(fd < MAX_OPEN_FILE && fd >= 0){
		//volReadFile(&file_table[fd],
		//	(unsigned char*)va2la(fsmsg.source,buffer),len);
		//return len;
		int n = len / SECTOR_SIZE,i = 0;
		while(file_table[i].eof != 1 && n --){
			volReadFile(&file_table[fd],
				(unsigned char*)va2la(fsmsg.source,(char*)buffer + i*SECTOR_SIZE),SECTOR_SIZE);
			i ++;
		}
		if(file_table[fd].eof != 1){
			volReadFile(&file_table[fd],
				(unsigned char*)va2la(fsmsg.source,(char*)buffer + i*SECTOR_SIZE),len % SECTOR_SIZE);
		}
		return len;
	}
	return -1;
}

int fsys_get_file_len(int fd)
{
	if(fd < MAX_OPEN_FILE && fd >= 0){
		return file_table[fd].file_len;
	}
	return -1;
}

int fsys_do_close(int fd)
{
	if(fd < MAX_OPEN_FILE && fd >= 0){
		volCloseFile(&file_table[fd]);
		return 0;
	}
	return -1;
}