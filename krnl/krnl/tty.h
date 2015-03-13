#ifndef _H_TTY
#define _H_TTY
#include "ctype.h"

/* Color */
/*
 * e.g. MAKE_COLOR(BLUE, RED)
 *      MAKE_COLOR(BLACK, RED) | BRIGHT
 *      MAKE_COLOR(BLACK, RED) | BRIGHT | FLASH
 */
#define BLACK   0x0     /* 0000 */
#define WHITE   0x7     /* 0111 */
#define RED     0x4     /* 0100 */
#define GREEN   0x2     /* 0010 */
#define BLUE    0x1     /* 0001 */
#define FLASH   0x80    /* 1000 0000 */
#define BRIGHT  0x08    /* 0000 1000 */
#define MAKE_COLOR(x,y) (x | y) /* MAKE_COLOR(Background,Foreground) */
#define DEFAULT_CHAR_COLOR	0x07	/* 0000 0111 ºÚµ×°××Ö */

/* VGA */
#define	CRTC_ADDR_REG	0x3D4	/* CRT Controller Registers - Addr Register */
#define	CRTC_DATA_REG	0x3D5	/* CRT Controller Registers - Data Register */
#define	START_ADDR_H	0xC	/* reg index of video mem start addr (MSB) */
#define	START_ADDR_L	0xD	/* reg index of video mem start addr (LSB) */
#define	CURSOR_H	0xE	/* reg index of cursor position (MSB) */
#define	CURSOR_L	0xF	/* reg index of cursor position (LSB) */
#define	V_MEM_BASE	0xB8000	/* base of color video memory */
#define	V_MEM_SIZE	0x8000	/* 32K: B8000H -> BFFFFH */

#define	TTY_IN_BYTES			256		//	tty input queue size

#define	NR_CONSOLES			3

#define TTY_FIRST			tty_table
#define	TTY_END				(tty_table + NR_CONSOLES)

#define SCREEN_SIZE		(80 * 25)
#define SCREEN_WIDTH		80

#define SCR_UP	1	/* scroll forward */
#define SCR_DN	-1	/* scroll backward */

typedef struct _tagCONSOLE{
	uint32_t	current_start_addr;
	uint32_t	original_addr;
	uint32_t	v_mem_limit;
	uint32_t	cursor;
}CONSOLE;

typedef struct _tagTTY{
	uint32_t	in_buf[TTY_IN_BYTES];
	uint32_t*	p_inbuf_head;
	uint32_t*	p_inbuf_tial;
	int32_t		inbuf_count;

	CONSOLE*	p_console;
}TTY;

void task_tty();

void in_process(TTY* p_tty,uint32_t key);

void init_tty(TTY* p_tty);

int is_current_console(CONSOLE* p_con);

void tty_do_read(TTY* p_tty);

void tty_do_write(TTY* p_tty);

void out_char(CONSOLE* p_con,char ch);

void set_cursor(uint32_t position);

void init_screen(TTY* p_tty);

void select_console(int nr_console);

void set_video_start_addr(uint32_t	addr);

void scroll_screen(CONSOLE* p_con,int direction);

void put_key(TTY* p_tty,uint32_t key);

void flush(CONSOLE* p_con);

void tty_write(TTY* p_tty,char* buf,int len);

#endif