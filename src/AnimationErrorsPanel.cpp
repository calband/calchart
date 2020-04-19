/*
 * AnimationErrorsPanel
 */

/*
   Copyright (C) 1995-2011  Garrick Brian Meeker, Richard Michael Powell

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

#include "AnimationErrorsPanel.h"
#include "CalChartView.h"
#include "animate_types.h"
#include "basic_ui.h"

#include <wx/artprov.h>
#include <wx/treelist.h>

BEGIN_EVENT_TABLE(AnimationErrorsPanel, wxScrolledWindow)
EVT_TREELIST_SELECTION_CHANGED(wxID_ANY, AnimationErrorsPanel::OnSelectionChanged)
EVT_TREELIST_ITEM_ACTIVATED(wxID_ANY, AnimationErrorsPanel::OnItemActivated)
END_EVENT_TABLE()

AnimationErrorsPanel::AnimationErrorsPanel(wxWindow* parent)
    : super(parent)
{
    // create a sizer and populate
    auto topSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topSizer);

    // add a horizontal bar to make things clear:
    mTreeCtrl = new wxTreeListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    mTreeCtrl->AppendColumn("Errors", wxCOL_WIDTH_AUTOSIZE, wxALIGN_LEFT, wxCOL_RESIZABLE | wxCOL_SORTABLE);

    wxSize iconSize = wxArtProvider::GetSizeHint(wxART_LIST);
    if (iconSize == wxDefaultSize) {
        iconSize = wxSize(16, 16);
    }
    auto mImageList = new wxImageList(iconSize.x, iconSize.y);
    mImageList->Add(wxArtProvider::GetIcon(wxART_ERROR, wxART_LIST, iconSize));
    mTreeCtrl->SetImageList(mImageList);

    AddToSizerExpand(topSizer, mTreeCtrl);

    // now fit the frame to the elements
    topSizer->Fit(this);
    topSizer->SetSizeHints(this);

    // now update the current screen
    OnUpdate();
}

void AnimationErrorsPanel::UpdateErrors(std::vector<CalChart::AnimationErrors> const& errors)
{
    if (errors == mCurrentErrors) {
        return;
    }
    // if we have new set of errors
    mCurrentErrors = errors;
    // now regenerate the table.

    mTreeCtrl->DeleteAllItems();

    // we create a mapping between the generated errors and the items we've put in the list
    // categorize by animation error, mapping them to the sheet and ErrorMarker
    std::map<CalChart::AnimateError, std::vector<std::tuple<int, CalChart::ErrorMarker>>> allErrors;
    for (auto i = 0; i < mCurrentErrors.size(); ++i) {
        for (auto&& error : mCurrentErrors[i].GetErrors()) {
            allErrors[error.first].push_back({ i, error.second });
        }
    }

    // now if we have any, create an error node, and then start filling it up.
    if (allErrors.size()) {
        for (auto&& errorType : allErrors) {
            auto itemId1 = mTreeCtrl->AppendItem(mTreeCtrl->GetRootItem(), CalChart::s_animate_err_msgs[errorType.first], 0, 0);
            for (auto&& error : errorType.second) {
                auto itemId2 = mTreeCtrl->AppendItem(itemId1, std::string("Sheet ") + std::to_string(std::get<0>(error)));
                // now we need to be able to get back to the errors and select things
                mErrorLookup[itemId2] = error;
            }
        }
    }
}

void AnimationErrorsPanel::OnUpdate()
{
    if (mView) {
        auto errors = mView->GetAnimationErrors();
        UpdateErrors(errors);
    }
    Refresh();
}

void AnimationErrorsPanel::OnSelectionChanged(wxTreeListEvent& event)
{
    if (auto error = mErrorLookup.find(event.GetItem()); error != mErrorLookup.end()) {
        printf("Found error at %d\n", std::get<0>(error->second));
        mView->GoToSheetAndSetSelection(std::get<0>(error->second), std::get<1>(error->second).pntgroup);
    }
}

void AnimationErrorsPanel::OnItemActivated(wxTreeListEvent& event)
{
    if (auto error = mErrorLookup.find(event.GetItem()); error != mErrorLookup.end()) {
        printf("Found error at %d\n", std::get<0>(error->second));
        mView->GoToSheetAndSetSelection(std::get<0>(error->second), std::get<1>(error->second).pntgroup);
    }
}
