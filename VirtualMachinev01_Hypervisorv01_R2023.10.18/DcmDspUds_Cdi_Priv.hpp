

#ifndef DCMDSPUDS_CDI_PRIV_H
#define DCMDSPUDS_CDI_PRIV_H

#if(DCM_CFG_DSP_CLEARDIAGNOSTICINFORMATION_ENABLED != DCM_CFG_OFF)
#define DCM_CDI_REQ_LEN              0x03u
#define DCM_POS_DTC_HIGHBYTE         0x00u
#define DCM_POS_DTC_MIDDLEBYTE       0x01u
#define DCM_POS_DTC_LOWBYTE          0x02u
#define DCM_POS_DTC_BIT16            16u
#define DCM_POS_DTC_BIT8             8u
#define DCM_DTC_GRP_MASK             0x00FFFFFFu
#endif
#endif

