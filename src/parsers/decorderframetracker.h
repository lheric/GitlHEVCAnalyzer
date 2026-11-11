#ifndef DECORDERFRAMETRACKER_H
#define DECORDERFRAMETRACKER_H

#include <QSet>

/// Maps the "<POC,addr> ..." lines of the per-CTU decoder files (written in
/// decoding order) to the index of their picture in decoding order.
///
/// A new picture starts when the POC changes, or when an address repeats
/// within the current picture: consecutive pictures can share a POC (e.g. an
/// all-IDR stream, where every picture has POC 0), while CTU addresses (or a
/// tile's first CTU address) are unique within one picture.
class DecOrderFrameTracker
{
public:
    int next(int iPoc, int iAddr)
    {
        if( m_iDecOrder < 0 || iPoc != m_iLastPoc || m_cSeenAddrs.contains(iAddr) )
        {
            ++m_iDecOrder;
            m_cSeenAddrs.clear();
        }
        m_iLastPoc = iPoc;
        m_cSeenAddrs.insert(iAddr);
        return m_iDecOrder;
    }

private:
    int       m_iDecOrder = -1;
    int       m_iLastPoc  = 0;
    QSet<int> m_cSeenAddrs;
};

#endif // DECORDERFRAMETRACKER_H
