#include <minix/syslib.h>
#include <minix/drivers.h>
#include <minix/com.h>
#include "i8254.h"
#include "timer.h"

static const char kernel_call_failure[] = "kernel call returned non-zero value";

static unsigned long timerCount = 0;
static int hook_id = TIMER0_IRQSET;

// TODO Reduce Code Size!!
int timer_delay(unsigned int time) {
	int ipc_status;
	message msg;

	unsigned long elapsed = 0;	// interrupts handled since device hooked

	int irq_set;
	if ((irq_set = BIT(timer_subscribe_int())) < 0) { // hook_id returned for TIMER 0
		fprintf(stderr, "Error: %s\n", "device subscribe unsuccessful");
		return 1;
	}
	unsigned int freq = 60;	// Set timer frequence to 60 TODO: fetch frequence and wait accordingly?
	timer_set_square(0, freq);
	int r;

	while (elapsed < (time * freq)) {
		/* Get a request message. */
		if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
			fprintf(stderr, "driver_receive failed with: %d\n", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) { /* received notification */
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE: /* hardware interrupt notification */
				if (msg.NOTIFY_ARG & irq_set) { /* subscribed interrupt */
					++elapsed;
				}
				break;
			default:
				break; /* no other notifications expected: do nothing */
			}
		} else { /* received a standard message, not a notification */
			/* no standard messages expected: do nothing */
			fprintf(stderr, "Error: %s\n",
					"received unexpected standard message");
		}
	}

	if (timer_unsubscribe_int() < 0) {
		fprintf(stderr, "Error: %s\n", "device UNsubscribe unsuccessful");
		return 1;
	}

	return OK;
}

int timer_set_square(unsigned long timer, unsigned long freq) {
	const unsigned long bit_mask = 0xFF;	//Selects the 8 lsb bits

	//If it's not any of the possible timers
	if (timer != 0 && timer != 1 && timer != 2) {
		fprintf(stderr, "Error: %s\n", "non-existent timer selected.");
		return 1;
	}

	//Frequency can not be 0 nor above the max allowed
	if (freq == 0 || (freq > TIMER_FREQ)) {
		fprintf(stderr, "Error: %s\n",
				"frequency can not be 0 nor above the max allowed.");
		return 1;
	}

	//Setting the lsb and msb
	unsigned long Div = TIMER_FREQ / freq;
	unsigned long lsb = Div & bit_mask;
	unsigned long msb = (Div >> 8) & bit_mask;

	//Initialization of the control word we will use
	unsigned char CTRL_Word = 0;

	timer_get_conf(timer, &CTRL_Word);

	/* We do not mess with the 4th 1st bits.
	 * We activate both BIT(5) and BIT(4) because we want to access the LSB followed by the MSB.
	 * The counter selection will be programmed further ahead. */
	CTRL_Word = CTRL_Word | BIT(4) | BIT(5);

	if (timer == 0) {
		//Not changing any bit will make it access the Timer 0
		if (sys_outb(TIMER_CTRL, CTRL_Word) != OK) {
			fprintf(stderr, "Error: %s.\n", kernel_call_failure);
			return 1;
		}

		if (sys_outb(TIMER_0, lsb) != OK) {
			fprintf(stderr, "Error: %s.\n", kernel_call_failure);
			return 1;
		}

		if (sys_outb(TIMER_0, msb) != OK) {
			fprintf(stderr, "Error: %s.\n", kernel_call_failure);
			return 1;
		}
	} else if (timer == 1) {
		CTRL_Word = CTRL_Word | BIT(6); //Setting the 6th Bit to 1 so we will access the Timer 1

		if (sys_outb(TIMER_CTRL, CTRL_Word) != OK) {
			fprintf(stderr, "Error: %s.\n", kernel_call_failure);
			return 1;
		}

		if (sys_outb(TIMER_1, lsb) != OK) {
			fprintf(stderr, "Error: %s.\n", kernel_call_failure);
			return 1;
		}

		if (sys_outb(TIMER_1, msb) != OK) {
			fprintf(stderr, "Error: %s.\n", kernel_call_failure);
			return 1;
		}
	} else if (timer == 2) {
		CTRL_Word = CTRL_Word | BIT(7); //Setting the 7ht Bit to 1 so we will access the Timer2

		if (sys_outb(TIMER_CTRL, CTRL_Word) != OK) {
			fprintf(stderr, "Error: %s.\n", kernel_call_failure);
			return 1;
		}

		if (sys_outb(TIMER_2, lsb) != OK) {
			fprintf(stderr, "Error: %s.\n", kernel_call_failure);
			return 1;
		}

		if (sys_outb(TIMER_2, msb) != OK) {
			fprintf(stderr, "Error: %s.\n", kernel_call_failure);
			return 1;
		}
	}

	return OK;
}

int timer_subscribe_int(void) {

	if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id) != OK) {
		fprintf(stderr, "Error: %s.\n", kernel_call_failure);
		return -1;
	}

	if (sys_irqenable(&hook_id) != OK) {
		fprintf(stderr, "Error: %s.\n", kernel_call_failure);
		return -1;
	}

	return TIMER0_IRQSET;
}

int timer_unsubscribe_int() {
	if (sys_irqdisable(&hook_id) != OK) {
		fprintf(stderr, "Error: %s.\n", kernel_call_failure);
		return -1;
	}

	if (sys_irqrmpolicy(&hook_id) != OK) {
		fprintf(stderr, "Error: %s.\n", kernel_call_failure);
		return -1;
	}

	return OK;
}

int timer_get_conf(unsigned long timer, unsigned char *st) {

	port_t port;
	u8_t config = TIMER_RB_CMD;
	config |= TIMER_RB_COUNT_;	// 0 value is active!
	// deactivated COUNT, meaning only STATUS command is active

	if (timer == 0) {
		port = TIMER_0;
		config |= TIMER_RB_SEL(0);
	} else if (timer == 1) {
		port = TIMER_1;
		config |= TIMER_RB_SEL(1);
	} else if (timer == 2) {
		port = TIMER_2;
		config |= TIMER_RB_SEL(2);
	} else {
		fprintf(stderr, "Error: %s.\n",
				"timer must be an integer in range [0, 2]");
		return 1;	// not successful
	}

	// Send ReadBack command to the control register
	if (sys_outb(TIMER_CTRL, config)) {
		fprintf(stderr, "Error: %s.\n", kernel_call_failure);
		return 1;
	}

	// Read TIMER status register
	if (sys_inb(port, (unsigned long int *) st)) {
		fprintf(stderr, "Error: %s\n", kernel_call_failure);
		return 1;
	}

	return OK;
}

int timer_test_square(unsigned long freq) {
	return timer_set_square(0, freq);
}
