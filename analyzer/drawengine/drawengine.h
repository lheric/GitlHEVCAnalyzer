#ifndef DRAWENGINE_H
#define DRAWENGINE_H

#include <QObject>
#include <QPixmap>
#include <QVector>
#include "gitlmodual.h"
#include "../common/comsequence.h"
#include "filterloader.h"

class DrawEngine : public QObject
{
    Q_OBJECT
public:
    explicit DrawEngine();

    /*!
     * draw one frame
     * \param pcSequence    current sequence
     * \param iPoc          POC of the frame to be draw
     * \param pcPixmap      picture to draw on
     * \param bDrawPU       draw PU info. or not
     */
    bool drawFrame  ( ComSequence* pcSequence, int iPoc, QPixmap *pcPixmap );


    /*!
     * \brief drawCU
     * \param pcSequence
     * \param pcLCU
     * \param piMode
     * \param piPred
     * \param piInterDir
     * \param pcMVs
     * \param piMergeIndex
     * \param piIntra
     * \param pcPainter
     * \param iPoc
     * \param iAddr
     * \param iCUX
     * \param iCUY
     * \param iCUSize
     * \param iDepth
     * \param iZorder
     * \return
     */
    bool drawCU     (ComSequence*    pcSequence,
                      QVector<int>&   aiMode,
                      QVector<int>&   aiPred,
                      QVector<int>&   aiInterDir,
                      QVector<ComMV*>&pcMVs,
                      QVector<int>&   aiMergeIndex,
                      QVector<int>&   aiIntraDir,
                      QPainter*       pcPainter,
                      int             iPoc,
                      int             iAddr,
                      int             iZOrder,
                      int             iDepth,
                      int             iCUX,
                      int             iCUY,
                      int             iCUSize);

    void xGetPUOffsetAndSize(int        iLeafCUSize,
                             PartSize   ePartSize,
                             int        uiPUIdx,
                             int&       riXOffset,
                             int&       riYOffset,
                             int&       riWidth,
                             int&       riHeight );



    int xGetPUNum( PartSize ePartSize );


    /*!
     * Scale of the frame
     */
    ADD_CLASS_FIELD(double, iFrameScale, getFrameScale, setFrameScale)


    /*!
     * Filter Loader
     */
    ADD_CLASS_FIELD_NOSETTER(FilterLoader, cFilterLoader, getFilterLoader)



signals:

public slots:

};

#endif // DRAWENGINE_H
