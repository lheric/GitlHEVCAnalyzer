/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2011, ITU/ISO/IEC
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

#include <vector>
#include <algorithm>
#include <ostream>

#include "NALread.h"
#include "TLibCommon/NAL.h"
#include "TLibCommon/TComBitStream.h"

using namespace std;

//! \ingroup TLibDecoder
//! \{

static void convertPayloadToRBSP(vector<uint8_t>& nalUnitBuf)
{
  unsigned zeroCount = 0;
  vector<uint8_t>::iterator it_read, it_write;
  for (it_read = it_write = nalUnitBuf.begin(); it_read != nalUnitBuf.end(); it_read++, it_write++)
  {
    if (zeroCount == 2 && *it_read == 0x03)
    {
      it_read++;
      zeroCount = 0;
    }
    zeroCount = (*it_read == 0x00) ? zeroCount+1 : 0;
    *it_write = *it_read;
  }

  nalUnitBuf.resize(it_write - nalUnitBuf.begin());
}

/**
 * create a NALunit structure with given header values and storage for
 * a bitstream
 */
void read(InputNALUnit& nalu, vector<uint8_t>& nalUnitBuf)
{
  /* perform anti-emulation prevention */
  convertPayloadToRBSP(nalUnitBuf);

  nalu.m_Bitstream = new TComInputBitstream(&nalUnitBuf);
  TComInputBitstream& bs = *nalu.m_Bitstream;

  bool forbidden_zero_bit = bs.read(1);
  assert(forbidden_zero_bit == 0);

  nalu.m_RefIDC = (NalRefIdc) bs.read(2);
  nalu.m_UnitType = (NalUnitType) bs.read(5);

  switch (nalu.m_UnitType)
  {
  case NAL_UNIT_CODED_SLICE:
  case NAL_UNIT_CODED_SLICE_IDR:
  case NAL_UNIT_CODED_SLICE_CDR:
    {
      nalu.m_TemporalID = bs.read(3);
      nalu.m_OutputFlag = bs.read(1);
      unsigned reserved_one_4bits = bs.read(4);
      assert(reserved_one_4bits == 1);
    }
    break;
  default:
    nalu.m_TemporalID = 0;
    nalu.m_OutputFlag = true;
    break;
  }
}
//! \}
