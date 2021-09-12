#pragma once

#include "ProtocolBindingContext.h"

VOID OnReceiveNetBufferLists(
    _In_  NDIS_HANDLE             ProtocolBindingContext,
    _In_  PNET_BUFFER_LIST        NetBufferLists,
    _In_  NDIS_PORT_NUMBER        PortNumber,
    _In_  ULONG                   NumberOfNetBufferLists,
    _In_  ULONG                   ReceiveFlags
    );


NTSTATUS ReadPacket(PIRP pIrp); // interface for client to read packet from IRP read dispatch routine
