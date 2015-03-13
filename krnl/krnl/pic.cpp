#include "pic.h"
#include "klib.h"

void init_8259a()
{
	//	master 8259,ICW1
	out_byte(INT_M_CTL,0x11);
	//	slave 8259,ICW1
	out_byte(INT_S_CTL,0x11);

	//	master 8259,ICW2, set mater 8259 interrupt entry to 0x20
	out_byte(INT_M_CTLMASK,INT_VECTOR_IRQ0);
	//	slave 8259,ICW2, set mater 8259 interrupt entry to 0x28
	out_byte(INT_S_CTLMASK,INT_VECTOR_IRQ8);

	//	master 8259,ICW3, IR2 refer to slave 8259
	out_byte(INT_M_CTLMASK,0x4);
	//	slave 8259,ICW3, refer to master 8259 IR2
	out_byte(INT_S_CTLMASK,0x2);

	//	master ICW4
	out_byte(INT_M_CTLMASK,0x1);
	//	slave ICW4
	out_byte(INT_S_CTLMASK,0x1);

	//	master OCW1
	out_byte(INT_M_CTLMASK,0xff);
	//	slave OCW1
	out_byte(INT_S_CTLMASK,0xff);
}

void i86_pic_initialize()
{
	init_8259a();
}