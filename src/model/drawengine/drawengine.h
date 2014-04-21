#ifndef DRAWENGINE_H
#define DRAWENGINE_H

#include <QObject>
#include <QPixmap>
#include <QVector>
#include "gitlmodual.h"
#include "model/common/comsequence.h"
#include "filterloader.h"

class DrawEngine : public QObject
{
    Q_OBJECT
public:
    explicit DrawEngine();

    /*!
     * \brief draw one frame
     * \param pcSequence    current sequence
     * \param iPoc          POC of the frame to be draw
     * \param pcPixmap      input frame to draw on
     * \param bDrawPU       draw PU info. or not
     * \return output frame (the drawn & scaled frame)
     */
    QPixmap* drawFrame  ( ComSequence* pcSequence, int iPoc, QPixmap *pcPixmap );

    /*!
     * \brief mousePress
     * \param pcPainter
     * \param pcScaledPos
     * \param scaledPos
     * \param dScale
     * \param eMouseBtn
     */
    void mousePress( const QPointF* pcScaledPos, Qt::MouseButton eMouseBtn);

    /*!
     * \brief mousePress
     * \param pcPainter
     * \param pcScaledPos
     * \param scaledPos
     * \param dScale
     * \param eMouseBtn
     */
    void keyPress(int iKeyPressed);

protected:

    /*!
     * \brief xDrawPU draw PU recursively
     * \param pcPainter
     * \param pcCU
     * \return
     */
    bool xDrawPU( QPainter* pcPainter,  ComCU* pcCU );



    /*!
     * \brief xDrawCU draw CU recursively
     * \param pcPainter
     * \param pcCU
     * \return
     */
    bool xDrawCU( QPainter* pcPainter,  ComCU* pcCU );



    /*!
     * \brief xDrawTU draw TU recursively
     * \param pcPainter
     * \param pcTU
     * \return
     */
    bool xDrawTU( QPainter* pcPainter,  ComCU* pcCU );
    bool xDrawTUHelper( QPainter* pcPainter,  ComTU* pcTU );

    /*!
     * \brief xScaleRect scale the (CU/PU) rect area accodring to zooming in/out
     * \param rcUnscaled
     * \param rcScaled
     */
    void xScaleRect(QRect *rcUnscaled, QRect *rcScaled );
    /*!
     * Scale of the frame
     */
    ADD_CLASS_FIELD(double, dScale, getScale, setScale)

    /*!
     * Current display frame
     */
    ADD_CLASS_FIELD_NOSETTER(ComFrame*, pcCurFrame, getCurFrame)

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
