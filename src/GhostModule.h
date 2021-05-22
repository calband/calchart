#pragma once
/*
 * GhostModule.h
 * Maintains the indirection of the Ghost sheet.
 */

#include "CalChartDoc.h"
#include "CalChartSheet.h"

namespace CalChart {
class Sheet;
}
class CalChartDoc;

enum class GhostSource {
    disabled,
    next,
    previous,
    specific,
};

class GhostModule {
public:
    void setGhostSource(GhostSource source, int which = 0)
    {
        mCurrentSource = source;
        mWhich = which;
    }

    auto isActive() const { return mCurrentSource != GhostSource::disabled; }

    CalChart::Sheet const* getGhostSheet(CalChartDoc const* doc, int currentSheet) const
    {
        if (!isActive() || (doc == nullptr)) {
            return nullptr;
        }
        auto targetSheet = (mCurrentSource == GhostSource::next) ? currentSheet + 1 : (mCurrentSource == GhostSource::previous) ? currentSheet - 1 : mWhich;
        if (targetSheet >= 0 && targetSheet < doc->GetNumSheets()) {
            return &(*(doc->GetNthSheet(targetSheet)));
        }
        return nullptr;
    }


private:
    GhostSource mCurrentSource = GhostSource::disabled;
    int mWhich = 0;
};
