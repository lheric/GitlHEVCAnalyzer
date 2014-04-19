#include "sequencemanager.h"

SequenceManager::SequenceManager()
{
    m_pcCurrentSequence = NULL;
}

SequenceManager::~SequenceManager()
{
    while(!m_apSequences.empty())
    {
        delete m_apSequences.back();
        m_apSequences.pop_back();
    }
}

ComSequence* SequenceManager::getCurrentSequence()
{
    if(m_pcCurrentSequence == NULL)
        qCritical() << "No Video Sequence Found..";
    //throw NoSequenceFoundException();
    return m_pcCurrentSequence;
}

void SequenceManager::setCurrentSequence(ComSequence *pcSequence)
{
    m_pcCurrentSequence = pcSequence;
}

void SequenceManager::addSequence(ComSequence *pcSequence)
{
    m_apSequences.push_back(pcSequence);
}

bool SequenceManager::delSequence(ComSequence *pcSequence)
{
    for(int i = 0; i < m_apSequences.size(); i++)
    {
        if( m_apSequences[i] == pcSequence)
        {         
            m_apSequences.remove(i);
            if(pcSequence == m_pcCurrentSequence)
                m_pcCurrentSequence = NULL;
            delete pcSequence;
            return true;
        }
    }
    return false;
}

QVector<ComSequence *> &SequenceManager::getAllSequences()
{
    return m_apSequences;
}

ComSequence* SequenceManager::getSequenceByFilename(const QString &strFilename)
{
    foreach(ComSequence* p, m_apSequences)
    {
        if(p->getFileName() == strFilename)
            return p;
    }
    return NULL;
}
