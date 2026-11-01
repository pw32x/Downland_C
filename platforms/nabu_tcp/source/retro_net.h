#ifndef RETRONET_H
#define RETRONET_H

#include <stdint.h>
#include "base_types.h"

/* TCP client. The handle is an adapter-side TCP handle supplied by the caller. */
dl_u8 rn_TCPOpen(dl_u8 hostnameLen, dl_u8 *hostname, dl_u16 port, dl_u8 tcpHandle);
void rn_TCPHandleClose(dl_u8 tcpHandle);
int32_t rn_TCPHandleSize(dl_u8 tcpHandle);
int32_t rn_TCPHandleRead(dl_u8 tcpHandle, dl_u8 *buffer, dl_u16 bufferOffset, dl_u16 readLength);
int32_t rn_TCPHandleWrite(dl_u8 tcpHandle, dl_u16 dataOffset, dl_u16 dataLen, dl_u8 *data);

/* TCP server. These functions operate on the adapter's configured TCP server. */
dl_u8 rn_TCPServerClientCnt(void);
dl_u8 rn_TCPServerAvailable(void);
dl_u8 rn_TCPServerRead(dl_u8 *buffer, dl_u16 bufferOffset, dl_u8 readLength);
void rn_TCPServerWrite(dl_u16 dataOffset, dl_u8 dataLen, dl_u8 *data);

#endif
