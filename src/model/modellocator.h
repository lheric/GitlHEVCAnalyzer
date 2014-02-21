#ifndef MODELLOCATOR_H
#define MODELLOCATOR_H

#include <QDataStream>
#include <QPixmap>
#include "gitlmodual.h"
#include "sequencemanager.h"
#include "io/yuv420rgbbuffer.h"
#include "drawengine/drawengine.h"
#include "preferences.h"
#include "parsers/decodergeneralparser.h"
#include "parsers/cupuparser.h"
#include "exceptions/nosequencefoundexception.h"

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

    /**
      * Preferences
      */
    ADD_CLASS_FIELD_NOSETTER(Preferences, cPreferences, getPreferences)             ///< Setting for cache directory, decoder path, etc.

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
