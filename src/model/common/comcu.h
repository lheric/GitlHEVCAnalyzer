#ifndef COMCU_H
#define COMCU_H

#include "compu.h"
#include "commv.h"
#include "comtu.h"

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
    explicit ComCU(ComFrame* pcParent);
    ~ComCU();

    bool operator < (const ComCU& cOther) const
    {
        return (m_iAddr < cOther.m_iAddr);
    }
    /*!
     * Sub-CUs (four at most)
     */
    ADD_CLASS_FIELD(QVector<ComCU*>, apcSCUs, getSCUs, setSCUs)

    /*!
     * PUs & TUs in this CU (only for leaf-CUs)
     */
    ADD_CLASS_FIELD(PartSize, ePartSize, getPartSize, setPartSize)
    ADD_CLASS_FIELD(QVector<ComPU*>, apcPUs, getPUs, setPUs)
    ADD_CLASS_FIELD_NOSETTER(ComTU, cTURoot, getTURoot)


    /*!
     * Regular infomations of CU
     */
    ADD_CLASS_FIELD(ComFrame*, pcFrame, getFrame, setFrame)                     ///< parent frame that holds this CU
    ADD_CLASS_FIELD(int, iX, getX, setX)                                        ///< X Position in frame
    ADD_CLASS_FIELD(int, iY, getY, setY)                                        ///< Y Position in frame
    ADD_CLASS_FIELD(int, iSize, getSize, setSize)                               ///< CU Size
    ADD_CLASS_FIELD(int, iAddr, getAddr, setAddr)                               ///< Raster index of LCU (which the sub-CU belongs to)
    ADD_CLASS_FIELD(int, iZorder, getZorder, setZorder)                         ///< Z-Order of CU
    ADD_CLASS_FIELD(int, iDepth, getDepth, setDepth)                            ///< Depth of CU

    /*!
     * Bit info (only for LCU)
     */
    ADD_CLASS_FIELD(int, iTotalBits, getTotalBits, setTotalBits)                ///< Bits comsumed by this LCU

public:
    static int getPUNum( PartSize ePartSize );
    static void getPUOffsetAndSize( int        iLeafCUSize,
                                    PartSize   ePartSize,
                                    int        uiPUIdx,
                                    int&       riXOffset,
                                    int&       riYOffset,
                                    int&       riWidth,
                                    int&       riHeight );



};

#endif // COMCU_H
