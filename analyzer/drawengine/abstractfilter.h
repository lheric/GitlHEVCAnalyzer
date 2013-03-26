#ifndef ABSTRACTFILTER_H
#define ABSTRACTFILTER_H
#include <QString>
#include <QPainter>
#include <QtPlugin>
#include "common/comsequence.h"


class SequenceManager;
class DrawEngine;
class YUV420RGBBuffer;
class FilterLoader;

/*!
 * \brief The FilterContext class
 * Parameters that will be sent to the filter plgins in AbstractFilter Interface
 * Basically it is just pointers to the objects in model, indicating the state of the system.
 */
struct FilterContext
{
    SequenceManager* pcSequenceManager;     /// sequences
    DrawEngine* pcDrawEngine;               /// draw engine
    YUV420RGBBuffer* pcBuffer;              /// yuv and rgb buffer
    FilterLoader* pcFilterLoader;           /// filter loader (all filters are here)
};

/*!
 * \brief The AbstractFilter class
 * Interface of the filter plugins
 */
class AbstractFilter
{
public:
    AbstractFilter()
    {
        m_bEnable = false;
        m_strName = "UNKNOWN";
    }

    virtual ~AbstractFilter()
    {
    }

    /*!
     * \brief init is called as soon as loaded in
     * \param pcContext \see FilterContext
     * \return
     */
    virtual bool init     (FilterContext* pcContext) = 0;

    /*!
     * \brief uninit is called before unload
     * \param pcContext \see FilterContext
     * \return
     */
    virtual bool uninit    (FilterContext* pcContext) = 0;

    /*!
     * \brief config is called when user push the 'config' button in user interface (or 'config' command in command line)
     *        Generally, we suggest a GUI here for adjusting the parameters for filters.
     * \param pcContext \see FilterContext
     * \return
     */
    virtual bool config   (FilterContext* pcContext) = 0;

    /*!
     * \brief drawFrame is called for every frame
     * \param pcPainter the QPainter of the QPixmap object which is being displayed on screen
     * \param pcContext \see FilterContext
     * \param pcSequence sequence which is selected as the currently displaying one
     * \param iPoc the POC of currently displaying frame (begin with 0)
     * \return true - success   false - fail
     */
    virtual bool drawFrame(QPainter* pcPainter, FilterContext* pcContext, ComSequence* pcSequence, int iPoc) = 0;

    /*!
     * \brief drawCTU is called for each CTU (Coding Tree Unit, i.e. LCU)
     * \param pcPainter the QPainter of the QPixmap object which is being displayed on screen
     * \param pcContext \see FilterContext
     * \param pcSequence sequence which is selected as the currently displaying one
     * \param iPoc the POC of currently displaying frame (begin with 0)
     * \param iAddr CTU in raster order (begin with 0)
     * \param iCTUX CTU x coordinate in this frame
     * \param iCTUY CTU y coordinate in this frame
     * \param iCTUSize size of the CTU
     * \return
     */
    virtual bool drawCTU(QPainter* pcPainter, FilterContext* pcContext, ComSequence* pcSequence, int iPoc, int iAddr, int iCTUX, int iCTUY, int iCTUSize) = 0;


    /*!
     * \brief drawCU is called for each leaf CU (each leaf node of the Coding Tree Unit )
     * \param pcPainter the QPainter of the QPixmap object which is being displayed on screen
     * \param pcContext \see FilterContext
     * \param pcSequence sequence which is selected as the currently displaying one
     * \param iPoc the POC of currently displaying frame (begin with 0)
     * \param iAddr parent CTU in raster order (begin with 0)
     * \param iZOrder CU in z-order in this CTU
     * \param iDepth CU depth
     * \param iCUX CU x coordinate in this frame
     * \param iCUY CU y coordinate in this frame
     * \param iCUSize size of the CU
     * \return
     */
    virtual bool drawCU   (QPainter* pcPainter, FilterContext* pcContext, ComSequence* pcSequence, int iPoc, int iAddr, int iZOrder, int iDepth, int iCUX, int iCUY, int iCUSize) = 0;

    /*!
     * \brief drawPU
     * \param pcPainter the QPainter of the QPixmap object which is being displayed on screen
     * \param pcContext \see FilterContext
     * \param pcSequence sequence which is selected as the currently displaying one
     * \param pcPU the PU to be draw
     * \param iPoc the POC of currently displaying frame (begin with 0)
     * \param iAddr parent CTU in raster order (begin with 0)
     * \param iZOrder parent CU in z-order in this CTU
     * \param iDepth parent CU depth
     * \param ePartSize \see PartSize
     * \param iPUIndex PU in raster order in parent CU
     * \param iPUX PU x coordinate in this frame
     * \param iPUY PU y coordinate in this frame
     * \param iPUWidth PU width
     * \param iPUHeight PU height
     * \return
     */
    virtual bool drawPU   (QPainter* pcPainter, FilterContext* pcContext, ComSequence* pcSequence, ComPU* pcPU, int iPoc, int iAddr, int iZOrder, int iDepth, PartSize ePartSize, int iPUIndex, int iPUX, int iPUY, int iPUWidth, int iPUHeight) = 0;

    /*! This is the filter name displayed in the user interface
     */
    ADD_CLASS_FIELD(QString, strName, getName, setName)


    /*! This is the switch for turn off or on this filter
     *  false - this filter will not be applied
     *  true  - this filter will be applied
     */
    ADD_CLASS_FIELD(bool, bEnable, getEnable, setEnable)


};

/// This is required by the Qt Plugin System.
Q_DECLARE_INTERFACE(AbstractFilter, "cn.edu.gitl.sysu.gitlhevcanalyzer.AbstractFilter")
Q_DECLARE_METATYPE(AbstractFilter*)

#endif // ABSTRACTFILTER_H
