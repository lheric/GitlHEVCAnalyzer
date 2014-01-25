#ifndef MVDISPLAYFILTER_H
#define MVDISPLAYFILTER_H

#include <QObject>
#include <QPen>
#include "model/drawengine/abstractfilter.h"
#include "views/filterconfigdialog.h"

/*!
 * \brief The MVDisplayFilterConfig class MV Display filter configs
 */
class MVDisplayFilterConfig
{
public:
    MVDisplayFilterConfig()
    {
        m_bShowMVOrigin = true;
        m_bShowZeroMV = true;
        m_cL0Color = QColor(Qt::blue);
        m_cL1Color = QColor(Qt::red);
        m_dOpaque = 0.7;
    }

    ADD_CLASS_FIELD(bool, bShowMVOrigin, getShowMVOrigin, setShowMVOrigin)
    ADD_CLASS_FIELD(bool, bShowZeroMV,   getShowZeroMV,   setShowZeroMV)
    ADD_CLASS_FIELD(QColor, cL0Color, getL0Color, setL0Color)
    ADD_CLASS_FIELD(QColor, cL1Color, getL1Color, setL1Color)
    ADD_CLASS_FIELD(double, dOpaque, getOpaque, setOpaque)
};


class MVDisplayFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.sysu.gitl.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    explicit MVDisplayFilter(QObject *parent = 0);

    virtual bool config(FilterContext *pcContext);

    virtual bool drawPU   (FilterContext* pcContext, QPainter* pcPainter,
                           ComPU* pcPU, double dScale,
                           QRect *pcScaledArea);


signals:
    
public slots:

    ADD_CLASS_FIELD_PRIVATE(MVDisplayFilterConfig, cConfig)     ///< filter configurations
    ADD_CLASS_FIELD_PRIVATE(FilterConfigDialog, cConfigDialog)  ///< config GUI

    ADD_CLASS_FIELD_PRIVATE(bool, bShowRefPOC)  ///< show reference POC or not

    ADD_CLASS_FIELD_PRIVATE(QPen, cPenL0)       ///< for drawing L0 MV
    ADD_CLASS_FIELD_PRIVATE(QPen, cPenL1)       ///< for drawing L1 MV
    ADD_CLASS_FIELD_PRIVATE(QPen, cPenText)     ///< for drawing text


    ADD_CLASS_FIELD_PRIVATE(QBrush, cCircleL0Fill)  ///< for filling L0 mv circles
    ADD_CLASS_FIELD_PRIVATE(QBrush, cCircleL1Fill)  ///< for filling L1 mv circles


};

#endif // MVDISPLAYFILTER_H
