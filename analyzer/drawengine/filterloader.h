#ifndef FILTERLOADER_H
#define FILTERLOADER_H

#include "gitldef.h"
#include "gitlmodual.h"
#include "abstractfilter.h"
#include <QPluginLoader>
#include <QRect>

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
    virtual bool reloadAllFilters     ();
    virtual bool unloadAllFilters   ();
    virtual bool config   (int iFilterIndex);
    virtual bool config   (AbstractFilter* pcFilter);
    virtual bool drawPU   (QPainter* pcPainter, ComPU *pcPU,       double dScale,  QRect* pcScaledArea);
    virtual bool drawCU   (QPainter* pcPainter, ComCU *pcCU,       double dScale,  QRect* pcScaledArea);
    virtual bool drawCTU  (QPainter* pcPainter, ComCU *pcCU,       double dScale,  QRect* pcScaledArea);
    virtual bool drawFrame(QPainter* pcPainter, ComFrame *pcFrame, double dScale,  QRect* pcScaledArea);


    /*!
     * \brief getFitlerByName get filter by filter name, if there is multiple filters with the same name, the first one will be returned
     * \param strFilterName the filter name
     * \return NULL will be returned if no filter matches
     */
    AbstractFilter* getFitlerByName(QString strFilterName);


    /*!
     * \brief xReadAndSortFilters sort filters according to loaded configurations (previous saved qsettings)
     */
    void readFilterOrderAndSort();

    /*!
     * \brief xSaveFilterOrder save filter orders to file
     */
    void saveFilterOrder();

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
