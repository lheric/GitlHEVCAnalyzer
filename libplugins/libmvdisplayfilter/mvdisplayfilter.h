#ifndef MVDISPLAYFILTER_H
#define MVDISPLAYFILTER_H

#include <QObject>
#include <QPen>
#include "model/drawengine/abstractfilter.h"

class MVDisplayFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.gitl.sysu.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    explicit MVDisplayFilter(QObject *parent = 0);

    virtual bool config(FilterContext *pcContext);

    virtual bool drawPU   (FilterContext* pcContext, QPainter* pcPainter,
                           ComPU* pcPU, double dScale,
                           QRect *pcScaledArea);


signals:
    
public slots:


    ADD_CLASS_FIELD_PRIVATE(bool, bShowRefPOC)  ///< show reference POC or not

    ADD_CLASS_FIELD_PRIVATE(QPen, cPenL0)       ///< for drawing L0 MV
    ADD_CLASS_FIELD_PRIVATE(QPen, cPenL1)       ///< for drawing L1 MV
    ADD_CLASS_FIELD_PRIVATE(QPen, cPenText)     ///< for drawing text


    ADD_CLASS_FIELD_PRIVATE(QBrush, cCircleL0Fill)  ///< for filling L0 mv circles
    ADD_CLASS_FIELD_PRIVATE(QBrush, cCircleL1Fill)  ///< for filling L1 mv circles
};

#endif // MVDISPLAYFILTER_H
