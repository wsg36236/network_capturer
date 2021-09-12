#pragma once

#include "ProtocolBindingContext.h"

NDIS_STATUS ndisprotDoRequest(
    IN PROTOCOL_BINDING_CONTEXT*    pOpenContext,
    IN NDIS_PORT_NUMBER             PortNumber,
    IN NDIS_REQUEST_TYPE            RequestType,
    IN NDIS_OID                     Oid,
    IN PVOID                        InformationBuffer,
    IN ULONG                        InformationBufferLength,
    OUT PULONG                      pBytesProcessed
);

VOID OnRequestComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_OID_REQUEST       pNdisRequest,
    IN  NDIS_STATUS             Status
    );

VOID OnStatusChange(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_STATUS_INDICATION StatusIndication
    );
