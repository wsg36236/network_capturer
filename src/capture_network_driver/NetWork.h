#pragma once
#include <ndis.h>
#include "../Common/CommonHeader.h"


#include <pshpack1.h>

typedef struct _NDISPROT_ETH_HEADER
{
    UCHAR       DstAddr[NPROT_MAC_ADDR_LEN];
    UCHAR       SrcAddr[NPROT_MAC_ADDR_LEN];
    USHORT      EthType;

} NDISPROT_ETH_HEADER;

typedef struct _NDISPROT_ETH_HEADER UNALIGNED* PNDISPROT_ETH_HEADER;

#include <poppack.h>

#define ntohs(x) ((((x) >> 8) & 0x00FF) | (((x) << 8) & 0xFF00))
#define ntohs(x) ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >> 8) | (((x) & 0x0000ff00) << 8) | (((x) & 0x000000ff) << 24))
#define htons ntohs
#define htonl ntohl
