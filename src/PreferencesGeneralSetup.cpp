/*
 * CalChartPreferences.cpp
 * Dialox box for preferences
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

#include "PreferencesGeneralSetup.h"
#include "PreferencesUtils.h"

enum {
    AUTOSAVE_INTERVAL = 1000,
    BEEP_ON_COLLISIONS,
    SCROLL_NATURAL,
    SETSHEET_UNDO,
    SELECTION_UNDO,
};

BEGIN_EVENT_TABLE(GeneralSetup, PreferencePage)
END_EVENT_TABLE()

IMPLEMENT_CLASS(GeneralSetup, PreferencePage)

GeneralSetup::GeneralSetup(CalChartConfiguration& config, wxWindow* parent, wxWindowID id, wxString const& caption, wxPoint const& pos, wxSize const& size, long style)
    : PreferencePage(config)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

void GeneralSetup::CreateControls()
{
    auto topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    auto boxsizer = new wxStaticBoxSizer(new wxStaticBox(this, -1, wxT("Autosave settings")), wxVERTICAL);
    topsizer->Add(boxsizer);

    auto sizer1 = new wxBoxSizer(wxVERTICAL);
    boxsizer->Add(sizer1, LeftBasicSizerFlags());

    AddTextboxWithCaption(this, sizer1, AUTOSAVE_INTERVAL, wxT("Autosave Interval"));
    AddCheckboxWithCaption(this, sizer1, BEEP_ON_COLLISIONS, wxT("Beep on animation collisions"));
    AddCheckboxWithCaption(this, sizer1, SCROLL_NATURAL, wxT("Scroll Direction: Natural"));
    AddCheckboxWithCaption(this, sizer1, SETSHEET_UNDO, wxT("Set Sheet is undo-able"));
    AddCheckboxWithCaption(this, sizer1, SELECTION_UNDO, wxT("Point selection is undo-able"));

    TransferDataToWindow();
}

void GeneralSetup::Init()
{
    mAutoSave_Interval.Printf(wxT("%d"), static_cast<int>(mConfig.Get_AutosaveInterval()));
    mBeep_On_Collisions = mConfig.Get_BeepOnCollisions();
    mScroll_Natural = mConfig.Get_ScrollDirectionNatural();
    mSetSheet_Undo = mConfig.Get_CommandUndoSetSheet();
    mSelection_Undo = mConfig.Get_CommandUndoSelection();
}

bool GeneralSetup::TransferDataToWindow()
{
    static_cast<wxTextCtrl*>(FindWindow(AUTOSAVE_INTERVAL))->SetValue(mAutoSave_Interval);
    static_cast<wxCheckBox*>(FindWindow(BEEP_ON_COLLISIONS))->SetValue(mBeep_On_Collisions);
    static_cast<wxCheckBox*>(FindWindow(SCROLL_NATURAL))->SetValue(mScroll_Natural);
    static_cast<wxCheckBox*>(FindWindow(SETSHEET_UNDO))->SetValue(mSetSheet_Undo);
    static_cast<wxCheckBox*>(FindWindow(SELECTION_UNDO))->SetValue(mSelection_Undo);
    return true;
}

bool GeneralSetup::TransferDataFromWindow()
{
    // read out the values from the window
    mAutoSave_Interval = static_cast<wxTextCtrl*>(FindWindow(AUTOSAVE_INTERVAL))->GetValue();
    mBeep_On_Collisions = static_cast<wxCheckBox*>(FindWindow(BEEP_ON_COLLISIONS))->GetValue();
    mScroll_Natural = static_cast<wxCheckBox*>(FindWindow(SCROLL_NATURAL))->GetValue();
    mSetSheet_Undo = static_cast<wxCheckBox*>(FindWindow(SETSHEET_UNDO))->GetValue();
    mSelection_Undo = static_cast<wxCheckBox*>(FindWindow(SELECTION_UNDO))->GetValue();
    if (long val = 0; mAutoSave_Interval.ToLong(&val)) {
        mConfig.Set_AutosaveInterval(val);
    }
    mConfig.Set_BeepOnCollisions(mBeep_On_Collisions);
    mConfig.Set_ScrollDirectionNatural(mScroll_Natural);
    mConfig.Set_CommandUndoSetSheet(mSetSheet_Undo);
    mConfig.Set_CommandUndoSelection(mSelection_Undo);
    return true;
}

bool GeneralSetup::ClearValuesToDefault()
{
    mConfig.Clear_AutosaveInterval();
    mConfig.Clear_CalChartFrameAUILayout_3_6_0();
    mConfig.Clear_FieldFrameZoom_3_6_0();
    mConfig.Clear_FieldFrameWidth();
    mConfig.Clear_FieldFrameHeight();
    mConfig.Clear_FieldFramePositionX();
    mConfig.Clear_FieldFramePositionY();
    mConfig.Clear_UseSprites();
    Init();
    TransferDataToWindow();
    return true;
}

