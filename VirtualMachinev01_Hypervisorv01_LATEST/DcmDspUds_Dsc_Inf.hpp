#pragma once

#include "Dcm.hpp"
#include "Dcm_Cfg_SchM.hpp"

#if(DCM_CFG_DSPUDSSUPPORT_ENABLED != DCM_CFG_OFF)
#include "DcmCore_DslDsd_Prot.hpp"
#include "DcmDspUds_Uds_Prot.hpp"

#if(DCM_CFG_DSP_DIAGNOSTICSESSIONCONTROL_ENABLED != DCM_CFG_OFF)
#include "DcmDspUds_Dsc_Prot.hpp"
#endif

#if(DCM_CFG_DET_SUPPORT_ENABLED != DCM_CFG_OFF)
#include "Det.hpp"
#endif
#endif

#include "DcmAppl.hpp"
