#ifndef FILTERLOADER_H
#define FILTERLOADER_H

#include "gitldef.h"
#include "gitlmodual.h"
#include "abstractfilter.h"
#include <QPluginLoader>

/*!
 * \brief The FilterLoader class
 *  This class serve as the plugin manager of the plugin filters. It
 */
class FilterLoader :public GitlModual
{
public:
    explicit FilterLoader();
    ~FilterLoader();

    /*!
     * Bacially it is a waper of filter interface \see AbstractFilter
     */
    virtual bool init     ();
    virtual bool uninit   ();
    virtual bool config   (int iFilterIndex);
    virtual bool drawPU   (QPainter* pcPainter, ComSequence* pcSequence, ComPU *pcPU, int iPoc, int iAddr, int iZOrder, int iDepth, PartSize ePartSize, int iPUIndex, int iPUX, int iPUY, int iPUWidth, int iPUHeight);
    virtual bool drawCU   (QPainter* pcPainter, ComSequence* pcSequence, int iPoc, int iAddr, int iZOrder, int iDepth, int iCUX, int iCUY, int iCUSize);
    virtual bool drawCTU  (QPainter* pcPainter, ComSequence* pcSequence, int iPoc, int iAddr, int iCTUX, int iCTUY, int iCTUSize);
    virtual bool drawFrame(QPainter* pcPainter, ComSequence* pcSequence, int iPoc);


    /*!
     * \brief getFitlerByName get filter by filter name, if there is multiple filters with the same name, the first one will be returned
     * \param strFilterName the filter name
     * \return NULL will be returned if no filter matches
     */
    AbstractFilter* getFitlerByName(QString strFilterName);

    /*!
     * Plugin Directory
     */
    ADD_CLASS_FIELD(QString, strPluginDir, getPluginDir, setPluginDir)


    /*! Plugin loader & instances
     */
    ADD_CLASS_FIELD_NOSETTER(QVector<QPluginLoader*>, apcPluginLoaders, getPluginLoaders)
    ADD_CLASS_FIELD_NOSETTER(QVector<AbstractFilter*>, apcFilters, getFilters)

    /*! Filter Context \see FilterContext
     */
    ADD_CLASS_FIELD_PRIVATE(FilterContext, cFilterContext)

};

#endif // FILTERLOADER_H
