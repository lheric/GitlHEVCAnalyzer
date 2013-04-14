#ifndef COMPU_H
#define COMPU_H
#include "comdef.h"
#include "commv.h"

enum PredMode
{
  MODE_SKIP,            ///< SKIP mode
  MODE_INTER,           ///< inter-prediction mode
  MODE_INTRA,           ///< intra-prediction mode
  MODE_NONE = 15
};

class ComPU
{
public:
    ComPU(){}    
    ComMV* getMV(int iIdx) { return m_pcMV+iIdx; }

private:
    ComMV m_pcMV[2];
	
    ADD_CLASS_FIELD( PredMode, ePredMode, getPredMode, setPredMode)
    ADD_CLASS_FIELD( int, mergeIndex, getMergeIndex, setMergeIndex )
    ADD_CLASS_FIELD( int, iInterDir, getInterDir, setInterDir)
    ADD_CLASS_FIELD( int, iIntraDirLuma, getIntraDirLuma, setIntraDirLuma)
    ADD_CLASS_FIELD( int, iIntraDirChroma, getIntraDirChroma, setIntraDirChroma)
};
#endif // COMPU_H
