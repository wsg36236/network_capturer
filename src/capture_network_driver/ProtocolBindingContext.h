// protocol binding context util

#pragma once
#include <ndis.h>
#include "../Common/CommonHeader.h"

typedef enum _NDISPROT_OPEN_STATE {
    NdisprotInitializing,
    NdisprotRunning,
    NdisprotPausing,
    NdisprotPaused,
    NdisprotRestarting,
    NdisprotClosing
} NDISPROT_OPEN_STATE;

//
//  Definitions for Flags above.
//
#define NPROTO_BIND_IDLE             0x00000000
#define NPROTO_BIND_OPENING          0x00000001
#define NPROTO_BIND_FAILED           0x00000002
#define NPROTO_BIND_ACTIVE           0x00000004
#define NPROTO_BIND_CLOSING          0x00000008
#define NPROTO_BIND_FLAGS            0x0000000F  // State of the binding

#define NPROTO_OPEN_IDLE             0x00000000
#define NPROTO_OPEN_ACTIVE           0x00000010
#define NPROTO_OPEN_FLAGS            0x000000F0  // State of the I/O open

#define NPROTO_RESET_IN_PROGRESS     0x00000100
#define NPROTO_NOT_RESETTING         0x00000000
#define NPROTO_RESET_FLAGS           0x00000100

#define NPROTO_MEDIA_CONNECTED       0x00000000
#define NPROTO_MEDIA_DISCONNECTED    0x00000200
#define NPROTO_MEDIA_FLAGS           0x00000200

#define NPROTO_READ_SERVICING        0x00100000  // Is the read service
                                                // routine running?
#define NPROTO_READ_FLAGS            0x00100000

#define NPROTO_UNBIND_RECEIVED       0x10000000  // Seen NDIS Unbind?
#define NPROTO_UNBIND_FLAGS          0x10000000


#define NPROT_ALLOCATED_NBL          0x10000000
#define NPROT_NBL_RETREAT_RECV_RSVD  0x20000000

//
//  The Open Context represents an open of our device object.
//  We allocate this on processing a BindAdapter from NDIS,
//  and free it when all references (see below) to it are gone.
//
//  Binding/unbinding to an NDIS device:
//
//  On processing a BindAdapter call from NDIS, we set up a binding
//  to the specified NDIS device (miniport). This binding is
//  torn down when NDIS asks us to Unbind by calling
//  our UnbindAdapter handler.
//
//  Receiving data:
//
//  While an NDIS binding exists, read IRPs are queued on this
//  structure, to be processed when packets are received.
//  If data arrives in the absence of a pended read IRP, we
//  queue it, to the extent of one packet, i.e. we save the
//  contents of the latest packet received. We fail read IRPs
//  received when no NDIS binding exists (or is in the process
//  of being torn down).
//
//  Sending data:
//
//  Write IRPs are used to send data. Each write IRP maps to
//  a single NDIS packet. Packet send-completion is mapped to
//  write IRP completion. We use NDIS 5.1 CancelSend to support
//  write IRP cancellation. Write IRPs that arrive when we don't
//  have an active NDIS binding are failed.
//
//  Reference count:
//
//  The following are long-lived references:
//  OPEN_DEVICE ioctl (goes away on processing a Close IRP)
//  Pended read IRPs
//  Queued received packets
//  Uncompleted write IRPs (outstanding sends)
//  Existence of NDIS binding
//
#define MAX_MULTICAST_ADDRESS 0x20
typedef struct _NDIS_BINDING_CONTEXT
{
    LIST_ENTRY              Link;           // Link into global list
    ULONG                   Flags;          // State information
    ULONG                   RefCount;
    NDIS_SPIN_LOCK          Lock;

    PFILE_OBJECT            pFileObject;    // Set on OPEN_DEVICE

    NDIS_HANDLE             BindingHandle;
    NDIS_HANDLE             SendNetBufferListPool;

    ULONG                   MacOptions;
    ULONG                   MaxFrameSize;
    ULONG                   DataBackFillSize;
    ULONG                   ContextBackFillSize;

    LIST_ENTRY              PendedWrites;   // pended Write IRPs
    ULONG                   PendedSendCount;

    LIST_ENTRY              PendedReads;    // pended Read IRPs
    ULONG                   PendedReadCount;
    NPAGED_LOOKASIDE_LIST   RecvPacketBuffLookasideList;    // for RecvPacketList to alloc memory from unpaged look aside list
    LIST_ENTRY              RecvPacketList; // a RecvPacketList list
    ULONG                   RecvPacketListCount;

    NET_DEVICE_POWER_STATE  PowerState;
    NDIS_EVENT              PoweredUpEvent; // signalled iff PowerState is D0
    NDIS_STRING             DeviceName;     // used in NdisOpenAdapter
    NDIS_STRING             DeviceDescr;	// friendly name

    NDIS_STATUS             BindStatus;     // for Open/CloseAdapter
    NDIS_EVENT              BindEvent;      // for Open/CloseAdapter

    ULONG                   oc_sig;         // Signature for sanity
    NDISPROT_OPEN_STATE     State;
    NDIS_EVENT              ClosingEvent;
    UCHAR                   CurrentAddress[NPROT_MAC_ADDR_LEN];
    UCHAR                   MCastAddress[MAX_MULTICAST_ADDRESS][NPROT_MAC_ADDR_LEN];
} PROTOCOL_BINDING_CONTEXT, *PNDIS_BINDING_CONTEXT;

// util
PROTOCOL_BINDING_CONTEXT* CreateProtocolBindingContext(IN PNDIS_BIND_PARAMETERS BindParamters);
LONG IncreaseRef(IN PROTOCOL_BINDING_CONTEXT* pNdisBindingContext);
LONG DecreaseRef(IN PROTOCOL_BINDING_CONTEXT* pNdisBindingContext);
void Lock(IN PROTOCOL_BINDING_CONTEXT* pNdisBindingContext);
void Unlock(IN PROTOCOL_BINDING_CONTEXT* pNdisBindingContext);

void SetState(IN PROTOCOL_BINDING_CONTEXT* pOpenContext, IN NDISPROT_OPEN_STATE state);
BOOLEAN AdapterIsOk(IN PROTOCOL_BINDING_CONTEXT* pOpenContext);

// ****************************** flag ****************************** //
NDIS_STATUS EnableAdapterToReceivePacket(IN PROTOCOL_BINDING_CONTEXT* pNdisBindingContext);
NDIS_STATUS DisableAdapterToReceivePacket(IN PROTOCOL_BINDING_CONTEXT* pNdisBindingContext);
ULONG GetFlag(IN PROTOCOL_BINDING_CONTEXT* pNdisBindingContext);
void  SetFlag(IN PROTOCOL_BINDING_CONTEXT* pNdisBindingContext, IN ULONG mask, IN ULONG flag);
BOOLEAN CheckFlag(IN PROTOCOL_BINDING_CONTEXT* pNdisBindingContext, IN ULONG mask, IN ULONG flag);
BOOLEAN CheckAndSetFlag(IN PROTOCOL_BINDING_CONTEXT* pNdisBindingContext, IN ULONG mask, IN ULONG nCheckFlag, IN ULONG nSetFlag);

// ****************************** pending read irp ****************************** //
BOOLEAN IsReadyToRead(IN PROTOCOL_BINDING_CONTEXT* pOpenContext);
void PushPendingReadIrp(IN PROTOCOL_BINDING_CONTEXT* pOpenContext, IN OUT PIRP pIrp);
PIRP PopPendingReadIrp(IN PROTOCOL_BINDING_CONTEXT* pOpenContext, OUT BOOLEAN* bHasCanceledIrp);
void RemovePendingReadIrp(IN PROTOCOL_BINDING_CONTEXT* pOpenContext, IN OUT PIRP pIrp);
void CancelAllPendingReadIrps(IN PROTOCOL_BINDING_CONTEXT* pOpenContext);
void WaitForPendingIO(IN PROTOCOL_BINDING_CONTEXT* pOpenContext, IN BOOLEAN bHasCanceledIrp);

// ****************************** pending context irp ****************************** //
BOOLEAN SetOpenContextToIrp(IN PROTOCOL_BINDING_CONTEXT* pOpenContext, OUT PIRP pIrp);
void RemoveOpenContextFromIrp(OUT PIRP pIrp);
PROTOCOL_BINDING_CONTEXT* GetOpenContextFromIrp(OUT PIRP pIrp);


