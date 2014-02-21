#ifndef PREDDISPLAYFILTER_H
#define PREDDISPLAYFILTER_H
#include "model/drawengine/abstractfilter.h"
#include "views/filterconfigdialog.h"
#include "gitldef.h"
#include <QObject>
#include <QColor>


/*!
 * \brief The PredDisplayFilterConfig class Predition Mode Display Filter Config Options
 * For backward compatability, we also support drawing skip mode (which no longer exsist after HM-8.0)
 */
class PredDisplayFilterConfig
{
public:
    PredDisplayFilterConfig()
    {
        m_cSkipColor  = QColor(0,0,0);
        m_cInterColor = QColor(51,0,204);
        m_cIntraColor = QColor(204,255,51);
        m_dOpaque = 0.7;
        applyOpaque();
    }

    void applyOpaque()
    {
        m_cSkipColor.setAlphaF(m_dOpaque);
        m_cInterColor.setAlphaF(m_dOpaque);
        m_cIntraColor.setAlphaF(m_dOpaque);
    }

    ADD_CLASS_FIELD(QColor, cSkipColor,  getSkipColor,  setSkipColor)
    ADD_CLASS_FIELD(QColor, cInterColor, getInterColor, setInterColor)
    ADD_CLASS_FIELD(QColor, cIntraColor, getIntraColor, setIntraColor)
    ADD_CLASS_FIELD(double, dOpaque, getOpaque, setOpaque)
};

class PredDisplayFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.sysu.gitl.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    explicit PredDisplayFilter(QObject *parent = 0);

    virtual bool config   (FilterContext* pcContext);


    virtual bool drawPU   (FilterContext* pcContext, QPainter* pcPainter,
                           ComPU *pcPU, double dScale,  QRect* pcScaledArea);

    ADD_CLASS_FIELD_PRIVATE(PredDisplayFilterConfig, cConfig)     ///< filter configurations
    ADD_CLASS_FIELD_PRIVATE(FilterConfigDialog, cConfigDialog)    ///< config GUI
    
public slots:
    
};

#endif // PREDDISPLAYFILTER_H
