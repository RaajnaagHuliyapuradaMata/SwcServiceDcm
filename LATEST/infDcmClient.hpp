#pragma once
/******************************************************************************/
/* File   : infDcmClient.hpp                                                  */
/* Author : NAGARAJA HM (c) since 1982. All rights reserved.                  */
/******************************************************************************/

/******************************************************************************/
/* #INCLUDES                                                                  */
/******************************************************************************/
#include "CompilerCfg_Dcm.hpp"

/******************************************************************************/
/* #DEFINES                                                                   */
/******************************************************************************/

/******************************************************************************/
/* MACROS                                                                     */
/******************************************************************************/

/******************************************************************************/
/* TYPEDEFS                                                                   */
/******************************************************************************/
class infDcmClient{
/******************************************************************************/
/* CONSTS                                                                     */
/******************************************************************************/
   public:
      CONST(Std_TypeVersionInfo, DCM_CONST) VersionInfo = { //TBD: remove const
            STD_AR_RELEASE_VERSION_MAJOR
         ,  STD_AR_RELEASE_VERSION_MINOR
         ,  0x00
         ,  0x00
         ,  0x01
         ,  '0'
         ,  '1'
         ,  '0'
      };

   public:
/******************************************************************************/
/* FUNCTIONS                                                                  */
/******************************************************************************/
      FUNC(void, DCM_CODE) GetVersionInfo(
         CONSTP2VAR(Std_TypeVersionInfo, DCM_VAR, DCM_CONST) lptrVersionInfo
      );
};

/******************************************************************************/
/* PARAMS                                                                     */
/******************************************************************************/

/******************************************************************************/
/* OBJECTS                                                                    */
/******************************************************************************/

/******************************************************************************/
/* EOF                                                                        */
/******************************************************************************/

