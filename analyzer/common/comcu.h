#ifndef COMCU_H
#define COMCU_H

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



class ComCU
{
public:
    explicit ComCU();
    ~ComCU();

    /*!
     * Sub-CUs (four at most)
     */
    ADD_CLASS_FIELD(QVector<ComCU*>, apcSCUs, getSCUs, setSCUs)

    /*!
     * PUs in this CU (only for leaf-CUs)
     */
    ADD_CLASS_FIELD(PartSize, ePartSize, getPartSize, setPartSize)
    ADD_CLASS_FIELD(QVector<ComPU*>, apcPUs, getPUs, setPUs)

    /*!
     * Regular infomations of CU
     */
    ADD_CLASS_FIELD(int, iAddr, getAddr, setAddr)                               ///< Raster index of LCU
    ADD_CLASS_FIELD(int, iTotalPartNum, getTotalPartNum, setTotalPartNum)       ///< "Smallest" parts in LCU (same with HM)

};

#endif // COMCU_H
