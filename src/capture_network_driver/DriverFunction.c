#include "DriverFunction.h"
#include "../Common/CommonHeader.h"
#include "IRPUtil.h"
#include "NDISMacro.h"
#include "DriverLog.h"
#include "NDISAdapter.h"
#include "ProtocolBindingContextList.h"
#include "NDISReceivePacket.h"
#include "ReceivePacketBuff.h"

BOOLEAN g_bOpen = FALSE;

NTSTATUS DefaultDispatch(PIRP pIrp, NTSTATUS status, ULONG_PTR information)
{
    CompleteIrp(pIrp, status, information);
    return status;
}


NTSTATUS DispatchCreate(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
    LOG_DEBUG("call dispatch create");
    NTSTATUS status = STATUS_SUCCESS;
    if (ExGetPreviousMode() == UserMode)
    {
        if (g_bOpen)
        {
            status = STATUS_INVALID_DEVICE_REQUEST;
            LOG_DEBUG("repeat to create device %WZ", &pDeviceObject->DriverObject->DriverName);
        }
        else
        {
            g_bOpen = TRUE;
        }
    }

    return ;
}

NTSTATUS DispatchCleanUp(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
    LOG_DEBUG("call dispatch cleanup");
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    PROTOCOL_BINDING_CONTEXT* pNdisBindingContext = GetOpenContextFromIrp(pIrp);

    // disable adapter ref to driver device
    if (pNdisBindingContext)
    {
        // stop to receive packets
        ndisStatus = DisableAdapterToReceivePacket(pNdisBindingContext);
        if (ndisStatus != NDIS_STATUS_SUCCESS)
        {
            LOG_DEBUG("disable adapter to receive packet fail, status is 0x%x", ndisStatus);
        }

        // Cancel any pending reads
        CancelAllPendingReadIrps(pNdisBindingContext);

        // Clean up the receive packet queue
        ClearReceivePacketBuff(pNdisBindingContext);

        // set flag as NPROTO_OPEN_IDLE
        SetFlag(pNdisBindingContext, NPROTO_OPEN_IDLE, NPROTO_OPEN_IDLE);
        RemoveOpenContextFromIrp(pIrp);
    }

    return DefaultDispatch(pIrp, STATUS_SUCCESS, 0);
}

NTSTATUS DispatchClose(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
    LOG_DEBUG("call dispatch ");
    g_bOpen = FALSE;
    
    return DefaultDispatch(pIrp, STATUS_SUCCESS, 0);
}

NTSTATUS OpenAdapter(IN OUT PIRP pIrp, IN WCHAR* pAdapterName)
{
    NTSTATUS status = STATUS_SUCCESS;
    PROTOCOL_BINDING_CONTEXT* pProtocolBindingContext = NULL;
    PIO_STACK_LOCATION pCurrentStack = IoGetCurrentIrpStackLocation(pIrp);

    do 
    {
        NDIS_STATUS nNdisStatus = NDIS_STATUS_SUCCESS;
        pProtocolBindingContext = LookupProtocolBindingContext(pAdapterName, pCurrentStack->Parameters.DeviceIoControl.InputBufferLength);
        if (pProtocolBindingContext == NULL)
        {
            status = STATUS_OBJECT_NAME_NOT_FOUND;
            break;
        }

        // check adapter status, only open once for one adapter
        if (!CheckFlag(pProtocolBindingContext, NPROTO_OPEN_FLAGS, NPROTO_OPEN_IDLE))
        {
            status = STATUS_DEVICE_BUSY;
            break;
        }

        if (!SetOpenContextToIrp(pProtocolBindingContext, pIrp))
        {
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }
        nNdisStatus = EnableAdapterToReceivePacket(pProtocolBindingContext);
        NDIS_STATUS_TO_NT_STATUS(nNdisStatus, &status);

        if (status != STATUS_SUCCESS)
        {
            LOG_DEBUG("enable adapter fail for %WZ, status is %x", &pProtocolBindingContext->DeviceName, status);
            RemoveOpenContextFromIrp(pIrp);
            break;
        }

        SetFlag(pProtocolBindingContext, NPROTO_OPEN_FLAGS, NPROTO_OPEN_ACTIVE);
    } while (FALSE);

    // if fail, to here
    if (pProtocolBindingContext)
    {
        DecreaseRef(pProtocolBindingContext);
    }
    return status;
}

NTSTATUS DispatchDeviceControl(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG_PTR information = 0;
    PIO_STACK_LOCATION pCurrentStack = IoGetCurrentIrpStackLocation(pIrp);
    ULONG uIoControlCode = pCurrentStack->Parameters.DeviceIoControl.IoControlCode;
    PVOID pSystemBuffer = pIrp->AssociatedIrp.SystemBuffer;
    ULONG nOutputBufferLength = pCurrentStack->Parameters.DeviceIoControl.OutputBufferLength;

    LOG_DEBUG("call dispatch device control, IO control code is %x", uIoControlCode);

    switch (uIoControlCode)
    {
    case IOCTL_NDISPROT_BIND_WAIT:
    {
        if (!NdisWaitEvent(&Globals.BindsComplete, 5000))
        {
            status = STATUS_TIMEOUT;
        }
        break;
    }
    case IOCTL_NDISPROT_QUERY_BINDING:
    {
        NDIS_STATUS nNdisStatus = QueryAllBindingAdapter((AdapterInfoBuff*)pSystemBuffer, &nOutputBufferLength);
        NDIS_STATUS_TO_NT_STATUS(nNdisStatus, &status);
        information = nOutputBufferLength;
        break;
    }
    case IOCTL_NDISPROT_OPEN_DEVICE:
    {
        status = OpenAdapter(pIrp, (WCHAR*)pSystemBuffer);
        break;
    }
    default:
        break;
    }

    return DefaultDispatch(pIrp, status, information);
}

NTSTATUS DispatchRead(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
    NTSTATUS status = STATUS_SUCCESS;
    LOG_DEBUG("call dispatch read");

    NDIS_STATUS_TO_NT_STATUS(ReadPacket(pIrp), &status);

    return status;
}

