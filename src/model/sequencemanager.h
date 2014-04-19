#ifndef SEQUENCEMANAGER_H
#define SEQUENCEMANAGER_H
#include "gitldef.h"
#include "common/comsequence.h"
#include "exceptions/nosequencefoundexception.h"
#include <QVector>
#include <QDebug>
/*!
 * \brief The SequenceManager class
 * This class contains serveral sequences, in order to support multi-sequence analysis (difference comparasion, etc.)
 */

class SequenceManager
{
public:
    explicit SequenceManager();
    ~SequenceManager();

    ComSequence* getCurrentSequence();

    void setCurrentSequence(ComSequence* pcSequence);

    void addSequence(ComSequence* pcSequence);

    bool delSequence(ComSequence* pcSequence);

    QVector<ComSequence*>& getAllSequences();

    ComSequence* getSequenceByFilename(const QString& strFilename);

    ADD_CLASS_FIELD_PRIVATE(ComSequence*, pcCurrentSequence)
    ADD_CLASS_FIELD_PRIVATE(QVector<ComSequence*>, apSequences)
};
#endif // SEQUENCEMANAGER_H
