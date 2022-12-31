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
#include <wxUI/wxUI.h>

BEGIN_EVENT_TABLE(GeneralSetup, PreferencePage)
END_EVENT_TABLE()

IMPLEMENT_CLASS(GeneralSetup, PreferencePage)

void GeneralSetup::CreateControls()
{
    wxUI::VSizer{
        LeftBasicSizerFlags(),
        wxUI::VSizer{
            "General Settings",
            VLabelWidget("Autosave Interval", mAutoSave_Interval = wxUI::TextCtrl{}.withSize({ 100, -1 })),
            mBeep_On_Collisions = wxUI::CheckBox{ "Beep on animation collisions " },
            mScroll_Natural = wxUI::CheckBox{ "Scroll Direction: Natural" },
            mSetSheet_Undo = wxUI::CheckBox{ "Set Sheet is undo-able" },
            mSelection_Undo = wxUI::CheckBox{ "Point selection is undo-able" },
        },
    }
        .attachTo(this);

    TransferDataToWindow();
}

void GeneralSetup::InitFromConfig()
{
    *mAutoSave_Interval = std::to_string(mConfig.Get_AutosaveInterval());
    *mBeep_On_Collisions = mConfig.Get_BeepOnCollisions();
    *mScroll_Natural = mConfig.Get_ScrollDirectionNatural();
    *mSetSheet_Undo = mConfig.Get_CommandUndoSetSheet();
    *mSelection_Undo = mConfig.Get_CommandUndoSelection();
}

bool GeneralSetup::TransferDataToWindow()
{
    return true;
}

bool GeneralSetup::TransferDataFromWindow()
{
    // read out the values from the window
    mConfig.Set_AutosaveInterval(std::stol(*mAutoSave_Interval));
    mConfig.Set_BeepOnCollisions(*mBeep_On_Collisions);
    mConfig.Set_ScrollDirectionNatural(*mScroll_Natural);
    mConfig.Set_CommandUndoSetSheet(*mSetSheet_Undo);
    mConfig.Set_CommandUndoSelection(*mSelection_Undo);
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
