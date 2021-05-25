/*****************************************************************************
 * MK61 Clone Calculator Firmware for ATmega16L/32A                          *
 * Author: Artem Bondarenko aka chronicle95                                  *
 * Date: 25/03/2021                                                          *
 *                                                                           *
 * Notes:                                                                    *
 *  - Disable JTAG before flashing, it interferes with keypad.               *
 *  - Set frequency to 8 MHz                                                 *
 *****************************************************************************/

#ifdef __AVR_ATmega16__
#define F_CPU 8000000UL
#elif  __AVR_ATmega32__
#define F_CPU 16000000UL
#else
#error Unsupported MCU
#endif

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "calc.h"
#include "ik13.h"
#include "ir2.h"
#include "roms.h"

void init_io();
void init_timer();

inline void display_hex(U8 n, U8 data)
{
	Display[n] = pgm_read_byte (&SegmentData[data]);
}

Ik13 p1302;
Ik13 p1303;
Ik13 p1306;
Ir2  m1;
Ir2  m2;

int main(void)
{
	init_io ();
	init_timer ();

	p1302.init ((Cmd23*)&cmds_1302, (Synch*)&syn_1302, (UCmd28*)&ucmds_1302);
	p1303.init ((Cmd23*)&cmds_1303, (Synch*)&syn_1303, (UCmd28*)&ucmds_1303);
	p1306.init ((Cmd23*)&cmds_1306, (Synch*)&syn_1306, (UCmd28*)&ucmds_1306);
	m1.init ();
	m2.init ();

	U8 key = KEY_NONE;

	// infinite simulation loop
	for (;;)
	{
		p1303.key_x = SwitchValue;
		p1303.key_y = 1;

		for (U16 j = 0; j < 560; j++)
		{
			// one full instruction cycle
			for (U8 i = 0; i < 42; i++)
			{
				p1302.win = m2.wout;
				p1302.step ();

				p1303.win = p1302.wout;
				p1303.step ();

				p1306.win = p1303.wout;
				p1306.step ();

				m1.win = p1306.wout;
				m1.step ();

				m2.win = m1.wout;
				m2.step ();

				// complete the circle
				p1302.writeToMemory(m2.wout);
			}

			// need to update the display?
			if (p1302.disp_upd)
			{
				for (U8 i = 0; i < 9; i++)
				{
					display_hex (i, p1302.readFromRegister((8 - i) * 3));
					Display[i] |= p1302.disp_commas[9 - i];
				}
				for (U8 i = 0; i < 3; i++)
				{
					display_hex (i + 9, p1302.readFromRegister((11 - i) * 3));
					Display[i + 9] |= p1302.disp_commas[12 - i];
				}
				p1302.disp_upd = 0;
			} 

			// check for pressed keys
			if (KeyValue != key)
			{
				key = KeyValue;
				p1302.key_x = pgm_read_byte (&KeyMapX[key]);
				p1302.key_y = pgm_read_byte (&KeyMapY[key]);
			}
		}
	}
}

void init_io()
{
	DDRA = 0xff;    // segment control
	PORTA = 0x00;   // 0 - dark, 1 - lit
	DDRB = 0x0f;    // column selector, hi 4 bits
	PORTB = 0x0f;   // all high by default (for common cathode)
	DDRC = 0x00;    // input port for rows and a switch
	PORTC = 0xff;   // pulled up
	DDRD = 0xff;    // column selector, lo 8 bits
	PORTD = 0xff;   // all high by default (for common cathode)
}

void init_timer()
{
	// normal mode, prescaling 1/64
	TCCR0 = (1 << CS01) | (1 << CS00);
	// interrupt on overflow
	TIMSK = (1 << TOIE0);
	// reset the counter
	TCNT0 = 0;
	sei();
}

ISR(TIMER0_OVF_vect)
{
	// prevent other interrupts
	cli ();

	// check keys and switch status
	char Tmp = PINC & 0x3f;
	if (Tmp != 0x3f)
	{
		char Off = 1;
		for (; Tmp & 1; Off += 5, Tmp >>= 1);
		KeyValue = Off + Column;
		KeyStatus = KEYS_DOWN;
	}
	SwitchValue = PINC & 0x80 ? SWITCH_DEG : SWITCH_RAD;

	// calculate new column number and check key status
	Column++;
	if (Column == sizeof(Display))
	{
		if (KeyStatus == KEYS_UP)
		{
			KeyValue = KEY_NONE;
		}
		KeyStatus = KEYS_UP;
		Column    = 0;
	}

	// clear segments to avoid shadowing
	PORTA = 0x00;

	// switch (select) new column
	if (Column < 8)
	{
		PORTD = ~(1 << Column);
		PORTB = 0x0f;
	}
	else
	{
		PORTD = 0xff;
		PORTB = ~(1 << (Column - 8));
	}

	// light the segments
	PORTA = Display[Column];

	// restore interrupts
	sei ();
}
