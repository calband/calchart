#pragma once
/*
 * CalChartLogTarget.h
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
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "CircularLogBuffer.hpp"
#include <wx/log.h>

// wxLog adapter that captures log messages into a CircularLogBuffer
// while chaining to the next logger in the log chain
class CalChartLogTarget : public wxLog {
public:
    explicit CalChartLogTarget(CalChart::CircularLogBuffer buffer);
    ~CalChartLogTarget() override = default;

    // Set the next logger in the chain for message forwarding
    void SetNextTarget(wxLog* next) { next_target_ = next; }

    // Get a copy of the global log buffer for bug reporting
    CalChart::CircularLogBuffer GetLogBuffer() const
    {
        return buffer_;
    }

protected:
    void DoLogRecord(wxLogLevel level, const wxString& msg,
        const wxLogRecordInfo& info) override;

private:
    CalChart::CircularLogBuffer buffer_;
    wxLog* next_target_ = nullptr;

    // Convert wxLog level to string representation
    static std::string GetLevelString(wxLogLevel level);
};
