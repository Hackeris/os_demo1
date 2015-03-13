#include "tty.h"
#include "kbd.h"
#include "klib.h"

#include "dbg.h"

TTY			tty_table[NR_CONSOLES];
CONSOLE		console_table[NR_CONSOLES];

uint32_t	nr_current_console;

uint32_t		disp_pos = 0;

void task_tty()
{
	TTY*	p_tty;

	initial_keyboard();

	for(p_tty = TTY_FIRST;p_tty < TTY_END;p_tty ++){
		init_tty(p_tty);
	}
	p_tty = tty_table;
	select_console(0);
	while(1){
		tty_do_read(&tty_table[nr_current_console]);
		tty_do_write(&tty_table[nr_current_console]);
	}
}

void init_tty(TTY* p_tty)
{
	p_tty->inbuf_count = 0;
	p_tty->p_inbuf_head = p_tty->p_inbuf_tial = p_tty->in_buf;

	init_screen(p_tty);
}

void in_process(TTY* p_tty,uint32_t key)
{
	if(!(key & FLAG_EXT)){
		if(p_tty->inbuf_count < TTY_IN_BYTES){
			*(p_tty->p_inbuf_head) = key;
			p_tty->p_inbuf_head ++;
			if(p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES){
				p_tty->p_inbuf_head = p_tty->in_buf;
			}
			p_tty->inbuf_count ++;
		}
	}
	else{
		int raw_code = key & MASK_RAW;
		switch(raw_code){
		case UP:
			if((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)){
				scroll_screen(p_tty->p_console,SCR_DN);
			}
			break;
		case DOWN:
			if((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)){
				scroll_screen(p_tty->p_console,SCR_UP);
			}
			break;
		case ENTER:
			put_key(p_tty,'\n');
			break;
		case BACKSPACE:
			put_key(p_tty,'\b');
			break;
		case F1:
		case F2:
		case F3:
		case F4:
		case F5:
		case F6:
		case F7:
		case F8:
		case F9:
		case F10:
		case F11:
		case F12:
			if(key & FLAG_ALT_L){
				select_console(raw_code - F1);
			}
		default:
			break;
		}
	}
}

int is_current_console(CONSOLE* p_con)
{
	return (p_con == &console_table[nr_current_console]);
}

void tty_do_read(TTY* p_tty)
{
	if(is_current_console(p_tty->p_console)){
		keyboard_read(p_tty);
	}
}

void tty_do_write(TTY* p_tty)
{
	if(p_tty->inbuf_count){
		char ch = *(p_tty->p_inbuf_tial);
		p_tty->p_inbuf_tial ++;
		if(p_tty->p_inbuf_tial == p_tty->in_buf+TTY_IN_BYTES){
			p_tty->p_inbuf_tial = p_tty->in_buf;
		}
		p_tty->inbuf_count --;

		out_char(p_tty->p_console,ch);
	}
}

void out_char(CONSOLE* p_con,char ch)
{
	uint8_t*	p_vmem = (uint8_t*)(V_MEM_BASE + p_con->cursor * 2);

	switch(ch){
	case '\n':
		if(p_con->cursor < p_con->original_addr + p_con->v_mem_limit - SCREEN_WIDTH){
			p_con->cursor = p_con->original_addr + SCREEN_WIDTH * 
				((p_con->cursor - p_con->original_addr) / SCREEN_WIDTH + 1);
		}
		break;
	case '\b':
		if(p_con->cursor > p_con->original_addr){
			p_con->cursor --;
			*(p_vmem - 2) = ' ';
			*(p_vmem - 1) = DEFAULT_CHAR_COLOR;
		}
		break;
	default:
		{
			if(p_con->cursor < p_con->original_addr + p_con->v_mem_limit - 1){
				*p_vmem++ = ch;
				*p_vmem++ = DEFAULT_CHAR_COLOR;
				p_con->cursor++;
			}
		}
		break;
	}
	while(p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE ){
		scroll_screen(p_con,SCR_DN);
	}
	if(nr_current_console == p_con - console_table){
		flush(p_con);
	}
}

void set_cursor(uint32_t position)
{
	disable_interrupt();
	out_byte(CRTC_ADDR_REG,CURSOR_H);
	out_byte(CRTC_DATA_REG,(position >> 8) & 0xff);
	out_byte(CRTC_ADDR_REG,CURSOR_L);
	out_byte(CRTC_DATA_REG,(position & 0xff));
	enable_interrupt();
}

void init_screen(TTY* p_tty)
{
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = &console_table[nr_tty];

	int v_mem_size = V_MEM_SIZE >> 1;

	int con_v_mem_size = v_mem_size / NR_CONSOLES;
	p_tty->p_console->original_addr	= nr_tty * con_v_mem_size;
	p_tty->p_console->v_mem_limit	= con_v_mem_size;
	p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;
	p_tty->p_console->cursor		= p_tty->p_console->original_addr;

	if(nr_tty == 0){
		p_tty->p_console->cursor = disp_pos / 2;
		disp_pos = 0;
	}
	else{
		out_char(p_tty->p_console,nr_tty + '0');
		out_char(p_tty->p_console,'#');
	}
	set_cursor(p_tty->p_console->cursor);
}

void select_console(int nr_console)
{
	if(nr_console < 0 || nr_console >= NR_CONSOLES){
		return;
	}
	nr_current_console = nr_console ;
	flush(&console_table[nr_console]);
}

void set_video_start_addr(uint32_t	addr)
{
	disable_interrupt();
	out_byte(CRTC_ADDR_REG,START_ADDR_H);
	out_byte(CRTC_DATA_REG,(addr >> 8) & 0xff);
	out_byte(CRTC_ADDR_REG,START_ADDR_L);
	out_byte(CRTC_DATA_REG,(addr) & 0xff);
	enable_interrupt();
}

void scroll_screen(CONSOLE* p_con,int direction)
{
	if(direction == SCR_UP){
		if(p_con->current_start_addr > p_con->original_addr){
			p_con->current_start_addr -= SCREEN_WIDTH;
		}
	}
	else if(direction ==SCR_DN){
		if(p_con->current_start_addr + SCREEN_SIZE <
			p_con->original_addr + p_con->v_mem_limit){
				p_con->current_start_addr += SCREEN_WIDTH;
		}
	}

	//set_video_start_addr(p_con->current_start_addr);
	//set_cursor(p_con->cursor);
}

void put_key(TTY* p_tty,uint32_t key)
{
	if(p_tty->inbuf_count < TTY_IN_BYTES){
		*(p_tty->p_inbuf_head) = key;
		p_tty->p_inbuf_head ++;
		if(p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES){
			p_tty->p_inbuf_head = p_tty->in_buf;
		}
		p_tty->inbuf_count ++;
	}
}

void flush(CONSOLE* p_con)
{
	set_cursor(p_con->cursor);
	set_video_start_addr(p_con->current_start_addr);
}

void tty_write(TTY* p_tty,char* buf,int len)
{
	char* p = buf;
	int i = len;
	while(i){
		out_char(p_tty->p_console,*p++);
		i--;
	}
}
