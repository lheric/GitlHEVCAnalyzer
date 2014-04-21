#ifndef SELECTIONMANAGER_H
#define SELECTIONMANAGER_H
#include "common/comsequence.h"
#include "common/comcu.h"
#include "common/compu.h"
#include "common/commv.h"
#include "common/comtu.h"

class SelectionManager
{
public:
    SelectionManager();
    ComCU* getLCU(ComFrame* pcFrame, const QPointF* pcUnscaledPos);
    ComCU* getSCU(ComFrame* pcFrame, const QPointF* pcUnscaledPos);
    ComPU* getPU(ComFrame* pcFrame, const QPointF* pcUnscaledPos);
    ComTU* getTU(ComFrame *pcFrame, const QPointF* pcUnscaledPos);
};

#endif // SELECTIONMANAGER_H
