#include <gccore.h>
#include <string.h>
#include "ipl.h"

u8 BS2Ntsc448IntAa[] = {
	0x00,0x00,0x00,0x00,
	0x02,0x50,0x00,0xE2,
	0x01,0xC0,0x00,0x28,
	0x00,0x10,0x02,0x80,
	0x01,0xC0,0x00,0x00,
	0x00,0x00,0x00,0x01,
	0x00,0x01,0x03,0x02,
	0x09,0x06,0x03,0x0A,
	0x03,0x02,0x09,0x06,
	0x03,0x0A,0x09,0x02,
	0x03,0x06,0x09,0x0A,
	0x09,0x02,0x03,0x06,
	0x09,0x0A,0x08,0x08,
	0x0A,0x0C,0x0A,0x08,
	0x08,0x00,0x00,0x00
};

// bootrom descrambler reversed by segher
// Copyright 2008 Segher Boessenkool <segher@kernel.crashing.org>
void Descrambler(unsigned char* data, unsigned int size)
{
	unsigned char acc = 0;
	unsigned char nacc = 0;

	unsigned short t = 0x2953;
	unsigned short u = 0xd9c2;
	unsigned short v = 0x3ff1;

	unsigned char x = 1;
	unsigned int it;
	for (it = 0; it < size; )
	{
		int t0 = t & 1;
		int t1 = (t >> 1) & 1;
		int u0 = u & 1;
		int u1 = (u >> 1) & 1;
		int v0 = v & 1;

		x ^= t1 ^ v0;
		x ^= (u0 | u1);
		x ^= (t0 ^ u1 ^ v0) & (t0 ^ u0);

		if (t0 == u0)
		{
			v >>= 1;
			if (v0)
				v ^= 0xb3d0;
		}

		if (t0 == 0)
		{
			u >>= 1;
			if (u0)
				u ^= 0xfb10;
		}

		t >>= 1;
		if (t0)
			t ^= 0xa740;

		nacc++;
		acc = 2*acc + x;
		if (nacc == 8)
		{
			data[it++] ^= acc;
			nacc = 0;
		}
	}
}

#define IPL_ROM_FONT_SJIS	0x1AFF00

#define DECRYPT_START		0x100
#define CODE_START			0x820

void load_ipl(unsigned char *buf, bool prog, bool sharp, int jingle, int type)
{
	Descrambler(buf + DECRYPT_START, IPL_ROM_FONT_SJIS - DECRYPT_START);
	memcpy((void*)0x81300000, buf + CODE_START, IPL_ROM_FONT_SJIS - CODE_START);
	
	//This is my GC's NTSC-U 1.2 IPL, April 15 2003
	if(*(u32 *)0x8130B904 == 0x3800000C) {
		// 480p video mode
		if(prog) {
			*(s16 *)0x81300876 = type; //swiss
			*(s16 *)0x8137ECBA = type; //2 = 480p
			*(s16 *)0x8137ECCE = 0; //single field
			// Vertical filter
			*(s16 *)0x8137ECEA = 0x0408;
			*(u32 *)0x8137ECEC = 0x0C100C08;
			*(s16 *)0x8137ECF0 = 0x0400;
		}
		else {
			//test if this removes the video refresh
			*(s16 *)0x81300876 = type; //swiss
			//*(s16 *)0x8137ECBA = type; //video mode
		}
		// Check for no deflicker
		if(sharp) {
			*(s16 *)0x8137ECEA = 0x0000;
			*(u32 *)0x8137ECEC = 0x15161500;
			*(s16 *)0x8137ECF0 = 0x0000;
		}
		
		// Accept any region code. JP games don't need this, but PAL ones do
		*(s16 *)0x81300ACE = 1;
		*(s16 *)0x81300AF2 = 1;
		
		// Force boot sound. 1=kid, 2=kabuki
		if(jingle > 0)
			*(u32 *)0x81303184 = 0x38600000 | jingle;
		
		// SUSO: JP games will show the NTSC-U IPL in Japanese
		// Force English language
		*(s16 *)0x8130B906 = 38;
		*(s16 *)0x8130B926 = 10;
		*(s16 *)0x8130B92E = 39;
		*(s16 *)0x8130B932 = 15;
		*(s16 *)0x8130B93A = 7;
		*(s16 *)0x8130B93E = 1;
		*(s16 *)0x8130B946 = 4;
		*(s16 *)0x8130B94A = 45;
		*(s16 *)0x8130B952 = 46;
		*(s16 *)0x8130B956 = 42;
		*(s16 *)0x8130B95E = 40;
		*(s16 *)0x8130B962 = 43;
		*(s16 *)0x8130B976 = 31;
		*(s16 *)0x8130B97A = 29;
		*(s16 *)0x8130B982 = 30;
		*(s16 *)0x8130B98E = 80;
	}
	else if(*(u32 *)0x8130B91C == 0x3800000C) { //other NTSC-U 1.2
		// 480p video mode
		if(prog) {
			*(s16 *)0x81300876 = type; //swiss
			*(s16 *)0x8137F13A = type; //2 = 480p
			*(s16 *)0x8137F14E = 0; //single field
			// Vertical filter
			*(s16 *)0x8137F16A = 0x0408;
			*(u32 *)0x8137F16C = 0x0C100C08;
			*(s16 *)0x8137F170 = 0x0400;
		}
		// Check for no deflicker
		if(sharp) {
			*(s16 *)0x8137F16A = 0x0000;
			*(u32 *)0x8137F16C = 0x15161500;
			*(s16 *)0x8137F170 = 0x0000;
		}
		
		// Accept any region code.
		*(s16 *)0x81300ACE = 1;
		*(s16 *)0x81300AF2 = 1;
		
		// Force boot sound. 1=kid, 2=kabuki
		if(jingle > 0)
			*(u32 *)0x8130319C = 0x38600000 | jingle;
		
		// Force English language
		*(s16 *)0x8130B91E = 38;
		*(s16 *)0x8130B93E = 10;
		*(s16 *)0x8130B946 = 39;
		*(s16 *)0x8130B94A = 15;
		*(s16 *)0x8130B952 = 7;
		*(s16 *)0x8130B956 = 1;
		*(s16 *)0x8130B95E = 4;
		*(s16 *)0x8130B962 = 45;
		*(s16 *)0x8130B96A = 46;
		*(s16 *)0x8130B96E = 42;
		*(s16 *)0x8130B976 = 40;
		*(s16 *)0x8130B97A = 43;
		*(s16 *)0x8130B98E = 31;
		*(s16 *)0x8130B992 = 29;
		*(s16 *)0x8130B99A = 30;
		*(s16 *)0x8130B9A6 = 80;
	}
	else if(*(u32 *)0x8130B590 == 0x3800000C) { //NTSC-U 1.1
		// 480p video mode
		if(prog) {
			*(s16 *)0x81300522 = type; //swiss
			*(s16 *)0x8137D9F2 = type; //2 = 480p
			*(s16 *)0x8137DA06 = 0; //single field
			// Vertical filter
			*(s16 *)0x8137DA22 = 0x0408;
			*(u32 *)0x8137DA24 = 0x0C100C08;
			*(s16 *)0x8137DA28 = 0x0400;
		}
		// Check for no deflicker
		if(sharp) {
			*(s16 *)0x8137DA22 = 0x0000;
			*(u32 *)0x8137DA24 = 0x15161500;
			*(s16 *)0x8137DA28 = 0x0000;
		}
		
		// Accept any region code.
		*(s16 *)0x8130077E = 1;
		*(s16 *)0x813007A2 = 1;
		
		// Force boot sound. 1=kid, 2=kabuki
		if(jingle > 0)
			*(u32 *)0x81302DE8 = 0x38600000 | jingle;
		
		// Force English language
		*(s16 *)0x8130B592 = 38;
		*(s16 *)0x8130B5B2 = 10;
		*(s16 *)0x8130B5BA = 39;
		*(s16 *)0x8130B5BE = 15;
		*(s16 *)0x8130B5C6 = 7;
		*(s16 *)0x8130B5CA = 1;
		*(s16 *)0x8130B5D2 = 4;
		*(s16 *)0x8130B5D6 = 45;
		*(s16 *)0x8130B5DE = 46;
		*(s16 *)0x8130B5E2 = 42;
		*(s16 *)0x8130B5EA = 40;
		*(s16 *)0x8130B5EE = 43;
		*(s16 *)0x8130B602 = 31;
		*(s16 *)0x8130B606 = 29;
		*(s16 *)0x8130B60E = 30;
		*(s16 *)0x8130B61A = 80;
	}
	else if(*(u32 *)0x8130B408 == 0x3800000B) { //NTSC-U 1.0
		// 480p video mode
		if(prog) {
			*(s16 *)0x81300712 = 2 & ~0x3; //swiss, 480p does not work
		//	*(s16 *)0x8135DDE2 = 2; //2 = 480p
		//	*(s16 *)0x8135DDF6 = 0; //field rendering
			// Vertical filter
			*(s16 *)0x8135DE12 = 0x0408;
			*(u32 *)0x8135DE14 = 0x0C100C08;
			*(s16 *)0x8135DE18 = 0x0400;
		}
		// Check for no deflicker
		if(sharp) {
			*(s16 *)0x8135DE12 = 0x0000;
			*(u32 *)0x8135DE14 = 0x15161500;
			*(s16 *)0x8135DE18 = 0x0000;
		}
		
		// Accept any region code
		*(s16 *)0x81300E8A = 1;
		*(s16 *)0x81300EA2 = 1;
		*(s16 *)0x81300EAA = 1;
		
		// Force boot sound. 1=kid, 2=kabuki
		if(jingle > 0)
			*(u32 *)0x81302F00 = 0x38600000 | jingle;
		
		// Force English language
		*(s16 *)0x8130B40A = 38;
		*(s16 *)0x8130B412 = 38;
		*(s16 *)0x8130B416 = 39;
		*(s16 *)0x8130B422 = 15;
		*(s16 *)0x8130B42E = 7;
		*(s16 *)0x8130B43A = 1;
		*(s16 *)0x8130B446 = 4;
		*(s16 *)0x8130B44E = 45;
		*(s16 *)0x8130B452 = 46;
		*(s16 *)0x8130B45A = 42;
		*(s16 *)0x8130B45E = 40;
		*(s16 *)0x8130B466 = 43;
		*(s16 *)0x8130B476 = 31;
		*(s16 *)0x8130B47E = 29;
		*(s16 *)0x8130B482 = 30;
		*(s16 *)0x8130B48E = 80;
	}
	else if(*(u32 *)0x81300610 == 0x38600004) { //PAL 1.2
		// 480p video mode, this needs to be fixed for PAL60
		if(prog) {
			*(s16 *)0x81300612 = type; //swiss
			
			// Correct animation speed
			*(s16 *)0x8130F306 = 10;
			*(s16 *)0x8130F30A = 255;
			*(s16 *)0x8130F316 = 7;
			*(s16 *)0x8130F31A = 6;
			*(s16 *)0x8130F31E = 5;
			*(s16 *)0x8130F322 = 16;
			*(s16 *)0x8130F326 = 18;
			*(s16 *)0x8130F32A = 20;
			*(s16 *)0x8130F332 = 40;
			*(s16 *)0x8130F336 = 60;
			*(u32 *)0x8130F370 = 0xB3E30016;
			*(u32 *)0x8130F378 = 0x9963000B;
			*(u32 *)0x8130F37C = 0x9BC3001C;
			*(u32 *)0x8130F380 = 0x9BA30037;
			*(u32 *)0x8130F384 = 0x99430035;
			
			memcpy((void*)0x81382470, BS2Ntsc448IntAa, sizeof(BS2Ntsc448IntAa));
			
			*(s16 *)0x81382472 = type; //2 = 480p
			*(s16 *)0x81382486 = 0; //field rendering
			// Vertical filter
			*(s16 *)0x813824A2 = 0x0408;
			*(u32 *)0x813824A4 = 0x0C100C08;
			*(s16 *)0x813824A8 = 0x0400;
		}
		// Check for no deflicker
		if(sharp) {
			*(s16 *)0x813824A2 = 0x0000;
			*(u32 *)0x813824A4 = 0x15161500;
			*(s16 *)0x813824A8 = 0x0000;
		}
		
		// Accept any region code
		*(s16 *)0x81300882 = 1;
		*(s16 *)0x813008A6 = 1;
		
		// Force boot sound. 1=kid, 2=kabuki
		if(jingle > 0)
			*(u32 *)0x81302F50 = 0x38600000 | jingle;
	}
	else if(*(u32 *)0x81300520 == 0x38600004) { //PAL 1.0
		// 480p video mode, this needs to be fixed for PAL60
		if(prog) {
			*(s16 *)0x81300522 = type; //swiss
			
			// Correct animation speed
			*(s16 *)0x8130F1C6 = 10;
			*(s16 *)0x8130F1CA = 255;
			*(s16 *)0x8130F1D6 = 7;
			*(s16 *)0x8130F1DA = 6;
			*(s16 *)0x8130F1DE = 5;
			*(s16 *)0x8130F1E2 = 16;
			*(s16 *)0x8130F1E6 = 18;
			*(s16 *)0x8130F1EA = 20;
			*(s16 *)0x8130F1F2 = 40;
			*(s16 *)0x8130F1F6 = 60;
			*(u32 *)0x8130F230 = 0xB3E30016;
			*(u32 *)0x8130F238 = 0x9963000B;
			*(u32 *)0x8130F23C = 0x9BC3001C;
			*(u32 *)0x8130F240 = 0x9BA30037;
			*(u32 *)0x8130F244 = 0x99430035;
			
			memcpy((void*)0x81380FD0, BS2Ntsc448IntAa, sizeof(BS2Ntsc448IntAa));
			
			*(s16 *)0x81380FD2 = type; //2 = 480p
			*(s16 *)0x81380FE6 = 0; //field rendering
			// Vertical filter
			*(s16 *)0x81381002 = 0x0408;
			*(u32 *)0x81381004 = 0x0C100C08;
			*(s16 *)0x81381008 = 0x0400;
		}
		// Check for no deflicker
		if(sharp) {
			*(s16 *)0x81381002 = 0x0000;
			*(u32 *)0x81381004 = 0x15161500;
			*(s16 *)0x81381008 = 0x0000;
		}
		
		// Accept any region code
		*(s16 *)0x8130077E = 1;
		*(s16 *)0x813007A2 = 1;
		
		// Force boot sound. 1=kid, 2=kabuki
		if(jingle > 0)
			*(u32 *)0x81302DE8 = 0x38600000 | jingle;
	}
	else if(*(u32 *)0x81300520 == 0x38600008) { //MPAL 1.1
		// 480p video mode
		if(prog) {
			*(s16 *)0x81300522 = type; //swiss
			
			// Correct animation speed for 50Hz
		/*	*(s16 *)0x8130E9D6 = 8;
			*(s16 *)0x8130E9DA = 15;
			*(s16 *)0x8130E9E6 = 6;
			*(s16 *)0x8130E9EA = 5;
			*(s16 *)0x8130E9EE = 4;
			*(s16 *)0x8130E9F2 = 13;
			*(s16 *)0x8130E9F6 = 255;
			*(s16 *)0x8130E9FA = 17;
			*(s16 *)0x8130EA02 = 33;
			*(s16 *)0x8130EA06 = 50;
			*(u32 *)0x8130EA40 = 0xB1630016;
			*(u32 *)0x8130EA48 = 0x9943000B;
			*(u32 *)0x8130EA4C = 0x9BA3001C;
			*(u32 *)0x8130EA50 = 0x9B830037;
			*(u32 *)0x8130EA54 = 0x99630035; */
			
			memcpy((void*)0x8137D910, BS2Ntsc448IntAa, sizeof(BS2Ntsc448IntAa));
			
			*(s16 *)0x8137D912 = type; //2 = 480p
			*(s16 *)0x8137D926 = 0; //field rendering
			// Vertical filter
			*(s16 *)0x8137D942 = 0x0408;
			*(u32 *)0x8137D944 = 0x0C100C08;
			*(s16 *)0x8137D948 = 0x0400;
		}
		// Check for no deflicker
		if(sharp) {
			*(s16 *)0x8137D942 = 0x0000;
			*(u32 *)0x8137D944 = 0x15161500;
			*(s16 *)0x8137D948 = 0x0000;
		}
		
		// Accept any region code
		*(s16 *)0x8130077E = 1;
		*(s16 *)0x813007A2 = 1;
		
		// Force boot sound. 1=kid, 2=kabuki
		if(jingle > 0)
			*(u32 *)0x81302DE8 = 0x38600000 | jingle;
	}
	
	DCFlushRange((void*)0x81300000, IPL_ROM_FONT_SJIS - CODE_START);
}
