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

void GeneralSetup::CreateControls()
{
    SetSizer(VStack([this](auto& sizer) {
        NamedVBoxStack(this, sizer, "General settings", [this](auto& sizer) {
            VStack(sizer, [this](auto& sizer) {
                CreateTextboxWithCaption(this, sizer, AUTOSAVE_INTERVAL, "Autosave Interval");
                CreateCheckboxWithCaption(this, sizer, BEEP_ON_COLLISIONS, "Beep on animation collisions");
                CreateCheckboxWithCaption(this, sizer, SCROLL_NATURAL, "Scroll Direction: Natural");
                CreateCheckboxWithCaption(this, sizer, SETSHEET_UNDO, "Set Sheet is undo-able");
                CreateCheckboxWithCaption(this, sizer, SELECTION_UNDO, "Point selection is undo-able");
            });
        });
    }));

    TransferDataToWindow();
}

void GeneralSetup::InitFromConfig()
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
    mConfig.Clear_CalChartFrameAUILayout_3_6_1();
    mConfig.Clear_FieldFrameZoom_3_6_0();
    mConfig.Clear_FieldFrameWidth();
    mConfig.Clear_FieldFrameHeight();
    mConfig.Clear_FieldFramePositionX();
    mConfig.Clear_FieldFramePositionY();
    mConfig.Clear_UseSprites();
    mConfig.Clear_SpriteBitmapScale();
    mConfig.Clear_SpriteBitmapOffsetY();
    InitFromConfig();
    TransferDataToWindow();
    return true;
}
