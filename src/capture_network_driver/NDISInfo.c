#include "NDISInfo.h"
#include "NDISCommon.h"
#include "NDISMacro.h"
#include "ProtocolBindingContextList.h"

NDIS_STATUS ndisprotDoRequest(
    IN PROTOCOL_BINDING_CONTEXT* pOpenContext,
    IN NDIS_PORT_NUMBER             PortNumber,
    IN NDIS_REQUEST_TYPE            RequestType,
    IN NDIS_OID                     Oid,
    IN PVOID                        InformationBuffer,
    IN ULONG                        InformationBufferLength,
    OUT PULONG                      pBytesProcessed
)
/*++

Routine Description:

    Utility routine that forms and sends an NDIS_REQUEST to the
    miniport, waits for it to complete, and returns status
    to the caller.

    NOTE: this assumes that the calling routine ensures validity
    of the binding handle until this returns.

Arguments:

    pOpenContext - pointer to our open context
    PortNumber - the port to issue the request
    RequestType - NdisRequest[Set|Query|Method]Information
    Oid - the object being set/queried
    InformationBuffer - data for the request
    InformationBufferLength - length of the above
    pBytesProcessed - place to return bytes read/written

Return Value:

    Status of the set/query/method request

--*/
{
    NDISPROT_REQUEST            ReqContext;
    PNDIS_OID_REQUEST           pNdisRequest = &ReqContext.Request;
    NDIS_STATUS                 Status;


    NdisZeroMemory(&ReqContext, sizeof(ReqContext));

    NPROT_INIT_EVENT(&ReqContext.ReqEvent);
    pNdisRequest->Header.Type = NDIS_OBJECT_TYPE_OID_REQUEST;
    pNdisRequest->Header.Revision = NDIS_OID_REQUEST_REVISION_1;
    pNdisRequest->Header.Size = sizeof(NDIS_OID_REQUEST);
    pNdisRequest->RequestType = RequestType;
    pNdisRequest->PortNumber = PortNumber;

    switch (RequestType)
    {
    case NdisRequestQueryInformation:
        pNdisRequest->DATA.QUERY_INFORMATION.Oid = Oid;
        pNdisRequest->DATA.QUERY_INFORMATION.InformationBuffer = InformationBuffer;
        pNdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength = InformationBufferLength;
        break;

    case NdisRequestSetInformation:
        pNdisRequest->DATA.SET_INFORMATION.Oid = Oid;
        pNdisRequest->DATA.SET_INFORMATION.InformationBuffer = InformationBuffer;
        pNdisRequest->DATA.SET_INFORMATION.InformationBufferLength = InformationBufferLength;
        break;

    default:
        ASSERT(FALSE);
        break;
    }

    pNdisRequest->RequestId = NPROT_GET_NEXT_CANCEL_ID();
    Status = NdisOidRequest(pOpenContext->BindingHandle, pNdisRequest);

    if (Status == NDIS_STATUS_PENDING)
    {
        // event will be set in OnRequestComplete()
        NPROT_WAIT_EVENT(&ReqContext.ReqEvent, 0);
        Status = ReqContext.Status;
    }

    if (Status == NDIS_STATUS_SUCCESS)
    {
        *pBytesProcessed = (RequestType == NdisRequestQueryInformation) ?
            pNdisRequest->DATA.QUERY_INFORMATION.BytesWritten :
            pNdisRequest->DATA.SET_INFORMATION.BytesRead;

        //
        // The driver below should set the correct value to BytesWritten 
        // or BytesRead. But now, we just truncate the value to InformationBufferLength
        //
        if (*pBytesProcessed > InformationBufferLength)
        {
            *pBytesProcessed = InformationBufferLength;
        }
    }

    return (Status);
}

VOID OnRequestComplete(IN NDIS_HANDLE ProtocolBindingContext, IN PNDIS_OID_REQUEST pNdisRequest, IN NDIS_STATUS Status)
/*++

Routine Description:

    NDIS entry point indicating completion of a pended NDIS_REQUEST.

Arguments:

    ProtocolBindingContext - pointer to open context
    pNdisRequest - pointer to NDIS request
    Status - status of reset completion

Return Value:

    None

--*/
{
    PNDISPROT_REQUEST            pReqContext;

    // Get at the request context.
    // NDISPROT_REQUEST expand PNDIS_OID_REQUEST with event and status members
    pReqContext = CONTAINING_RECORD(pNdisRequest, NDISPROT_REQUEST, Request);

    //  Save away the completion status.
    pReqContext->Status = Status;

    //  Wake up the thread blocked for this request to complete.
    NPROT_SIGNAL_EVENT(&pReqContext->ReqEvent);
}

VOID OnStatusChange(IN NDIS_HANDLE ProtocolBindingContext, IN PNDIS_STATUS_INDICATION StatusIndication)
/*++

Routine Description:

    Protocol entry point called by NDIS to indicate a change
    in status at the miniport.

    We make note of reset and media connect status indications.

Arguments:

    ProtocolBindingContext - pointer to open context
    StatusIndication - pointer to NDIS_STATUS_INDICATION

Return Value:

    None

--*/
{
    NDIS_STATUS                  GeneralStatus;
    PNDIS_LINK_STATE             LinkState;
    PNDIS_BINDING_CONTEXT        pOpenContext = (PNDIS_BINDING_CONTEXT)ProtocolBindingContext;
    
    if ((StatusIndication->Header.Type != NDIS_OBJECT_TYPE_STATUS_INDICATION)
        || (StatusIndication->Header.Size != sizeof(NDIS_STATUS_INDICATION)))
    {
        return;
    }

    /*
        NdisDeregisterProtocolDriver() will call NdisprotUnbindAdapter() at first, then call NdisprotStatus() with status NDIS_STATUS_OPER_STATUS,
        but pOpenContext is free after NdisprotUnbindAdapter(), so verify it
    */
    if (!ProtocolBindingContextIsExists(pOpenContext))
    {
        return;
    }

    GeneralStatus = StatusIndication->StatusCode;
    Lock(pOpenContext);

    do
    {
        if (pOpenContext->PowerState != NetDeviceStateD0)
        {
            //
            //
            //  The device is in a low power state.

            //
            //  We continue and make note of status indications
            //

            //
            //  NOTE that any actions we take based on these
            //  status indications should take into account
            //  the current device power state.
            //
        }

        switch (GeneralStatus)
        {
        case NDIS_STATUS_RESET_START:
        {
            ASSERT(!NPROT_TEST_FLAGS(pOpenContext->Flags,
                    NPROTO_RESET_FLAGS,
                    NPROTO_RESET_IN_PROGRESS));

            NPROT_SET_FLAGS(pOpenContext->Flags, NPROTO_RESET_FLAGS, NPROTO_RESET_IN_PROGRESS);
            break;
        }
        case NDIS_STATUS_RESET_END:
        {
            ASSERT(NPROT_TEST_FLAGS(pOpenContext->Flags,
                NPROTO_RESET_FLAGS,
                NPROTO_RESET_IN_PROGRESS));

            NPROT_SET_FLAGS(pOpenContext->Flags, NPROTO_RESET_FLAGS, NPROTO_NOT_RESETTING);
            break;
        }
        case NDIS_STATUS_LINK_STATE:
        {
            ASSERT(StatusIndication->StatusBufferSize >= sizeof(NDIS_LINK_STATE));

            LinkState = (PNDIS_LINK_STATE)StatusIndication->StatusBuffer;

            if (LinkState->MediaConnectState == MediaConnectStateConnected)
            {
                NPROT_SET_FLAGS(pOpenContext->Flags, NPROTO_MEDIA_FLAGS, NPROTO_MEDIA_CONNECTED);
            }
            else
            {
                NPROT_SET_FLAGS(pOpenContext->Flags, NPROTO_MEDIA_FLAGS, NPROTO_MEDIA_DISCONNECTED);
            }

            break;
        }
        default:
            break;
        }
    } while (FALSE);

    Unlock(pOpenContext);
}
