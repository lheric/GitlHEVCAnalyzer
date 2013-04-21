#ifndef MODELLOCATOR_H
#define MODELLOCATOR_H

#include <QDataStream>
#include <QPixmap>
#include "gitlmodual.h"
#include "io/yuv420rgbbuffer.h"
#include "drawengine/drawengine.h"
#include "parsers/encodergeneralparser.h"
#include "parsers/decodergeneralparser.h"
#include "parsers/cupuparser.h"
#include "parsers/meparser.h"
#include "exceptions/nosequencefoundexception.h"
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
    ADD_CLASS_FIELD_PRIVATE(ComSequence*, pcCurrentSequence)
    ADD_CLASS_FIELD_PRIVATE(QVector<ComSequence*>, apSequences)
};


/*!
 * \brief The ModelLocator class
 *  This class serves as the model, saving the state of the system
 *  Singleton Pattern
 */

class ModelLocator : GitlModual
{
public:
    /**
      * Frame buffer for currently showing frame
      */
    ADD_CLASS_FIELD_NOSETTER(YUV420RGBBuffer, cFrameBuffer, getFrameBuffer)         ///< YUV & RGB buffer of current frame

    /**
      * Sequence meta infomations
      */
    ADD_CLASS_FIELD_NOSETTER(SequenceManager, cSequenceManager, getSequenceManager) ///< Sequences meta info

    /**
      * Draw Engine
      */
    ADD_CLASS_FIELD_NOSETTER(DrawEngine, cDrawEngine, getDrawEngine)                ///< Draw engin for displaying CU PU ME info

public:
    /**
      * SINGLETON ( design pattern )
      */
    SINGLETON_PATTERN_DECLARE(ModelLocator)
private:
    explicit ModelLocator();
public:
    ~ModelLocator();

};

#endif // MODELLOCATOR_H
