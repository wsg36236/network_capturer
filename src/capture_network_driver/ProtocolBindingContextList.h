// protocol binding context list manager

#pragma once

#include "ProtocolBindingContext.h"


void InitProtocolBindingContextList();
void UninitProtocolBindingContextList();
BOOLEAN AddProtocolBindingContext(IN PROTOCOL_BINDING_CONTEXT* pProtocolBindingContext);
void RemoveProtocolBindingContext(IN PROTOCOL_BINDING_CONTEXT* pProtocolBindingContext);
BOOLEAN ProtocolBindingContextIsExists(IN PROTOCOL_BINDING_CONTEXT* pProtocolBindingContext);

PROTOCOL_BINDING_CONTEXT* LookupProtocolBindingContext(
    _In_reads_bytes_(uAdapterNameBufferLength) IN WCHAR*    pAdapterNameBuffer,
    IN ULONG                                                uAdapterNameBufferLength
);

NDIS_STATUS QueryAllBindingAdapter(
    IN OUT AdapterInfoBuff* pAdapterInfoBuff,
    IN OUT ULONG*           nBuffSize   // AdapterInfo count in pAdapterInfoBuff, if there is no enough space to contain adapter info, return fail
);

