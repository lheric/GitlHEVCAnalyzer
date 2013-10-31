#ifndef SEQUENCEMANAGER_H
#define SEQUENCEMANAGER_H
#include "gitldef.h"
#include "common/comsequence.h"
#include "exceptions/nosequencefoundexception.h"
#include <QVector>
/*!
 * \brief The SequenceManager class
 * This class contains serveral sequences, in order to support multi-sequence analysis (difference comparasion, etc.)
 */

class SequenceManager
{
public:
    explicit SequenceManager(){ m_pcCurrentSequence = NULL; }
    ~SequenceManager()
    {
        while(!m_apSequences.empty())
        {
            delete m_apSequences.back();
            m_apSequences.pop_back();
        }
    }

    ComSequence& getCurrentSequence()
    {
        if(m_pcCurrentSequence == NULL)
            throw NoSequenceFoundException();
        return *m_pcCurrentSequence;
    }

    void setCurrentSequence(ComSequence* pcSequence)
    {
        m_pcCurrentSequence = pcSequence;
    }

    void addSequence(ComSequence* pcSequence)
    {
        m_apSequences.push_back(pcSequence);
    }

    QVector<ComSequence*>& getAllSequences()
    {
        return m_apSequences;
    }

    ComSequence* getSequenceByFilename(const QString& strFilename)
    {
        foreach(ComSequence* p, m_apSequences)
        {
            if(p->getFileName() == strFilename)
                return p;
        }
        return NULL;
    }

    ADD_CLASS_FIELD_PRIVATE(ComSequence*, pcCurrentSequence)
    ADD_CLASS_FIELD_PRIVATE(QVector<ComSequence*>, apSequences)
};
#endif // SEQUENCEMANAGER_H
