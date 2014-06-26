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
    virtual bool config    (int iFilterIndex);
    virtual bool config    (AbstractFilter* pcFilter);
    virtual bool drawTU    (QPainter* pcPainter, ComTU *pcTU,       double dScale,  QRect* pcScaledArea);
    virtual bool drawPU    (QPainter* pcPainter, ComPU *pcPU,       double dScale,  QRect* pcScaledArea);
    virtual bool drawCU    (QPainter* pcPainter, ComCU *pcCU,       double dScale,  QRect* pcScaledArea);
    virtual bool drawCTU   (QPainter* pcPainter, ComCU *pcCU,       double dScale,  QRect* pcScaledArea);
    virtual bool drawTile (QPainter* pcPainter, ComFrame *pcFrame, double dScale,  QRect* pcScaledArea);
    virtual bool drawFrame (QPainter* pcPainter, ComFrame *pcFrame, double dScale,  QRect* pcScaledArea);
    virtual bool mousePress(QPainter* pcPainter, ComFrame *pcFrame, const QPointF* pcUnscaledPos, const QPointF* scaledPos, double dScale, Qt::MouseButton eMouseBtn);
    virtual bool keyPress  (QPainter* pcPainter, ComFrame *pcFrame, int iKeyPressed);

    /*!
     * \brief initAllFilter Init all filters
     * \return
     */
    virtual bool initAllFilters();

    /*!
     * \brief uninitAllFilter Uninit all filters
     * \return
     */
    virtual bool uninitAllFilters();

    /*!
     * \brief reinitAllFilters Re-init all filters
     * \return
     */
    virtual bool reinitAllFilters();

    /*!
     * \brief reloadAllFilters reload filters in the directory
     * \return
     */
    virtual bool reloadAllFilters();

    /*!
     * \brief unloadAllFilters unload all filters
     * \return
     */
    virtual bool unloadAllFilters();

    /*!
     * \brief getFitlerByName get filter by filter name, if there is multiple filters with the same name, the first one will be returned
     * \param strFilterName the filter name
     * \return NULL will be returned if no filter matches
     */
    AbstractFilter* getFilterByName(const QString& strFilterName);


    /*!
     * \brief moveUpFilter
     * \param strFilterName
     * \return
     */
    bool moveUpFilter(const QString& strFilterName);

    /*!
     * \brief moveDownFilter
     * \param strFilterName
     * \return
     */
    bool moveDownFilter(const QString& strFilterName);

    /*!
     * \brief saveFilterOrder Save filter orders to file
     */
    void saveFilterOrder();

    /*!
     * \brief getFilterNames Get all loaded filter names
     * \return
     */
    QStringList getFilterNames();

    /*!
     * \brief getFilterNames Get all loaded filter enable status.
     *                       The filter status orders are the same with getFilterNames
     * \return
     */
    QVector<bool> getEnableStatus();


protected:
    /*!
     * \brief xReadAndSortFilters sort filters according to loaded configurations (previous saved qsettings)
     */
    void xReadAndSortFilters();

    void xPrepareFilterContext();


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
