/*
    Copyright 2020 VUKOZ

    This file is part of 3D Forest.

    3D Forest is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    3D Forest is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with 3D Forest.  If not, see <https://www.gnu.org/licenses/>.
*/

/** @file TableWidgetItem.hpp */

#ifndef TABLE_WIDGET_ITEM_HPP
#define TABLE_WIDGET_ITEM_HPP

// Include 3D Forest.
#include <Brush.hpp>
#include <Color.hpp>
#include <Ui.hpp>
#include <Util.hpp>
class TableWidget;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** TableWidgetItem. */
class EXPORT_UI_COMMON TableWidgetItem
{
public:
    explicit TableWidgetItem(const std::string &text = "");

    TableWidgetItem(const TableWidgetItem &other);

    TableWidgetItem &operator=(const TableWidgetItem &) = delete;

    const std::string &text() const { return text_; }
    void setText(const std::string &text, bool notify = false);

    bool isNumeric() const { return numeric_; }
    void setNumeric(bool numeric);

    int flags() const { return flags_; }
    void setFlags(int flags);

    bool hasCheckState() const { return checkState_ >= 0; }
    Ui::CheckState checkState() const;
    void setCheckState(Ui::CheckState state, bool notify = false);

    const Brush &background() const { return background_; }
    void setBackground(const Brush &background);

    int row() const { return row_; }
    int column() const { return column_; }

private:
    friend class TableWidget;

    void changed(bool notify);

    std::string text_;
    bool numeric_{false};

    int flags_{Ui::ItemIsSelectable | Ui::ItemIsEditable | Ui::ItemIsEnabled};

    int checkState_{-1};
    Brush background_;

    TableWidget *table_{nullptr};
    int row_{-1};
    int column_{-1};
};

#include <WarningsEnable.hpp>

#endif /* TABLE_WIDGET_ITEM_HPP */
