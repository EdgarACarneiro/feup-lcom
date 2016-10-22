#include <minix/syslib.h>
#include <minix/drivers.h>
#include <minix/com.h>
#include <minix/sysutil.h>
#include "timer.h"
#include "keyboard.h"

#define DELAY_TO		20000   // KBC respond Time-Out in micro seconds

static int kbd_hook_id = KBD_INITIAL_HOOK_ID;

int kbd_subscribe_int(void)
{
	if ( sys_irqsetpolicy (KBC_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &kbd_hook_id) != OK ) {
		printf("kbd_subscribe_int() -> FAILED sys_irqsetpolicy()\n");
		return -1;
	}
	if ( sys_irqenable (&kbd_hook_id) != OK ) {
		printf("kbd_subscribe_int() -> FAILED sys_irqenable()\n");
		return -1;
	}

	return KBD_INITIAL_HOOK_ID;
}

int kbd_unsubscribe_int(void)
{
	if ( sys_irqdisable (&kbd_hook_id) != OK ) {
		printf("kbd_unsubscribe_int() -> FAILED sys_irqdisable()\n");
		return -1;
	}
	if ( sys_irqrmpolicy (&kbd_hook_id) != OK ) {
		printf("kbd_unsubscribe_int() -> FAILED sys_irqrmpolicy()\n");
		return -1;
	}

	return KBD_INITIAL_HOOK_ID;
}

u8_t keyboard_read()	// Reads Keyboard Data from OutPut Buffer
{
	u8_t stat, data;
	unsigned iter;
	while( iter++ < maxIter ) {
		if ( sys_inb(STAT_REG, &stat) != OK ) {
			printf("keyboard_read() -> FAILED sys_inb()");
			return -1;
		}
		/* loop while 8042 output buffer is empty */
		if( stat & STAT_OBF ) {
			if ( sys_inb(OUT_BUF, &data) != OK ) {
				printf("keyboard_read() -> FAILED sys_inb()");
				return -1;	// Returns -1 or 0xFF on failure
			}
			if ( (stat & (STAT_PARITY | STAT_TIMEOUT)) == 0 )	// Error Ocurred ?
				return data;
			else
				return -1;	// Returns -1 or 0xFF on failure
		}
	tickdelay(micros_to_ticks(DELAY_US));
	}
}

int keyboard_write(char command, char arg) {
    unsigned char kbcResponse;

    while ( 1 ) {
        if (STAT_REG & STAT_IBF)
            return 1; //Input buffer is full

        if (sys_outb (KB_IN_BUF, command)) {
            printf ("keyboard_write() -> FAILED sys_outb()\n");
            return 1;
        }

        if (sys_inb (KBD_IN_BUF, * kbcResponse)) {
            printf ("keyboard_write() -> FAILED sys_outb()\n");
            return 1;
        }

        if (kbcResponse != IN_RESEND && kbcResponse != IN_ERROR)
        {
            while (1) {
                if (sys_outb (KBD_IN_BUF, arg)) {
                    printf ("keyboard_write() -> FAILED sys_outb()\n");
                    return 1;
                }

                if (sys_inb (KBD_IN_BUF, * kbcResponse)) {
                    printf ("keyboard_write() -> FAILED sys_outb()\n");
                    return 1;
                }

                if (kbcResponse != IN_RESEND && kbcResponse != IN_ERROR)
                    return OK;

                if (kbcResponse == IN_ERROR)
                    brake;
                }
        }
    }
}

// TODO: passar prints para outra função
int keyboard_handler(int * status)	// To be called on KBC Interrupt
{
	u8_t data;
	if ( (data = keyboard_read()) == -1 ) {
		printf("keyboard_handler() -> FAILED keyboard_read()");
		return 1;
	}

	if ( data == 0xE0 ) {
		*status = 1;	// Set status to 1 (awaiting next byte)
		return OK;
	}

	// TODO: mudar para outra funcao, talvez o if acim tb
	// MakeCode or BreakCode ?
	bool isBreak = data & 0xF0;
	if ( isBreak )
		printf("Breakcode: ");
	else
		printf("Makecode:  ");

	if ( status ) {
		printf("0xE0 %x\n", data);
		*status = 0;	// Set status to 0
		return 0;
	} else if ( data & 0x81 ) {	// ESC BreakCode
		printf("%x - ESC BreakCode\n", data);
		*status = 2;	// Set status to 2 (ESC BreakCode Detected)
		return 0;
	} else {
		printf("%x\n", data);
		*status = 0;	// Set status to 0
		return 0;
	}
}