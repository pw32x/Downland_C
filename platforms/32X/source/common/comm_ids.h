#ifndef COMM_IDS_INCLUDE
#define COMM_IDS_INCLUDE

#define SH2MD_COMMAND_READ_SRAM						0x0100 // reads from save ram
#define SH2MD_COMMAND_WRITE_SRAM					0x0200 // writes to save ram

#define SH2MD_COMMAND_POLL_MOUSE					0x0500

#define SH2MD_COMMAND_START_MUSIC					0x0300
#define SH2MD_COMMAND_STOP_MUSIC					0x0400
#define SH2MD_COMMAND_UPDATE_CD						0x0600
#define SH2MD_COMMAND_USE_CD						0x0700

#define SH2MD_SET_MUSIC_VOLUME						0x1800

#define SH2MD_COMMAND_SET_CURSOR					0x0800 // set current md cursor
#define SH2MD_COMMAND_GET_CURSOR					0x0900 // get current md cursor
#define SH2MD_COMMAND_SET_FONT_AND_FGBG_COLORS		0x0A00 // set font fg and bg colors
#define SH2MD_COMMAND_GET_FONT_AND_FGBG_COLORS		0x0B00 // get font fg and bg colors
#define SH2MD_COMMAND_SET_PALETTE_SELECT			0x0C00 // set palette select
#define SH2MD_COMMAND_PUT_CHAR						0x0D00 // put char at current cursor pos

#define SH2MD_COMMAND_CLEAR_NAME_TABLE_A			0x0E00 // clear name table A
#define SH2MD_COMMAND_START_DEBUG_QUEUE				0x0F00 // start debug queue
#define SH2MD_COMMAND_QUEUE_DEBUG_ENTRY				0x1000 // queue debug entry
#define SH2MD_COMMAND_END_DEBUG_QUEUE				0x1100 // end debug queue and display

#define SH2MD_COMMAND_GET_BYTE_FROM_NETWORK			0x1200
#define SH2MD_COMMAND_SEND_BYTE_TO_NETWORK			0x1300
#define SH2MD_SETUP_NETWORK							0x1400
#define SH2MD_CLEANUP_NETWORKING					0x1500
#define SH2MD_SET_NETLINK_TIMEOUT					0x1700

#define SH2MD_COMMAND_SET_BANK_PAGE					0x1600 // set bank page

#define SH2MD_COMMAND_CTL_MD_VDP					0x1900 // ???
#define SH2MD_COMMAND_STORE_WORD_COLUMN_IN_MD_VRAM	0x1A00 // ???
#define SH2MD_COMMAND_LOAD_WORD_COLUMN_FROM_MD_VRAM 0x1A01 // ???
#define SH2MD_COMMAND_SWAP_WORD_COLUMN_WITH_MD_VRAM	0x1A02 // ???

#endif