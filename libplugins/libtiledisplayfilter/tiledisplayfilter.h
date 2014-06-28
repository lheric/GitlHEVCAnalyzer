#ifndef TILEDISPLAYFILTER_H
#define TILEDISPLAYFILTER_H

#include "model/drawengine/abstractfilter.h"

#include <QObject>

class TileDisplayFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.sysu.gitl.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    explicit TileDisplayFilter(QObject *parent = 0);

    virtual bool drawTile(FilterContext *pcContext, QPainter *pcPainter, ComTile *pcTile, double dScale, QRect *pcScaledArea);


signals:

public slots:

};

#endif // TILEDISPLAYFILTER_H
