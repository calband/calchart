#include "ghost_module.h"

#include "CalChartDoc.h"
#include "cc_sheet.h"

void GhostModule::setGhostSource(GhostSource source, int which)
{
    mCurrentSource = source;
    mWhich = which;
}

CalChart::Sheet const* GhostModule::getGhostSheet(CalChartDoc const* doc, int currentSheet) const
{
    if ((doc == nullptr) || (mCurrentSource == disabled)) {
        return nullptr;
    }
    auto targetSheet = (mCurrentSource == next) ? currentSheet + 1 : (mCurrentSource == previous) ? currentSheet - 1 : mWhich;
    if (targetSheet >= 0 && targetSheet < doc->GetNumSheets()) {
        return &(*(doc->GetNthSheet(targetSheet)));
    }
    return nullptr;
}

bool GhostModule::isActive() const { return mCurrentSource != disabled; }
