// Sonic Advance 2 avoid null reads
// Moves two function calls to unused space in the ROM
// and checks that the sprite pointer is not null before calling.
// Allows AGBE to play the character end scenes all the way.

// The SA2 decompilation project https://github.com/SAT-R/sa2
// was used to get the functions needed.

.gba

.open "SA2_HQ_11khz.gba","SA2_HQ_11khz_NUL.gba",0x08000000 


// Leftover from replaced code
.org 0x080939C0
dh 0x0000


// The functions in question
.thumb
UpdateSpriteAnimation EQU 0x08004558
DisplaySprite EQU 0x080051E8


.org 0x080979F8 //0x080D55E0 might not be safe

.area 0x20

.align 4

.func CheckNUL
	cmp r6,0
	beq finish
	
	;original instructions
	add r0,r6,#0
	bl UpdateSpriteAnimation
	add r0,r6,#0
	bl DisplaySprite
	
	b finish

finish:
	ldr r1,=0x080939C2  ;exit back
	mov r15,r1          ;r15 is the pc, it's an alternate long branch

.pool
.endfunc
.endarea


// Hooking into the end scene
.org 0x080939B6
	ldr r0,=0x080979F8
	mov r15,r0

.pool

.close