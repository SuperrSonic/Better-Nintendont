// Nintendont (kernel): GameCube Memory Card functions.
// Used by EXI.c.

#include "GCNCard.h"
#include "Config.h"
#include "debug.h"
#include "ff_utf8.h"

#include "NAND.h"

// ENABLE ring giver in Heroes/Shadow/SMCSonic2
//#define spRings 1

#define TOTAL_RINGS 0x13003434
#define NEW_RINGS 0x13003438

extern u32 useAGB;

// AGB Emulator SRAM addr for 8 MB ROMs
// 0x80D6AA00
//pointer 1 = 09A0E0
//pointer 2 = 0A18E8

//cart pointer 8009A0B0

// MadeInWario, can play Zakeru2, and the pause menu in Kirby1 doesn't glitch
// but both games have constant audio pops.

#define SRAM_ADDR_WARIO  0x0008CD98 //SRAM loads from ISO first
#define SRAM_ADDR_NINJA2 0x0009A0E0
#define SRAM_ADDR_MvsDK  0x0009A320
#define SRAM_ADDR_PKMBX  0x017DBDB0 //0x001FCA68, however
//0x00B77EC0 - direct

// Used for reading ROM ID for handling 128 KB saves
#define ROM_ADDR_MvsDK   0x0009A2F0 // ptr
#define ROM_ADDR_WARIO   0x0008CD68
#define ROM_ADDR_PKMBX   0x00BB802C

//With this it's possible to load e.g. sd:/global.tpl to mem2
//and patch this address so that it loads it for every copy.
//Worked on dolphin, not wii.
#define TPL_ADDR_MvsDK   0x00389FBC


//Mario vs DK
//~2:30 minute timer if no input, causes reset
//0x800A1598 contains timer, writing 00002328 triggers reset
//0x8000833C = function for timer, NOP to disable timer.
//or, of course, we could also use a code to write to the timer constantly.

//SRAM ptr: 0x8009A320

//BG RGBA: 0x800A1C60
//GM RGBA: 0x800A1C6C

// TPL header
u32 tpl_hdr[16] = { 0x0020AF30, 1, 0xC, 0x14, 0, 0x00D80238, 0xE, 0x40, 0, 0, 1, 1, 0, 0, 0, 0 };

u32 CheckSaves()
{
	u32 size = 0x8000; //32 KB
	u32 rom_ptr = 0xAC;
	
	if(useAGB == 2)
		rom_ptr += read32(ROM_ADDR_MvsDK);
	else if(useAGB == 3)
		rom_ptr += read32(ROM_ADDR_WARIO);
	else if(useAGB == 4) {
		//rom_ptr += read32(ROM_ADDR_PKMBX);
		//might as well assume, since compatibility is so tight
		return 0x20000;
	}
	
	rom_ptr &= 0x0FFFFFFF;
	
	//128 KB saves
	if (read32(rom_ptr) >> 8 == 0x415850 || // Sapphire
		read32(rom_ptr) >> 8 == 0x415856 || // Ruby
		read32(rom_ptr) >> 8 == 0x425045 || // Emerald
		read32(rom_ptr) >> 8 == 0x425052 || // FireRed
		read32(rom_ptr) >> 8 == 0x425047 || // LeafGreen
	//	read32(rom_ptr) >> 8 == 0x423234 || // Red Rescue Team is 32 MB
		read32(rom_ptr) >> 8 == 0x415834 || // SMA4 - SMB3
		read32(rom_ptr) >> 8 == 0x424654)   // F-Zero Climax
		size = 0x20000;
	//64 KB flash, only the sonic games, they don't save, but can load correctly if SRAM patched
	else if(read32(rom_ptr) >> 8 == 0x413356 || // Sonic Pinball Party
			read32(rom_ptr) >> 8 == 0x41534F || // Advance
			read32(rom_ptr) >> 8 == 0x41324E || // Advance 2
			read32(rom_ptr) >> 8 == 0x423353 || // Advance 3
			read32(rom_ptr) >> 8 == 0x425342)   // Sonic Battle
			size = 0x10000;

	return size;
}

void AGB_Load(void)
{
		char *path	= (char*)malloca( 0x40, 32 );
		//_sprintf(path, "/apps/gc_devo/agb/%x.sav", ConfigGetGameID());
		_sprintf(path, "%sdata.sav", ConfigGetGamePath());
		dbgprintf("\r\nAGB SRAM path: %s\r\n", path);
		FIL f;
		int ret = f_open_char(&f, path, FA_READ|FA_OPEN_EXISTING);
		if (ret == FR_OK)
		{
			u32 dat_size = CheckSaves();
			u32 sram_address = 0;
			if(useAGB == 1)
				sram_address = read32(SRAM_ADDR_NINJA2);
			else if(useAGB == 2)
				sram_address = read32(SRAM_ADDR_MvsDK);
			else if(useAGB == 3)
				sram_address = read32(SRAM_ADDR_WARIO);
			else if(useAGB == 4)
				sram_address = SRAM_ADDR_PKMBX;
			
			//zero the 8 from address
			sram_address &= 0x0FFFFFFF;
			
			UINT read;
			f_lseek(&f, 0);
			f_read(&f, (void*)sram_address, dat_size, &read);
			f_close(&f);
			dbgprintf("\r\nAGB: data loaded.\r\n");
		}
		else
		{
			dbgprintf("\r\nAGB: Unable to load data: %u\r\n", ret);
		}
		free(path);
}

void AGB_Save(void)
{
	char *path	= (char*)malloca( 0x40, 32 );
	//_sprintf(path, "/apps/gc_devo/agb/%x.sav", ConfigGetGameID());
	_sprintf(path, "%sdata.sav", ConfigGetGamePath());
	FIL f;
	int ret = f_open_char(&f, path, FA_WRITE|FA_OPEN_EXISTING);
	if (ret == FR_OK)
	{
		u32 dat_size = CheckSaves();
		u32 sram_address = 0;
		if(useAGB == 1)
			sram_address = read32(SRAM_ADDR_NINJA2);
		else if(useAGB == 2)
			sram_address = read32(SRAM_ADDR_MvsDK);
		else if(useAGB == 3)
			sram_address = read32(SRAM_ADDR_WARIO);
		else if(useAGB == 4)
			sram_address = SRAM_ADDR_PKMBX;
		
		//zero the 8 from address
		sram_address &= 0x0FFFFFFF;
		
		UINT wrote;
		f_lseek(&f, 0);
		f_write(&f, (void*)sram_address, dat_size, &wrote);
		f_close(&f);
		dbgprintf("\r\nAGB: data saved.\r\n");
	}
	else
	{
		dbgprintf("\r\nAGB: Unable to save data: %u\r\n", ret);
	}
	free(path);


	// PB has a screenshot feature, let's use it
	if(ConfigGetGameID() == 0x47505845)
	{
		// A wallpaper was saved to Brigette's room
		if(read32(0x9E5304) == 0x01000000)
		{
			path = (char*)malloca( 0x40, 32 );
			_sprintf(path, "/private/pb_picture.tpl");
			ret = f_open_char(&f, path, FA_WRITE|FA_OPEN_ALWAYS);
			if(ret == FR_OK)
			{
				UINT wrote;
				f_lseek(&f, 0);
				f_write(&f, tpl_hdr, 0x40, &wrote);
				f_write(&f, (void*)0x9E5320, 0xEFA0, &wrote);
				f_close(&f);
				dbgprintf("\r\nAGB: picture saved.\r\n");
			}
			free(path);
		}
	}
}

//SMC ROM LOADER
#if 1
void SMC_Load(char* title, u8 posValue)
{
		char *path	= (char*)malloca( 0x40, 32 );
		char *tmp_path	= (char*)malloca( 0x40, 32 );
		//_sprintf(path, "/apps/gc_devo/agb/%x.sav", ConfigGetGameID());
		
		_sprintf(tmp_path, "%s", ConfigGetGamePath());
		u32 len = strlen(tmp_path);
		tmp_path[len - 8] = '\0';
	//	char *afterslash = strrchr(tmp_path, '/');
	//	if (afterslash != NULL)
	//		*(afterslash + 1) = '\0';
		_sprintf(path, "%s%s", tmp_path, title);
		
		dbgprintf("\r\nSMC ROM path: %s\r\n", path);
	//if(read32(TOTAL_RINGS) != 0) {
		FIL f;
		int ret = f_open_char(&f, path, FA_READ|FA_OPEN_EXISTING);
		if (ret == FR_OK)
		{
			u32 dat_size = read32(0x2AF0C4 + (posValue * 4));
			
			//zero the 8 from address
			//sram_address &= 0x0FFFFFFF;
			
			UINT read;
			f_lseek(&f, 0);
			f_read(&f, (void*)0x11200010, dat_size, &read);
			f_close(&f);
			
			write32(0x11200000, dat_size);
			dbgprintf("\r\nSMC: ROM loaded.\r\n");
		}
		else
		{
			dbgprintf("\r\nSMC: Unable to load ROM: %u\r\n", ret);
		}
		free(path);
		free(tmp_path);
}
#endif

//debug by dumping ram
void DumpSAILORMOON(void)
{
		char *path	= (char*)malloca( 0x40, 32 );
		//_sprintf(path, "/apps/gc_devo/agb/%x.sav", ConfigGetGameID());
		_sprintf(path, "%smemdata.bin", ConfigGetGamePath());
		FIL f;
		int ret = f_open_char(&f, path, FA_WRITE|FA_CREATE_ALWAYS);
		if (ret == FR_OK)
		{
			//zero the 8 from address
			//sram_address &= 0x0FFFFFFF;
			
			UINT wrote;
			f_lseek(&f, 0);
			f_write(&f, (void*)0x254CA8, 0x90, &wrote);
			f_close(&f);
			dbgprintf("\r\nSMC: RAM dumped.\r\n");
		}
		else
		{
			dbgprintf("\r\nSMC: Unable to dump RAM: %u\r\n", ret);
		}
		free(path);
}


#define SECONDS_TO_2000 946684800LL
#define TICKS_PER_SECOND 60750000LL

typedef struct
{
	u32	checksum;
	u32	data[31];
/*	union
	{
		u32 data[31];
		struct
		{
			u16 name[42];
			u64 ticks_boot;
			u64 ticks_last;
			char title_id[6];
			char unknown[18];
		} ATTRIBUTE_PACKED;
	};*/
} __attribute__((packed)) PlayRec;

static u64 getWiiTime(void)
{
	return TICKS_PER_SECOND * (GetCurrentTime() - SECONDS_TO_2000);
}

// Wii Message Board playlog
void UpdatePlaylog(void)
{
	if(ncfg->SkipPlaylog)
		return;

	u32 sum = 0;
	u8 i;
	u32  *size	= (u32*) malloca( sizeof(u32), 32 );
	char *path	= (char*)malloca( 0x30, 32 );

	_sprintf( path, "/title/00000001/00000002/data/play_rec.dat");

	//IOS_Read did not work, so loading with this function
	PlayRec *playrec_buf = (PlayRec *)NANDLoadFile( path, size );
	if( playrec_buf == NULL )
	{
		free( path );
		free( size );
		dbgprintf("PLAYLOG: No file found.\r\n");
		return;
	}

	// NOTE: ULGX doesn't create a playlog file for GC.

	//update time
	u64 stime = getWiiTime();
	playrec_buf->data[23] = stime >> 32;
	playrec_buf->data[24] = (u32)stime;

	//write the gameid and maker code
	playrec_buf->data[25] = ConfigGetGameID();
	playrec_buf->data[26] = read32(4) & 0xFFFF0000;

	//calc checksum
	for(i = 0; i < 31; i++)
		sum += playrec_buf->data[i];

	playrec_buf->checksum = sum;

	s32 fd = IOS_Open( path, 2 );
	if( fd < 0 )
	{
		dbgprintf("PLAYLOG: No file found.\r\n");
		free( path );
		free( size );
		return;
	}

	s32 r = IOS_Write( fd, playrec_buf, 0x80 );
	if( r != 0x80 || r < 0 )
	{
		dbgprintf("PLAYLOG: Could only update %d bytes.\r\n", r);
		free( path );
		free( size );
		IOS_Close( fd );
		return;
	}

	IOS_Close( fd );
	free( path );
	free( size );
}

u32 useSP = 0;
u32 current_rings = 0;

#if 0
//This is how sp.bin looks like, more games can be supported
typedef struct _SPData_ctx {
	u32 RING_MAGICK; //RING
	u32 HEROES_RING;
	u32 SHADOW_RING;
	u32 SMC_RING; //Sonic 2
	//u32 GEMS_RING; //Sonic CD, maybe
} SPData_ctx;
#endif

// Triforce variables.
extern vu32 TRIGame;

// Memory Card context.
static u8 *const GCNCard_base = (u8*)(0x11000000);

typedef struct _GCNCard_ctx {
	char filename[0x20];    // Memory Card filename.
	u8 *base;               // Base address.
	u32 size;               // Size, in bytes.
	u32 code;               // Memory card "code".

	// BlockOffLow starts from 0xA000; does not include "system" blocks.
	// For system blocks, check 'changed_system'.
	bool changed;		// True if the card has been modified at all.
				// (NOTE: Reset after calling GCNCard_CheckChanges().)
	bool changed_system;	// True if the system area (first 5 blocks)
				// has been modified. These blocks are NOT
				// included in BlockOffLow / BlockOffHigh.

	// NOTE: BlockOff is in bytes, not blocks.
	u32 BlockOff;           // Current offset.
	u32 BlockOffLow;        // Low address of last modification.
	u32 BlockOffHigh;       // High address of last modification.
	u32 CARDWriteCount;     // Write count. (TODO: Is this used anywhere?)
} GCNCard_ctx;
#ifdef GCNCARD_ENABLE_SLOT_B
static GCNCard_ctx memCard[2] __attribute__((aligned(32)));
#else /* !GCNCARD_ENABLE_SLOT_B */
static GCNCard_ctx memCard[1] __attribute__((aligned(32)));
#endif /* GCNCARD_ENABLE_SLOT_B */

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

static void GCNCard_InitCtx(GCNCard_ctx *ctx)
{
	memset(ctx, 0, sizeof(*ctx));
	ctx->BlockOffLow = 0xFFFFFFFF;
}

/**
 * Is a memory card enabled?
 * @param slot Slot number. (0 == Slot A, 1 == Slot B)
 * @return 0 if disabled; 1 if enabled.
 */
inline u32 GCNCard_IsEnabled(int slot)
{
	if (slot < 0 || slot >= ARRAY_SIZE(memCard))
		return 0;

	// Card is enabled if it's larger than 0 bytes.
	return (memCard[slot].size > 0);
}

/**
 * Load a memory card from disk.
 * @param slot Slot number. (0 == Slot A, 1 == Slot B)
 * NOTE: Slot B is not valid on Triforce.
 * @return 0 on success; non-zero on error.
 */
int GCNCard_Load(int slot)
{
	switch (slot)
	{
		case 0:
			// Slot A
			break;
#ifdef GCNCARD_ENABLE_SLOT_B
		case 1:
			// Slot B (not valid on Triforce)
			if (TRIGame != 0)
				return -1;
			break;
#endif /* GCNCARD_ENABLE_SLOT_B */
		default:
			// Invalid slot.
			return -2;
	}

#ifdef spRings
	// NOTE: Only load rings saved if slot A is started.
	if(slot == 0) {
		dbgprintf("\r\nRINGS BEFORE: %d\r\n", read32(TOTAL_RINGS));
		FIL f;
		int ret = f_open_char(&f, "/apps/chao transfer tool/sp.bin", FA_READ|FA_OPEN_EXISTING);
		if (ret == FR_OK)
		{
			UINT read;
			u32 rings = 0;
			
			f_read(&f, &rings, 4, &read);
			//dbgprintf("\r\nRINGS BUG 1: %X\r\n", rings);
			useSP = rings;
			
			if(rings == 0x52494E47) { //SUCCESS
				if(read32(0) >> 8 == 0x473953) //Heroes
					f_lseek(&f, 4);
				else if(read32(0) >> 8 == 0x475550) //Shadow
					f_lseek(&f, 8);
				else if(read32(0) >> 8 == 0x47534F) //Mega
					f_lseek(&f, 0xC);

				f_read(&f, &rings, 4, &read);

				write32(TOTAL_RINGS, rings);
				sync_after_write((void*)TOTAL_RINGS, 0x20);
				current_rings = rings;
			} else
				dbgprintf("\r\nRINGS FORMAT UNKNOWN.\r\n");
			f_close(&f);

			dbgprintf("\r\nRINGS AFTER: %d\r\n", read32(TOTAL_RINGS));
		}
		else
		{
			dbgprintf("\r\nRINGS: Unable to load data: %u\r\n", ret);
		}
	}
#endif

	// Get the Game ID.
	const u32 GameID = ConfigGetGameID();

	// Set up the Memory Card context.
	GCNCard_ctx *const ctx = &memCard[slot];
	GCNCard_InitCtx(ctx);
	memcpy(ctx->filename, "/apps/gc_devo/", 14);
	char *fname_ptr = &ctx->filename[14];
	if (ConfigGetConfig(NIN_CFG_MC_MULTI))
	{
		// "Multi" mode enabled. (one card for all saves, per region)
		memcpy(fname_ptr, "memcard", 7);
		fname_ptr += 7;

		if (BI2region == BI2_REGION_JAPAN ||
		    BI2region == BI2_REGION_SOUTH_KOREA)
		{
			// JPN game. Append a 'j'.
			*(fname_ptr+0) = '_';
			*(fname_ptr+1) = 'j';
			fname_ptr += 2;
		}

#ifdef GCNCARD_ENABLE_SLOT_B
		if (slot)
		{
			// Slot B. Append a 'b'.
			*(fname_ptr+0) = '_';
			*(fname_ptr+1) = 'b';
			fname_ptr += 2;
		}
#endif /* GCNCARD_ENABLE_SLOT_B */

		// Append the file extension. (with NULL terminator)
		memcpy(fname_ptr, ".bin", 5);
	}
	else
	{
		// Single mode. One card per game.
		memcpy(fname_ptr, &GameID, 4);
		fname_ptr += 4;

#ifdef GCNCARD_ENABLE_SLOT_B
		if (slot)
		{
			// Slot B. Append "_B".
			*(fname_ptr+0) = '_';
			*(fname_ptr+1) = 'B';
			fname_ptr += 2;
		}
#endif /* GCNCARD_ENABLE_SLOT_B */

		// Append the file extension. (with NULL terminator)
		memcpy(fname_ptr, ".bin", 5);
	}

	sync_after_write(ctx->filename, sizeof(ctx->filename));

	dbgprintf("EXI: Trying to open %s\r\n", ctx->filename);
	FIL fd;
	int ret = f_open_char(&fd, ctx->filename, FA_READ|FA_OPEN_EXISTING);
	if (ret != FR_OK || fd.obj.objsize == 0)
	{
#ifdef DEBUG_EXI
		dbgprintf("EXI: Slot %c: Failed to open %s: %u\r\n", (slot+'A'), ctx->filename, ret );
#endif
#ifdef GCNCARD_ENABLE_SLOT_B
		if (slot == 0)
		{
			// Slot A failure is fatal.
			Shutdown();
		}

		// Slot B failure will simply disable Slot B.
		dbgprintf("EXI: Slot %c has been disabled.\r\n", (slot+'A'));
		return -3;
#else /* !GCNCARD_ENABLE_SLOT_B */
		// Slot A failure is fatal.
		Shutdown();
#endif /* GCNCARD_ENABLE_SLOT_B */
	}

#ifdef DEBUG_EXI
	dbgprintf("EXI: Loading memory card for Slot %c...", (slot+'A'));
#endif

	// Check if the card filesize is valid.
	u32 FindBlocks = 0;
	for (FindBlocks = 0; FindBlocks <= MEM_CARD_MAX; FindBlocks++)
	{
		if (MEM_CARD_SIZE(FindBlocks) == fd.obj.objsize)
			break;
	}
	if (FindBlocks > MEM_CARD_MAX)
	{
		dbgprintf("EXI: Slot %c unexpected size %s: %u\r\n",
				(slot+'A'), ctx->filename, fd.obj.objsize);
#ifdef GCNCARD_ENABLE_SLOT_B
		if (slot == 0)
		{
			// Slot A failure is fatal.
			Shutdown();
		}

		// Slot B failure will simply disable Slot B.
		dbgprintf("EXI: Slot %c has been disabled.\r\n", (slot+'A'));
		f_close(&fd);
		return -4;
#else /* !GCNCARD_ENABLE_SLOT_B */
		// Slot A failure is fatal.
		Shutdown();
#endif /* GCNCARD_ENABLE_SLOT_B */
	}

#if GCNCARD_ENABLE_SLOT_B
	if (slot == 0)
	{
		// Slot A starts at GCNCard_base.
		ctx->base = GCNCard_base;
		// Set the memory card size for Slot A only.
		ConfigSetMemcardBlocks(FindBlocks);
	}
	else
	{
		// Slot B starts immediately after Slot A.
		// Make sure both cards fit within 16 MB.
		if (memCard[0].size + fd.obj.objsize > (16*1024*1024))
		{
			// Not enough memory for both cards.
			// Disable Slot B.
			dbgprintf("EXI: Slot A is %u MB; not enough space for Slot %c, which is %u MB.\r\n",
					"EXI: Slot %c has been disabled.\r\n",
					memCard[0].size / 1024 / 1024, (slot+'A'),
					fd.obj.objsize / 1024 / 1024, (slot+'A'));
			f_close(&fd);
			return -4;
		}
		ctx->base = memCard[0].base + memCard[0].size;
	}
#else /* !GCNCARD_ENABLE_SLOT_B */
	// Slot A starts at GCNCard_base.
	ctx->base = GCNCard_base;
	// Set the memory card size for Slot A only.
	ConfigSetMemcardBlocks(FindBlocks);
#endif /* GCNCARD_ENABLE_SLOT_B */

	// Size and "code".
	ctx->size = fd.obj.objsize;
	ctx->code = MEM_CARD_CODE(FindBlocks);

	// Read the memory card contents into RAM.
	UINT read;
	f_lseek(&fd, 0);
	f_read(&fd, ctx->base, ctx->size, &read);
	f_close(&fd);

	// Reset the low/high offsets to indicate that everything was just loaded.
	ctx->BlockOffLow = 0xFFFFFFFF;
	ctx->BlockOffHigh = 0x00000000;

#ifdef DEBUG_EXI
	dbgprintf("EXI: Loaded Slot %c memory card size %u\r\n", (slot+'A'), ctx->size);
#endif

	// Synchronize the memory card data.
	sync_after_write(ctx->base, ctx->size);

#ifdef GCNCARD_ENABLE_SLOT_B
	if (slot == 1)
	{
		// Slot B card image loaded successfully.
		ncfg->Config |= NIN_CFG_MC_SLOTB;
	}
#endif /* GCNCARD_ENABLE_SLOT_B */

	return 0;
}

/**
* Get the total size of the loaded memory cards.
* @return Total size, in bytes.
*/
u32 GCNCard_GetTotalSize(void)
{
#ifdef GCNCARD_ENABLE_SLOT_B
	return (memCard[0].size + memCard[1].size);
#else /* !GCNCARD_ENABLE_SLOT_B */
	return memCard[0].size;
#endif /* GCNCARD_ENABLE_SLOT_B */
}

/**
* Check if the memory cards have changed.
* @return True if either memory card has changed; false if not.
*/
bool GCNCard_CheckChanges(void)
{
	int slot;
	bool ret = false;
	for (slot = 0; slot < ARRAY_SIZE(memCard); slot++)
	{
		// CHeck if the memory card is dirty in general.
		if (memCard[slot].changed)
		{
			memCard[slot].changed = false;
			ret = true;
		}
	}
	return ret;
}

#if 0
typedef struct
{
	u32 SignatureType1;
	u16 unk1;
	u8	doubleStrike;
	u8	unk_byte;
	u8	Padding0[0x8];
	u32 HeroesRings;
	u32 ShadowRings;
	u32 MegaRings;
	u32 GemsRings;

} __attribute__((packed)) VCRingData;
#endif

//static bool disableMCE = false;

/**
* Save the memory card(s).
*/
void GCNCard_Save(void)
{
	//if (TRIGame || (read32(RESET_STATUS) == 0x7DEB && disableMCE))
	if (TRIGame)
	{
	/*	if(read32(RESET_STATUS) == 0x7DEB) {
			disableMCE = true;
			write32(RESET_STATUS, 0x7DEA);
			sync_after_write((void*)RESET_STATUS, 0x20);
		}*/
		
		// Triforce doesn't use the standard EXI CARD interface.
		return;
	}

	//TEST LOG
	//UpdatePlaylog();

#ifdef spRings
	//vu32 global_rings = read32(TOTAL_RINGS);
  /* if(show_dbg_once && global_rings > 0) {
      dbgprintf("\r\nFINAL RINGS BEFORE: %d\r\n", global_rings);
      show_dbg_once = 0;
	}*/
	dbgprintf("\r\nFINAL RINGS BEFORE: %d\r\n", read32(TOTAL_RINGS));
	FIL f;
	int ret = f_open_char(&f, "/apps/chao transfer tool/sp.bin", FA_WRITE|FA_OPEN_EXISTING);
	if (ret == FR_OK)
	{
		UINT wrote;
		//u32 ring_buf;

		//data is not being read into the u32
		//f_read(&f, &ring_buf, 4, &wrote);
		//dbgprintf("\r\nRINGS BUG: %X\r\n", ring_buf);
		//useSP = ring_buf;
		
		if(read32(0) >> 8 == 0x473953) //Heroes
			f_lseek(&f, 4);
		else if(read32(0) >> 8 == 0x475550) //Shadow
			f_lseek(&f, 8);
		else if(read32(0) >> 8 == 0x47534F) //Mega
			f_lseek(&f, 0xC);
		else
			useSP = 0;
		
		//f_read(&f, &ring_buf, 4, &wrote);
		//dbgprintf("\r\nREAD RINGS: %d\r\n", ring_buf);
		
		//current_rings = ring_buf;

		if(useSP == 0x52494E47) { //SUCCESS
			//data is always read as 0, so it always writes, grrr
			//f_read(&f, &ring_buf, 4, &wrote);
			//dbgprintf("\r\nREAD RINGS: %d\r\n", ring_buf);
			
			u32 global_rings = 0;
			global_rings = read32(TOTAL_RINGS);
			/*
			global_rings += read32(NEW_RINGS);
			write32(NEW_RINGS, 0);
			sync_after_write((void*)NEW_RINGS, 0x20);
			
			if(global_rings > 9999999) {
				global_rings = 9999999;
				write32(TOTAL_RINGS, 9999999);
				sync_after_write((void*)TOTAL_RINGS, 0x20);
			}
			*/
			// Save data, if necessary.
			if (global_rings != current_rings)
			{
				f_write(&f, &global_rings, 4, &wrote);
				dbgprintf("\r\nWROTE RINGS: %d\r\n", global_rings-current_rings);
				current_rings = global_rings;
			}
		}
		f_close(&f);

		dbgprintf("\r\nTOTAL RINGS: %d\r\n", read32(TOTAL_RINGS));
	}
	else
	{
		dbgprintf("\r\nRINGS: Unable to save data: %u\r\n", ret);
	}
#endif

	int slot;
	for (slot = 0; slot < ARRAY_SIZE(memCard); slot++)
	{
		if (!GCNCard_IsEnabled(slot))
		{
			// Card isn't initialized.
			continue;
		}

		// Does this card have any unsaved changes?
		GCNCard_ctx *const ctx = &memCard[slot];
		if (ctx->changed_system ||
		    ctx->BlockOffLow < ctx->BlockOffHigh)
		{
//#ifdef DEBUG_EXI
			//dbgprintf("EXI: Saving memory card in Slot %c...", (slot+'A'));
//#endif
			FIL fd;
			int ret = f_open_char(&fd, ctx->filename, FA_WRITE|FA_OPEN_EXISTING);
			if (ret == FR_OK)
			{
				UINT wrote;
				sync_before_read(ctx->base, ctx->size);

				// Save the system area, if necessary.
				if (ctx->changed_system)
				{
					f_lseek(&fd, 0);
					f_write(&fd, ctx->base, 0xA000, &wrote);
				}

				// Save the general area, if necessary.
				if (ctx->BlockOffLow < ctx->BlockOffHigh)
				{
					f_lseek(&fd, ctx->BlockOffLow);
					f_write(&fd, &ctx->base[ctx->BlockOffLow],
						(ctx->BlockOffHigh - ctx->BlockOffLow), &wrote);
				}

				f_close(&fd);
//#ifdef DEBUG_EXI
				//dbgprintf("Done!\r\n");
//#endif
			}
			else
			{
				dbgprintf("\r\nEXI: Unable to open Slot %c memory card file: %u\r\n", (slot+'A'), ret);
			}

			// Reset the low/high offsets to indicate that everything has been saved.
			ctx->BlockOffLow = 0xFFFFFFFF;
			ctx->BlockOffHigh = 0x00000000;
			ctx->changed_system = false;
		}
	}
}

/** Functions used by EXIDeviceMemoryCard(). **/

void GCNCard_ClearWriteCount(int slot)
{
	if (!GCNCard_IsEnabled(slot))
		return;

	memCard[slot].CARDWriteCount = 0;
}

/**
 * Set the current block offset.
 * @param slot Slot number.
 * @param data Block offset from the EXI command.
 */
void GCNCard_SetBlockOffset(int slot, u32 data)
{
	if (!GCNCard_IsEnabled(slot))
		return;

	u32 BlockOff = ((data>>16)&0xFF)  << 17;
	BlockOff    |= ((data>> 8)&0xFF)  << 9;
	BlockOff    |= ((data&0xFF)  &3)  << 7;
	memCard[slot].BlockOff = BlockOff;
}

/**
 * Write data to the card using the current block offset.
 * @param slot Slot number.
 * @param data Data to write.
 * @param length Length of data to write, in bytes.
 */
void GCNCard_Write(int slot, const void *data, u32 length)
{
	if (!GCNCard_IsEnabled(slot))
		return;
	GCNCard_ctx *const ctx = &memCard[slot];
	ctx->changed = true;

	// Is this update entirely within the "system area"?
	if (ctx->BlockOff < 0xA000 && ctx->BlockOff + length < 0xA000)
	{
		// This update is entirely within the "system area".
		// Only set the flag; don't set block offsets.
		ctx->changed_system = true;
	}
	else
	{
		// Update the block offsets for saving.
		if (ctx->BlockOff < ctx->BlockOffLow)
			ctx->BlockOffLow = ctx->BlockOff;
		if (ctx->BlockOff + length > ctx->BlockOffHigh)
			ctx->BlockOffHigh = ctx->BlockOff + length;
		ctx->changed = true;

		if (ctx->BlockOff < 0xA000)
		{
			// System area as well as general area.
			ctx->changed_system = true;
		}
		if (ctx->BlockOffLow < 0xA000)
		{
			// BlockOffLow shouldn't be less than 0xA000.
			// Otherwise, we end up with double writing.
			// (Not a problem; just wastes time.)
			ctx->BlockOffLow = 0xA000;
			ctx->changed_system = true;
		}
	}

	// FIXME: Verify that this doesn't go out of bounds.
	sync_before_read((void*)data, length);
	memcpy(&ctx->base[ctx->BlockOff], data, length);
	sync_after_write(&ctx->base[ctx->BlockOff], length);
}

/**
 * Read data from the card using the current block offset.
 * @param slot Slot number.
 * @param data Buffer for the read data.
 * @param length Length of data to read, in bytes.
 */
void GCNCard_Read(int slot, void *data, u32 length)
{
	if (!GCNCard_IsEnabled(slot))
		return;
	GCNCard_ctx *const ctx = &memCard[slot];

	// FIXME: Verify that this doesn't go out of bounds.
	sync_before_read(&ctx->base[ctx->BlockOff], length);
	memcpy(data, &ctx->base[ctx->BlockOff], length);
	sync_after_write(data, length);
}

/**
 * Get the card's "code" value.
 * @param slot Slot number.
 * @param Card's "code" value.
 */
u32 GCNCard_GetCode(int slot)
{
	if (!GCNCard_IsEnabled(slot))
		return 0;
	return memCard[slot].code;
}

/**
 * Set the current block offset. (ERASE mode; uses sector values only.)
 * @param slot Slot number.
 * @param Data Block offset (sector values) from the EXI command.
 */
void GCNCard_SetBlockOffset_Erase(int slot, u32 data)
{
	if (!GCNCard_IsEnabled(slot))
		return;

	u32 BlockOff = (((u32)data>>16)&0xFF)  << 17;
	BlockOff    |= (((u32)data>> 8)&0xFF)  << 9;
	memCard[slot].BlockOff = BlockOff;
}

#ifdef DEBUG_EXI
/**
 * Get the current block offset. (decoded value, for debugging purposes)
 * @param slot Slot number.
 * @return Block offset, decoded.
 */
u32 GCNCard_GetBlockOffset(int slot)
{
	if (!GCNCard_IsEnabled(slot))
		return 0;
	return memCard[slot].BlockOff;
}
#endif
