/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.  
 *
 * Copyright (c) 2010-2012, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     ContextTables.h
    \brief    Defines constants and tables for SBAC
    \todo     number of context models is not matched to actual use, should be fixed
*/

#ifndef __CONTEXTTABLES__
#define __CONTEXTTABLES__

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Constants
// ====================================================================================================================

#define MAX_NUM_CTX_MOD             512       ///< maximum number of supported contexts

#define NUM_SPLIT_FLAG_CTX            3       ///< number of context models for split flag
#define NUM_SKIP_FLAG_CTX             3       ///< number of context models for skip flag

#define NUM_MERGE_FLAG_EXT_CTX        1       ///< number of context models for merge flag of merge extended
#define NUM_MERGE_IDX_EXT_CTX         4       ///< number of context models for merge index of merge extended

#define NUM_ALF_CTRL_FLAG_CTX         1       ///< number of context models for ALF control flag
#define NUM_PART_SIZE_CTX             4       ///< number of context models for partition size
#define NUM_CU_X_POS_CTX              2       ///< number of context models for partition size (AMP)
#define NUM_CU_Y_POS_CTX              2       ///< number of context models for partition size (AMP)
#define NUM_PRED_MODE_CTX             1       ///< number of context models for prediction mode

#define NUM_ADI_CTX                   1       ///< number of context models for intra prediction

#define NUM_CHROMA_PRED_CTX           2       ///< number of context models for intra prediction (chroma)
#define NUM_INTER_DIR_CTX             4       ///< number of context models for inter prediction direction
#define NUM_MV_RES_CTX                2       ///< number of context models for motion vector difference

#define NUM_REF_NO_CTX                4       ///< number of context models for reference index
#define NUM_TRANS_SUBDIV_FLAG_CTX     10      ///< number of context models for transform subdivision flags
#define NUM_QT_CBF_CTX                5       ///< number of context models for QT CBF
#define NUM_QT_ROOT_CBF_CTX           1       ///< number of context models for QT ROOT CBF
#define NUM_DELTA_QP_CTX              3       ///< number of context models for dQP

#define NUM_SIG_CG_FLAG_CTX           2       ///< number of context models for MULTI_LEVEL_SIGNIFICANCE

#define NUM_SIG_FLAG_CTX              48      ///< number of context models for sig flag

#define NUM_SIG_FLAG_CTX_LUMA         27      ///< number of context models for luma sig flag
#define NUM_SIG_FLAG_CTX_CHROMA       21      ///< number of context models for chroma sig flag
#define NUM_CTX_LAST_FLAG_XY          18      ///< number of context models for last coefficient position

#define NUM_ONE_FLAG_CTX              32      ///< number of context models for greater than 1 flag
#define NUM_ONE_FLAG_CTX_LUMA         24      ///< number of context models for greater than 1 flag of luma
#define NUM_ONE_FLAG_CTX_CHROMA        8      ///< number of context models for greater than 1 flag of chroma
#define NUM_ABS_FLAG_CTX              24      ///< number of context models for greater than 2 flag
#define NUM_ABS_FLAG_CTX_LUMA         18      ///< number of context models for greater than 2 flag of luma
#define NUM_ABS_FLAG_CTX_CHROMA        6      ///< number of context models for greater than 2 flag of chroma

#define NUM_MVP_IDX_CTX               2       ///< number of context models for MVP index

#define NUM_ALF_FLAG_CTX              1       ///< number of context models for ALF flag
#define NUM_ALF_UVLC_CTX              2       ///< number of context models for ALF UVLC (filter length)
#define NUM_ALF_SVLC_CTX              3       ///< number of context models for ALF SVLC (filter coeff.)

#define NUM_SAO_FLAG_CTX              1       ///< number of context models for SAO flag
#define NUM_SAO_UVLC_CTX              2       ///< number of context models for SAO UVLC
#define NUM_SAO_SVLC_CTX              3       ///< number of context models for SAO SVLC

#define CNU                          119      ///< dummy initialization value for unused context models 'Context model Not Used'

// ====================================================================================================================
// Tables
// ====================================================================================================================

// initial probability for split flag
static const UChar
INIT_SPLIT_FLAG[3][NUM_SPLIT_FLAG_CTX] =
{
  {
     87,  74, 107,
    
  },
  {
     84, 103, 105,
    
  },
  {
     84, 103, 105,
    
  },
};

// initial probability for skip flag
static const UChar
INIT_SKIP_FLAG[3][NUM_SKIP_FLAG_CTX] =
{
  {
    CNU, CNU, CNU,
    
  },
  {
    165, 168, 154,
    
  },
  {
    165, 168, 154,
    
  },
};

// initial probability for skip flag
static const UChar
INIT_ALF_CTRL_FLAG[3][NUM_ALF_CTRL_FLAG_CTX] =
{
  {
    153,
    
  },
  {
     87,
    
  },
  {
    135,
    
  },
};

// initial probability for merge flag
static const UChar
INIT_MERGE_FLAG_EXT[3][NUM_MERGE_FLAG_EXT_CTX] =
{
  {
    CNU,
    
  },
  {
    72,
    
  },
  {
    119,
    
  },
};

static const UChar
INIT_MERGE_IDX_EXT[3][NUM_MERGE_IDX_EXT_CTX] =
{
  {
    CNU, CNU, CNU, CNU,
    
  },
  {
    100,  86, 102, 133,
    
  },
  {
    116,  87, 119, 103,
    
  },
};

// initial probability for PU size
static const UChar
INIT_PART_SIZE[3][NUM_PART_SIZE_CTX] =
{
  {
    167, CNU, CNU, CNU,
    
  },
  {
    119,  87, CNU, CNU,
    
  },
  {
    119,  87, CNU, CNU,
    
  },
};

// initial probability for AMP split position (X)
static const UChar
INIT_CU_X_POS[3][NUM_CU_X_POS_CTX] =
{
  {
    CNU, CNU,
    
  },
  {
    119, 103,
    
  },
  {
    119, 103,
    
  },
};

// initial probability for AMP split position (Y)
static const UChar
INIT_CU_Y_POS[3][NUM_CU_Y_POS_CTX] =
{
  {
    CNU, CNU,
    
  },
  {
    119, 119,
    
  },
  {
    119, 103,
    
  },
};

// initial probability for prediction mode
static const UChar
INIT_PRED_MODE[3][NUM_PRED_MODE_CTX] =
{
  {
    CNU,
    
  },
  {
    114,
    
  },
  {
    98,
    
  },
};

// initial probability for intra direction of luma
static const UChar
INIT_INTRA_PRED_MODE[3][NUM_ADI_CTX] =
{
  {
    167,
    
  },
  {
    119,
    
  },
  {
    150,
    
  },
};

// initial probability for intra direction of chroma
static const UChar
INIT_CHROMA_PRED_MODE[3][NUM_CHROMA_PRED_CTX] =
{
  {
    53, 103,
    
  },
  {
    85,  87,
    
  },
  {
    101,  87,
    
  },
};

// initial probability for temporal direction
static const UChar
INIT_INTER_DIR[3][NUM_INTER_DIR_CTX] =
{
  {
    CNU, CNU, CNU, CNU,
    
  },
  {
    CNU, CNU, CNU, CNU,
    
  },
  {
    41,  39,  38,  36,
    
  },
};

// initial probability for motion vector difference
static const UChar
INIT_MVD[3][NUM_MV_RES_CTX] =
{
  {
    CNU, CNU,
    
  },
  {
    120, 166,
    
  },
  {
    135, 166,
    
  },
};

// initial probability for reference frame index
static const UChar
INIT_REF_PIC[3][NUM_REF_NO_CTX] =
{
  {
    CNU, CNU, CNU, CNU,
    
  },
  {
    102, 118, 103, CNU,
    
  },
  {
    118, 118, 134, CNU,
    
  },
};

// initial probability for dQP
static const UChar
INIT_DQP[3][NUM_DELTA_QP_CTX] =
{
  {
    CNU, CNU, CNU, 
    
  },
  {
    CNU, CNU, CNU, 
    
  },
  {
    CNU, CNU, CNU, 
    
  },
};

static const UChar
INIT_QT_CBF[3][2*NUM_QT_CBF_CTX] =
{
  {
     73,  74, CNU, CNU, CNU,
     55,  86, 133, CNU, CNU,
    
  },
  {
    102,  89, CNU, CNU, CNU,
    114,  84, 117, CNU, CNU,
    
  },
  {
    102,  89, CNU, CNU, CNU,
    114,  68, 117, CNU, CNU,
    
  },
};

static const UChar
INIT_QT_ROOT_CBF[3][NUM_QT_ROOT_CBF_CTX] =
{
  {
    CNU,
    
  },
  {
    39,
    
  },
  {
    39,
    
  },
};

static const UChar
INIT_LAST[3][2*NUM_CTX_LAST_FLAG_XY] =
{
  {
    72,  72,  71,  72, 104,  89,  71,  88,  89,  59,  73,  86,  89, 106,  60,  59,  43,  55,
    54,  70,  53,  53,  87,  71,  69,  54,  88,  73,  72,  53, CNU, CNU, CNU, CNU, CNU, CNU,
    
  },
  {
    57,  72,  71,  72,  57,  72, 102,  88,  73,  73,  72, 102, 103,  73,  89,  73,  57,  87,
    54,  70,  54, 101,  71,  55,  70, 116, 103,  72,  72, 119, CNU, CNU, CNU, CNU, CNU, CNU,
    
  },
  {
    88,  72,  71,  72,  57,  72, 102,  88,  73,  73,  72, 118, 103,  73,  89,  73,  57,  87,
    54,  70,  69,  85,  71,  55,  70,  85, 103,  72,  72, 119, CNU, CNU, CNU, CNU, CNU, CNU,
    
  },
};

static const UChar
INIT_SIG_CG_FLAG[3][2 * NUM_SIG_CG_FLAG_CTX] = 
{
  {
    83, 122,
    98, 121,
    
  },
  {
    99, 120,
    67, 119,
    
  },
  {
    99, 120,
    67, 119,
    
  },
};

static const UChar
INIT_SIG_FLAG[3][NUM_SIG_FLAG_CTX] =
{
  {
    74,  73,  88,  72,  72,  55,  71,  54,  71,  88, 103,  71,  53,  87, 134,  86,  84,  70,  68,  89,  90,  84,  88,  74, 130, 118,  88,
    120,  87, 149,  70,  52, 118, 133, 116, 114, 129, 132, 162, 115,  51, 115,  66, 120,  74, 115,  87,  89,
  },
  {
    152, 119, 103, 118,  87,  70,  70,  53, 118, 134, 118, 101,  68,  85, 101, 116, 100,  68,  67, 136, 168, 147, 150, 120, 115, 118, 119,
    136, 102,  70,  53,  67, 117, 102, 117, 115, 114,  84, 115,  99, 100,  83, 114, 152, 168, 131, 150, 120,
  },
  {
    152, 119, 103, 118,  87,  70,  70,  53,  71, 103, 118, 101,  68,  85, 101, 116, 116,  68,  67, 152, 168, 147, 150, 120, 115, 118, 119,
    136, 102,  86,  84,  67, 117, 102, 117, 115,  99, 100, 115,  99, 100,  83, 114, 152, 152, 131, 150, 120,
  },
};

static const UChar
INIT_ONE_FLAG[3][NUM_ONE_FLAG_CTX] =
{
  {
    104,  68, 116,  86, 104, 132,  86,  87, 105, 134,  87, 103, 102,  66, 114,  68,  87,  84, 100, 101,  72,  69, 101,  86,
    104, 130, 147, 149, 104, 196, 100, 165,
  },
  {
    119, 179, 179, 164, 119,  85, 117, 149, 136, 103, 103, 103, 133,  98, 114, 115, 118,  99, 115, 116,  87, 100,  85, 117,
    135, 146, 147, 164, 119, 148, 116, 133,
  },
  {
    119, 179, 148, 164, 119,  85, 117, 149, 136,  87, 103, 103, 133,  98, 114, 115, 118,  99, 115, 100,  87,  84,  85,  85,
    135, 177, 147, 164, 119, 132, 148, 149,
  },
};


static const UChar
INIT_ABS_FLAG[3][NUM_ABS_FLAG_CTX] =
{
  {
    86, 103,  73, 102, 103,  73, 103,  88,  89, 115, 117, 103, 117, 118, 103, 102, 103,  72,
    101, 103, 104, 101, 167, 121,
  },
  {
    84, 102,  88, 117, 118, 104, 103, 119, 136,  83, 116, 118, 100, 117,  87,  85,  86, 103,
    84, 118, 120, 117, 150, 120,
  },
  {
    84, 102,  88, 117, 118, 104,  87, 119, 136,  83, 116, 118,  84, 117,  87,  69,  86,  87,
    84, 118, 120, 117, 150, 120,
  },
};

// initial probability for motion vector predictor index
static const UChar
INIT_MVP_IDX[3][NUM_MVP_IDX_CTX] =
{
  {
    CNU, CNU,
    
  },
  {
    134, CNU,
    
  },
  {
    134, CNU,
    
  },
};

// initial probability for ALF flag
static const UChar
INIT_ALF_FLAG[3][NUM_ALF_FLAG_CTX] =
{
  {
    118,
    
  },
  {
    102,
    
  },
  {
    102,
    
  },
};

// initial probability for ALF side information (unsigned)
static const UChar
INIT_ALF_UVLC[3][NUM_ALF_UVLC_CTX] =
{
  {
    120, 119,
    
  },
  {
    119, 119,
    
  },
  {
    119, 119,
    
  },
};

// initial probability for ALF side information (signed)
static const UChar
INIT_ALF_SVLC[3][NUM_ALF_SVLC_CTX] =
{
  {
    139, 119, 124,
    
  },
  {
     90, 119, 140,
    
  },
  {
     90, 119, 124,
    
  },
};

// initial probability for SAO flag
static const UChar
INIT_SAO_FLAG[3][NUM_SAO_FLAG_CTX] =
{
  {
    119,
    
  },
  {
    102,
    
  },
  {
    102,
    
  },
};

// initial probability for SAO side information (unsigned)
static const UChar
INIT_SAO_UVLC[3][NUM_SAO_UVLC_CTX] =
{
  {
     61, 104,
    
  },
  {
    168, 120,
    
  },
  {
    184, 120,
    
  },
};

// initial probability for SAO side information (signed)
static const UChar
INIT_SAO_SVLC[3][NUM_SAO_SVLC_CTX] =
{
  {
    171, 119, 199,
    
  },
  {
    169, 119, 151,
    
  },
  {
    169, 119, 151,
    
  },
};

static const UChar
INIT_TRANS_SUBDIV_FLAG[3][NUM_TRANS_SUBDIV_FLAG_CTX] =
{
  {
    CNU, 162, 148, 100, CNU, CNU, CNU, CNU, CNU, CNU,
    
  },
  {
    CNU,  71,  86,  55, CNU, CNU, CNU, CNU, CNU, CNU,
    
  },
  {
    CNU, 102,  86,  86, CNU, CNU, CNU, CNU, CNU, CNU,
    
  },
};

//! \}


#endif

