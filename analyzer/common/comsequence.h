#ifndef COMSEQUENCE_H
#define COMSEQUENCE_H

#include "comanalyzerdef.h"
#include "comframe.h"

/*!
 * @brief This class represents a video sequence
 */

class ComSequence
{
public:
    explicit ComSequence();
    ~ComSequence();

    /*! Initialization
      */
    void init();

    /*! Frames in this sequence */
    ADD_CLASS_FIELD(QVector<ComFrame*>, cFrames, getFrames, setFrames)


    /*! Sequence Parameter Set */
    ADD_CLASS_FIELD( QString, strFileName, getFileName, setFileName )     /// filename
    ADD_CLASS_FIELD( int, iWidth, getWidth, setWidth )                    /// width
    ADD_CLASS_FIELD( int, iHeight, getHeight, setHeight )                 /// height
    ADD_CLASS_FIELD( int, iTotalFrames, getTotalFrames, setTotalFrames )  /// total frame count

    ADD_CLASS_FIELD( int, iMaxCUSize, getMaxCUSize, setMaxCUSize )      /// max CU size (64 as general)
    ADD_CLASS_FIELD( int, iMaxCUDepth, getMaxCUDepth, setMaxCUDepth )   /// max CU Tree depth (4 as general)
    ADD_CLASS_FIELD( int, iMinTUDepth, getMinTUDepth, setMinTUDepth )   /// max TU size
    ADD_CLASS_FIELD( int, iMaxTUDepth, getMaxTUDepth, setMaxTUDepth )   /// max TU Tree depth


    /*! Decoded File Location */
    ADD_CLASS_FIELD( QString, strDeocdingFolder, getDecodingFolder, setDecodingFolder)

    /*!
     * Optional info
     * Obsolescent
     */
    ADD_CLASS_FIELD(QString, strEncoderVersion, getEncoderVersion, setEncoderVersion) ///
    ADD_CLASS_FIELD(double, dTotalDecTime, getTotalDecTime, setTotalDecTime)
    ADD_CLASS_FIELD(double, dPSNR, getPSNR, setPSNR)
    ADD_CLASS_FIELD(double, dBitrate, getBitrate, setBitrate)
    ADD_CLASS_FIELD(double, dTotalEncTime, getTotalEncTime, setTotalEncTime)
    ADD_CLASS_FIELD(double, dSameCUModePercent, getSameCUModePercent, setSameCUModePercent)
    ADD_CLASS_FIELD(double, dMeanCUDepthError, getMeanCUDepthError, setMeanCUDepthError)


};

#endif // COMSEQUENCE_H
