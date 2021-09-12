#pragma once

#include "NDISCommon.h"

// adapter opration
PROTOCOL_BIND_ADAPTER_EX NdisprotBindAdapter;                   // NIC start nodify callback after call NdisOpenAdapterEx to bind NIC, if new NIC is plug in, this function will be call too.
PROTOCOL_OPEN_ADAPTER_COMPLETE_EX NdisprotOpenAdapterComplete;  // when all NIC bind completely after register protocol driver, NDIS will notify it in NetPnPEventNofification in NdisprotPnPEventHandler
PROTOCOL_UNBIND_ADAPTER_EX NdisprotUnbindAdapter;               // unbind adapter by system, and notified unbind completely in 
PROTOCOL_CLOSE_ADAPTER_COMPLETE_EX NdisprotCloseAdapterComplete;// yunbind notify callback after call NdisCloseAdapter

// PnP event(NIC event, for example: all NIC bind completely)
PROTOCOL_NET_PNP_EVENT NdisprotPnPEventHandler;

// adapter info/config
PROTOCOL_STATUS_EX NdisprotStatus;                              // IRQL: dispatch_level, notfiy status change, example : reset
PROTOCOL_OID_REQUEST_COMPLETE NdisprotRequestComplete;          // IRQL: dispatch_level, NdisRequest complete callback, OID(NDIS Object Identifier) is request type

// send and receive packet
PROTOCOL_SEND_NET_BUFFER_LISTS_COMPLETE NdisprotSendComplete;   // IRQL: dispatch_level
PROTOCOL_RECEIVE_NET_BUFFER_LISTS NdisprotReceiveNetBufferLists;// IRQL: dispatch_level

NDIS_STATUS RegisterNdisProtocolDriver();
NDIS_STATUS UnRegisterNdisProtocolDriver();
