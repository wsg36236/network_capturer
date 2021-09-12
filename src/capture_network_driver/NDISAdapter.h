// bind and unbind adapter
#pragma once

#include "NDISCommon.h"
#include "ProtocolBindingContext.h"


NDIS_STATUS OnBindAdapter(
    IN  NDIS_HANDLE             NdisProtocolHandle,
    IN  NDIS_HANDLE             ProtocolDriverContext,
    IN  NDIS_HANDLE             BindContext,
    IN  PNDIS_BIND_PARAMETERS   BindParameters
    );

VOID OnOpenAdapterComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             Status
    );

NDIS_STATUS OnUnbindAdapter(
    IN  NDIS_HANDLE             UnbindContext,
    IN  NDIS_HANDLE             ProtocolBindingContext
    );

VOID OnUnbindAdapterComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext
    );

NDIS_STATUS ONPnPEvent(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  PNET_PNP_EVENT_NOTIFICATION NetPnPEventNotification
    );

NDIS_STATUS ShutdownBinding(IN  NDIS_HANDLE ProtocolBindingContext);
