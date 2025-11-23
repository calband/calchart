/*
 * CalChartConfiguration.cpp
 * Dialog for debugging configuration
 */

/*
   Copyright (C) 2024  Richard Michael Powell

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

#include "ConfigurationDebugDialog.h"
#include "basic_ui.h"
#include <wx/config.h>
#include <wx/treelist.h>

namespace {

auto GetEntry(wxConfigBase& config, std::string const& entry) -> std::string
{
    switch (config.GetEntryType(entry)) {
    case wxConfigBase::Type_String: {
        auto def = wxString{};
        if (config.Read(entry, &def)) {
            return def.ToStdString();
        }
    } break;
    case wxConfigBase::Type_Boolean: {
        auto def = bool{};
        if (config.Read(entry, &def)) {
            return (def ? "true" : "false");
        }
    } break;
    case wxConfigBase::Type_Integer: {
        auto def = long{};
        if (config.Read(entry, &def)) {
            return std::to_string(def);
        }
    } break;
    case wxConfigBase::Type_Float: {
        auto def = double{};
        if (config.Read(entry, &def)) {
            return std::to_string(def);
        }
    } break;
    case wxConfigBase::Type_Unknown:
        return {};
    }
    return {};
}

struct RestorePath {
    static auto SetRoot(wxConfigBase* config) -> RestorePath
    {
        auto result = RestorePath(config);
        if (config != nullptr) {
            config->SetPath("/");
        }
        return result;
    }

    static auto Descend(wxConfigBase* config, wxString const& str) -> RestorePath
    {
        auto result = RestorePath(config);
        if (config != nullptr) {
            config->SetPath(config->GetPath() + "/" + str);
        }
        return result;
    }

    explicit RestorePath(wxConfigBase* config)
        : mConfig{ config }
        , mPath{ config != nullptr ? config->GetPath() : wxString{} }
    {
    }

    void swap(RestorePath& other) noexcept
    {
        using std::swap;
        swap(mConfig, other.mConfig);
        swap(mPath, other.mPath);
    }

    RestorePath(RestorePath const&) = delete;
    auto operator=(RestorePath const&) -> RestorePath& = delete;
    RestorePath(RestorePath&& other) noexcept
        : mConfig{ other.mConfig }
        , mPath{ std::move(other.mPath) }
    {
        other.mConfig = nullptr;
    }
    auto operator=(RestorePath&& arg) noexcept -> RestorePath&
    {
        RestorePath tmp{ std::move(arg) };
        swap(tmp);
        return *this;
    }

    ~RestorePath()
    {
        if (mConfig != nullptr) {
            mConfig->SetPath(mPath);
        }
    }

private:
    wxConfigBase* mConfig;
    wxString mPath{};
};

auto PrintEntry(std::ostream& os, wxConfigBase& config, std::string const& entry) -> std::ostream&
{
    return os << GetEntry(config, entry);
}

auto PrintConfigHelper(std::ostream& os, wxConfigBase& config, int depth) -> std::ostream&
{
    long entry = 0;
    wxString str{};
    auto result = config.GetFirstGroup(str, entry);
    while (result) {
        os << std::string(depth * 4, ' ') << str << "\n";
        {
            auto restore = RestorePath::Descend(&config, str);
            PrintConfigHelper(os, config, depth + 1);
        }
        result = config.GetNextGroup(str, entry);
    }
    result = config.GetFirstEntry(str, entry);
    while (result) {
        os << std::string(depth * 4 - 2, ' ') << str << " : ";
        PrintEntry(os, config, str.ToStdString());
        os << "\n";

        result = config.GetNextEntry(str, entry);
    }
    return os;
}

auto PrintConfig(std::ostream& os, wxConfigBase& config) -> std::ostream&
{
    auto restore = RestorePath::SetRoot(&config);
    return PrintConfigHelper(os, config, 0);
}

auto operator<<(std::ostream& os, wxConfigBase* config) -> std::ostream&
{
    if (config == nullptr) {
        return os;
    }
    return PrintConfig(os, *config);
}

void AddEntries(wxConfigBase& config, wxTreeListCtrl& ctrl, wxTreeListItem& root)
{
    long entry = 0;
    wxString str{};
    auto result = config.GetFirstEntry(str, entry);
    while (result) {
        auto item = ctrl.AppendItem(root, str);
        ctrl.SetItemText(item, 1, GetEntry(config, str.ToStdString()));
        result = config.GetNextEntry(str, entry);
    }
}

// we add groups
void AddGroups(wxConfigBase& config, wxTreeListCtrl& ctrl, wxTreeListItem& root)
{
    AddEntries(config, ctrl, root);
    long group = 0;
    wxString str{};
    auto result = config.GetFirstGroup(str, group);
    while (result) {
        auto item = ctrl.AppendItem(root, str);
        {
            auto restore = RestorePath::Descend(&config, str);
            AddGroups(config, ctrl, item);
        }
        result = config.GetNextGroup(str, group);
    }
}

void ConstructConfig(wxConfigBase* config, wxTreeListCtrl& ctrl)
{
    if (config == nullptr) {
        return;
    }

    auto restore = RestorePath::SetRoot(config);
    wxTreeListItem root = ctrl.GetRootItem();
    AddGroups(*config, ctrl, root);
}

}

ConfigurationDebug::ConfigurationDebug(wxWindow* parent, wxConfigBase* config)
    : ConfigurationDebug::super(parent, wxID_ANY, "Configuration Debug", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    , mConfig(config)
{
    std::cout << mConfig << "\n";
    wxUI::VSizer{
        BasicSizerFlags(),
        mTreeCtrl = wxUI::Generic{
            ExpandSizerFlags(),
            [this] {
                auto* tree = new wxTreeListCtrl(this, wxID_ANY, wxDefaultPosition, { 400, 400 });

                tree->AppendColumn("Key",
                    wxCOL_WIDTH_AUTOSIZE,
                    wxALIGN_LEFT,
                    wxCOL_RESIZABLE | wxCOL_SORTABLE);
                tree->AppendColumn("Value",
                    tree->WidthFor(std::string(40, ' ')),
                    wxALIGN_RIGHT,
                    wxCOL_RESIZABLE | wxCOL_SORTABLE);
                ConstructConfig(mConfig, *tree);
                return tree;
            }() },
        wxUI::HSizer{
            wxUI::Button{ wxID_OK },
        }
    }.fitTo(this);
}
