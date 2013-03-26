#ifndef COMLCU_H
#define COMLCU_H

#include "comdef.h"
#include "compu.h"
#include "commv.h"


class ComFrame;

enum PartSize
{
  SIZE_2Nx2N,           ///< symmetric motion partition,  2Nx2N
  SIZE_2NxN,            ///< symmetric motion partition,  2Nx N
  SIZE_Nx2N,            ///< symmetric motion partition,   Nx2N
  SIZE_NxN,             ///< symmetric motion partition,   Nx N

  SIZE_2NxnU,           ///< asymmetric motion partition, 2Nx( N/2) + 2Nx(3N/2)
  SIZE_2NxnD,           ///< asymmetric motion partition, 2Nx(3N/2) + 2Nx( N/2)
  SIZE_nLx2N,           ///< asymmetric motion partition, ( N/2)x2N + (3N/2)x2N
  SIZE_nRx2N,           ///< asymmetric motion partition, (3N/2)x2N + ( N/2)x2N

  SIZE_NONE = 15
};



class ComLCU
{
public:
    explicit ComLCU();
    ~ComLCU();

    /*! Regular infomations of LCU
      */
    ADD_CLASS_FIELD(int, iAddr, getAddr, setAddr)                               ///< Raster index of LCU
    ADD_CLASS_FIELD(int, iTotalPartNum, getTotalPartNum, setTotalPartNum)       ///< "Smallest" parts in LCU (same with HM)
    ADD_CLASS_FIELD(QVector<int>, aiCUPUMode, getCUPUMode, setCUPUMode)         ///< Optimal mode by recursive search

    ADD_CLASS_FIELD(QVector<int>, aiPredType, getPredType, setPredType)         ///< Prediction mode (0-Skip, 1-Inter, 2-intra)
    ADD_CLASS_FIELD(QVector<int>, aiMergeIndex, getMergeIndex, setMergeIndex)   ///< Merge Index (-1 = Invalid)
    ADD_CLASS_FIELD(QVector<ComMV*>, cMVs, getMVs, setMVs)                      ///< Motion vectors of the PUs in this LCU
    ADD_CLASS_FIELD(QVector<int>, aiInterDir, getInterDir, setInterDir)         ///< Inter prediction direction
    ADD_CLASS_FIELD(QVector<int>, aiIntraDir, getIntraDir, setIntraDir)         ///< Intra prediction direction (Luma Chroma)

//    ADD_CLASS_FIELD(double, dRecrCost, getRecrCost, setRecrCost)                ///< Cost of optimal mode

    /*! Obsolescent
     * Fast CU (splitting) mode decision info.
     * Two phases: Prediction and Refinment
     */
//    ADD_CLASS_FIELD(QVector<int>, puhPredMode, getPredMode, setPredMode)        ///< Predicted CU mode
//    ADD_CLASS_FIELD(double, dPredCost, getPredCost, setPredCost)                ///< Cost of prediction mode

//    ADD_CLASS_FIELD(QVector<LCUModeTry*>, cModeTry, getModeTry, setModeTry)     ///< Fast CU mode refinment search process

//    ADD_CLASS_FIELD(QVector<int>, puhFastMode, getFastMode, setFastMode)        ///< Result of fast algorithm
//    ADD_CLASS_FIELD(double, dFastCost, getFastCost, setFastCost)                ///< Cost of mode by fast algorithm


};

#endif // COMLCU_H
