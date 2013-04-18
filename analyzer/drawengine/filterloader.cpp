#include "comdef.h"
#include "filterloader.h"
#include "../io/analyzermsgsender.h"
#include "exceptions/invaildfilterindexexception.h"
#include "model/modellocator.h"
#include "events/eventnames.h"

#include <QDir>

FilterLoader::FilterLoader()
{
    this->m_strPluginDir = PLUGIN_DIRECTORY;

    /// load filters
    init();
}

FilterLoader::~FilterLoader()
{
    /// unload filters
    uninit();
}

bool FilterLoader::init()
{

    QDir cPluginsDir(m_strPluginDir);
    QStringList cFilters;
    cFilters << "*.dll" << "*.so";
    /// try to load each file as plugin in the directory
    foreach(QString strPluginFileName, cPluginsDir.entryList(cFilters, QDir::Files))
    {
        AnalyzerMsgSender::getInstance()->msgOut(QString("Trying to Load Plugin Filter %1 ...").arg(strPluginFileName));
        QPluginLoader* pLoader = new QPluginLoader(cPluginsDir.absoluteFilePath(strPluginFileName));
        AbstractFilter *pPlugin =  qobject_cast<AbstractFilter*>(pLoader->instance());
        /// success or fail
        if (pPlugin)
        {
            m_apcPluginLoaders.push_back(pLoader);
            m_apcFilters.push_back(pPlugin);
            AnalyzerMsgSender::getInstance()->msgOut(QString("Plugin Filter %1 Loading Succeeded!").arg(strPluginFileName));

            GitlEvent cEvt( g_strPluginFilterLoaded );                                      ///
            cEvt.getEvtData().setParameter("filter", QVariant::fromValue((void*)pPlugin));  /// plugin loaded event
            dispatchEvt(&cEvt);                                                             ///

        }
        else
        {
            delete pLoader;
            AnalyzerMsgSender::getInstance()->msgOut(QString("Plugin Filter %1 Loading Failed!").arg(strPluginFileName));
        }
    }

    /// init each filter
    for(int i = 0; i < m_apcFilters.size(); i++)
    {
        m_apcFilters[i]->init(&m_cFilterContext);
    }


    return true;
}


bool FilterLoader::uninit()
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

        GitlEvent cEvt( g_strPluginFilterUnloaded );                   ///
        cEvt.getEvtData().setParameter("filter_name", strFilterName);  /// plugin unloaded event
        dispatchEvt(&cEvt);                                            ///
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
bool FilterLoader::drawPU  (QPainter* pcPainter, ComSequence* pcSequence, ComPU* pcPU,
                            int iPoc, int iAddr, int iZOrder, int iDepth,
                            PartSize ePartSize,  int iPUIndex,
                            int iPUX, int iPUY,
                            int iPUWidth, int iPUHeight, double dScale)
{
    for(int i = 0; i < m_apcFilters.size(); i++)
    {
        AbstractFilter* pFilter = m_apcFilters[i];
        if( pFilter->getEnable() )
        {
            pFilter->drawPU(pcPainter, &m_cFilterContext, pcSequence, pcPU,
                            iPoc, iAddr, iZOrder, iDepth,
                            ePartSize, iPUIndex,
                            iPUX, iPUY, iPUWidth, iPUHeight, dScale);
        }
    }
    return true;
}

bool FilterLoader::drawCU  ( QPainter* pcPainter, ComSequence* pcSequence,
                             int iPoc, int iAddr, int iZOrder, int iDepth,
                             int iCUX, int iCUY,  int iCUSize, double dScale)
{
    for(int i = 0; i < m_apcFilters.size(); i++)
    {
        AbstractFilter* pFilter = m_apcFilters[i];
        if( pFilter->getEnable() )
        {
            pFilter->drawCU( pcPainter, &m_cFilterContext, pcSequence,
                            iPoc, iAddr, iZOrder, iDepth, iCUX, iCUY, iCUSize, dScale);
        }
    }


    return true;
}

bool FilterLoader::drawCTU  (QPainter* pcPainter, ComSequence* pcSequence,
                             int iPoc, int iAddr,
                             int iCTUX, int iCTUY, int iCTUSize, double dScale)

{
    for(int i = 0; i < m_apcFilters.size(); i++)
    {
        AbstractFilter* pFilter = m_apcFilters[i];
        if( pFilter->getEnable() )
        {
            pFilter->drawCTU(pcPainter, &m_cFilterContext, pcSequence, iPoc, iAddr, iCTUX, iCTUY, iCTUSize, dScale);
        }
    }
    return true;
}


bool FilterLoader::drawFrame(QPainter* pcPainter,
                             ComSequence* pcSequence,
                             int iPoc, double dScale)
{
    for(int i = 0; i < m_apcFilters.size(); i++)
    {
        AbstractFilter* pFilter = m_apcFilters[i];
        if( pFilter->getEnable() )
        {
            pFilter->drawFrame(pcPainter, &m_cFilterContext, pcSequence, iPoc, dScale);
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
