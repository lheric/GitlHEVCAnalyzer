#include "filterloader.h"
#include "../io/analyzermsgsender.h"
#include "exceptions/invaildfilterindexexception.h"
#include "model/modellocator.h"
#include "common/comrom.h"
#include <QDir>

#define PLUGIN_DIRECTORY "plugins" ///< plugin directory

FilterLoader::FilterLoader()
{
    this->m_strPluginDir = PLUGIN_DIRECTORY;
}

FilterLoader::~FilterLoader()
{
}

bool FilterLoader::reloadAllFilters()
{
    unloadAllFilters();

    QDir cPluginsDir(m_strPluginDir);
    QStringList cFilters;
    cFilters << "*.dll" << "*.so";
    /// try to load each file as plugin in the directory
    foreach(QString strPluginFileName, cPluginsDir.entryList(cFilters, QDir::Files))
    {
        qDebug() << QString("Trying to Load Plugin Filter %1 ...").arg(strPluginFileName);
        QPluginLoader* pLoader = new QPluginLoader(cPluginsDir.absoluteFilePath(strPluginFileName));
        AbstractFilter *pPlugin =  qobject_cast<AbstractFilter*>(pLoader->instance());
        /// success or fail
        if (pPlugin)
        {
            m_apcPluginLoaders.push_back(pLoader);
            m_apcFilters.push_back(pPlugin);
            qDebug() << QString("Plugin Filter %1 Loading Succeeded!").arg(strPluginFileName);

        }
        else
        {
            delete pLoader;
            qWarning() << QString("Plugin Filter %1 Loading Failed!").arg(strPluginFileName);
        }
    }

    /// sort
    readFilterOrderAndSort();

    /// init each filter
    for(int i = 0; i < m_apcFilters.size(); i++)
    {

        if( m_apcFilters[i]->init(&m_cFilterContext) == false )
            qWarning() << QString("Plugin Filter %1 Init Failed!").arg(m_apcFilters[i]->getName());
    }


    return true;
}


bool FilterLoader::unloadAllFilters()
{
    /// uninit each filter
    for(int i = 0; i < m_apcFilters.size(); i++)
    {
        m_apcFilters[i]->uninit(&m_cFilterContext);
    }


    /// traverse the list and free all
    while( !m_apcFilters.empty() )
    {
        AbstractFilter* pFilter = m_apcFilters.back();
        QString strFilterName = pFilter->getName();
        delete pFilter;
        m_apcFilters.pop_back();

    }

    while( !m_apcPluginLoaders.empty() )
    {
        QPluginLoader* pLoader = m_apcPluginLoaders.back();
        delete pLoader;
        m_apcPluginLoaders.pop_back();
    }

    return true;
}

bool FilterLoader::config(int iFilterIndex)
{

    if(iFilterIndex >= m_apcFilters.size())
        throw InvaildFilterIndexException();

    // prepare filter context
    ModelLocator* pModel = ModelLocator::getInstance();
    m_cFilterContext.pcBuffer = &pModel->getFrameBuffer();
    m_cFilterContext.pcDrawEngine = &pModel->getDrawEngine();
    m_cFilterContext.pcSequenceManager = &pModel->getSequenceManager();
    m_cFilterContext.pcFilterLoader = this;

    // config filter
    m_apcFilters[iFilterIndex]->config(&m_cFilterContext);
    return true;
}

bool FilterLoader::config(AbstractFilter* pcFilter)
{
    if(pcFilter == NULL)
        throw InvaildFilterIndexException();

    // prepare filter context
    ModelLocator* pModel = ModelLocator::getInstance();
    m_cFilterContext.pcBuffer = &pModel->getFrameBuffer();
    m_cFilterContext.pcDrawEngine = &pModel->getDrawEngine();
    m_cFilterContext.pcSequenceManager = &pModel->getSequenceManager();
    m_cFilterContext.pcFilterLoader = this;

    // config filter
    pcFilter->config(&m_cFilterContext);
    return true;
}



bool FilterLoader::drawPU  (QPainter* pcPainter, ComPU *pcPU,  double dScale, QRect* pcScaledArea)
{
    for(int i = 0; i < m_apcFilters.size(); i++)
    {
        AbstractFilter* pFilter = m_apcFilters[i];
        if( pFilter->getEnable() )
        {
            pFilter->drawPU(&m_cFilterContext, pcPainter, pcPU, dScale, pcScaledArea);
        }
    }
    return true;
}

bool FilterLoader::drawCU  (QPainter* pcPainter, ComCU *pcCU, double dScale, QRect* pcScaledArea)
{
    for(int i = 0; i < m_apcFilters.size(); i++)
    {
        AbstractFilter* pFilter = m_apcFilters[i];
        if( pFilter->getEnable() )
        {
            pFilter->drawCU(&m_cFilterContext, pcPainter, pcCU, dScale, pcScaledArea);
        }
    }


    return true;
}

bool FilterLoader::drawCTU  (QPainter* pcPainter, ComCU *pcCU, double dScale, QRect* pcScaledArea)

{
    for(int i = 0; i < m_apcFilters.size(); i++)
    {
        AbstractFilter* pFilter = m_apcFilters[i];
        if( pFilter->getEnable() )
        {
            pFilter->drawCTU(&m_cFilterContext, pcPainter, pcCU, dScale, pcScaledArea);
        }
    }
    return true;
}


bool FilterLoader::drawFrame(QPainter* pcPainter, ComFrame *pcFrame, double dScale, QRect* pcScaledArea)
{
    for(int i = 0; i < m_apcFilters.size(); i++)
    {
        AbstractFilter* pFilter = m_apcFilters[i];
        if( pFilter->getEnable() )
        {
            pFilter->drawFrame(&m_cFilterContext, pcPainter, pcFrame, dScale, pcScaledArea);
        }
    }
    return true;
}


AbstractFilter* FilterLoader::getFitlerByName(QString strFilterName)
{
    AbstractFilter* pFilter = NULL;
    for(int i = 0; i < m_apcFilters.size(); i++)
    {
        pFilter = m_apcFilters[i];
        if( pFilter->getName() == strFilterName )
        {
            break;
        }
    }
    return pFilter;
}
void FilterLoader::readFilterOrderAndSort()
{
    ///read filter order
    QVector<QString> cLastOrder;
    g_cAppSetting.beginGroup("Filters");
    int iSize = g_cAppSetting.beginReadArray("filter_order");
    for (int i = 0; i < iSize; ++i)
    {
        g_cAppSetting.setArrayIndex(i);
        QString& strFilterName = g_cAppSetting.value("filter_name").toString();
        cLastOrder.push_back(strFilterName);
    }
    g_cAppSetting.endArray();
    g_cAppSetting.endGroup();



    /// sorting
    for (int i = 0; i < cLastOrder.size(); ++i)
    {
        for(int j = 0; j < m_apcFilters.size(); j++)
        {
            if(m_apcFilters.at(j)->getName() == cLastOrder.at(i))
            {
                /// swap position i & j
                AbstractFilter* tempFilter = m_apcFilters.at(i);
                m_apcFilters.replace(i, m_apcFilters.at(j));
                m_apcFilters.replace(j, tempFilter);

                QPluginLoader* tempLoader = m_apcPluginLoaders.at(i);
                m_apcPluginLoaders.replace(i, m_apcPluginLoaders.at(j));
                m_apcPluginLoaders.replace(j, tempLoader);
            }
        }
    }

}

void FilterLoader::saveFilterOrder()
{
    ///save filter order
    g_cAppSetting.beginGroup("Filters");
    g_cAppSetting.beginWriteArray("filter_order");
    for (int i = 0; i < m_apcFilters.size(); ++i)
    {
        g_cAppSetting.setArrayIndex(i);
        g_cAppSetting.setValue("filter_name", m_apcFilters.at(i)->getName());
    }
    g_cAppSetting.endArray();
    g_cAppSetting.endGroup();
    g_cAppSetting.sync();
}
