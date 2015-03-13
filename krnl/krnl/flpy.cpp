#include "flpy.h"
#include "proc.h"
#include "ipc.h"
#include "pic.h"
#include "idt.h"
#include "klib.h"
#include "dma.h"

#include "dbg.h"

void task_flpy()
{
	MESSAGE	msg;

	//	set drive 0 as current drive
	flpydsk_set_working_drive (0);

	enable_irq(FLOPPY_IRQ);
	//	install floppy disk to IR 38, uses IRQ 6
	flpydsk_install (38);
	DbgPrintf("Floppy initialized.\n");

//	send_recv(BOTH,TASK_FS,&msg);

	while(1){
		send_recv(RECEIVE,ANY_TASK,&msg);

		int src = msg.source;

		switch(msg.type){
			case DEV_OPEN:
				break;
			case DEV_READ:
				{
					//DbgPrintf("%d ",msg.SECTOR);
					msg.BUF = (void*)flpydsk_read_sector(msg.SECTOR);
					//uint8_t* buf;
					//buf = (uint8_t*)msg.BUF;
					//int i;
					//for(i = 0; i < 512; i++){
					//	DbgPrintf("%x",buf[i]);
					//}
				}
				break;
			case DEV_WRITE:
				break;
			case DEV_CLOSE:
				break;
			default:
				break;
		}
		send_recv(SEND,src,&msg);
	}
}

// current working drive. Defaults to 0 which should be fine on most systems
static uint8_t	_current_drive = 0;


bool _cdecl dma_initialize_floppy(uint8_t* buffer, unsigned length)
{
   union{
      uint8_t byte[4];//Lo[0], Mid[1], Hi[2]
      unsigned long l;
   }a, c;

   a.l=(unsigned)buffer;
   c.l=(unsigned)length-1;

   //Check for buffer issues
   if ((a.l >> 24) || (c.l >> 16) || (((a.l & 0xffff)+c.l) >> 16)){
#ifdef _DEBUG
      _asm{
         mov      eax, 0x1337
         cli
         hlt
      }
#endif
      return false;
   }

   dma_reset (1);
   dma_mask_channel( FDC_DMA_CHANNEL );//Mask channel 2
   dma_reset_flipflop ( 1 );//Flipflop reset on DMA 1

   dma_set_address( FDC_DMA_CHANNEL, a.byte[0],a.byte[1]);//Buffer address
   dma_reset_flipflop( 1 );//Flipflop reset on DMA 1

   dma_set_count( FDC_DMA_CHANNEL, c.byte[0],c.byte[1]);//Set count
   dma_set_read ( FDC_DMA_CHANNEL );

   dma_unmask_all( 1 );//Unmask channel 2

   return true;
}


// return fdc status
uint8_t flpydsk_read_status () 
{
	// just return main status register
	return inportb (FLPYDSK_MSR);
}

// write to the fdc dor
void flpydsk_write_dor (uint8_t val )
{
	// write the digital output register
	outportb (FLPYDSK_DOR, val);
}

// send command byte to fdc
void flpydsk_send_command (uint8_t cmd)
{
	// wait until data register is ready. We send commands to the data register
	for (int i = 0; i < 500; i++ )
		if ( flpydsk_read_status () & FLPYDSK_MSR_MASK_DATAREG )
			return outportb (FLPYDSK_FIFO, cmd);
}

// get data from fdc
uint8_t flpydsk_read_data () 
{
	// same as above function but returns data register for reading
	for (int i = 0; i < 500; i++ )
		if ( flpydsk_read_status () & FLPYDSK_MSR_MASK_DATAREG )
			return inportb (FLPYDSK_FIFO);
	return 0;
}


void flpydsk_write_ccr (uint8_t val)
{
	outportb (FLPYDSK_CTRL, val);
}

inline void flpydsk_wait_irq ()
{
	MESSAGE	msg;
	send_recv(RECEIVE,INTERRUPT,&msg);
}


//	floppy disk irq handler
void _cdecl i86_flpy_irq () 
{
		save();
	_asm{
		in	al,INT_M_CTLMASK
		or	al,( 1 << FLOPPY_IRQ )
		out	INT_M_CTLMASK,al

		mov al,EOI
		out INT_M_CTL,al
	}
	_asm sti
	inform_int(TASK_FLPY);
	_asm cli
	_asm{
		in	al,INT_M_CTLMASK
		and	al,~(1 << FLOPPY_IRQ)
		out	INT_M_CTLMASK,al
	}
	_asm ret
}

void flpydsk_check_int (uint32_t* st0, uint32_t* cyl)
{
	flpydsk_send_command (FDC_CMD_CHECK_INT);

	*st0 = flpydsk_read_data ();
	*cyl = flpydsk_read_data ();
}

// turns the current floppy drives motor on/off
void flpydsk_control_motor (bool b)
{
	// sanity check: invalid drive
	if (_current_drive > 3)
		return;

	uint8_t motor = 0;

	// select the correct mask based on current drive
	switch (_current_drive) {

		case 0:
			motor = FLPYDSK_DOR_MASK_DRIVE0_MOTOR;
			break;
		case 1:
			motor = FLPYDSK_DOR_MASK_DRIVE1_MOTOR;
			break;
		case 2:
			motor = FLPYDSK_DOR_MASK_DRIVE2_MOTOR;
			break;
		case 3:
			motor = FLPYDSK_DOR_MASK_DRIVE3_MOTOR;
			break;
	}

	// turn on or off the motor of that drive
	if (b)
		flpydsk_write_dor (uint8_t(_current_drive | motor | FLPYDSK_DOR_MASK_RESET | FLPYDSK_DOR_MASK_DMA));
	else
		flpydsk_write_dor (FLPYDSK_DOR_MASK_RESET);

	//sleep (20);
	//milli_delay(20);
	delay(20);
}

// configure drive
void flpydsk_drive_data (uint8_t stepr, uint8_t loadt, uint8_t unloadt, bool dma )
{
	uint8_t data = 0;

	// send command
	flpydsk_send_command (FDC_CMD_SPECIFY);
	data = ( (stepr & 0xf) << 4) | (unloadt & 0xf);
		flpydsk_send_command (data);
	data = (( loadt << 1 ) | ( (dma) ? 0 : 1 ) );
		flpydsk_send_command (data);
}

// calibrates the drive
int flpydsk_calibrate (uint8_t drive)
{
	uint32_t st0, cyl;

	if (drive >= 4)
		return -2;

	// turn on the motor
	flpydsk_control_motor (true);

	for (int i = 0; i < 10; i++) {

		// send command
		flpydsk_send_command ( FDC_CMD_CALIBRATE );
		flpydsk_send_command ( drive );
		flpydsk_wait_irq ();
		flpydsk_check_int ( &st0, &cyl);

		// did we fine cylinder 0? if so, we are done
		if (!cyl) {

			flpydsk_control_motor (false);
			return 0;
		}
	}

	flpydsk_control_motor (false);
	return -1;
}

// disable controller
void flpydsk_disable_controller ()
{
	flpydsk_write_dor (0);
}

// enable controller
void flpydsk_enable_controller ()
{
	flpydsk_write_dor ( FLPYDSK_DOR_MASK_RESET | FLPYDSK_DOR_MASK_DMA);
}

// reset controller
void flpydsk_reset ()
{
	uint32_t st0, cyl;

	// reset the controller
	flpydsk_disable_controller ();
	flpydsk_enable_controller ();
	flpydsk_wait_irq ();

	// send CHECK_INT/SENSE INTERRUPT command to all drives
	for (int i=0; i<4; i++)
		flpydsk_check_int (&st0,&cyl);

	// transfer speed 500kb/s
	flpydsk_write_ccr (0);

	// pass mechanical drive info. steprate=3ms, unload time=240ms, load time=16ms
	flpydsk_drive_data (3,16,240,true);

	// calibrate the disk
	flpydsk_calibrate ( _current_drive );
}

// read a sector
void flpydsk_read_sector_imp (uint8_t head, uint8_t track, uint8_t sector)
{
	uint32_t st0, cyl;

	// initialize DMA
	dma_initialize_floppy ((uint8_t*) DMA_BUFFER, 512 );

	// set the DMA for read transfer
	dma_set_read ( FDC_DMA_CHANNEL );

	// read in a sector
	flpydsk_send_command (
				FDC_CMD_READ_SECT | FDC_CMD_EXT_MULTITRACK | FDC_CMD_EXT_SKIP | FDC_CMD_EXT_DENSITY);
	flpydsk_send_command ( head << 2 | _current_drive );
	flpydsk_send_command ( track);
	flpydsk_send_command ( head);
	flpydsk_send_command ( sector);
	flpydsk_send_command ( FLPYDSK_SECTOR_DTL_512 );
	flpydsk_send_command ( ( ( sector + 1 ) >= FLPY_SECTORS_PER_TRACK ) ? FLPY_SECTORS_PER_TRACK : sector + 1 );
	flpydsk_send_command ( FLPYDSK_GAP3_LENGTH_3_5 );
	flpydsk_send_command ( 0xff );

	// wait for irq
	flpydsk_wait_irq ();

	// read status info
	for (int j=0; j<7; j++)
		flpydsk_read_data ();

	// let FDC know we handled interrupt
	flpydsk_check_int (&st0,&cyl);
}

// seek to given track/cylinder
int flpydsk_seek ( uint8_t cyl, uint8_t head )
{
	uint32_t st0, cyl0;

	if (_current_drive >= 4)
		return -1;

	for (int i = 0; i < 10; i++ ) {

		// send the command
		flpydsk_send_command (FDC_CMD_SEEK);
		flpydsk_send_command ((head) << 2 | _current_drive);
		flpydsk_send_command (cyl);

		// wait for the results phase IRQ
		flpydsk_wait_irq ();
		flpydsk_check_int (&st0,&cyl0);

		// found the cylinder?
		if ( cyl0 == cyl)
			return 0;
	}

	return -1;
}

void flpydsk_lba_to_chs (int lba,int *head,int *track,int *sector)
{
   *head = ( lba % ( FLPY_SECTORS_PER_TRACK * 2 ) ) / ( FLPY_SECTORS_PER_TRACK );
   *track = lba / ( FLPY_SECTORS_PER_TRACK * 2 );
   *sector = lba % FLPY_SECTORS_PER_TRACK + 1;
}

// install floppy driver
void flpydsk_install (int irq) 
{
	init_idt_desc(irq,i86_flpy_irq,DA_386IGATE,PRIVILEGE_KRNL,0x08);

	// reset the fdc
	flpydsk_reset ();

	// set drive information
	flpydsk_drive_data (13, 1, 0xf, true);
}

// set current working drive
void flpydsk_set_working_drive (uint8_t drive) {

	if (drive < 4)
		_current_drive = drive;
}

// get current working drive
uint8_t flpydsk_get_working_drive ()
{
	return _current_drive;
}

// read a sector
uint8_t* flpydsk_read_sector (int sectorLBA) 
{
	if (_current_drive >= 4)
		return 0;

	// convert LBA sector to CHS
	int head=0, track=0, sector=1;
	flpydsk_lba_to_chs (sectorLBA, &head, &track, &sector);

	// turn motor on and seek to track
	flpydsk_control_motor (true);
	if (flpydsk_seek ((uint8_t)track, (uint8_t)head) != 0)
		return 0;
	// read sector and turn motor off
	flpydsk_read_sector_imp ((uint8_t)head, (uint8_t)track, (uint8_t)sector);
	flpydsk_control_motor (false);

	// warning: this is a bit hackish
	return (uint8_t*) DMA_BUFFER;
}

uint8_t* flpy_do_read_sector(int sectorLBA)
{
	MESSAGE msg;

	msg.type = DEV_READ;
	msg.SECTOR = sectorLBA;
	send_recv(BOTH,TASK_FLPY,&msg);

	return (uint8_t*)msg.BUF;
}