/*
 * CalChartLogTarget.cpp
 * wxLog adapter that feeds messages into CircularLogBuffer
 */

/*
   Copyright (C) 1995-2025  Garrick Brian Meeker, Richard Michael Powell

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/\>.
*/

#include "CalChartLogTarget.h"
#include <wx/string.h>

CalChartLogTarget::CalChartLogTarget(CalChart::CircularLogBuffer buffer)
    : buffer_(std::move(buffer))
{
}

void CalChartLogTarget::DoLogRecord(wxLogLevel level, const wxString& msg,
    const wxLogRecordInfo& info)
{
    // Capture the log message into our circular buffer
    buffer_.AddMessage(GetLevelString(level), std::string(msg.mb_str(wxConvUTF8)));

    // Forward to the next logger in the chain if one exists
    if (next_target_) {
        next_target_->LogRecord(level, msg, info);
    }
}

std::string CalChartLogTarget::GetLevelString(wxLogLevel level)
{
    switch (level) {
    case wxLOG_FatalError:
        return "Fatal Error";
    case wxLOG_Error:
        return "Error";
    case wxLOG_Warning:
        return "Warning";
    case wxLOG_Message:
        return "Info";
    case wxLOG_Status:
        return "Status";
    case wxLOG_Info:
        return "Info";
    case wxLOG_Debug:
        return "Debug";
    case wxLOG_Trace:
        return "Trace";
    default:
        return "Unknown";
    }
}
