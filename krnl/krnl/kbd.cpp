#include "kbd.h"
#include "idt.h"
#include "klib.h"
#include "proc.h"
#include "pic.h"
#include "tty.h"
#include "const.h"

#include "dbg.h"

extern int32_t  k_reenter;

uint32_t keymap[NR_SCAN_CODES * MAP_COLS] = {

/* scan-code			!Shift		Shift		E0 XX	*/
/* ==================================================================== */
/* 0x00 - none		*/	0,			0,			0,
/* 0x01 - ESC		*/	ESC,		ESC,		0,
/* 0x02 - '1'		*/	'1',		'!',		0,
/* 0x03 - '2'		*/	'2',		'@',		0,
/* 0x04 - '3'		*/	'3',		'#',		0,
/* 0x05 - '4'		*/	'4',		'$',		0,
/* 0x06 - '5'		*/	'5',		'%',		0,
/* 0x07 - '6'		*/	'6',		'^',		0,
/* 0x08 - '7'		*/	'7',		'&',		0,
/* 0x09 - '8'		*/	'8',		'*',		0,
/* 0x0A - '9'		*/	'9',		'(',		0,
/* 0x0B - '0'		*/	'0',		')',		0,
/* 0x0C - '-'		*/	'-',		'_',		0,
/* 0x0D - '='		*/	'=',		'+',		0,
/* 0x0E - BS		*/	BACKSPACE,	BACKSPACE,	0,
/* 0x0F - TAB		*/	TAB,		TAB,		0,
/* 0x10 - 'q'		*/	'q',		'Q',		0,
/* 0x11 - 'w'		*/	'w',		'W',		0,
/* 0x12 - 'e'		*/	'e',		'E',		0,
/* 0x13 - 'r'		*/	'r',		'R',		0,
/* 0x14 - 't'		*/	't',		'T',		0,
/* 0x15 - 'y'		*/	'y',		'Y',		0,
/* 0x16 - 'u'		*/	'u',		'U',		0,
/* 0x17 - 'i'		*/	'i',		'I',		0,
/* 0x18 - 'o'		*/	'o',		'O',		0,
/* 0x19 - 'p'		*/	'p',		'P',		0,
/* 0x1A - '['		*/	'[',		'{',		0,
/* 0x1B - ']'		*/	']',		'}',		0,
/* 0x1C - CR/LF		*/	ENTER,		ENTER,		PAD_ENTER,
/* 0x1D - l. Ctrl	*/	CTRL_L,		CTRL_L,		CTRL_R,
/* 0x1E - 'a'		*/	'a',		'A',		0,
/* 0x1F - 's'		*/	's',		'S',		0,
/* 0x20 - 'd'		*/	'd',		'D',		0,
/* 0x21 - 'f'		*/	'f',		'F',		0,
/* 0x22 - 'g'		*/	'g',		'G',		0,
/* 0x23 - 'h'		*/	'h',		'H',		0,
/* 0x24 - 'j'		*/	'j',		'J',		0,
/* 0x25 - 'k'		*/	'k',		'K',		0,
/* 0x26 - 'l'		*/	'l',		'L',		0,
/* 0x27 - ';'		*/	';',		':',		0,
/* 0x28 - '\''		*/	'\'',		'"',		0,
/* 0x29 - '`'		*/	'`',		'~',		0,
/* 0x2A - l. SHIFT	*/	SHIFT_L,	SHIFT_L,	0,
/* 0x2B - '\'		*/	'\\',		'|',		0,
/* 0x2C - 'z'		*/	'z',		'Z',		0,
/* 0x2D - 'x'		*/	'x',		'X',		0,
/* 0x2E - 'c'		*/	'c',		'C',		0,
/* 0x2F - 'v'		*/	'v',		'V',		0,
/* 0x30 - 'b'		*/	'b',		'B',		0,
/* 0x31 - 'n'		*/	'n',		'N',		0,
/* 0x32 - 'm'		*/	'm',		'M',		0,
/* 0x33 - ','		*/	',',		'<',		0,
/* 0x34 - '.'		*/	'.',		'>',		0,
/* 0x35 - '/'		*/	'/',		'?',		PAD_SLASH,
/* 0x36 - r. SHIFT	*/	SHIFT_R,	SHIFT_R,	0,
/* 0x37 - '*'		*/	'*',		'*',    	0,
/* 0x38 - ALT		*/	ALT_L,		ALT_L,  	ALT_R,
/* 0x39 - ' '		*/	' ',		' ',		0,
/* 0x3A - CapsLock	*/	CAPS_LOCK,	CAPS_LOCK,	0,
/* 0x3B - F1		*/	F1,			F1,			0,
/* 0x3C - F2		*/	F2,			F2,			0,
/* 0x3D - F3		*/	F3,			F3,			0,
/* 0x3E - F4		*/	F4,			F4,			0,
/* 0x3F - F5		*/	F5,			F5,			0,
/* 0x40 - F6		*/	F6,			F6,			0,
/* 0x41 - F7		*/	F7,			F7,			0,
/* 0x42 - F8		*/	F8,			F8,			0,
/* 0x43 - F9		*/	F9,			F9,			0,
/* 0x44 - F10		*/	F10,		F10,		0,
/* 0x45 - NumLock	*/	NUM_LOCK,	NUM_LOCK,	0,
/* 0x46 - ScrLock	*/	SCROLL_LOCK,SCROLL_LOCK,0,
/* 0x47 - Home		*/	PAD_HOME,	'7',		HOME,
/* 0x48 - CurUp		*/	PAD_UP,		'8',		UP,
/* 0x49 - PgUp		*/	PAD_PAGEUP,	'9',		PAGEUP,
/* 0x4A - '-'		*/	PAD_MINUS,	'-',		0,
/* 0x4B - Left		*/	PAD_LEFT,	'4',		LEFT,
/* 0x4C - MID		*/	PAD_MID,	'5',		0,
/* 0x4D - Right		*/	PAD_RIGHT,	'6',		RIGHT,
/* 0x4E - '+'		*/	PAD_PLUS,	'+',		0,
/* 0x4F - End		*/	PAD_END,	'1',		END,
/* 0x50 - Down		*/	PAD_DOWN,	'2',		DOWN,
/* 0x51 - PgDown	*/	PAD_PAGEDOWN,'3',		PAGEDOWN,
/* 0x52 - Insert	*/	PAD_INS,	'0',		INSERT,
/* 0x53 - Delete	*/	PAD_DOT,	'.',		DELETE,
/* 0x54 - Enter		*/	0,			0,			0,
/* 0x55 - ???		*/	0,			0,			0,
/* 0x56 - ???		*/	0,			0,			0,
/* 0x57 - F11		*/	F11,		F11,		0,	
/* 0x58 - F12		*/	F12,		F12,		0,	
/* 0x59 - ???		*/	0,			0,			0,	
/* 0x5A - ???		*/	0,			0,			0,	
/* 0x5B - ???		*/	0,			0,			GUI_L,	
/* 0x5C - ???		*/	0,			0,			GUI_R,	
/* 0x5D - ???		*/	0,			0,			APPS,	
/* 0x5E - ???		*/	0,			0,			0,	
/* 0x5F - ???		*/	0,			0,			0,
/* 0x60 - ???		*/	0,			0,			0,
/* 0x61 - ???		*/	0,			0,			0,	
/* 0x62 - ???		*/	0,			0,			0,	
/* 0x63 - ???		*/	0,			0,			0,	
/* 0x64 - ???		*/	0,			0,			0,	
/* 0x65 - ???		*/	0,			0,			0,	
/* 0x66 - ???		*/	0,			0,			0,	
/* 0x67 - ???		*/	0,			0,			0,	
/* 0x68 - ???		*/	0,			0,			0,	
/* 0x69 - ???		*/	0,			0,			0,	
/* 0x6A - ???		*/	0,			0,			0,	
/* 0x6B - ???		*/	0,			0,			0,	
/* 0x6C - ???		*/	0,			0,			0,	
/* 0x6D - ???		*/	0,			0,			0,	
/* 0x6E - ???		*/	0,			0,			0,	
/* 0x6F - ???		*/	0,			0,			0,	
/* 0x70 - ???		*/	0,			0,			0,	
/* 0x71 - ???		*/	0,			0,			0,	
/* 0x72 - ???		*/	0,			0,			0,	
/* 0x73 - ???		*/	0,			0,			0,	
/* 0x74 - ???		*/	0,			0,			0,	
/* 0x75 - ???		*/	0,			0,			0,	
/* 0x76 - ???		*/	0,			0,			0,	
/* 0x77 - ???		*/	0,			0,			0,	
/* 0x78 - ???		*/	0,			0,			0,	
/* 0x78 - ???		*/	0,			0,			0,	
/* 0x7A - ???		*/	0,			0,			0,	
/* 0x7B - ???		*/	0,			0,			0,	
/* 0x7C - ???		*/	0,			0,			0,	
/* 0x7D - ???		*/	0,			0,			0,	
/* 0x7E - ???		*/	0,			0,			0,
/* 0x7F - ???		*/	0,			0,			0
};

KBD_INPUT		kbd_in;

uint8_t			_scan_code;

int32_t			code_with_E0 = 0;
int32_t			shift_l;
int32_t			shift_r;
int32_t			alt_l;
int32_t			alt_r;
int32_t			ctrl_l;
int32_t			ctrl_r;
int32_t			caps_lock;
int32_t			num_lock;
int32_t			scroll_lock;
int32_t			column;

const uint8_t	pause_scode[]={	0xe1,0x1d,0x45,
						0xe1,0x9d,0xc5};

void interrupt __cdecl keyboard_irq()
{
	save();
	//	get keyboard buffer
	_scan_code = in_byte(0x60);
	out_byte(INT_M_CTL,EOI);
	
	//	for debug
	//(*((char*)0xb8000 + 11))++;

	_asm sti
	keyboard_irq_handler(0);
	_asm cli
	_asm ret

}

void initial_keyboard()
{
	DbgPrintf("Keyboard irq handler installed.\n");
	init_idt_desc(33,keyboard_irq,DA_386IGATE,PRIVILEGE_KRNL,0x8);

	shift_l = 0;
	shift_r = 0;
	alt_l	= 0;
	alt_r	= 0;
	ctrl_l	= 0;
	ctrl_r	= 0;

	caps_lock = 0;
	num_lock  = 1;
	scroll_lock = 0;

	//	initialize keyboard buffer
	kbd_in.count = 0;
	kbd_in.p_head = kbd_in.p_tail = kbd_in.buf;
	enable_irq(KEYBOARD_IRQ);
}

void keyboard_irq_handler(int irq)
{
	//	the handler just save received scan code
	if(kbd_in.count < KB_IN_BYTES){
		*(kbd_in.p_head) = _scan_code;
		kbd_in.p_head ++;
		if(kbd_in.p_head == kbd_in.buf + KB_IN_BYTES){
			kbd_in.p_head = kbd_in.buf;
		}
		kbd_in.count ++;
	}
}

uint8_t	get_byte_from_kbuf()
{
	uint8_t		scan_code = 0;

	while(kbd_in.count <= 0){_asm nop}

	disable_interrupt();
	scan_code = *(kbd_in.p_tail);
	kbd_in.p_tail ++;
	if(kbd_in.p_tail == kbd_in.buf + KB_IN_BYTES){
		kbd_in.p_tail = kbd_in.buf;
	}
	kbd_in.count --;
	enable_interrupt();

	return scan_code;
}

void keyboard_read(TTY* p_tty)
{
	uint8_t		scan_code;
	//int8_t		output[2];
	int32_t		make;

	uint32_t	key = 0;
	uint32_t*	keyrow;

	if(kbd_in.count > 0){

		code_with_E0 = 0;

		scan_code = get_byte_from_kbuf();

		//	decode the scan code
		if(scan_code == 0xe1){
			int i ;
			int	is_pausebreak = 1;
			for(i = 1;i<6; i++){
				if(get_byte_from_kbuf() != pause_scode[i]){
					is_pausebreak = 0;
					break;
				}
			}
			if(is_pausebreak){
				key = PAUSEBREAK;
			}
		}
		else if(scan_code == 0xe0){

			scan_code = get_byte_from_kbuf();

			//	print_screen down
			if(scan_code == 0x2a){
				if(get_byte_from_kbuf() == 0xe0){
					if(get_byte_from_kbuf() == 0x37){
						key = PRINTSCREEN;
						make = 1;
					}
				}
			}
			//	print_screen up
			if(scan_code == 0xb7){
				if(get_byte_from_kbuf() == 0xe0){
					if(get_byte_from_kbuf() == 0xaa){
						key = PRINTSCREEN;
						make = 0;
					}
				}
			}
			if(key == 0){
				code_with_E0 = 1;
			}
		}
		if((key != PAUSEBREAK) && (key != PRINTSCREEN)) {
			//	make code or break code
			make = (scan_code & FLAG_BREAK ? FALSE : TRUE);
			//	locate which vol
			keyrow = & keymap[(scan_code & 0x7f) * MAP_COLS];

			column = 0;
			if(shift_l || shift_r){
				column = 1;
			}
			if(code_with_E0){
				column = 2;
				code_with_E0 = 0;
			}

			key = keyrow[column];

			switch(key){
			case SHIFT_L:
				{
					shift_l = make;
				}
				break;
			case SHIFT_R:
				{
					shift_r = make;
				}
				break;
			case CTRL_L:
				{
					ctrl_l = make;
				}
				break;
			case CTRL_R:
				{
					ctrl_r = make;
				}
				break;
			case ALT_L:
				{
					alt_l = make;
				}
				break;
			case ALT_R:
				{
					alt_r = make;
				}
				break;
			default:
				break;
			}
			if( make ){
				key |= shift_l	?	FLAG_SHIFT_L	:	0;
				key |= shift_r	?	FLAG_SHIFT_R	:	0;
				key |= ctrl_l	?	FLAG_CTRL_L		:	0;
				key |= ctrl_r	?	FLAG_CTRL_R		:	0;
				key |= alt_l	?	FLAG_ALT_L		:	0;
				key |= alt_r	?	FLAG_ALT_R		:	0;
				in_process(p_tty,key);
			}
		}
	}
}