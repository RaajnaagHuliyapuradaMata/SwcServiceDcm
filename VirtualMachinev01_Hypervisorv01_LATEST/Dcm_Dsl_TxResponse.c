#include "Std_Types.hpp"

#include "DcmCore_DslDsd_Inf.hpp"
#include "DcmCore_DslDsd_Prot.hpp"

#define DCM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Dcm_Cfg_MemMap.hpp"
VAR(Dcm_DslTxType_tst,DCM_VAR) Dcm_DslTransmit_st;
#define DCM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Dcm_Cfg_MemMap.hpp"

#define DCM_START_SEC_VAR_CLEARED_BOOLEAN
#include "Dcm_Cfg_MemMap.hpp"
VAR(boolean,DCM_VAR) Dcm_isSessionStored_b;
#define DCM_STOP_SEC_VAR_CLEARED_BOOLEAN
#include "Dcm_Cfg_MemMap.hpp"

LOCAL_INLINE FUNC(boolean, DCM_CODE) Dcm_Lok_isConfirmationOnActiveConnection(VAR(Type_SwcServiceCom_tIdPdu, AUTOMATIC) DcmTxPduId)
{
    return (CfgSwcServiceDcm_stDsld.ptr_txtable_pca[DcmTxPduId] == Dcm_DsldGlobal_st.dataActiveTxPduId_u8);
}

LOCAL_INLINE FUNC(boolean, DCM_CODE) Dcm_Lok_isPositiveResponseSupressed(\
        P2CONST(Dcm_MsgContextType, AUTOMATIC,DCM_INTERN_CONST) pMsgContext)
{
    return((FALSE != (pMsgContext->msgAddInfo).suppressPosResponse) && \
            (Dcm_DsldGlobal_st.cntrWaitpendCounter_u8 == 0x00u));
}

LOCAL_INLINE FUNC(boolean, DCM_CODE) Dcm_Lok_isNegativeResponseSupressed(\
        P2CONST(Dcm_MsgContextType, AUTOMATIC,DCM_INTERN_CONST) pMsgContext,VAR(uint8, AUTOMATIC) NrcValue_u8)
{
    return((pMsgContext->dcmRxPduId >= DCM_INDEX_FUNC_RX_PDUID) && (DCM_IS_KWPPROT_ACTIVE() == FALSE) && \
               (DCM_CFG_SUPPRESS_NRC(NrcValue_u8)) && (Dcm_DsldGlobal_st.cntrWaitpendCounter_u8 == 0x00u));
}

LOCAL_INLINE FUNC(boolean, DCM_CODE) Dcm_Lok_isDcmWaitingForTxConfirmation(void){
    return (!(((Dcm_Lok_GetDslState() == DSL_STATE_REQUEST_RECEIVED)           && \
            (Dcm_Lok_GetDslSubState() == DSL_SUBSTATE_P2MAX_TIMEMONITORING))   || \
            ((Dcm_Lok_GetDslState()   == DSL_STATE_RESPONSE_TRANSMISSION)      && \
            (Dcm_Lok_GetDslSubState() == DSL_SUBSTATE_SEND_GENERAL_REJECT))    || \
            (Dcm_Lok_GetDslState()    == DSL_STATE_ROETYPE1_RECEIVED)));
}

LOCAL_INLINE FUNC(boolean, DCM_CODE) Dcm_Lok_isNormalResponseConfirmationProcessed(void){
    return((Dcm_Lok_GetDsdState() == DSD_SENDTXCONF_APPL) && \
       (Dcm_DsldMsgContext_st.msgAddInfo.sourceofRequest == DCM_UDS_TESTER_SOURCE) && \
       (Dcm_DsldGlobal_st.idxActiveSession_u8 == DCM_DEFAULT_SESSION_IDX)
        );
}

LOCAL_INLINE FUNC(boolean, DCM_CODE) Dcm_Lok_isConfirmationForPendingResponse(void){
    return (Dcm_DsldGlobal_st.cntrWaitpendCounter_u8 > 0x0u);
}

LOCAL_INLINE FUNC(boolean, DCM_CODE) Dcm_Lok_CheckWaitPendCounterOverflow(void){
    return((Dcm_DsldGlobal_st.cntrWaitpendCounter_u8 < DCM_CFG_MAX_WAITPEND) \
            || (DCM_CFG_MAX_WAITPEND == DCM_MAXNUMRESPONSEPENDING));
}

LOCAL_INLINE FUNC(boolean, DCM_CODE) Dcm_Lok_isConfirmationReceivedForNrc21Response(VAR(Type_SwcServiceCom_tIdPdu, AUTOMATIC) DcmTxPduId
   ,     VAR(uint8, AUTOMATIC) idxRxPduId)
{
   VAR(boolean, AUTOMATIC) confirmStatus_b = FALSE;
   VAR(uint8, AUTOMATIC) ConnectionId_u8     = Dcm_DsldRxTable_pcu8[idxRxPduId];
   VAR(uint8, AUTOMATIC) idxProtocol_u8      = Dcm_DsldConnTable_pcst[ConnectionId_u8].protocol_num_u8;
   VAR(Type_SwcServiceCom_tIdPdu, AUTOMATIC) idxTxpduid_u8   = Dcm_DsldConnTable_pcst[ConnectionId_u8].txpduid_num_u8;
   if(( FALSE   != Dcm_DsldProtocol_pcst[idxProtocol_u8].nrc21_b)          && \
        ( DCM_SERVICEID_DEFAULT_VALUE !=  Dcm_DslRxPduArray_ast[idxRxPduId].Dcm_DslServiceId_u8 )   && \
        ( idxTxpduid_u8 == CfgSwcServiceDcm_stDsld.ptr_txtable_pca[DcmTxPduId] ))
   {
        confirmStatus_b = TRUE;
   }
    return confirmStatus_b;
}

LOCAL_INLINE FUNC(boolean, DCM_CODE) Dcm_Lok_isResponseSentForDSCService(void){
    return ((Dcm_DsldGlobal_st.dataSid_u8 == DCM_SID_DIAGNOSTICSESSIONCONTROL) &&
            (Dcm_DsldGlobal_st.stResponseType_en == DCM_DSLD_POS_RESPONSE) &&
            (Dcm_DsldMsgContext_st.msgAddInfo.sourceofRequest == DCM_UDS_TESTER_SOURCE));
}

#define DCM_START_SEC_CODE
#include "Dcm_Cfg_MemMap.hpp"
static FUNC(void, DCM_CODE) Dcm_Lok_InactivateComMChannel(void){
    SchM_Enter_Dcm_Global_NoNest();
   if(FALSE != Dcm_Lok_isNormalResponseConfirmationProcessed())
   {
        ComM_DCM_InactiveDiagnostic(Dcm_active_commode_e[\
                         Dcm_DsldConnTable_pcst[Dcm_DsldGlobal_st.nrActiveConn_u8].channel_idx_u8].ComMChannelId);
   }
    SchM_Exit_Dcm_Global_NoNest();
}

static FUNC(void, DCM_CODE) Dcm_Lok_ProcessSessionChangeOnWarmResp (void){
    Dcm_SesChgOnWarmResp_b = FALSE;
    Dcm_DsldSetsessionTiming(Dcm_Dsp_Session[Dcm_ctDiaSess_u8].P2str_max_u32,Dcm_Dsp_Session[Dcm_ctDiaSess_u8].P2_max_u32);
    Dcm_Lok_SetSesCtrlType(Dcm_Dsp_Session[Dcm_ctDiaSess_u8].session_level);
}

static FUNC(void, DCM_CODE) Dcm_Lok_DsdConfirmation (void){
   if(Dcm_Lok_GetDslState() != DSL_STATE_ROETYPE1_RECEIVED){
        Dcm_Lok_ReloadS3Timer();
   }
   if(Dcm_Lok_GetDsdState() == DSD_WAITFORTXCONF){
        Dcm_Lok_SetDsdState(DSD_SENDTXCONF_APPL);
   }
   if(Dcm_SesChgOnWarmResp_b == TRUE){
        Dcm_Lok_ProcessSessionChangeOnWarmResp();
   }
    Dcm_Lok_InactivateComMChannel();
}

static FUNC(void, DCM_CODE) Dcm_Lok_SetNewSession(void){
   VAR(uint8, AUTOMATIC) nrSessions_u8 = 0u;
   VAR(uint8, AUTOMATIC) idxSession_u8 = 0u;
   if(FALSE != Dcm_Lok_isResponseSentForDSCService())
   {
        Dcm_DsldGlobal_st.PreviousSessionIndex = Dcm_DsldGlobal_st.idxActiveSession_u8;
        nrSessions_u8 = DCM_CFG_NUM_UDS_SESSIONS;
        for(idxSession_u8 = 0x0; idxSession_u8 < nrSessions_u8 ; idxSession_u8++)
        {
            if(Dcm_DsldSessionTable_pcu8[idxSession_u8]== Dcm_Dsp_Session[Dcm_ctDiaSess_u8].session_level)
            {
                Dcm_DsldGlobal_st.idxActiveSession_u8  = idxSession_u8;
                break;
            }
        }
        Dcm_isSessionStored_b =  TRUE;
   }
}

static FUNC(Std_ReturnType, DCM_CODE) Dcm_Lok_CheckP2StarTimer(void){
   VAR(Std_ReturnType, AUTOMATIC) halfP2timeStatus = E_NOT_OK;
   VAR(uint32, AUTOMATIC) halfP2timer_u32   = 0u;
   if(Dcm_DsldGlobal_st.cntrWaitpendCounter_u8 == 0u)
   {
        halfP2timeStatus = E_OK;
   }
   else{
        halfP2timer_u32 = Dcm_DsldTimer_st.dataTimeoutP2StrMax_u32 >> 1u;
        if(Dcm_DsldGlobal_st.dataTimeoutMonitor_u32 < (halfP2timer_u32/DCM_CFG_TASK_TIME_US))
        {
            halfP2timeStatus = E_OK;
        }
   }
    return(halfP2timeStatus);
}

FUNC(void, DCM_CODE) Dcm_TriggerTransmit(VAR(Type_SwcServiceCom_tIdPdu, AUTOMATIC) TxPduId,P2VAR(Type_SwcServiceCom_stInfoPdu, AUTOMATIC, DCM_INTERN_DATA) PduInfoPtr )
{
    (void)TxPduId;
    (void)PduInfoPtr;
}

FUNC(void, DCM_CODE) Dcm_SetNegResponse ( P2CONST(Dcm_MsgContextType, AUTOMATIC,DCM_INTERN_CONST) pMsgContext
   ,     VAR(Dcm_NegativeResponseCodeType, AUTOMATIC) ErrorCode)
{
   if((pMsgContext->msgAddInfo.sourceofRequest == DCM_UDS_TESTER_SOURCE) || \
          (Dcm_Lok_GetDslState() == DSL_STATE_ROETYPE1_RECEIVED))
   {
        if(pMsgContext->dcmRxPduId == Dcm_DsldGlobal_st.dataActiveRxPduId_u8)
        {
            SchM_Enter_Dcm_Global_NoNest();
            if(Dcm_DsldGlobal_st.stResponseType_en == DCM_DSLD_POS_RESPONSE)
            {
                Dcm_DsldGlobal_st.stResponseType_en = DCM_DSLD_NEG_RESPONSE;
                Dcm_DsldGlobal_st.adrActiveTxBuffer_tpu8[0]= DCM_NEGRESPONSE_INDICATOR;
                Dcm_DsldGlobal_st.adrActiveTxBuffer_tpu8[1]= Dcm_DsldGlobal_st.dataSid_u8;
                Dcm_DsldGlobal_st.adrActiveTxBuffer_tpu8[2]= ErrorCode;
            }
            SchM_Exit_Dcm_Global_NoNest();
        }
   }
   else{
   }
}

FUNC(void, DCM_CODE) Dcm_Lok_SendResponse(P2CONST(Type_SwcServiceCom_stInfoPdu, AUTOMATIC, DCM_INTERN_DATA) adrPduStrucutre_pcst)
{
   VAR(boolean, AUTOMATIC) Result_b = TRUE;
        Result_b = FALSE;
        if( DCM_CHKFULLCOMM_MODE(Dcm_DsldConnTable_pcst[Dcm_DsldGlobal_st.nrActiveConn_u8].channel_idx_u8) )
        {
            if(E_NOT_OK != PduR_DcmTransmit(Dcm_DsldGlobal_st.dataActiveTxPduId_u8, adrPduStrucutre_pcst))
            {
                Result_b = TRUE;
            }
        }
   if(FALSE == Result_b)
   {
        if(Dcm_Lok_GetDslNextState() != DSL_STATE_IDLE)
        {
            Dcm_Lok_SetDslState(DSL_STATE_REQUEST_RECEIVED);
            Dcm_Lok_SetDslSubState(DSL_SUBSTATE_P2MAX_TIMEMONITORING);
            if(FALSE != Dcm_Lok_isForcePendingResponse())
            {
                Dcm_DslTransmit_st.isForceResponsePendRequested_b = FALSE;
                Dcm_Lok_ConfirmationRespPendForBootloader(DCM_RES_NEG_NOT_OK);
                (void)Dcm_ConfirmationRespPend(DCM_RES_NEG_NOT_OK);
            }
        }
        else{
            Dcm_DsldGlobal_st.dataResult_u8 = E_NOT_OK;
            Dcm_isGeneralRejectSent_b       = FALSE;
            Dcm_Lok_DsdConfirmation();
            Dcm_Lok_SetDslState(DSL_STATE_IDLE);
            Dcm_Lok_SetDslSubState(DSL_SUBSTATE_S3_OR_P3_TIMEMONITORING);
        }
   }
}

static FUNC(void, DCM_CODE) Dcm_Lok_TransmitNormalResponse(void){
   VAR(boolean, AUTOMATIC) SendResponse_b = FALSE;
   if(Dcm_DslTransmit_st.TxResponseLength_u32 == 0u)
   {
        Dcm_Lok_SetNewSession();
        Dcm_DsldGlobal_st.dataResult_u8 = E_OK;
        Dcm_Lok_DsdConfirmation();
        Dcm_Lok_SetDslState(DSL_STATE_IDLE);
        Dcm_Lok_SetDslSubState(DSL_SUBSTATE_S3_OR_P3_TIMEMONITORING);
   }
   else{
        SchM_Enter_Dcm_Global_NoNest();
            if(FALSE == Dcm_Lok_isDcmWaitingForTxConfirmation())
            {
                Dcm_DsldPduInfo_st.SduDataPtr = Dcm_DslTransmit_st.TxBuffer_tpu8;
                Dcm_DsldPduInfo_st.SduLength = (Type_SwcServiceCom_tLengthPdu) Dcm_DslTransmit_st.TxResponseLength_u32;
                Dcm_DsldGlobal_st.cntrWaitpendCounter_u8 = 0x0u;
                if(Dcm_Lok_GetDslState() != DSL_STATE_ROETYPE1_RECEIVED)
                {
                    Dcm_Lok_SetDslState(DSL_STATE_WAITING_FOR_TXCONFIRMATION);
                    Dcm_Lok_SetDslNextState(DSL_STATE_IDLE);
                    Dcm_Lok_SetDslNextSubState(DSL_SUBSTATE_S3_OR_P3_TIMEMONITORING);
                }
                SendResponse_b = TRUE;
            }
            else{
                if(Dcm_Lok_GetDslState() == DSL_STATE_WAITING_FOR_TXCONFIRMATION)
                {
                    Dcm_Lok_SetDslNextState(DSL_STATE_RESPONSE_TRANSMISSION);
                    Dcm_Lok_SetDslNextSubState(DSL_SUBSTATE_SEND_FINAL_RESPONSE);
                }
            }
        SchM_Exit_Dcm_Global_NoNest();
        if(SendResponse_b != FALSE)
        {
            Dcm_Lok_SendResponse(&Dcm_DsldPduInfo_st);
        }
   }
}

static FUNC(void, DCM_CODE) Dcm_Lok_TransmitPendingReponse(void){
   if((Dcm_Lok_GetDslState() == DSL_STATE_REQUEST_RECEIVED) &&\
            (Dcm_Lok_GetDslSubState() == DSL_SUBSTATE_P2MAX_TIMEMONITORING))
   {
            SchM_Enter_Dcm_Global_NoNest();
            Dcm_DslWaitPendBuffer_au8[0] = DCM_NEGRESPONSE_INDICATOR;
            Dcm_DslWaitPendBuffer_au8[1] = Dcm_DsldGlobal_st.dataSid_u8;
            Dcm_DslWaitPendBuffer_au8[2] = DCM_E_REQUESTCORRECTLYRECEIVED_RESPONSEPENDING ;
            Dcm_DsldPduInfo_st.SduDataPtr = Dcm_DslWaitPendBuffer_au8;
            Dcm_DsldPduInfo_st.SduLength  = DCM_NEGATIVE_RESPONSE_LENGTH;
            if(Dcm_DsldGlobal_st.cntrWaitpendCounter_u8 < DCM_MAXNUMRESPONSEPENDING)
            {
                Dcm_DsldGlobal_st.cntrWaitpendCounter_u8++;
            }
            Dcm_Lok_SetDslState(DSL_STATE_WAITING_FOR_TXCONFIRMATION);
            Dcm_Lok_SetDslNextState(DSL_STATE_REQUEST_RECEIVED);
            Dcm_Lok_SetDslNextSubState(DSL_SUBSTATE_P2MAX_TIMEMONITORING);
            SchM_Exit_Dcm_Global_NoNest();
            Dcm_Lok_SendResponse(&Dcm_DsldPduInfo_st);
   }
}

FUNC(void, DCM_CODE) Dcm_Lok_TriggerTransmit(void){
   if(FALSE != Dcm_Lok_isForcePendingResponse())
   {
        if(E_OK == Dcm_Lok_CheckP2StarTimer())
        {
            if(FALSE != Dcm_Lok_CheckWaitPendCounterOverflow())
            {
                Dcm_Lok_TransmitPendingReponse();
            }
            else{
                Dcm_Lok_SetDslState(DSL_STATE_RESPONSE_TRANSMISSION);
                Dcm_Lok_SetDslSubState(DSL_SUBSTATE_SEND_GENERAL_REJECT);
                Dcm_DslTransmit_st.isForceResponsePendRequested_b = FALSE;
                Dcm_Lok_ConfirmationRespPendForBootloader(DCM_RES_NEG_NOT_OK);
                (void)Dcm_ConfirmationRespPend(DCM_RES_NEG_NOT_OK);
            }
        }
        else{
            Dcm_DslTransmit_st.isForceResponsePendRequested_b = FALSE;
            Dcm_Lok_ConfirmationRespPendForBootloader(DCM_RES_NEG_OK);
            (void)Dcm_ConfirmationRespPend(DCM_RES_NEG_OK);
        }
   }
   else{
        Dcm_Lok_TransmitNormalResponse();
   }
}

static FUNC(void, DCM_CODE)Dcm_Lok_TransmitCurrentResponse(P2CONST(Dcm_MsgContextType, AUTOMATIC,DCM_INTERN_CONST) pMsgContext)
{
   VAR(uint8, AUTOMATIC) NrcValue_u8     = 0u;
   VAR(uint32, AUTOMATIC) bufferSize_u32 = 0u;
   VAR(Dcm_DsdStatesType_ten, AUTOMATIC) DsdState_en = Dcm_Lok_GetDsdState();
   if((DsdState_en == DSD_CALL_SERVICE)||(FALSE != Dcm_DsldGlobal_st.dataResponseByDsd_b))
   {
        Dcm_Lok_SetDsdState(DSD_WAITFORTXCONF);
   }
   if(Dcm_DsldGlobal_st.stResponseType_en == DCM_DSLD_POS_RESPONSE)
   {
        if(FALSE == Dcm_DsldGlobal_st.dataResponseByDsd_b)
        {
            bufferSize_u32 = pMsgContext->resMaxDataLen - pMsgContext->resDataLen;
            DcmAppl_DcmModifyResponse(Dcm_DsldGlobal_st.dataSid_u8, NrcValue_u8,\
                    &(pMsgContext->resData[pMsgContext->resDataLen]), &bufferSize_u32);
        }
        if(FALSE != Dcm_Lok_isPositiveResponseSupressed(pMsgContext))
        {
            Dcm_DslTransmit_st.TxBuffer_tpu8 = NULL_PTR;
            Dcm_DslTransmit_st.TxResponseLength_u32 = 0x00u;
        }
        else{
            Dcm_DsldGlobal_st.adrActiveTxBuffer_tpu8[2] = Dcm_DsldGlobal_st.dataSid_u8 | DCM_SERVICEID_ADDEND;
            Dcm_DslTransmit_st.TxBuffer_tpu8 = &Dcm_DsldGlobal_st.adrActiveTxBuffer_tpu8[2] ;
            Dcm_DslTransmit_st.TxResponseLength_u32 = pMsgContext->resDataLen + 1u + bufferSize_u32;
        }
   }
   else{
        if(FALSE == Dcm_DsldGlobal_st.dataResponseByDsd_b)
        {
            bufferSize_u32 = pMsgContext->resMaxDataLen - pMsgContext->resDataLen;
            DcmAppl_DcmModifyResponse(Dcm_DsldGlobal_st.dataSid_u8, Dcm_DsldGlobal_st.adrActiveTxBuffer_tpu8[2], \
                    &(Dcm_DsldGlobal_st.adrActiveTxBuffer_tpu8[2]), &bufferSize_u32);
        }
        if(FALSE != Dcm_Lok_isNegativeResponseSupressed(pMsgContext,Dcm_DsldGlobal_st.adrActiveTxBuffer_tpu8[2]))
        {
            Dcm_DslTransmit_st.TxBuffer_tpu8 = NULL_PTR ;
            Dcm_DslTransmit_st.TxResponseLength_u32 = 0x00u;
        }
        else{
            Dcm_DslTransmit_st.TxBuffer_tpu8 = Dcm_DsldGlobal_st.adrActiveTxBuffer_tpu8;
            Dcm_DslTransmit_st.TxResponseLength_u32 = DCM_NEGATIVE_RESPONSE_LENGTH;
        }
   }
        if(Dcm_Lok_GetDsdState() == DSD_WAITFORTXCONF)
        {
            Dcm_Lok_TriggerTransmit();
        }
}

FUNC(void, DCM_CODE)Dcm_ProcessingDone(P2CONST(Dcm_MsgContextType, AUTOMATIC,DCM_INTERN_CONST) pMsgContext)
{
   if(((pMsgContext->msgAddInfo.sourceofRequest == DCM_UDS_TESTER_SOURCE)         || \
                (Dcm_Lok_GetDslState() == DSL_STATE_ROETYPE1_RECEIVED))            && \
                (pMsgContext->dcmRxPduId == Dcm_DsldGlobal_st.dataActiveRxPduId_u8))
   {
        Dcm_Lok_TransmitCurrentResponse(pMsgContext);
   }
   else{
   }
}

static FUNC(void, DCM_CODE) Dcm_Lok_ProcessConfirmationForCurrentResponse (VAR(Std_ReturnType, AUTOMATIC) Result)
{
   Dcm_DsldGlobal_st.dataResult_u8 = Result;
   Dcm_Lok_SetNewSession();
   Dcm_Lok_DsdConfirmation();
       Dcm_Lok_SetDslState(Dcm_Lok_GetDslNextState());
       Dcm_Lok_SetDslSubState(Dcm_Lok_GetDslNextSubState());
}

static FUNC(void, DCM_CODE) Dcm_Lok_ProcessConfiramationForPendingResponse (VAR(Std_ReturnType, AUTOMATIC) Result)
{
   VAR(Dcm_ConfirmationStatusType, AUTOMATIC) confirmationStatus = DCM_RES_NEG_NOT_OK;
   if(E_OK == Result)
   {
       DCM_TimerStart(Dcm_DsldGlobal_st.dataTimeoutMonitor_u32, \
            (Dcm_DsldTimer_st.dataTimeoutP2StrMax_u32 - \
            Dcm_DsldProtocol_pcst[Dcm_DsldGlobal_st.idxCurrentProtocol_u8].dataP2StarTmrAdjust),\
            Dcm_P2OrS3StartTick_u32,Dcm_P2OrS3TimerStatus_uchr)
        confirmationStatus = DCM_RES_NEG_OK;
   }
   if(FALSE != Dcm_Lok_isForcePendingResponse())
   {
       Dcm_Lok_ConfirmationRespPendForBootloader(confirmationStatus);
       (void)Dcm_ConfirmationRespPend(confirmationStatus);
       Dcm_DslTransmit_st.isForceResponsePendRequested_b = FALSE;
   }
   Dcm_Lok_SetDslState(Dcm_Lok_GetDslNextState());
   Dcm_Lok_SetDslSubState(Dcm_Lok_GetDslNextSubState());
}

static FUNC(void, DCM_CODE)Dcm_Lok_ProcessTxConfirmation(VAR(Type_SwcServiceCom_tIdPdu, AUTOMATIC) TxPduId
   ,     VAR(Std_ReturnType, AUTOMATIC) result)
{
   VAR(uint8, AUTOMATIC) idxRxPduId_u8;
   if(FALSE != Dcm_Lok_isConfirmationOnActiveConnection(TxPduId))
   {
       if(FALSE != Dcm_Lok_isConfirmationForPendingResponse())
       {
           Dcm_Lok_ProcessConfiramationForPendingResponse(result);
       }
       else{
               Dcm_Lok_ProcessConfirmationForCurrentResponse(result);
       }
   }
   for(idxRxPduId_u8 = 0; idxRxPduId_u8 < DCM_CFG_NUM_RX_PDUID ; idxRxPduId_u8++)
   {
       if(FALSE != Dcm_Lok_isConfirmationReceivedForNrc21Response(TxPduId,idxRxPduId_u8))
       {
           Dcm_DslRxPduArray_ast[idxRxPduId_u8].Dcm_DslServiceId_u8 = DCM_SERVICEID_DEFAULT_VALUE;
           break;
       }
   }
}

FUNC(void, DCM_CODE) Dcm_TpTxConfirmation (VAR(Type_SwcServiceCom_tIdPdu, AUTOMATIC) id,VAR(Std_ReturnType, AUTOMATIC) result)
{
   if( id >= DCM_NUM_TX_PDUID )
   {
         DCM_DET_ERROR(DCM_TPTXCONFIRMATION_ID ,DCM_E_DCMTXPDUID_RANGE_EXCEED)
   }
   else{
        if(FALSE != Dcm_isCancelTransmitInvoked_b)
        {
            Dcm_isCancelTransmitInvoked_b = FALSE;
        }
        else{
            Dcm_Lok_ProcessTxConfirmation(id,result);
        }
   }
}

FUNC(void, DCM_CODE) Dcm_TxConfirmation (VAR(Type_SwcServiceCom_tIdPdu, AUTOMATIC) DcmTxPduId)
{
   VAR(uint8, AUTOMATIC) idxTxPduid_u8 = 0u;
   if( DcmTxPduId >= DCM_NUM_TX_PDUID )
   {
        DCM_DET_ERROR(DCM_TXCONFIRMATION_ID ,DCM_E_DCMRXPDUID_RANGE_EXCEED)
   }
    (void)idxTxPduid_u8;
}

#define DCM_STOP_SEC_CODE
#include "Dcm_Cfg_MemMap.hpp"
