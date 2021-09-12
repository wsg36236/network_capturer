#include "ProtocolBindingContextList.h"
#include "NDISMacro.h"
#include "DriverLog.h"

LIST_ENTRY g_ProtocolBindingContextList = { 0 };
NDIS_SPIN_LOCK g_ProtocolBindingContextListLock = { 0 };    // to protect g_ProtocolBindingContextList

void InitProtocolBindingContextList()
{
    NPROT_INIT_LOCK(&g_ProtocolBindingContextListLock);
    InitializeListHead(&g_ProtocolBindingContextList);
}

// all open context should be free in unbind, no need to free in here
void UninitProtocolBindingContextList()
{
    LIST_ENTRY* pListEntry = NULL;
    PROTOCOL_BINDING_CONTEXT* pOpenContext = NULL;

    NPROT_ACQUIRE_LOCK(&g_ProtocolBindingContextListLock, FALSE);

    for (pListEntry = g_ProtocolBindingContextList.Flink;
         pListEntry != &g_ProtocolBindingContextList;
         pListEntry = pListEntry->Flink)
    {
        pOpenContext = CONTAINING_RECORD(pListEntry, PROTOCOL_BINDING_CONTEXT, Link);
        DecreaseRef(pOpenContext);
    }

    InitializeListHead((&g_ProtocolBindingContextList);
    NPROT_RELEASE_LOCK(&g_ProtocolBindingContextListLock, FALSE);
}

// lookup protocol binding context withou lock, and not increace the ref just use by internal
PROTOCOL_BINDING_CONTEXT* LookupProtocolBindingContextWithoutLock(_In_reads_bytes_(uAdapterNameBufferLength) IN WCHAR* pAdapterNameBuffer, IN ULONG uAdapterNameBufferLength)
{
    LIST_ENTRY* pListEntry = NULL;
    PROTOCOL_BINDING_CONTEXT* pOpenContext = NULL;
    for (pListEntry = g_ProtocolBindingContextList.Flink;
        pListEntry != &g_ProtocolBindingContextList;
        pListEntry = pListEntry->Flink)
    {
        pOpenContext = CONTAINING_RECORD(pListEntry, PROTOCOL_BINDING_CONTEXT, Link);
        if (pOpenContext->DeviceName.Length == uAdapterNameBufferLength &&
            NdisEqualMemory(pAdapterNameBuffer, pOpenContext->DeviceName.Buffer, uAdapterNameBufferLength))
        {
            break;
        }

        pOpenContext = NULL;
    }

    return pOpenContext;
}

// warning : pProtocolBindingContext.DeviceName should init, and this function will lookup device name in g_ProtocolBindingContextList
// if find, this will return FALSE
BOOLEAN AddProtocolBindingContext(IN PROTOCOL_BINDING_CONTEXT* pProtocolBindingContext)
{
    BOOLEAN ret = FALSE;
    if (pProtocolBindingContext->DeviceName.Length == 0)
    {
        return ret;
    }

    NPROT_ACQUIRE_LOCK(&g_ProtocolBindingContextListLock, FALSE);

    if (!LookupProtocolBindingContextWithoutLock(pProtocolBindingContext->DeviceName.Buffer, pProtocolBindingContext->DeviceName.Length))
    {
        IncreaseRef(pProtocolBindingContext);
        InsertTailList(&g_ProtocolBindingContextList, &pProtocolBindingContext->Link);
        ret = TRUE;
    }

    NPROT_RELEASE_LOCK(&g_ProtocolBindingContextListLock, FALSE);

    return ret;
}

void RemoveProtocolBindingContext(IN PROTOCOL_BINDING_CONTEXT* pProtocolBindingContext)
{
    NPROT_ACQUIRE_LOCK(&g_ProtocolBindingContextListLock, FALSE);

    RemoveEntryList(&g_ProtocolBindingContextList);
    DecreaseRef(pProtocolBindingContext);

    NPROT_RELEASE_LOCK(&g_ProtocolBindingContextListLock, FALSE);
}

BOOLEAN ProtocolBindingContextIsExists(IN PROTOCOL_BINDING_CONTEXT* pProtocolBindingContext)
{
    BOOLEAN ret = FALSE;
    LIST_ENTRY* pListEntry = NULL;
    PROTOCOL_BINDING_CONTEXT* pTempOpenContext = NULL;

    NPROT_ACQUIRE_LOCK(&g_ProtocolBindingContextListLock, FALSE);

    for (pListEntry = g_ProtocolBindingContextList.Flink;
        pListEntry != &g_ProtocolBindingContextList;
        pListEntry = pListEntry->Flink)
    {
        pTempOpenContext = CONTAINING_RECORD(pListEntry, PROTOCOL_BINDING_CONTEXT, Link);
        if (pTempOpenContext == pProtocolBindingContext)
        {
            ret = TRUE;
            break;
        }
    }

    NPROT_RELEASE_LOCK(&g_ProtocolBindingContextListLock, FALSE);
    return ret;
}

// if return PROTOCOL_BINDING_CONTEXT is not null, it will increase it's ref
PROTOCOL_BINDING_CONTEXT* LookupProtocolBindingContext(_In_reads_bytes_(uAdapterNameBufferLength) IN WCHAR* pAdapterNameBuffer, IN ULONG uAdapterNameBufferLength)
{
    PROTOCOL_BINDING_CONTEXT* pOpenContext = NULL;

    NPROT_ACQUIRE_LOCK(&g_ProtocolBindingContextListLock, FALSE);
    pOpenContext = LookupProtocolBindingContextWithoutLock(pAdapterNameBuffer, uAdapterNameBufferLength);
    NPROT_RELEASE_LOCK(&g_ProtocolBindingContextListLock, FALSE);
    
    if (pOpenContext)
    {
        IncreaseRef(pOpenContext);
    }

    return pOpenContext;
}

// nBuffSize : pAdapterInfoBuff buffer size, if there is no enough space to contain adapter info, return fail
NDIS_STATUS QueryAllBindingAdapter(IN OUT AdapterInfoBuff* pAdapterInfoBuff, IN OUT ULONG* nBuffSize)
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    AdapterInfo* pAdapterInfo = &pAdapterInfoBuff->adapterInfo[0];
    unsigned int nMaxAdpterInfoCount = *nBuffSize / sizeof(AdapterInfo);
    pAdapterInfoBuff->nCount = 0;

    LOG_DEBUG("adapter info buff address is %p", pAdapterInfoBuff);
    NPROT_ACQUIRE_LOCK(&g_ProtocolBindingContextListLock, FALSE);

    for (LIST_ENTRY* pListEntry = g_ProtocolBindingContextList.Flink;
         pListEntry != &g_ProtocolBindingContextList;
         pListEntry = pListEntry->Flink)
    {
        PROTOCOL_BINDING_CONTEXT*  pOpenContext = CONTAINING_RECORD(pListEntry, PROTOCOL_BINDING_CONTEXT, Link);
        
        // skip no active adapter
        if (!CheckFlag(pOpenContext, NPROTO_BIND_FLAGS, NPROTO_BIND_ACTIVE))
        {
            continue;
        }

        // check buff size
        if (pAdapterInfoBuff->nCount >= nMaxAdpterInfoCount)
        {
            status = NDIS_STATUS_BUFFER_TOO_SHORT;
            break;
        }

        // check adapter name and description size
        if (sizeof(pAdapterInfo->AdapterName < pOpenContext->DeviceName.Length) ||
            sizeof(pAdapterInfo->AdapterDescription) < pOpenContext->DeviceDescr.Length)
        {
            status = NDIS_STATUS_BUFFER_OVERFLOW;
            break;
        }

        // copy adapter name and description
        // TODO : is system IRP IO buffer is non-page memory?
        RtlZeroMemory(pAdapterInfo, sizeof(AdapterInfo));
        RtlCopyMemory(pAdapterInfo->AdapterName, pOpenContext->DeviceName.Buffer, pOpenContext->DeviceName.Length);
        RtlCopyMemory(pAdapterInfo->AdapterDescription, pOpenContext->DeviceDescr.Buffer, pOpenContext->DeviceDescr.Length);

        pAdapterInfo++;
        pAdapterInfoBuff->nCount++;
    }

    NPROT_RELEASE_LOCK(&g_ProtocolBindingContextListLock, FALSE);

    if (status == NDIS_STATUS_SUCCESS)
    {
        *nBuffSize = BUFF_SIZE_FOR_ADAPTER_INFO_BUFF(pAdapterInfoBuff->nCount);
    }
    else
    {
        *nBuffSize = 0;
    }

    return status;
}
