

#ifndef DCMDSPOBD_MODE9_PRIV_H
#define DCMDSPOBD_MODE9_PRIV_H

#if(DCM_CFG_DSP_OBDMODE9_ENABLED != DCM_CFG_OFF)

#define DCM_OBDMODE09_REQ_LEN_MIN   0x00u
#define DCM_OBDMODE09_REQ_LEN       0x01u
#define DCM_OBDMODE09_REQ_LEN_MAX   0x07u
#define DCM_OBDMODE09_SUPPINFOTYPE  0x20u

typedef enum{
    DCM_DSP_MODE09_UNINIT
   ,   DCM_DSP_MODE09_INIT
   ,   DCM_DSP_MODE09_RUNNING
}Dcm_DspMode09Type_ten;

#endif

#endif
