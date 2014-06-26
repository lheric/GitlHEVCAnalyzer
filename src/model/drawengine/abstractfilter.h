#ifndef ABSTRACTFILTER_H
#define ABSTRACTFILTER_H
#include <QString>
#include <QPainter>
#include <QtPlugin>
#include "model/common/comsequence.h"

class SequenceManager;
class DrawEngine;
class YUV420RGBBuffer;
class FilterLoader;
class SelectionManager;

/*!
 * \brief The FilterContext class
 * Parameters which will be sent to the filter plgins in AbstractFilter Interface
 * Basically it is just pointers to the objects in model, indicating the state of the system.
 */
struct FilterContext
{
    SequenceManager* pcSequenceManager;     /// sequences
    DrawEngine* pcDrawEngine;               /// draw engine
    YUV420RGBBuffer* pcBuffer;              /// yuv and rgb buffer
    FilterLoader* pcFilterLoader;           /// filter loader (all filters are here)
    SelectionManager* pcSelectionManager;   /// selection helper function
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
    virtual bool init     (FilterContext* pcContext)
    {
        return true;
    }

    /*!
     * \brief uninit is called before unload
     * \param pcContext \see FilterContext
     * \return
     */
    virtual bool uninit    (FilterContext* pcContext)
    {
        return true;
    }

    /*!
     * \brief config is called when user push the 'config' button in user interface (or 'config' command in command line)
     *        Generally, we suggest a GUI here for adjusting the parameters for filters.
     * \param pcContext \see FilterContext
     * \return
     */
    virtual bool config   (FilterContext* pcContext)
    {
        return true;
    }

    /*!
     * \brief drawFrame is called for every frame
     * \param pcContext \see FilterContext
     * \param pcPainter the QPainter of the QPixmap object which is being displayed on screen
     * \param pcSequence sequence which is selected as the currently displaying one
     * \param iPoc the POC of currently displaying frame (begin with 0)
     * \return true - success   false - fail
     */
    virtual bool drawFrame(FilterContext* pcContext, QPainter* pcPainter,
                           ComFrame *pcFrame, double dScale, QRect* pcScaledArea)
    {
        return true;
    }

    /*!
     * \brief drawCTU is called for each CTU (Coding Tree Unit, i.e. LCU)
     * \param pcContext \see FilterContext
     * \param pcPainter the QPainter of the QPixmap object which is being displayed on screen
     * \param pcCU the CTU(i.e. LCU) to be draw
     * \param dScale the scale of current display
     * \param pcScaledArea the scaled size of current PU
     * \return
     */
    virtual bool drawCTU(FilterContext* pcContext, QPainter* pcPainter,
                         ComCU *pcCTU, double dScale, QRect* pcScaledArea)
    {
        return true;
    }


    /*!
     * \brief drawCU is called for each leaf CU (each leaf node of the Coding Tree Unit )
     * \param pcContext \see FilterContext
     * \param pcPainter the QPainter of the QPixmap object which is being displayed on screen     
     * \param pcCU the CU to be draw
     * \param dScale the scale of current display
     * \param pcScaledArea the scaled size of current PU
     * \return
     */
    virtual bool drawCU   (FilterContext* pcContext, QPainter* pcPainter,
                           ComCU *pcCU, double dScale,  QRect* pcScaledArea)
    {
        return true;
    }

    /*!
     * \brief drawPU
     * \param pcContext \see FilterContext
     * \param pcPainter the QPainter of the QPixmap object which is being displayed on screen
     * \param pcPU the PU to be draw
     * \param dScale the scale of current display
     * \param pcScaledArea the scaled size of current PU
     * \return
     */
    virtual bool drawPU   (FilterContext* pcContext, QPainter* pcPainter,
                           ComPU *pcPU, double dScale,  QRect* pcScaledArea)
    {
        return true;
    }

    /*!
     * \brief drawTU
     * \param pcContext \see FilterContext
     * \param pcPainter the QPainter of the QPixmap object which is being displayed on screen
     * \param pcTU the TU to be draw
     * \param dScale the scale of current display
     * \param pcScaledArea the scaled size of current PU
     * \return
     */
    virtual bool drawTU   (FilterContext* pcContext, QPainter* pcPainter,
                           ComTU *pcTU, double dScale,  QRect* pcScaledArea)
    {
        return true;
    }



    /*!
     * \brief mousePress when user press mouse button on the dispalyed frame
     * \param pcContext \see FilterContext
     * \param pcPainter the QPainter of the QPixmap object which is being displayed on screen
     * \param pcUnscaledPos position in the unscaled frame
     * \param pcScaledPos position in the scaled frame
     * \param dScale the scale of current display
     * \param eMouseBtn mouse button that is pressed
     * \return
     */
    virtual bool mousePress(FilterContext* pcContext, QPainter* pcPainter, ComFrame *pcFrame,
                            const QPointF* pcUnscaledPos, const QPointF* pcScaledPos,
                            double dScale, Qt::MouseButton eMouseBtn)
    {
        return true;
    }

    /*!
     * \brief keyPress when user press a key
     * \param pcContext \see FilterContext
     * \param pcPainter the QPainter of the QPixmap object which is being displayed on screen
     * \param iKeyPressed position in the unscaled frame
     * \return
     */
    virtual bool keyPress  (FilterContext* pcContext, QPainter* pcPainter, ComFrame *pcFrame,
                            int iKeyPressed)
    {
        return true;
    }

    /*! This is the filter name displayed in the user interface
     */
    ADD_CLASS_FIELD(QString, strName, getName, setName)


    /*! This is the switch for turn off or on this filter
     *  false - this filter will not be applied
     *  true  - this filter will be applied
     */
    ADD_CLASS_FIELD(bool, bEnable, getEnable, setEnable)





};

/// This is required by the Qt plugin system.
Q_DECLARE_INTERFACE(AbstractFilter, "cn.edu.sysu.gitl.gitlhevcanalyzer.AbstractFilter")
Q_DECLARE_METATYPE(AbstractFilter*)

#endif // ABSTRACTFILTER_H
