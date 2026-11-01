/*
 * Minimal RetroNET TCP client/server support for NABU.
 *
 * This retains the original RetroNET TCP opcodes and the interruptless HCCA
 * transport used by RetroNET-FileStore.c. File, directory, printer, and other
 * FileStore commands have been removed.
 *
 * This source expects the NABU-LIB interface to provide ayRead(), ayWrite(),
 * NABU_DisableInterrupts(), NABU_EnableInterrupts(), IOPORTA, IOPORTB,
 * INT_MASK_HCCARX, and INT_MASK_HCCATX.
 */

#include <stdint.h>
#include <arch/nabu.h>
#include "mini_nabu.h"
#include "retro_net.h"

/* Saved interrupt mask while an interruptless HCCA command is in progress. */
static dl_u8 rn_int_backup;

/* ------------------------------------------------------------------------- */
/* Interruptless HCCA transport                                               */
/* ------------------------------------------------------------------------- */

static void hcca_DiFocusInterrupts(void)
{
    NABU_DisableInterrupts();

    rn_int_backup = ayRead(IOPORTA);
    ayWrite(IOPORTA, INT_MASK_HCCARX);
}

static void hcca_DiRestoreInterrupts(void)
{
    ayWrite(IOPORTA, rn_int_backup);
    NABU_EnableInterrupts();
}

static void hcca_DiWriteByte(dl_u8 value)
{
    ayWrite(IOPORTA, INT_MASK_HCCATX);

    IO_AYLATCH = IOPORTB;
    while (IO_AYDATA & 0x04)
    {
        /* Wait until HCCA is ready to accept a byte. */
    }

    IO_HCCA = value;
    ayWrite(IOPORTA, INT_MASK_HCCARX);
}

static dl_u8 hcca_DiReadByte(void)
{
    IO_AYLATCH = IOPORTB;

    while (IO_AYDATA & 0x02)
    {
        /* Wait until a byte is available from HCCA. */
    }

    return IO_HCCA;
}

static void hcca_DiWriteUInt16(dl_u16 value)
{
    hcca_DiWriteByte((dl_u8)(value & 0xff));
    hcca_DiWriteByte((dl_u8)(value >> 8));
}

static void hcca_DiWriteUInt32(dl_u32 value)
{
    hcca_DiWriteByte((dl_u8)(value & 0xff));
    hcca_DiWriteByte((dl_u8)((value >> 8) & 0xff));
    hcca_DiWriteByte((dl_u8)((value >> 16) & 0xff));
    hcca_DiWriteByte((dl_u8)((value >> 24) & 0xff));
}

static int32_t hcca_DiReadInt32(void)
{
    dl_u32 value;

    value  = (dl_u32)hcca_DiReadByte();
    value |= (dl_u32)hcca_DiReadByte() << 8;
    value |= (dl_u32)hcca_DiReadByte() << 16;
    value |= (dl_u32)hcca_DiReadByte() << 24;

    return (int32_t)value;
}

static void hcca_DiWriteBytes(dl_u16 offset, dl_u16 length, dl_u8 *bytes)
{
    dl_u16 i;

    for (i = 0; i < length; ++i)
    {
        hcca_DiWriteByte(bytes[offset + i]);
    }
}

static void hcca_DiReadBytes(dl_u8 *buffer, dl_u16 offset, dl_u16 length)
{
    dl_u16 i;

    for (i = 0; i < length; ++i)
    {
        buffer[offset + i] = hcca_DiReadByte();
    }
}

/* ------------------------------------------------------------------------- */
/* TCP client                                                                */
/* ------------------------------------------------------------------------- */

dl_u8 rn_TCPOpen(dl_u8 hostnameLen, dl_u8 *hostname, dl_u16 port, dl_u8 tcpHandle)
{
    dl_u8 result;

    /* 0xd0 */
    hcca_DiFocusInterrupts();

    hcca_DiWriteByte(0xd0);
    hcca_DiWriteByte(hostnameLen);
    hcca_DiWriteBytes(0, hostnameLen, hostname);
    hcca_DiWriteUInt16(port);
    hcca_DiWriteByte(tcpHandle);

    result = hcca_DiReadByte();

    hcca_DiRestoreInterrupts();
    return result;
}

void rn_TCPHandleClose(dl_u8 tcpHandle)
{
    /* 0xd1 */
    hcca_DiFocusInterrupts();

    hcca_DiWriteByte(0xd1);
    hcca_DiWriteByte(tcpHandle);

    hcca_DiRestoreInterrupts();
}

int32_t rn_TCPHandleSize(dl_u8 tcpHandle)
{
    int32_t size;

    /* 0xd2 */
    hcca_DiFocusInterrupts();

    hcca_DiWriteByte(0xd2);
    hcca_DiWriteByte(tcpHandle);
    size = hcca_DiReadInt32();

    hcca_DiRestoreInterrupts();
    return size;
}

int32_t rn_TCPHandleRead(dl_u8 tcpHandle, dl_u8 *buffer, dl_u16 bufferOffset, dl_u16 readLength)
{
    int32_t toRead;

    /* 0xd3 */
    hcca_DiFocusInterrupts();

    hcca_DiWriteByte(0xd3);
    hcca_DiWriteByte(tcpHandle);
    hcca_DiWriteUInt16(readLength);

    toRead = hcca_DiReadInt32();
    if (toRead > 0)
    {
        hcca_DiReadBytes(buffer, bufferOffset, (dl_u16)toRead);
    }

    hcca_DiRestoreInterrupts();
    return toRead;
}

int32_t rn_TCPHandleWrite(dl_u8 tcpHandle, dl_u16 dataOffset, dl_u16 dataLen, dl_u8 *data)
{
    int32_t wrote;

    /* 0xd4 */
    hcca_DiFocusInterrupts();

    hcca_DiWriteByte(0xd4);
    hcca_DiWriteByte(tcpHandle);
    hcca_DiWriteUInt16(dataLen);
    hcca_DiWriteBytes(dataOffset, dataLen, data);
    wrote = hcca_DiReadInt32();

    hcca_DiRestoreInterrupts();
    return wrote;
}

/* ------------------------------------------------------------------------- */
/* TCP server                                                                */
/* ------------------------------------------------------------------------- */

dl_u8 rn_TCPServerClientCnt(void)
{
    dl_u8 count;

    /* 0xd5 */
    hcca_DiFocusInterrupts();
    hcca_DiWriteByte(0xd5);
    count = hcca_DiReadByte();
    hcca_DiRestoreInterrupts();

    return count;
}

dl_u8 rn_TCPServerAvailable(void)
{
    dl_u8 available;

    /* 0xd6 */
    hcca_DiFocusInterrupts();
    hcca_DiWriteByte(0xd6);
    available = hcca_DiReadByte();
    hcca_DiRestoreInterrupts();

    return available;
}

dl_u8 rn_TCPServerRead(dl_u8 *buffer, dl_u16 bufferOffset, dl_u8 readLength)
{
    dl_u8 toRead;

    /* 0xd7 */
    hcca_DiFocusInterrupts();

    hcca_DiWriteByte(0xd7);
    hcca_DiWriteByte(readLength);

    toRead = hcca_DiReadByte();
    if (toRead > 0)
    {
        hcca_DiReadBytes(buffer, bufferOffset, toRead);
    }

    hcca_DiRestoreInterrupts();
    return toRead;
}

void rn_TCPServerWrite(dl_u16 dataOffset, dl_u8 dataLen, dl_u8 *data)
{
    /* 0xd8 */
    hcca_DiFocusInterrupts();

    hcca_DiWriteByte(0xd8);
    hcca_DiWriteByte(dataLen);
    hcca_DiWriteBytes(dataOffset, dataLen, data);

    hcca_DiRestoreInterrupts();
}
