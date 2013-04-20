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
     * \param pcPixmap      input frame to draw on
     * \param bDrawPU       draw PU info. or not
     * \return output frame (the drawn & scaled frame)
     */
    QPixmap* drawFrame  ( ComSequence* pcSequence, int iPoc, QPixmap *pcPixmap );

protected:
    /*!
     * \brief xDrawCU
     * \param pcSequence
     * \param pcCU
     * \param pcPainter
     * \param iPoc
     * \param iAddr
     * \param iZOrder
     * \param iDepth
     * \param iCUX
     * \param iCUY
     * \param iCUSize
     * \return
     */

    bool xDrawCU     (ComSequence*    pcSequence,
                      ComCU *pcCU,
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




    /*!
     * Scale of the frame
     */
    ADD_CLASS_FIELD(double, dFrameScale, getFrameScale, setFrameScale)

    /*!
     * Scaled frame
     */
    ADD_CLASS_FIELD_NOSETTER(QPixmap, cDrawnPixmap, getDrawnPixmap)


    /*!
     * Filter Loader
     */
    ADD_CLASS_FIELD_NOSETTER(FilterLoader, cFilterLoader, getFilterLoader)



signals:

public slots:

};

#endif // DRAWENGINE_H
