/*
	TinyLoad - a simple region free (original) game launcher in 4k

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

/* This code comes from HBC's stub which was based on dhewg's geckoloader stub */
// Copyright 2008-2009  Andre Heider  <dhewg@wiibrew.org>
// Copyright 2008-2009  Hector Martin  <marcan@marcansoft.com>

#include "utils.h"
#include "ios.h"
#include "cache.h"
#include "usbgecko.h"
#include "memory.h"

#define IOCTL_ES_LAUNCH					0x08
#define IOCTL_ES_GETVIEWCNT				0x12
#define IOCTL_ES_GETVIEWS				0x13

#define TITLE_ID(x,y)					(((u64)(x) << 32) | (y))

#define SYSTEM_MENU			0x0000000100000002ULL

#define framebuffer ((u32*)(0x81600000))

#define framebuffer2 ((u32*)(0x81700000))
#define framebuffer3 ((u32*)(0x81800000))

static struct ioctlv vecs[16] ALIGNED(64);

static int es_fd;

static int
es_init(void)
{
	es_fd = ios_open("/dev/es", 0);
	return es_fd;
}

void memset32(u32 *addr, u32 data, u32 count) __attribute__ ((externally_visible));

void memset32(u32 *addr, u32 data, u32 count) 
{
	int sc = count;
	void *sa = addr;
	while(count--)
		*addr++ = data;
	sync_after_write(sa, 4*sc);
}

static int
es_launchtitle(u64 titleID)
{
	static u64 xtitleID __attribute__((aligned(32)));
	static u32 cntviews __attribute__((aligned(32)));
	static u8 tikviews[0xd8*4] __attribute__((aligned(32)));
	int ret;
	
	xtitleID = titleID;
	vecs[0].data = &xtitleID;
	vecs[0].len = 8;
	vecs[1].data = &cntviews;
	vecs[1].len = 4;
	ret = ios_ioctlv(es_fd, IOCTL_ES_GETVIEWCNT, 1, 1, vecs);
	if(ret<0) return ret;
	if(cntviews>4) return -1;

	vecs[2].data = tikviews;
	vecs[2].len = 0xd8*cntviews;
	ret = ios_ioctlv(es_fd, IOCTL_ES_GETVIEWS, 2, 1, vecs);
	if(ret<0) return ret;
	vecs[1].data = tikviews;
	vecs[1].len = 0xd8;
	ret = ios_ioctlvreboot(es_fd, IOCTL_ES_LAUNCH, 2, 0, vecs);
	return ret;
}

void
_main(void)
{
	//YAGCD says this can blackout the screen but it doesn't work
	//*(vu32*)(0xCC002008) |= 1 << 0;


	// This replicates VISetBlack which calls __setVerticalRegs
	u32 vto = 0;
	u32 vte = 0;
	u8 equ = *(vu16*)(0xCC002000) & 0xF;
	u16 acv = *(vu16*)(0xCC002000) >> 4;
	u16 dispSizeY = 0;
	u16 dispPosY = 0;
	u16 psbOdd  = *(vu32*)(0xCC00200C) >> 16;
	u16 psbEven = *(vu32*)(0xCC002010) >> 16;
	u16 prbOdd  = *(vu16*)(0xCC00200E);
	u16 prbEven = *(vu16*)(0xCC002012);
	
	u16 tmp;
	u32 div1,div2;
	u32 psb,prb;
	u32 psbodd,prbodd;
	u32 psbeven,prbeven;

	div1 = 2;
	div2 = 1;
	if(equ>=10) {
		div1 = 1;
		div2 = 2;
	}

	prb = div2*dispPosY;
	psb = div2*(((acv*div1)-dispSizeY)-dispPosY);
	if(dispPosY%2) {
		prbodd = prbEven+prb;
		psbodd = psbEven+psb;
		prbeven = prbOdd+prb;
		psbeven = psbOdd+psb;
	} else {
		prbodd = prbOdd+prb;
		psbodd = psbOdd+psb;
		prbeven = prbEven+prb;
		psbeven = psbEven+psb;
	}

	tmp = dispSizeY/div1;
	//if(black) {
	prbodd += ((tmp<<1)-2);
	prbeven += ((tmp<<1)-2);
	psbodd += 2;
	psbeven += 2;
	tmp = 0;
	//}

	u8 sum = (prbodd & 0xF0) >> 4;
	u8 sum2 = (psbodd & 0xF0) >> 4;
	prbodd |= (sum + sum2) << 4;
	u8 move = psbodd >> 8;
	prbodd |= move << 8;
 	psbodd &= 0xF;
	vto = prbodd | psbodd << 16;

	//other field
	sum = (prbeven & 0xF0) >> 4;
	sum2 = (psbeven & 0xF0) >> 4;
	prbeven |= (sum + sum2) << 4;
	move = psbeven >> 8;
	prbeven |= move << 8;
    psbeven &= 0xF;
	vte = prbeven | psbeven << 16;

	*(vu32*)(0xCC002000) &= 0x000FFFFF;
	*(vu32*)(0xCC00200C) = vto;
	*(vu32*)(0xCC002010) = vte;


	// Height info, might be useful for screenshot feature
	// 576p black= 08 04BC, og= 06 003E
	// Rebel Strike 304 height reg 005B0070 005A0071
	// 0986 0001
	// 130C 0005, this must mean 05 doesn't mean 240p
	// in prog 450 00220050
	// in prog 304 00b600e0

#if DEBUG
	usbgecko_init();
	usbgecko_printf("_main()\n");
#endif
	sync_before_read((void*)0x93010010, 0x1800);
	_memcpy((void*)0x80001800, (void*)0x93010010, 0x1800);
	sync_after_write((void*)0x80001800, 0x1800);
	if(*(vu32*)0xC0001804 == 0x53545542 && *(vu32*)0xC0001808 == 0x48415858 && //stubhaxx
		*(vu32*)0xD3003438 == 0)
	{
		__asm(
			"sync ; isync\n"
			"lis %r3, 0x8000\n"
			"ori %r3, %r3, 0x1800\n"
			"mtlr %r3\n"
			"blr\n"
		);
	}
#if DEBUG
	usbgecko_printf("no loader stub, using internal\n");
#endif
	ios_cleanup();
#if DEBUG
	usbgecko_printf("ios_cleanup()\n");
#endif
	es_init();
#if DEBUG
	usbgecko_printf("es_init()\n");
#endif

	// If loading the sysmenu it will always reset the dimmer
	*(vu32*)(0xCD80001C) |= 1 << 7; // Enable dimming
	*(vu32*)(0xCD80001C) |= 1 << 5;
	*(vu32*)(0xCD80001C) |= 1 << 4;
	*(vu32*)(0xCD80001C) |= 1 << 3;
	*(vu32*)(0xCD80001C) |= 1 << 2; //chroma
	*(vu32*)(0xCD80001C) |= 1 << 1;
	*(vu32*)(0xCD80001C) |= 1 << 0;

	es_launchtitle(SYSTEM_MENU);
#if DEBUG
	usbgecko_printf("es_launchtitle()\n");
#endif

	while (1);
}

#define SYSCALL_VECTOR	((u8*)0x80000C00)
void
__init_syscall()
{
	u8* sc_vector = SYSCALL_VECTOR;
	u32 bytes = (u32)DCFlashInvalidate - (u32)__temp_abe;
	u8* from = (u8*)__temp_abe;
	for ( ; bytes != 0 ; --bytes )
	{
		*sc_vector = *from;
		sc_vector++;
		from++;
	}

	sync_after_write(SYSCALL_VECTOR, 0x100);
	ICInvalidateRange(SYSCALL_VECTOR, 0x100);
}

