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

/** \file     TAppEncCfg.cpp
    \brief    Handle encoder configuration parameters
*/

#include <stdlib.h>
#include <cassert>
#include <cstring>
#include <string>
#include "TLibCommon/TComRom.h"
#include "TAppEncCfg.h"
#include "TAppCommon/program_options_lite.h"

#ifdef WIN32
#define strdup _strdup
#endif

using namespace std;
namespace po = df::program_options_lite;

//! \ingroup TAppEncoder
//! \{

/* configuration helper funcs */
void doOldStyleCmdlineOn(po::Options& opts, const std::string& arg);
void doOldStyleCmdlineOff(po::Options& opts, const std::string& arg);

// ====================================================================================================================
// Local constants
// ====================================================================================================================

/// max value of source padding size
/** \todo replace it by command line option
 */
#define MAX_PAD_SIZE                16

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================

TAppEncCfg::TAppEncCfg()
{
  m_aidQP = NULL;
}

TAppEncCfg::~TAppEncCfg()
{
  if ( m_aidQP )
  {
    delete[] m_aidQP;
  }
}

Void TAppEncCfg::create()
{
}

Void TAppEncCfg::destroy()
{
}

std::istringstream &operator>>(std::istringstream &in, GOPEntry &entry)     //input
{
  in>>entry.m_iSliceType;
  in>>entry.m_iPOC;
  in>>entry.m_iQPOffset;
  in>>entry.m_iQPFactor;
  in>>entry.m_iTemporalId;
  in>>entry.m_iRefBufSize;
  in>>entry.m_bRefPic;
  in>>entry.m_iNumRefPics;
  for ( Int i = 0; i < entry.m_iNumRefPics; i++ )
  {
    in>>entry.m_aiReferencePics[i];
  }
  in>>entry.m_bInterRPSPrediction;
  if (entry.m_bInterRPSPrediction)
  {
    in>>entry.m_iDeltaRIdxMinus1;
    in>>entry.m_iDeltaRPS;
    in>>entry.m_iNumRefIdc;
    for ( Int i = 0; i < entry.m_iNumRefIdc; i++ )
    {
      in>>entry.m_aiRefIdc[i];
    }
  }
  return in;
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/** \param  argc        number of arguments
    \param  argv        array of arguments
    \retval             true when success
 */
Bool TAppEncCfg::parseCfg( Int argc, Char* argv[] )
{
  bool do_help = false;
  
  string cfg_InputFile;
  string cfg_BitstreamFile;
  string cfg_ReconFile;
  string cfg_dQPFile;
  string cfg_ColumnWidth;
  string cfg_RowHeight;
  string cfg_ScalingListFile;
  po::Options opts;
  opts.addOptions()
  ("help", do_help, false, "this help text")
  ("c", po::parseConfigFile, "configuration file name")
  
  /* File, I/O and source parameters */
  ("InputFile,i",     cfg_InputFile,     string(""), "original YUV input file name")
  ("BitstreamFile,b", cfg_BitstreamFile, string(""), "bitstream output file name")
  ("ReconFile,o",     cfg_ReconFile,     string(""), "reconstructed YUV output file name")
  ("LambdaModifier0,-LM0", m_adLambdaModifier[ 0 ], ( double )1.0, "Lambda modifier for temporal layer 0")
  ("LambdaModifier1,-LM1", m_adLambdaModifier[ 1 ], ( double )1.0, "Lambda modifier for temporal layer 1")
  ("LambdaModifier2,-LM2", m_adLambdaModifier[ 2 ], ( double )1.0, "Lambda modifier for temporal layer 2")
  ("LambdaModifier3,-LM3", m_adLambdaModifier[ 3 ], ( double )1.0, "Lambda modifier for temporal layer 3")
  ("SourceWidth,-wdt",      m_iSourceWidth,  0, "Source picture width")
  ("SourceHeight,-hgt",     m_iSourceHeight, 0, "Source picture height")
  ("InputBitDepth",         m_uiInputBitDepth, 8u, "bit-depth of input file")
  ("BitDepth",              m_uiInputBitDepth, 8u, "deprecated alias of InputBitDepth")
  ("OutputBitDepth",        m_uiOutputBitDepth, 0u, "bit-depth of output file")
  ("InternalBitDepth",      m_uiInternalBitDepth, 0u, "Internal bit-depth (BitDepth+BitIncrement)")
  ("HorizontalPadding,-pdx",m_aiPad[0],      0, "horizontal source padding size")
  ("VerticalPadding,-pdy",  m_aiPad[1],      0, "vertical source padding size")
  ("PAD",                   m_bUsePAD,   false, "automatic source padding of multiple of 16" )
  ("FrameRate,-fr",         m_iFrameRate,        0, "Frame rate")
  ("FrameSkip,-fs",         m_FrameSkip,         0u, "Number of frames to skip at start of input YUV")
  ("FramesToBeEncoded,f",   m_iFrameToBeEncoded, 0, "number of frames to be encoded (default=all)")
  ("FrameToBeEncoded",      m_iFrameToBeEncoded, 0, "depricated alias of FramesToBeEncoded")
  
  /* Unit definition parameters */
  ("MaxCUWidth",          m_uiMaxCUWidth,  64u)
  ("MaxCUHeight",         m_uiMaxCUHeight, 64u)
  /* todo: remove defaults from MaxCUSize */
  ("MaxCUSize,s",         m_uiMaxCUWidth,  64u, "max CU size")
  ("MaxCUSize,s",         m_uiMaxCUHeight, 64u, "max CU size")
  ("MaxPartitionDepth,h", m_uiMaxCUDepth,   4u, "CU depth")
  
  ("QuadtreeTULog2MaxSize", m_uiQuadtreeTULog2MaxSize, 6u)
  ("QuadtreeTULog2MinSize", m_uiQuadtreeTULog2MinSize, 2u)
  
  ("QuadtreeTUMaxDepthIntra", m_uiQuadtreeTUMaxDepthIntra, 1u)
  ("QuadtreeTUMaxDepthInter", m_uiQuadtreeTUMaxDepthInter, 2u)
  
  /* Coding structure paramters */
  ("IntraPeriod,-ip",m_iIntraPeriod, -1, "intra period in frames, (-1: only first frame)")
  ("DecodingRefreshType,-dr",m_iDecodingRefreshType, 0, "intra refresh, (0:none 1:CDR 2:IDR)")
  ("GOPSize,g",      m_iGOPSize,      1, "GOP size of temporal structure")
  ("MaxNumberOfReorderPictures",   m_numReorderFrames,               -1, "Max. number of reorder pictures: -1: encoder determines value, >=0: set explicitly")
  ("MaxNumberOfReferencePictures", m_uiMaxNumberOfReferencePictures, 6u, "Max. number of reference pictures")
  ("ListCombination,-lc", m_bUseLComb, true, "combined reference list flag for uni-prediction in B-slices")
  ("LCModification", m_bLCMod, false, "enables signalling of combined reference list derivation")
  ("DisableInter4x4", m_bDisInter4x4, true, "Disable Inter 4x4")
  ("NSQT", m_enableNSQT, true, "Enable non-square transforms")
  ("AMP", m_enableAMP, true, "Enable asymmetric motion partitions")
  /* motion options */
  ("FastSearch", m_iFastSearch, 1, "0:Full search  1:Diamond  2:PMVFAST")
  ("SearchRange,-sr",m_iSearchRange, 96, "motion search range")
  ("BipredSearchRange", m_bipredSearchRange, 4, "motion search range for bipred refinement")
  ("HadamardME", m_bUseHADME, true, "hadamard ME for fractional-pel")
  ("ASR", m_bUseASR, false, "adaptive motion search range")
  
  /* Quantization parameters */
  ("QP,q",          m_fQP,             30.0, "Qp value, if value is float, QP is switched once during encoding")
  ("DeltaQpRD,-dqr",m_uiDeltaQpRD,       0u, "max dQp offset for slice")
  ("MaxDeltaQP,d",  m_iMaxDeltaQP,        0, "max dQp offset for block")
  ("MaxCuDQPDepth,-dqd",  m_iMaxCuDQPDepth,        0, "max depth for a minimum CuDQP")

    ("ChromaQpOffset,   -cqo",   m_iChromaQpOffset,           0, "ChromaQpOffset")
    ("ChromaQpOffset2nd,-cqo2",  m_iChromaQpOffset2nd,        0, "ChromaQpOffset2nd")

#if ADAPTIVE_QP_SELECTION
    ("AdaptiveQpSelection,-aqps",   m_bUseAdaptQpSelect,           false, "AdaptiveQpSelection")
#endif

  ("AdaptiveQP,-aq", m_bUseAdaptiveQP, false, "QP adaptation based on a psycho-visual model")
  ("MaxQPAdaptationRange,-aqr", m_iQPAdaptationRange, 6, "QP adaptation range")
  ("dQPFile,m",     cfg_dQPFile, string(""), "dQP file name")
  ("RDOQ",          m_bUseRDOQ, true)
  ("TemporalLayerQPOffset_L0,-tq0", m_aiTLayerQPOffset[0], MAX_QP + 1, "QP offset of temporal layer 0")
  ("TemporalLayerQPOffset_L1,-tq1", m_aiTLayerQPOffset[1], MAX_QP + 1, "QP offset of temporal layer 1")
  ("TemporalLayerQPOffset_L2,-tq2", m_aiTLayerQPOffset[2], MAX_QP + 1, "QP offset of temporal layer 2")
  ("TemporalLayerQPOffset_L3,-tq3", m_aiTLayerQPOffset[3], MAX_QP + 1, "QP offset of temporal layer 3")
  
  ("TLayeringBasedOnCodingStruct,-tl", m_bTLayering, false, "Temporal ID is set based on the hierarchical coding structure")
  
  ("TLayerSwitchingFlag_L0,-ts0", m_abTLayerSwitchingFlag[0], false, "Switching flag for temporal layer 0")
  ("TLayerSwitchingFlag_L1,-ts1", m_abTLayerSwitchingFlag[1], false, "Switching flag for temporal layer 1")
  ("TLayerSwitchingFlag_L2,-ts2", m_abTLayerSwitchingFlag[2], false, "Switching flag for temporal layer 2")
  ("TLayerSwitchingFlag_L3,-ts3", m_abTLayerSwitchingFlag[3], false, "Switching flag for temporal layer 3")

  /* Entropy coding parameters */
  ("SBACRD", m_bUseSBACRD, true, "SBAC based RD estimation")
  
  /* Deblocking filter parameters */
  ("LoopFilterDisable", m_bLoopFilterDisable, false)
  ("LoopFilterOffsetInAPS", m_loopFilterOffsetInAPS, false)
  ("LoopFilterBetaOffset_div2", m_loopFilterBetaOffsetDiv2, 0 )
  ("LoopFilterTcOffset_div2", m_loopFilterTcOffsetDiv2, 0 )

  /* Coding tools */
  ("MRG", m_bUseMRG, true, "merging of motion partitions")

  ("LMChroma", m_bUseLMChroma, true, "intra chroma prediction based on recontructed luma")

  ("ALF", m_bUseALF, true, "Adaptive Loop Filter")
  ("SAO", m_bUseSAO, true, "SAO")   

  ("ALFEncodePassReduction", m_iALFEncodePassReduction, 0, "0:Original 16-pass, 1: 1-pass, 2: 2-pass encoding")

  ("ALFMaxNumFilter,-ALFMNF", m_iALFMaxNumberFilters, 16, "16: No Constrained, 1-15: Constrained max number of filter")

    ("SliceMode",            m_iSliceMode,           0, "0: Disable all Recon slice limits, 1: Enforce max # of LCUs, 2: Enforce max # of bytes")
    ("SliceArgument",        m_iSliceArgument,       0, "if SliceMode==1 SliceArgument represents max # of LCUs. if SliceMode==2 SliceArgument represents max # of bytes.")
    ("EntropySliceMode",     m_iEntropySliceMode,    0, "0: Disable all entropy slice limits, 1: Enforce max # of LCUs, 2: Enforce constraint based entropy slices")
    ("EntropySliceArgument", m_iEntropySliceArgument,0, "if EntropySliceMode==1 SliceArgument represents max # of LCUs. if EntropySliceMode==2 EntropySliceArgument represents max # of bins.")
    ("SliceGranularity",     m_iSliceGranularity,    0, "0: Slices always end at LCU borders. 1-3: slices may end at a depth of 1-3 below LCU level.")
    ("LFCrossSliceBoundaryFlag", m_bLFCrossSliceBoundaryFlag, true)

    ("ConstrainedIntraPred", m_bUseConstrainedIntraPred, false, "Constrained Intra Prediction")
    ("PCMEnabledFlag", m_usePCM         , false)
    ("PCMLog2MaxSize", m_pcmLog2MaxSize, 5u)
    ("PCMLog2MinSize", m_uiPCMLog2MinSize, 3u)

    ("PCMInputBitDepthFlag", m_bPCMInputBitDepthFlag, true)
    ("PCMFilterDisableFlag", m_bPCMFilterDisableFlag, false)
    ("weighted_pred_flag,-wpP",     m_bUseWeightPred, false, "weighted prediction flag (P-Slices)")
    ("weighted_bipred_idc,-wpBidc", m_uiBiPredIdc,    0u,    "weighted bipred idc (B-Slices)")
    ("TileInfoPresentFlag",         m_iColumnRowInfoPresent,         1,          "0: tiles parameters are NOT present in the PPS. 1: tiles parameters are present in the PPS")
    ("UniformSpacingIdc",           m_iUniformSpacingIdr,            0,          "Indicates if the column and row boundaries are distributed uniformly")
    ("TileBoundaryIndependenceIdc", m_iTileBoundaryIndependenceIdr,  1,          "Indicates if the column and row boundaries break the prediction")
    ("NumTileColumnsMinus1",        m_iNumColumnsMinus1,             0,          "Number of columns in a picture minus 1")
    ("ColumnWidthArray",            cfg_ColumnWidth,                 string(""), "Array containing ColumnWidth values in units of LCU")
    ("NumTileRowsMinus1",           m_iNumRowsMinus1,                0,          "Number of rows in a picture minus 1")
    ("RowHeightArray",              cfg_RowHeight,                   string(""), "Array containing RowHeight values in units of LCU")
    ("TileLocationInSliceHeaderFlag", m_iTileLocationInSliceHeaderFlag, 0,       "0: Disable transmission of tile location in slice header. 1: Transmit tile locations in slice header.")
    ("TileMarkerFlag",                m_iTileMarkerFlag,                0,       "0: Disable transmission of lightweight tile marker. 1: Transmit light weight tile marker.")
    ("MaxTileMarkerEntryPoints",    m_iMaxTileMarkerEntryPoints,    4,       "Maximum number of uniformly-spaced tile entry points (using light weigh tile markers). Default=4. If number of tiles < MaxTileMarkerEntryPoints then all tiles have entry points.")
    ("TileControlPresentFlag",       m_iTileBehaviorControlPresentFlag,         1,          "0: tiles behavior control parameters are NOT present in the PPS. 1: tiles behavior control parameters are present in the PPS")
    ("LFCrossTileBoundaryFlag",      m_bLFCrossTileBoundaryFlag,             true,          "1: cross-tile-boundary loop filtering. 0:non-cross-tile-boundary loop filtering")
    ("WaveFrontSynchro",            m_iWaveFrontSynchro,             0,          "0: no synchro; 1 synchro with TR; 2 TRR etc")
    ("WaveFrontFlush",              m_iWaveFrontFlush,               0,          "Flush and terminate CABAC coding for each LCU line")
    ("WaveFrontSubstreams",         m_iWaveFrontSubstreams,          1,          "# coded substreams wanted; per tile if TileBoundaryIndependenceIdc is 1, otherwise per frame")
    ("ScalingList",                 m_useScalingListId,              0,          "0: no scaling list, 1: default scaling lists, 2: scaling lists specified in ScalingListFile")
    ("ScalingListFile",             cfg_ScalingListFile,             string(""), "Scaling list file name")
  /* Misc. */
  ("SEIpictureDigest", m_pictureDigestEnabled, true, "Control generation of picture_digest SEI messages\n"
                                              "\t1: use MD5\n"
                                              "\t0: disable")

  ("TMVP", m_enableTMVP, true, "Enable TMVP" )

  ("FEN", m_bUseFastEnc, false, "fast encoder setting")
  ("ECU", m_bUseEarlyCU, false, "Early CU setting") 
  ("CFM", m_bUseCbfFastMode, false, "Cbf fast mode setting")
  /* Compatability with old style -1 FOO or -0 FOO options. */
  ("1", doOldStyleCmdlineOn, "turn option <name> on")
  ("0", doOldStyleCmdlineOff, "turn option <name> off")
  ;
  
  for(Int i=1; i<MAX_GOP+1; i++) {
    std::ostringstream cOSS;
    cOSS<<"Frame"<<i;
    opts.addOptions()(cOSS.str(), m_pcGOPList[i-1], GOPEntry());
  }
  po::setDefaults(opts);
  const list<const char*>& argv_unhandled = po::scanArgv(opts, argc, (const char**) argv);

  for (list<const char*>::const_iterator it = argv_unhandled.begin(); it != argv_unhandled.end(); it++)
  {
    fprintf(stderr, "Unhandled argument ignored: `%s'\n", *it);
  }
  
  if (argc == 1 || do_help)
  {
    /* argc == 1: no options have been specified */
    po::doHelp(cout, opts);
    xPrintUsage();
    return false;
  }
  
  /*
   * Set any derived parameters
   */
  /* convert std::string to c string for compatability */
  m_pchInputFile = cfg_InputFile.empty() ? NULL : strdup(cfg_InputFile.c_str());
  m_pchBitstreamFile = cfg_BitstreamFile.empty() ? NULL : strdup(cfg_BitstreamFile.c_str());
  m_pchReconFile = cfg_ReconFile.empty() ? NULL : strdup(cfg_ReconFile.c_str());
  m_pchdQPFile = cfg_dQPFile.empty() ? NULL : strdup(cfg_dQPFile.c_str());
  
  m_pchColumnWidth = cfg_ColumnWidth.empty() ? NULL: strdup(cfg_ColumnWidth.c_str());
  m_pchRowHeight = cfg_RowHeight.empty() ? NULL : strdup(cfg_RowHeight.c_str());
  m_scalingListFile = cfg_ScalingListFile.empty() ? NULL : strdup(cfg_ScalingListFile.c_str());
  
  // compute source padding size
  if ( m_bUsePAD )
  {
    if ( m_iSourceWidth%MAX_PAD_SIZE )
    {
      m_aiPad[0] = (m_iSourceWidth/MAX_PAD_SIZE+1)*MAX_PAD_SIZE - m_iSourceWidth;
    }
    
    if ( m_iSourceHeight%MAX_PAD_SIZE )
    {
      m_aiPad[1] = (m_iSourceHeight/MAX_PAD_SIZE+1)*MAX_PAD_SIZE - m_iSourceHeight;
    }
  }
  m_iSourceWidth  += m_aiPad[0];
  m_iSourceHeight += m_aiPad[1];
  
  // allocate slice-based dQP values
  m_aidQP = new Int[ m_iFrameToBeEncoded + m_iGOPSize + 1 ];
  ::memset( m_aidQP, 0, sizeof(Int)*( m_iFrameToBeEncoded + m_iGOPSize + 1 ) );
  
  // handling of floating-point QP values
  // if QP is not integer, sequence is split into two sections having QP and QP+1
  m_iQP = (Int)( m_fQP );
  if ( m_iQP < m_fQP )
  {
    Int iSwitchPOC = (Int)( m_iFrameToBeEncoded - (m_fQP - m_iQP)*m_iFrameToBeEncoded + 0.5 );
    
    iSwitchPOC = (Int)( (Double)iSwitchPOC / m_iGOPSize + 0.5 )*m_iGOPSize;
    for ( Int i=iSwitchPOC; i<m_iFrameToBeEncoded + m_iGOPSize + 1; i++ )
    {
      m_aidQP[i] = 1;
    }
  }
  
  // reading external dQP description from file
  if ( m_pchdQPFile )
  {
    FILE* fpt=fopen( m_pchdQPFile, "r" );
    if ( fpt )
    {
      Int iValue;
      Int iPOC = 0;
      while ( iPOC < m_iFrameToBeEncoded )
      {
        if ( fscanf(fpt, "%d", &iValue ) == EOF ) break;
        m_aidQP[ iPOC ] = iValue;
        iPOC++;
      }
      fclose(fpt);
    }
  }

  // check validity of input parameters
  xCheckParameter();
  
  // set global varibles
  xSetGlobal();
  
  // print-out parameters
  xPrintParameter();
  
  return true;
}

// ====================================================================================================================
// Private member functions
// ====================================================================================================================

Bool confirmPara(Bool bflag, const char* message);

Void TAppEncCfg::xCheckParameter()
{
  bool check_failed = false; /* abort if there is a fatal configuration problem */
#define xConfirmPara(a,b) check_failed |= confirmPara(a,b)
  // check range of parameters
  xConfirmPara( m_uiInputBitDepth < 8,                                                      "InputBitDepth must be at least 8" );
  xConfirmPara( m_iFrameRate <= 0,                                                          "Frame rate must be more than 1" );
  xConfirmPara( m_iFrameToBeEncoded <= 0,                                                   "Total Number Of Frames encoded must be more than 1" );
  xConfirmPara( m_iGOPSize < 1 ,                                                            "GOP Size must be more than 1" );
  xConfirmPara( m_iGOPSize > 1 &&  m_iGOPSize % 2,                                          "GOP Size must be a multiple of 2, if GOP Size is greater than 1" );
  xConfirmPara( (m_iIntraPeriod > 0 && m_iIntraPeriod < m_iGOPSize) || m_iIntraPeriod == 0, "Intra period must be more than GOP size, or -1 , not 0" );
  xConfirmPara( m_iDecodingRefreshType < 0 || m_iDecodingRefreshType > 2,                   "Decoding Refresh Type must be equal to 0, 1 or 2" );
  xConfirmPara( m_iQP < 0 || m_iQP > 51,                                                    "QP exceeds supported range (0 to 51)" );
  xConfirmPara( m_iALFEncodePassReduction < 0 || m_iALFEncodePassReduction > 2,             "ALFEncodePassReduction must be equal to 0, 1 or 2");

  xConfirmPara( m_iALFMaxNumberFilters < 1 || m_iALFMaxNumberFilters > 16,                  "ALFMaxNumFilter exceeds supported range (1 to 16)");  
  xConfirmPara( m_loopFilterBetaOffsetDiv2 < -13 || m_loopFilterBetaOffsetDiv2 > 13,          "Loop Filter Beta Offset div. 2 exceeds supported range (-13 to 13)");
  xConfirmPara( m_loopFilterTcOffsetDiv2 < -13 || m_loopFilterTcOffsetDiv2 > 13,              "Loop Filter Tc Offset div. 2 exceeds supported range (-13 to 13)");
  xConfirmPara( m_iFastSearch < 0 || m_iFastSearch > 2,                                     "Fast Search Mode is not supported value (0:Full search  1:Diamond  2:PMVFAST)" );
  xConfirmPara( m_iSearchRange < 0 ,                                                        "Search Range must be more than 0" );
  xConfirmPara( m_bipredSearchRange < 0 ,                                                   "Search Range must be more than 0" );
  xConfirmPara( m_iMaxDeltaQP > 7,                                                          "Absolute Delta QP exceeds supported range (0 to 7)" );
  xConfirmPara( m_iMaxCuDQPDepth > m_uiMaxCUDepth - 1,                                          "Absolute depth for a minimum CuDQP exceeds maximum coding unit depth" );

  xConfirmPara( m_iChromaQpOffset    < -12,   "Min. Chroma Qp Offset is -12"     );
  xConfirmPara( m_iChromaQpOffset    >  12,   "Max. Chroma Qp Offset is  12"     );
  xConfirmPara( m_iChromaQpOffset2nd < -12,   "Min. Chroma Qp Offset 2nd is -12" );
  xConfirmPara( m_iChromaQpOffset2nd >  12,   "Max. Chroma Qp Offset 2nd is  12" );

  xConfirmPara( m_iQPAdaptationRange <= 0,                                                  "QP Adaptation Range must be more than 0" );
  xConfirmPara( m_iFrameToBeEncoded != 1 && m_iFrameToBeEncoded < 2*m_iGOPSize,              "Total Number of Frames to be encoded must be at least 2 * GOP size for the current Reference Picture Set settings");
  if (m_iDecodingRefreshType == 2)
  {
    xConfirmPara( m_iIntraPeriod > 0 && m_iIntraPeriod <= m_iGOPSize ,                      "Intra period must be larger than GOP size for periodic IDR pictures");
  }
  xConfirmPara( (m_uiMaxCUWidth  >> m_uiMaxCUDepth) < 4,                                    "Minimum partition width size should be larger than or equal to 8");
  xConfirmPara( (m_uiMaxCUHeight >> m_uiMaxCUDepth) < 4,                                    "Minimum partition height size should be larger than or equal to 8");
  xConfirmPara( m_uiMaxCUWidth < 16,                                                        "Maximum partition width size should be larger than or equal to 16");
  xConfirmPara( m_uiMaxCUHeight < 16,                                                       "Maximum partition height size should be larger than or equal to 16");
  xConfirmPara( (m_iSourceWidth  % (m_uiMaxCUWidth  >> (m_uiMaxCUDepth-1)))!=0,             "Frame width should be multiple of minimum CU size");
  xConfirmPara( (m_iSourceHeight % (m_uiMaxCUHeight >> (m_uiMaxCUDepth-1)))!=0,             "Frame height should be multiple of minimum CU size");
  
  xConfirmPara( m_uiQuadtreeTULog2MinSize < 2,                                        "QuadtreeTULog2MinSize must be 2 or greater.");
  xConfirmPara( m_uiQuadtreeTULog2MinSize > 5,                                        "QuadtreeTULog2MinSize must be 5 or smaller.");
  xConfirmPara( m_uiQuadtreeTULog2MaxSize < 2,                                        "QuadtreeTULog2MaxSize must be 2 or greater.");
  xConfirmPara( m_uiQuadtreeTULog2MaxSize > 5,                                        "QuadtreeTULog2MaxSize must be 5 or smaller.");
  xConfirmPara( m_uiQuadtreeTULog2MaxSize < m_uiQuadtreeTULog2MinSize,                "QuadtreeTULog2MaxSize must be greater than or equal to m_uiQuadtreeTULog2MinSize.");
  xConfirmPara( (1<<m_uiQuadtreeTULog2MinSize)>(m_uiMaxCUWidth >>(m_uiMaxCUDepth-1)), "QuadtreeTULog2MinSize must not be greater than minimum CU size" ); // HS
  xConfirmPara( (1<<m_uiQuadtreeTULog2MinSize)>(m_uiMaxCUHeight>>(m_uiMaxCUDepth-1)), "QuadtreeTULog2MinSize must not be greater than minimum CU size" ); // HS
  xConfirmPara( ( 1 << m_uiQuadtreeTULog2MinSize ) > ( m_uiMaxCUWidth  >> m_uiMaxCUDepth ), "Minimum CU width must be greater than minimum transform size." );
  xConfirmPara( ( 1 << m_uiQuadtreeTULog2MinSize ) > ( m_uiMaxCUHeight >> m_uiMaxCUDepth ), "Minimum CU height must be greater than minimum transform size." );
  xConfirmPara( m_uiQuadtreeTUMaxDepthInter < 1,                                                         "QuadtreeTUMaxDepthInter must be greater than or equal to 1" );
  xConfirmPara( m_uiQuadtreeTUMaxDepthInter > m_uiQuadtreeTULog2MaxSize - m_uiQuadtreeTULog2MinSize + 1, "QuadtreeTUMaxDepthInter must be less than or equal to the difference between QuadtreeTULog2MaxSize and QuadtreeTULog2MinSize plus 1" );
  xConfirmPara( m_uiQuadtreeTUMaxDepthIntra < 1,                                                         "QuadtreeTUMaxDepthIntra must be greater than or equal to 1" );
  xConfirmPara( m_uiQuadtreeTUMaxDepthIntra > m_uiQuadtreeTULog2MaxSize - m_uiQuadtreeTULog2MinSize + 1, "QuadtreeTUMaxDepthIntra must be less than or equal to the difference between QuadtreeTULog2MaxSize and QuadtreeTULog2MinSize plus 1" );

  if( m_usePCM)
  {
    xConfirmPara(  m_uiPCMLog2MinSize < 3,                                      "PCMLog2MinSize must be 3 or greater.");
    xConfirmPara(  m_uiPCMLog2MinSize > 5,                                      "PCMLog2MinSize must be 5 or smaller.");
    xConfirmPara(  m_pcmLog2MaxSize > 5,                                        "PCMLog2MaxSize must be 5 or smaller.");
    xConfirmPara(  m_pcmLog2MaxSize < m_uiPCMLog2MinSize,                       "PCMLog2MaxSize must be equal to or greater than m_uiPCMLog2MinSize.");
  }

  xConfirmPara( m_iSliceMode < 0 || m_iSliceMode > 2, "SliceMode exceeds supported range (0 to 2)" );
  if (m_iSliceMode!=0)
  {
    xConfirmPara( m_iSliceArgument < 1 ,         "SliceArgument should be larger than or equal to 1" );
  }
  xConfirmPara( m_iEntropySliceMode < 0 || m_iEntropySliceMode > 2, "EntropySliceMode exceeds supported range (0 to 2)" );
  if (m_iEntropySliceMode!=0)
  {
    xConfirmPara( m_iEntropySliceArgument < 1 ,         "EntropySliceArgument should be larger than or equal to 1" );
  }
  xConfirmPara( m_iSliceGranularity >= m_uiMaxCUDepth, "SliceGranularity must be smaller than maximum cu depth");
  xConfirmPara( m_iSliceGranularity <0 || m_iSliceGranularity > 3, "SliceGranularity exceeds supported range (0 to 3)" );
  xConfirmPara( m_iSliceGranularity > m_iMaxCuDQPDepth, "SliceGranularity must be smaller smaller than or equal to maximum dqp depth" );
  
  // max CU width and height should be power of 2
  UInt ui = m_uiMaxCUWidth;
  while(ui)
  {
    ui >>= 1;
    if( (ui & 1) == 1)
      xConfirmPara( ui != 1 , "Width should be 2^n");
  }
  ui = m_uiMaxCUHeight;
  while(ui)
  {
    ui >>= 1;
    if( (ui & 1) == 1)
      xConfirmPara( ui != 1 , "Height should be 2^n");
  }
  
  Bool bVerified_GOP=false;
  Bool bError_GOP=false;
  Int iCheckGOP=1;
  Int iNumRefs = 1;
  Int aRefList[MAX_NUM_REF_PICS+1];
  aRefList[0]=0;
  Bool bIsOK[MAX_GOP];
  for(Int i=0; i<MAX_GOP; i++) {
    bIsOK[i]=false;
  }
  Int iNumOK=0;
  Int numReorderFramesRequired=0;
  m_uiMaxNumberOfReferencePictures=0;
  Int iLastDisp = -1;
  m_iExtraRPSs=0;
  while(!bVerified_GOP&&!bError_GOP) 
  {
    Int iCurGOP = (iCheckGOP-1)%m_iGOPSize;
    Int iCurPOC = ((iCheckGOP-1)/m_iGOPSize)*m_iGOPSize + m_pcGOPList[iCurGOP].m_iPOC;
    
    if(m_pcGOPList[iCurGOP].m_iPOC<0) {
      printf("\nError: found fewer Reference Picture Sets than GOPSize\n");
      bError_GOP=true;
    }
    else {
      Bool bBeforeI = false;
      for(Int i = 0; i< m_pcGOPList[iCurGOP].m_iNumRefPics; i++) 
      {
        Int iAbsPOC = iCurPOC+m_pcGOPList[iCurGOP].m_aiReferencePics[i];
        if(iAbsPOC < 0)
          bBeforeI=true;
        else {
          Bool bFound=false;
          for(Int j=0; j<iNumRefs; j++) {
            if(aRefList[j]==iAbsPOC) {
              bFound=true;
              for(Int k=0; k<m_iGOPSize; k++)
              {
                if(iAbsPOC%m_iGOPSize == m_pcGOPList[k].m_iPOC%m_iGOPSize)
                  m_pcGOPList[iCurGOP].m_aiUsedByCurrPic[i]=m_pcGOPList[k].m_iTemporalId<=m_pcGOPList[iCurGOP].m_iTemporalId;
                }
            }
          }
          if(!bFound)
          {
            printf("\nError: ref pic %d is not available for GOP frame %d\n",m_pcGOPList[iCurGOP].m_aiReferencePics[i],iCurGOP+1);
            bError_GOP=true;
          }
        }
      }
      if(!bBeforeI&&!bError_GOP)
      {
        //all ref frames were present
        if(!bIsOK[iCurGOP]) 
        {
          iNumOK++;
          bIsOK[iCurGOP]=true;
          if(iNumOK==m_iGOPSize)
            bVerified_GOP=true;
        }
      }
      else {
        
        m_pcGOPList[m_iGOPSize+m_iExtraRPSs]=m_pcGOPList[iCurGOP];
        Int iNewRefs=0;
        for(Int i = 0; i< m_pcGOPList[iCurGOP].m_iNumRefPics; i++) 
        {
          Int iAbsPOC = iCurPOC+m_pcGOPList[iCurGOP].m_aiReferencePics[i];
          if(iAbsPOC>=0)
          {
            m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_aiReferencePics[iNewRefs]=m_pcGOPList[iCurGOP].m_aiReferencePics[i];
            m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_aiUsedByCurrPic[iNewRefs]=m_pcGOPList[iCurGOP].m_aiUsedByCurrPic[i];
            iNewRefs++;
          }
        }
        Int iNumPrefRefs = m_pcGOPList[iCurGOP].m_iRefBufSize;
        
        for(Int iOffset = -1; iOffset>-iCheckGOP; iOffset--)
        {
          //step backwards in coding order and include pictures we might find useful. 
          Int iOffGOP = (iCheckGOP-1+iOffset)%m_iGOPSize;
          Int iOffPOC = ((iCheckGOP-1+iOffset)/m_iGOPSize)*m_iGOPSize + m_pcGOPList[iOffGOP].m_iPOC;
          if(iOffPOC>=0&&m_pcGOPList[iOffGOP].m_bRefPic&&m_pcGOPList[iOffGOP].m_iTemporalId<=m_pcGOPList[iCurGOP].m_iTemporalId) 
          {
            Bool bNewRef=false;
            for(Int i=0; i<iNumRefs; i++)
            {
              if(aRefList[i]==iOffPOC)
              {
                bNewRef=true;
              }
            }
            for(Int i=0; i<iNewRefs; i++) 
            {
              if(m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_aiReferencePics[i]==iOffPOC-iCurPOC)
              {
                bNewRef=false;
              }
            }
            if(bNewRef) 
            {
              Int iInsertPoint=iNewRefs;
              for(Int j=0; j<iNewRefs; j++)
              {
                if(m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_aiReferencePics[j]<iOffPOC-iCurPOC||m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_aiReferencePics[j]>0)
                {
                  iInsertPoint = j;
                  break;
                }
              }
              Int prev = iOffPOC-iCurPOC;
              Int prevUsed = m_pcGOPList[iOffGOP].m_iTemporalId<=m_pcGOPList[iCurGOP].m_iTemporalId;
              for(Int j=iInsertPoint; j<iNewRefs+1; j++)
              {
                Int newPrev = m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_aiReferencePics[j];
                Int newUsed = m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_aiUsedByCurrPic[j];
                m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_aiReferencePics[j]=prev;
                m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_aiUsedByCurrPic[j]=prevUsed;
                prevUsed=newUsed;
                prev=newPrev;
              }
              //m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_aiReferencePics[iNewRefs]=iOffPOC-iCurPOC;
              iNewRefs++;
            }
          }
          if(iNewRefs>=iNumPrefRefs)
            break;
        }
        m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_iNumRefPics=iNewRefs;
        m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_iPOC = iCurPOC;
        if (m_iExtraRPSs == 0)
        {
          m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_bInterRPSPrediction = 0;
          m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_iNumRefIdc = 0;
        }
        else
        {
          Int rIdx =  m_iGOPSize + m_iExtraRPSs - 1;
          Int iRefPOC = m_pcGOPList[rIdx].m_iPOC;
          Int iRefPics = m_pcGOPList[rIdx].m_iNumRefPics;
          Int iNewIdc=0;
          for(Int i = 0; i<= iRefPics; i++) 
          {
            Int deltaPOC = ((i != iRefPics)? m_pcGOPList[rIdx].m_aiReferencePics[i] : 0);  // check if the reference abs POC is >= 0
            Int iAbsPOCref = iRefPOC+deltaPOC;
            Int iRefIdc = 0;
            for (Int j = 0; j < m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_iNumRefPics; j++)
            {
              if ( (iAbsPOCref - iCurPOC) == m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_aiReferencePics[j])
              {
                if (m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_aiUsedByCurrPic[j])
                {
                  iRefIdc = 1;
                }
                else
                {
                  iRefIdc = 2;
                }
              }
            }
            m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_aiRefIdc[iNewIdc]=iRefIdc;
            iNewIdc++;
          }
          m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_bInterRPSPrediction = 1;  
          m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_iNumRefIdc = iNewIdc;
          m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_iDeltaRPS = iRefPOC - m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_iPOC; 
          m_pcGOPList[m_iGOPSize+m_iExtraRPSs].m_iDeltaRIdxMinus1 = 0; 
        }
        iCurGOP=m_iGOPSize+m_iExtraRPSs;
        m_iExtraRPSs++;
      }
      iNumRefs=0;
      for(Int i = 0; i< m_pcGOPList[iCurGOP].m_iNumRefPics; i++) 
      {
        Int iAbsPOC = iCurPOC+m_pcGOPList[iCurGOP].m_aiReferencePics[i];
        if(iAbsPOC >= 0) {
          aRefList[iNumRefs]=iAbsPOC;
          iNumRefs++;
        }
      }
      if(m_uiMaxNumberOfReferencePictures<iNumRefs)
        m_uiMaxNumberOfReferencePictures=iNumRefs;
      aRefList[iNumRefs]=iCurPOC;
      iNumRefs++;
      Int iNonDisplayed=0;
      for(Int i=0; i<iNumRefs; i++) {
        if(aRefList[i]==iLastDisp+1) {
          iLastDisp=aRefList[i];
          i=0;
        }
      }
      for(Int i=0; i<iNumRefs; i++) {
        if(aRefList[i]>iLastDisp)
          iNonDisplayed++;
      }
      if(iNonDisplayed>numReorderFramesRequired)
      {
        numReorderFramesRequired=iNonDisplayed;
      }
    }
    iCheckGOP++;
  }
  if (m_numReorderFrames == -1)
  {
    m_numReorderFrames = numReorderFramesRequired;
  }
  xConfirmPara(bError_GOP,"Invalid GOP structure given");
  for(Int i=0; i<m_iGOPSize; i++) 
  {
    xConfirmPara(m_pcGOPList[i].m_iSliceType!='B'&&m_pcGOPList[i].m_iSliceType!='P', "Slice type must be equal to B or P");
  }
  xConfirmPara( m_bUseLComb==false && m_numReorderFrames!=0, "ListCombination can only be 0 in low delay coding (more precisely when L0 and L1 are identical)" );  // Note however this is not the full necessary condition as ref_pic_list_combination_flag can only be 0 if L0 == L1.
  xConfirmPara( m_numReorderFrames < numReorderFramesRequired, "For the used GOP the encoder requires more pictures for reordering than specified in MaxNumberOfReorderPictures" );

  xConfirmPara( m_iWaveFrontSynchro < 0, "WaveFrontSynchro cannot be negative" );
  xConfirmPara( m_iWaveFrontFlush < 0, "WaveFrontFlush cannot be negative" );
  xConfirmPara( m_iWaveFrontSubstreams <= 0, "WaveFrontSubstreams must be positive" );
  xConfirmPara( m_iWaveFrontSubstreams > 1 && !m_iWaveFrontSynchro, "Must have WaveFrontSynchro > 0 in order to have WaveFrontSubstreams > 1" );

#undef xConfirmPara
  if (check_failed)
  {
    exit(EXIT_FAILURE);
  }
}

/** \todo use of global variables should be removed later
 */
Void TAppEncCfg::xSetGlobal()
{
  // set max CU width & height
  g_uiMaxCUWidth  = m_uiMaxCUWidth;
  g_uiMaxCUHeight = m_uiMaxCUHeight;
  
  // compute actual CU depth with respect to config depth and max transform size
  g_uiAddCUDepth  = 0;
  while( (m_uiMaxCUWidth>>m_uiMaxCUDepth) > ( 1 << ( m_uiQuadtreeTULog2MinSize + g_uiAddCUDepth )  ) ) g_uiAddCUDepth++;
  
  m_uiMaxCUDepth += g_uiAddCUDepth;
  g_uiAddCUDepth++;
  g_uiMaxCUDepth = m_uiMaxCUDepth;
  
  // set internal bit-depth and constants
#if FULL_NBIT
  g_uiBitDepth = m_uiInternalBitDepth;
  g_uiBitIncrement = 0;
#else
  g_uiBitDepth = 8;
  g_uiBitIncrement = m_uiInternalBitDepth - g_uiBitDepth;
#endif

  g_uiBASE_MAX     = ((1<<(g_uiBitDepth))-1);
  
#if IBDI_NOCLIP_RANGE
  g_uiIBDI_MAX     = g_uiBASE_MAX << g_uiBitIncrement;
#else
  g_uiIBDI_MAX     = ((1<<(g_uiBitDepth+g_uiBitIncrement))-1);
#endif
  
  if (m_uiOutputBitDepth == 0)
  {
    m_uiOutputBitDepth = m_uiInternalBitDepth;
  }

  g_uiPCMBitDepthLuma = m_uiPCMBitDepthLuma = ((m_bPCMInputBitDepthFlag)? m_uiInputBitDepth : m_uiInternalBitDepth);
  g_uiPCMBitDepthChroma = ((m_bPCMInputBitDepthFlag)? m_uiInputBitDepth : m_uiInternalBitDepth);
}

Void TAppEncCfg::xPrintParameter()
{
  printf("\n");
  printf("Input          File          : %s\n", m_pchInputFile          );
  printf("Bitstream      File          : %s\n", m_pchBitstreamFile      );
  printf("Reconstruction File          : %s\n", m_pchReconFile          );
  printf("Real     Format              : %dx%d %dHz\n", m_iSourceWidth - m_aiPad[0], m_iSourceHeight-m_aiPad[1], m_iFrameRate );
  printf("Internal Format              : %dx%d %dHz\n", m_iSourceWidth, m_iSourceHeight, m_iFrameRate );
  printf("Frame index                  : %u - %d (%d frames)\n", m_FrameSkip, m_FrameSkip+m_iFrameToBeEncoded-1, m_iFrameToBeEncoded );
  printf("CU size / depth              : %d / %d\n", m_uiMaxCUWidth, m_uiMaxCUDepth );
  printf("RQT trans. size (min / max)  : %d / %d\n", 1 << m_uiQuadtreeTULog2MinSize, 1 << m_uiQuadtreeTULog2MaxSize );
  printf("Max RQT depth inter          : %d\n", m_uiQuadtreeTUMaxDepthInter);
  printf("Max RQT depth intra          : %d\n", m_uiQuadtreeTUMaxDepthIntra);
  printf("Min PCM size                 : %d\n", 1 << m_uiPCMLog2MinSize);
  printf("Motion search range          : %d\n", m_iSearchRange );
  printf("Intra period                 : %d\n", m_iIntraPeriod );
  printf("Decoding refresh type        : %d\n", m_iDecodingRefreshType );
  printf("QP                           : %5.2f\n", m_fQP );
  printf("Max dQP signaling depth      : %d\n", m_iMaxCuDQPDepth);

  printf("Chroma Qp Offset             : %d\n", m_iChromaQpOffset   );
  printf("Chroma Qp Offset 2nd         : %d\n", m_iChromaQpOffset2nd);

  printf("QP adaptation                : %d (range=%d)\n", m_bUseAdaptiveQP, (m_bUseAdaptiveQP ? m_iQPAdaptationRange : 0) );
  printf("GOP size                     : %d\n", m_iGOPSize );
  printf("Internal bit depth           : %d\n", m_uiInternalBitDepth );
  printf("PCM sample bit depth         : %d\n", m_uiPCMBitDepthLuma );
  if((m_uiMaxCUWidth >> m_uiMaxCUDepth) == 4)
  {
    printf("DisableInter4x4              : %d\n", m_bDisInter4x4);  
  }
  printf("\n");
  
  printf("TOOL CFG: ");
  printf("ALF:%d ", m_bUseALF             );
  printf("IBD:%d ", !!g_uiBitIncrement);
  printf("HAD:%d ", m_bUseHADME           );
  printf("SRD:%d ", m_bUseSBACRD          );
  printf("RDQ:%d ", m_bUseRDOQ            );
  printf("SQP:%d ", m_uiDeltaQpRD         );
  printf("ASR:%d ", m_bUseASR             );
  printf("PAD:%d ", m_bUsePAD             );
  printf("LComb:%d ", m_bUseLComb         );
  printf("LCMod:%d ", m_bLCMod         );
  printf("FEN:%d ", m_bUseFastEnc         );
  printf("ECU:%d ", m_bUseEarlyCU         );
  printf("CFM:%d ", m_bUseCbfFastMode         );
  printf("RQT:%d ", 1     );
  printf("MRG:%d ", m_bUseMRG             ); // SOPH: Merge Mode
  printf("LMC:%d ", m_bUseLMChroma        ); 
  printf("Slice: G=%d M=%d ", m_iSliceGranularity, m_iSliceMode);
  if (m_iSliceMode!=0)
  {
    printf("A=%d ", m_iSliceArgument);
  }
  printf("EntropySlice: M=%d ",m_iEntropySliceMode);
  if (m_iEntropySliceMode!=0)
  {
    printf("A=%d ", m_iEntropySliceArgument);
  }
  printf("CIP:%d ", m_bUseConstrainedIntraPred);
  printf("SAO:%d ", (m_bUseSAO)?(1):(0));
  printf("PCM:%d ", ((1<<m_uiPCMLog2MinSize) <= m_uiMaxCUWidth)? 1 : 0);
  printf("WPP:%d ", (Int)m_bUseWeightPred);
  printf("WPB:%d ", m_uiBiPredIdc);
  printf("TileBoundaryIndependence:%d ", m_iTileBoundaryIndependenceIdr ); 
  printf("TileLocationInSliceHdr:%d ", m_iTileLocationInSliceHeaderFlag);
  printf("TileMarker:%d", m_iTileMarkerFlag);
  if (m_iTileMarkerFlag)
  {
    printf("[%d] ", m_iMaxTileMarkerEntryPoints);
  }
  else
  {
    printf(" ");
  }
  printf(" WaveFrontSynchro:%d WaveFrontFlush:%d WaveFrontSubstreams:%d",
          m_iWaveFrontSynchro, m_iWaveFrontFlush, m_iWaveFrontSubstreams);
  printf(" ScalingList:%d ", m_useScalingListId );

  printf("TMVP:%d ", m_enableTMVP     );

#if ADAPTIVE_QP_SELECTION
  printf("AQpS:%d", m_bUseAdaptQpSelect   );
#endif

  printf("\n\n");
  
  fflush(stdout);
}

Void TAppEncCfg::xPrintUsage()
{
  printf( "          <name> = ALF - adaptive loop filter\n");
  printf( "                   IBD - bit-depth increasement\n");
  printf( "                   GPB - generalized B instead of P in low-delay mode\n");
  printf( "                   HAD - hadamard ME for fractional-pel\n");
  printf( "                   SRD - SBAC based RD estimation\n");
  printf( "                   RDQ - RDOQ\n");
  printf( "                   LDC - low-delay mode\n");
  printf( "                   NRF - non-reference frame marking in last layer\n");
  printf( "                   BQP - hier-P style QP assignment in low-delay mode\n");
  printf( "                   PAD - automatic source padding of multiple of 16\n");
  printf( "                   ASR - adaptive motion search range\n");
  printf( "                   FEN - fast encoder setting\n");  
  printf( "                   ECU - Early CU setting\n");
  printf( "                   CFM - Cbf fast mode setting\n");
  printf( "                   MRG - merging of motion partitions\n"); // SOPH: Merge Mode

  printf( "                   LMC - intra chroma prediction based on luma\n");

  printf( "\n" );
  printf( "  Example 1) TAppEncoder.exe -c test.cfg -q 32 -g 8 -f 9 -s 64 -h 4\n");
  printf("              -> QP 32, hierarchical-B GOP 8, 9 frames, 64x64-8x8 CU (~4x4 PU)\n\n");
  printf( "  Example 2) TAppEncoder.exe -c test.cfg -q 32 -g 4 -f 9 -s 64 -h 4 -1 LDC\n");
  printf("              -> QP 32, hierarchical-P GOP 4, 9 frames, 64x64-8x8 CU (~4x4 PU)\n\n");
}

Bool confirmPara(Bool bflag, const char* message)
{
  if (!bflag)
    return false;
  
  printf("Error: %s\n",message);
  return true;
}

/* helper function */
/* for handling "-1/-0 FOO" */
void translateOldStyleCmdline(const char* value, po::Options& opts, const std::string& arg)
{
  const char* argv[] = {arg.c_str(), value};
  /* replace some short names with their long name varients */
  if (arg == "LDC")
  {
    argv[0] = "LowDelayCoding";
  }
  else if (arg == "RDQ")
  {
    argv[0] = "RDOQ";
  }
  else if (arg == "HAD")
  {
    argv[0] = "HadamardME";
  }
  else if (arg == "SRD")
  {
    argv[0] = "SBACRD";
  }
  else if (arg == "IBD")
  {
    argv[0] = "BitIncrement";
  }
  /* issue a warning for change in FEN behaviour */
  if (arg == "FEN")
  {
    /* xxx todo */
  }
  po::storePair(opts, argv[0], argv[1]);
}

void doOldStyleCmdlineOn(po::Options& opts, const std::string& arg)
{
  if (arg == "IBD")
  {
    translateOldStyleCmdline("4", opts, arg);
    return;
  }
  translateOldStyleCmdline("1", opts, arg);
}

void doOldStyleCmdlineOff(po::Options& opts, const std::string& arg)
{
  translateOldStyleCmdline("0", opts, arg);
}

//! \}
