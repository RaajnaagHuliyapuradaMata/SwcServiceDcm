

#ifndef DCMDSPUDS_REQUESTUPLOAD_INF_H
#define DCMDSPUDS_REQUESTUPLOAD_INF_H

#include "Dcm.hpp"

#if(DCM_CFG_DSPUDSSUPPORT_ENABLED != DCM_CFG_OFF)

#include "DcmCore_DslDsd_Prot.hpp"
#include "DcmDspUds_Uds_Prot.hpp"
#if(DCM_CFG_DSP_REQUESTUPLOAD_ENABLED != DCM_CFG_OFF)
#include "DcmDspUds_RequestUpload_Prot.hpp"
#include "DcmDspUds_Memaddress_Calc_Prot.hpp"
#include "DcmAppl.hpp"
#endif

#endif

#endif

