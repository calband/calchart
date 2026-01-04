/*
 * ViewerPreviewDialog.cpp
 */

/*
   Copyright (C) 1995-2024  Garrick Brian Meeker, Richard Michael Powell

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

#include "ViewerPreviewDialog.h"

#include "CalChartDoc.h"
#include "ViewerPanel.h"

#include <wx/button.h>
#include <wx/sizer.h>

ViewerPreviewDialog::ViewerPreviewDialog(wxWindow* parent, CalChartDoc* doc)
    : super(parent, wxID_ANY, "CalChart Viewer Preview", wxDefaultPosition, wxSize(1024, 768), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    , mDoc(doc)
{
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    mViewerPanel = new ViewerPanel(this, doc);
    mainSizer->Add(mViewerPanel, 1, wxEXPAND | wxALL, 8);

    auto* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* refreshButton = new wxButton(this, wxID_REFRESH, "Refresh");
    auto* closeButton = new wxButton(this, wxID_CANCEL, "Close");

    buttonSizer->AddStretchSpacer();
    buttonSizer->Add(refreshButton, 0, wxRIGHT, 8);
    buttonSizer->Add(closeButton, 0);

    mainSizer->Add(buttonSizer, 0, wxALIGN_RIGHT | wxALL, 8);

    SetSizer(mainSizer);
    SetEscapeId(wxID_CANCEL);

    refreshButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        RefreshFromDoc();
    });

    RefreshFromDoc();
}

void ViewerPreviewDialog::RefreshFromDoc()
{
    if (!mDoc || !mViewerPanel) {
        return;
    }

    auto json = mDoc->toViewerFileJSON();
    mViewerPanel->InjectShowDataWhenReady(json.dump(4));
}
