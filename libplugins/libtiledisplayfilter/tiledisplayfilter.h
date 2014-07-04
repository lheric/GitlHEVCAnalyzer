#ifndef TILEDISPLAYFILTER_H
#define TILEDISPLAYFILTER_H

#include "model/drawengine/abstractfilter.h"
#include "views/filterconfigdialog.h"
#include <QObject>
#include <QColor>

class TileDisplayFilterConfig
{
public:
    TileDisplayFilterConfig()
    {
        m_dPenWidth = 5.0;
    }

    ADD_CLASS_FIELD(QColor, cPenColor, getPenColor, setPenColor)
    ADD_CLASS_FIELD(double, dPenWidth, getPenWidth, setPenWidth)
};


class TileDisplayFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.sysu.gitl.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    explicit TileDisplayFilter(QObject *parent = 0);

    virtual bool drawTile(FilterContext *pcContext, QPainter *pcPainter, ComTile *pcTile, double dScale, QRect *pcScaledArea);

    virtual bool config(FilterContext *pcContext);

signals:

public slots:

    ADD_CLASS_FIELD_PRIVATE(FilterConfigDialog, cConfigDialog)  ///< config GUI
    ADD_CLASS_FIELD_PRIVATE(TileDisplayFilterConfig, cConfig)   ///
};

#endif // TILEDISPLAYFILTER_H
