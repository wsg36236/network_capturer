/*
this file is a part of ProtocolBindingContext.h and ProtocolBindingContext.cpp
to avoid too long file, so move read packet buff code to here
*/


#pragma once
#include "ProtocolBindingContext.h"

typedef struct _ReceivePacketList
{
    LIST_ENTRY  Link;
    ULONG       nPacketSize;
    UCHAR       pPacketBuff[MAX_PACKET_SIZE];
}ReceivePacketList;

NTSTATUS InitReceivePacketBuff(IN OUT PROTOCOL_BINDING_CONTEXT* pOpenContext);
void UnInitReceivePacketBuff(IN OUT PROTOCOL_BINDING_CONTEXT* pOpenContext);
NTSTATUS PushReceivePacket(IN OUT PROTOCOL_BINDING_CONTEXT* pOpenContext);
NTSTATUS PopReceivePacket(IN OUT PROTOCOL_BINDING_CONTEXT* pOpenContext);
void ClearReceivePacketBuff(IN OUT PROTOCOL_BINDING_CONTEXT* pOpenContext);
