/*
 * ViewHandlers.cpp
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

#include "ViewHandlers.hpp"
#include "CalChartView.h"

auto CreateContinuityBrowserHandlers(CalChartView* view) -> ContinuityBrowser::Handlers
{
    if (!view) {
        return ContinuityBrowser::Handlers{};
    }
    return {
        [view]() { return CalChart::Ranges::ToVector<std::optional<CalChart::Continuity>>(
                       CalChart::Ranges::zip_view(view->ContinuitiesInUse(), view->GetContinuities())
                       | std::views::transform([](auto&& inUseAndCont) -> std::optional<CalChart::Continuity> {
                             auto&& [inUse, cont] = inUseAndCont;
                             if (inUse) {
                                 return cont;
                             } else {
                                 return std::nullopt;
                             }
                         })); },
        {
            [view](CalChart::SYMBOL_TYPE sym, CalChart::Continuity const& new_cont) {
                view->DoSetContinuityCommand(sym, new_cont);
            },
            [view](CalChart::SYMBOL_TYPE symbol) {
                view->SetSelectionList(view->MakeSelectBySymbol(symbol));
            },
        }
    };
}

auto GetDebugContinuityHandlers(
    CalChart::Continuity const& plainCont,
    CalChart::Continuity const& solCont,
    std::function<void(CalChart::SYMBOL_TYPE, CalChart::Continuity const&)> onUpdate,
    std::function<void(CalChart::SYMBOL_TYPE)> onSetSelection) -> ContinuityBrowser::Handlers
{
    return {
        [&]() {
            return CalChart::Ranges::ToVector<std::optional<CalChart::Continuity>>(
                CalChart::k_symbols | std::views::transform([&](auto eachcont) -> std::optional<CalChart::Continuity> {
                    if (eachcont == CalChart::SYMBOL_PLAIN) {
                        return plainCont;
                    }
                    if (eachcont == CalChart::SYMBOL_SOL) {
                        return solCont;
                    }
                    return std::nullopt;
                }));
        },
        {
            [onUpdate](CalChart::SYMBOL_TYPE sym, CalChart::Continuity const& new_cont) {
                onUpdate(sym, new_cont);
            },
            [onSetSelection](CalChart::SYMBOL_TYPE symbol) {
                onSetSelection(symbol);
            },
        }
    };
}

auto CreateFieldThumbnailBrowserHandlers(CalChartView* view) -> FieldThumbnailBrowser::Handlers
{
    if (!view) {
        return FieldThumbnailBrowser::Handlers{};
    }
    return {
        [view]() {
            return view->GetShowFullSize();
        },
        [view]() {
            return view->GetNumSheets();
        },
        [view]() {
            return view->GetCurrentSheetNum();
        },
        [view]() {
            return view->GetSheetsName();
        },
        [view]() {
            return view->GenerateFieldWithMarchersDrawCommands();
        },
        [view](size_t sheet_num) {
            view->GoToSheet(sheet_num);
        },
    };
}
