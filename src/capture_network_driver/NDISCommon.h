#pragma once
#include <ndis.h>
#include "../Common/CommonHeader.h"

#pragma warning(disable:28930) // Unused assignment of pointer, by design in samples
#pragma warning(disable:28931) // Unused assignment of variable, by design in samples

//
// Update the driver version number every time you release a new driver
// The high word is the major version. The low word is the minor version.
// Also make sure that VER_FILEVERSION specified in the .RC file also
// matches with the driver version.
//
// Let's say we're version 4.2.
//
#define MAJOR_DRIVER_VERSION           0x04
#define MINOR_DRIVER_VERISON           0x02


//
// Define the NDIS protocol interface version that this driver targets.
//
#  define NDIS_PROT_MAJOR_VERSION             6
#  define NDIS_PROT_MINOR_VERSION             0

#define oc_signature        'OiuN'

//
//  Globals:
//
typedef struct _NDISPROT_GLOBALS
{
    PDRIVER_OBJECT          pDriverObject;
    PDEVICE_OBJECT          ControlDeviceObject;
    NDIS_HANDLE             NdisProtocolHandle;
    USHORT                  EthType;            // frame type we are interested in
    UCHAR                   PartialCancelId;    // for cancelling sends
    ULONG                   LocalCancelId;
    NDIS_EVENT              BindsComplete;      // have we seen NetEventBindsComplete?
} NDISPROT_GLOBALS, * PNDISPROT_GLOBALS;


//
//  The following are arranged in the way a little-endian processor
//  would read 2 bytes off the wire.
//
#define NPROT_ETH_TYPE               0x8e88
#define NPROT_8021P_TAG_TYPE         0x0081

//
//  NDIS Request context structure
//
typedef struct _NDISPROT_REQUEST
{
    NDIS_OID_REQUEST         Request;
    NDIS_EVENT               ReqEvent;
    ULONG                    Status;

} NDISPROT_REQUEST, * PNDISPROT_REQUEST;


extern NDISPROT_GLOBALS      Globals;

