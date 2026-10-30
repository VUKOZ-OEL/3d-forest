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

/** @file WebControls.cpp */

// Include 3D Forest.
#include <CheckBox.hpp>
#include <ColorSwitch.hpp>
#include <ComboBox.hpp>
#include <DoubleRangeSlider.hpp>
#include <DoubleSpinBox.hpp>
#include <GroupBox.hpp>
#include <Label.hpp>
#include <LineEdit.hpp>
#include <ProgressBar.hpp>
#include <PushButton.hpp>
#include <RadioButton.hpp>
#include <Slider.hpp>
#include <SpinBox.hpp>
#include <Splitter.hpp>
#include <TableWidget.hpp>
#include <TextEdit.hpp>
#include <ToolBar.hpp>
#include <ToolButton.hpp>
#include <TreeWidget.hpp>
#include <Viewer.hpp>
#include <WebAssets.hpp>
#include <WebControls.hpp>

// Include std.
#include <algorithm>
#include <cmath>
#include <stdexcept>

// Include local.
#define LOG_MODULE_NAME "WebControls"
#include <Log.hpp>

namespace
{
using J = JsonCpp::Value;
int integer(const J &e, const char *key, int low, int high)
{
    if (!e[key].isInt() || e[key].asInt() < low || e[key].asInt() > high)
        throw std::invalid_argument(std::string("Invalid ") + key);
    return e[key].asInt();
}
double number(const J &e, const char *key, double low, double high)
{
    if (!e[key].isNumeric())
        throw std::invalid_argument("Expected number");
    double v = e[key].asDouble();
    if (!std::isfinite(v) || v < low || v > high)
        throw std::invalid_argument("Number outside range");
    return v;
}
bool boolean(const J &e, const char *key)
{
    if (!e[key].isBool())
        throw std::invalid_argument("Expected boolean");
    return e[key].asBool();
}
std::string string(const J &e)
{
    if (!e["text"].isString() || e["text"].asString().size() > 1024 * 1024)
        throw std::invalid_argument("Invalid text");
    return e["text"].asString();
}
std::string brushColor(const Brush &b)
{
    return b.hasColor()
               ? webColor(Color(b.red(), b.green(), b.blue(), b.alpha()))
               : std::string{};
}
J strings(const std::vector<std::string> &values)
{
    J a(JsonCpp::arrayValue);
    for (auto &s : values)
        a.append(s);
    return a;
}
J treeItems(TreeWidget *tree,
            const std::vector<std::unique_ptr<TreeWidgetItem>> &items,
            const std::string &prefix)
{
    J rows(JsonCpp::arrayValue);
    for (std::size_t i = 0; i < items.size(); ++i)
    {
        auto *item = items[i].get();
        J r(JsonCpp::objectValue);
        const auto path = prefix + std::to_string(i);
        r["path"] = path;
        r["selected"] = item->isSelected();
        r["editable"] = item->isEditable();
        r["cells"] = J(JsonCpp::arrayValue);
        for (int c = 0; c < tree->columnCount(); ++c)
        {
            J cell(JsonCpp::objectValue);
            cell["text"] = item->text(c);
            cell["check"] =
                item->isCheckable(c) ? int(item->checkState(c)) : -1;
            cell["background"] = brushColor(item->background(c));
            r["cells"].append(cell);
        }
        r["children"] = treeItems(tree, item->children(), path + "/");
        rows.append(r);
    }
    return rows;
}
TreeWidgetItem *treeItem(TreeWidget *tree, const std::string &path)
{
    if (path.empty() || path.size() > 1024)
        throw std::invalid_argument("Invalid tree path");
    TreeWidgetItem *item = nullptr;
    std::size_t start = 0;
    while (start < path.size())
    {
        auto end = path.find('/', start);
        if (end == std::string::npos)
            end = path.size();
        auto part = path.substr(start, end - start);
        if (part.empty() || part.size() > 9 ||
            part.find_first_not_of("0123456789") != std::string::npos)
            throw std::invalid_argument("Invalid tree path");
        int n = std::stoi(part);
        int count = item ? item->childCount() : tree->topLevelItemCount();
        if (n >= count)
            throw std::invalid_argument("Tree item removed");
        item = item ? item->child(n) : tree->topLevelItem(n);
        start = end + 1;
    }
    return item;
}
} // namespace

WebControls::WebControls(WebApplication &a, Widget *w, const std::string &type)
    : WebWidget(a, w, type)
{
}

void WebControls::bind()
{
    WebWidget::bind();
    const std::weak_ptr<WebWidget> weak = shared_from_this();
    auto changed = [weak]
    {
        if (auto base = weak.lock(); base && base->alive())
            ++static_cast<WebControls &>(*base).modelVersion_;
    };
    if (auto *table = dynamic_cast<TableWidget *>(widget_))
        table->tableReset.connect(changed);
    if (auto *tree = dynamic_cast<TreeWidget *>(widget_))
    {
        tree->clearing.connect(changed);
        tree->orderUpdated.connect(changed);
        tree->itemInserted.connect([changed](TreeWidgetItem *) { changed(); });
    }
}

std::string WebControls::typeOf(Widget *w)
{
#define TYPE(T)                                                                \
    if (dynamic_cast<T *>(w))                                                  \
    return #T
    TYPE(CheckBox);
    TYPE(ComboBox);
    TYPE(ColorSwitch);
    TYPE(Slider);
    TYPE(DoubleRangeSlider);
    TYPE(DoubleSpinBox);
    TYPE(SpinBox);
    TYPE(PushButton);
    TYPE(ToolButton);
    TYPE(RadioButton);
    TYPE(LineEdit);
    TYPE(TextEdit);
    TYPE(ProgressBar);
    TYPE(ToolBar);
    TYPE(Splitter);
    TYPE(TableWidget);
    TYPE(TreeWidget);
    TYPE(Label);
    TYPE(GroupBox);
    TYPE(Viewer);
#undef TYPE
    return "Widget";
}

void WebControls::sync(bool enabled, bool visible)
{
    WebWidget::sync(enabled, visible);
    if (!alive())
        return;
    J p(JsonCpp::objectValue);
    if (auto *w = dynamic_cast<CheckBox *>(widget_))
    {
        p["text"] = w->text();
        p["checked"] = w->isChecked();
    }
    else if (auto *w = dynamic_cast<RadioButton *>(widget_))
    {
        p["text"] = w->text();
        p["checked"] = w->isChecked();
    }
    else if (auto *w = dynamic_cast<ComboBox *>(widget_))
    {
        p["items"] = J(JsonCpp::arrayValue);
        for (int i = 0; i < w->count(); ++i)
            p["items"].append(w->itemText(i));
        p["value"] = w->currentIndex();
    }
    else if (auto *w = dynamic_cast<SpinBox *>(widget_))
    {
        p["minimum"] = w->minimum();
        p["maximum"] = w->maximum();
        p["value"] = w->value();
        p["singleStep"] = w->singleStep();
    }
    else if (auto *w = dynamic_cast<DoubleSpinBox *>(widget_))
    {
        p["minimum"] = w->minimum();
        p["maximum"] = w->maximum();
        p["value"] = w->value();
        p["singleStep"] = w->singleStep();
        p["decimals"] = 2;
    }
    else if (auto *w = dynamic_cast<Slider *>(widget_))
    {
        p["minimum"] = w->minimum();
        p["maximum"] = w->maximum();
        p["value"] = w->value();
        p["singleStep"] = w->singleStep();
        p["tickInterval"] = w->tickInterval();
        p["tickPosition"] = static_cast<int>(w->tickPosition());
        p["orientation"] = w->orientation();
    }
    else if (auto *w = dynamic_cast<DoubleRangeSlider *>(widget_))
    {
        p["minimum"] = w->minimum();
        p["maximum"] = w->maximum();
        p["low"] = w->minimumValue();
        p["high"] = w->maximumValue();
        p["singleStep"] = w->singleStep();
        p["orientation"] = w->orientation();
    }
    else if (auto *w = dynamic_cast<PushButton *>(widget_))
    {
        p["text"] = w->text();
        p["icon"] = webIcon(w->icon());
        p["primary"] = w->isPrimary();
    }
    else if (auto *w = dynamic_cast<ToolButton *>(widget_))
    {
        p["text"] = w->text();
        p["icon"] = webIcon(w->icon());
        p["toolTip"] = w->toolTip();
        p["buttonStyle"] = int(w->toolButtonStyle());
        p["autoRaise"] = w->autoRaise();
    }
    else if (auto *w = dynamic_cast<LineEdit *>(widget_))
    {
        p["text"] = w->text();
        p["readOnly"] = w->isReadOnly();
        p["placeholder"] = w->placeholderText();
    }
    else if (auto *w = dynamic_cast<TextEdit *>(widget_))
    {
        p["text"] = w->text();
        p["readOnly"] = w->isReadOnly();
    }
    else if (auto *w = dynamic_cast<ProgressBar *>(widget_))
    {
        p["minimum"] = w->minimum();
        p["maximum"] = w->maximum();
        p["value"] = w->value();
        p["textVisible"] = w->isTextVisible();
    }
    else if (auto *w = dynamic_cast<ColorSwitch *>(widget_))
    {
        p["foreground"] = webColor(w->foregroundColor());
        p["background"] = webColor(w->backgroundColor());
    }
    else if (auto *w = dynamic_cast<Splitter *>(widget_))
    {
        p["orientation"] = w->orientation();
        p["collapsible"] = w->childrenCollapsible();
        p["sizes"] = J(JsonCpp::arrayValue);
        for (int n : w->requestedSizes())
            p["sizes"].append(n);
    }
    else if (auto *w = dynamic_cast<ToolBar *>(widget_))
    {
        p["orientation"] = w->orientation();
        p["iconWidth"] = w->iconSize().width();
        p["iconHeight"] = w->iconSize().height();
    }
    else if (auto *w = dynamic_cast<Label *>(widget_))
    {
        p["icon"] = webIcon(w->icon());
    }
    else if (auto *w = dynamic_cast<TableWidget *>(widget_))
    {
        J model(JsonCpp::objectValue);
        model["rows"] = J(JsonCpp::arrayValue);
        model["headers"] = J(JsonCpp::arrayValue);
        for (int c = 0; c < w->columnCount(); ++c)
            model["headers"].append(w->horizontalHeaderItem(c)
                                        ? w->horizontalHeaderItem(c)->text()
                                        : "");
        for (int r = 0; r < w->rowCount(); ++r)
        {
            J row(JsonCpp::arrayValue);
            for (int c = 0; c < w->columnCount(); ++c)
            {
                J cell(JsonCpp::objectValue);
                auto *item = w->item(r, c);
                cell["text"] = item ? item->text() : "";
                cell["flags"] = item ? item->flags() : 0;
                cell["check"] = item && item->hasCheckState()
                                    ? int(item->checkState())
                                    : -1;
                cell["background"] = item ? brushColor(item->background()) : "";
                row.append(cell);
            }
            model["rows"].append(row);
        }
        if (model != model_)
        {
            model_ = model;
            ++modelVersion_;
        }
        p["model"] = model_;
        p["modelVersion"] = modelVersion_;
        p["selection"] = J(JsonCpp::arrayValue);
        for (auto cell : w->selectedCells())
        {
            J pair(JsonCpp::arrayValue);
            pair.append(cell.first);
            pair.append(cell.second);
            p["selection"].append(pair);
        }
        p["columnWidths"] = J(JsonCpp::objectValue);
        for (auto kv : w->columnWidths())
            p["columnWidths"][std::to_string(kv.first)] = kv.second;
        p["headerVisible"] = w->horizontalHeader()->isVisible();
        p["rowHeaderVisible"] = w->verticalHeader()->isVisible();
        p["defaultColumnWidth"] = w->horizontalHeader()->defaultSectionSize();
        p["rowHeight"] = w->verticalHeader()->defaultSectionSize();
        p["stretchLast"] = w->horizontalHeader()->stretchLastSection();
        p["alternating"] = w->alternatingRowColors();
        p["selectionMode"] = w->selectionMode();
        p["selectionBehavior"] = w->selectionBehavior();
        p["sorting"] = w->isSortingEnabled();
        p["sortColumn"] = w->sortColumn();
        p["sortOrder"] = int(w->sortOrder());
    }
    else if (auto *w = dynamic_cast<TreeWidget *>(widget_))
    {
        J model = treeItems(w, w->items(), "");
        // Selection is separate from the revision that protects item addresses.
        J comparable = model;
        std::function<void(J &)> strip = [&](J &rows)
        {
            for (auto &row : rows)
            {
                row.removeMember("selected");
                strip(row["children"]);
            }
        };
        strip(comparable);
        if (comparable != model_)
        {
            model_ = comparable;
            ++modelVersion_;
        }
        p["rows"] = model;
        p["modelVersion"] = modelVersion_;
        p["headers"] = strings(w->headerLabels());
        p["columns"] = w->columnCount();
        p["selectionMode"] = w->selectionMode();
        p["selectionBehavior"] = w->selectionBehavior();
        p["sorting"] = w->isSortingEnabled();
        p["sortColumn"] = w->sortColumn();
        p["sortOrder"] = int(w->sortOrder());
        p["fitRowsToContents"] = w->fitRowsToContents();
    }
    patch(p);
}

void WebControls::handleEvent(const J &e)
{
    const auto name = e["event"].asString();
    const auto life = widgetLife_;
    if (auto *w = dynamic_cast<CheckBox *>(widget_))
    {
        if (name == "toggled")
        {
            w->setChecked(boolean(e, "checked"), !w->signalsBlocked());
            return;
        }
    }
    else if (auto *w = dynamic_cast<RadioButton *>(widget_))
    {
        if (name == "toggled")
        {
            if (boolean(e, "checked"))
                w->setChecked(true, true);
            return;
        }
    }
    else if (auto *w = dynamic_cast<ComboBox *>(widget_))
    {
        if (name == "activated")
        {
            w->setValue(integer(e, "value", 0, w->count() - 1),
                        !w->signalsBlocked());
            return;
        }
    }
    else if (auto *w = dynamic_cast<PushButton *>(widget_))
    {
        if (name == "clicked")
        {
            w->click();
            return;
        }
    }
    else if (auto *w = dynamic_cast<ToolButton *>(widget_))
    {
        if (name == "clicked")
        {
            if (!w->signalsBlocked())
                w->clicked();
            return;
        }
    }
    else if (auto *w = dynamic_cast<SpinBox *>(widget_))
    {
        if (name == "valueChanged" || name == "editingFinished")
        {
            w->setValue(integer(e, "value", w->minimum(), w->maximum()), true);
            if (name == "editingFinished" && !life.expired() &&
                !w->signalsBlocked())
                w->editingFinished();
            return;
        }
    }
    else if (auto *w = dynamic_cast<DoubleSpinBox *>(widget_))
    {
        if (name == "valueChanged" || name == "editingFinished")
        {
            w->setValue(number(e, "value", w->minimum(), w->maximum()), true);
            if (name == "editingFinished" && !life.expired() &&
                !w->signalsBlocked())
                w->editingFinished();
            return;
        }
    }
    else if (auto *w = dynamic_cast<Slider *>(widget_))
    {
        if (name == "valueChanged" || name == "sliderReleased")
        {
            const int value = integer(e, "value", w->minimum(), w->maximum());
            w->setValue(value, true);
            if (name == "sliderReleased" && !life.expired() &&
                !w->signalsBlocked())
                w->sliderReleased();
            return;
        }
    }
    else if (auto *w = dynamic_cast<DoubleRangeSlider *>(widget_))
    {
        if (name == "valuesChanged" || name == "sliderReleased")
        {
            double lo = number(e, "low", w->minimum(), w->maximum()),
                   hi = number(e, "high", lo, w->maximum());
            w->setValues(lo, hi, true);
            if (name == "sliderReleased" && !life.expired() &&
                !w->signalsBlocked())
                w->sliderReleased();
            return;
        }
    }
    else if (auto *w = dynamic_cast<LineEdit *>(widget_))
    {
        if (name == "textChanged" || name == "editingFinished" ||
            name == "returnPressed")
        {
            if (w->isReadOnly())
                throw std::invalid_argument("Read-only field");
            w->setText(string(e), true);
            if (!life.expired() && !w->signalsBlocked())
            {
                if (name == "editingFinished")
                    w->editingFinished();
                if (name == "returnPressed")
                    w->returnPressed();
            }
            return;
        }
    }
    else if (auto *w = dynamic_cast<TextEdit *>(widget_))
    {
        if (name == "textChanged")
        {
            if (w->isReadOnly())
                throw std::invalid_argument("Read-only text");
            w->setText(string(e), true);
            return;
        }
    }
    else if (auto *w = dynamic_cast<ColorSwitch *>(widget_))
    {
        if (name == "colorChanged")
        {
            if (!e["foreground"].isArray() || e["foreground"].size() != 4 ||
                !e["background"].isArray() || e["background"].size() != 4)
                throw std::invalid_argument("Invalid colors");
            auto color = [](const J &a)
            {
                for (const auto &v : a)
                    if (!v.isInt() || v.asInt() < 0 || v.asInt() > 255)
                        throw std::invalid_argument("Invalid channel");
                return Color(a[0].asInt(),
                             a[1].asInt(),
                             a[2].asInt(),
                             a[3].asInt());
            };
            const Color fg = color(e["foreground"]),
                        bg = color(e["background"]);
            w->setForegroundColor(fg);
            w->setBackgroundColor(bg);
            if (!w->signalsBlocked())
                w->colorChanged();
            return;
        }
    }
    else if (auto *w = dynamic_cast<Splitter *>(widget_))
    {
        if (name == "splitterMoved")
        {
            if (!e["sizes"].isArray() || int(e["sizes"].size()) != w->count())
                throw std::invalid_argument("Invalid splitter sizes");
            std::vector<int> sizes;
            for (auto v : e["sizes"])
            {
                if (!v.isInt() || v.asInt() < 0 || v.asInt() > 100000)
                    throw std::invalid_argument("Invalid size");
                sizes.push_back(v.asInt());
            }
            int index = integer(e, "index", 1, w->count() - 1),
                pos = integer(e, "position", 0, 100000);
            w->setSizes(sizes);
            if (!life.expired() && !w->signalsBlocked())
                w->splitterMoved(pos, index);
            return;
        }
    }
    else if (auto *w = dynamic_cast<TableWidget *>(widget_))
    {
        if (!e["modelVersion"].isUInt() ||
            e["modelVersion"].asUInt() != modelVersion_)
            throw std::invalid_argument(
                "Table changed; retry with current rows");
        if (name == "sort")
        {
            if (!w->isSortingEnabled())
                throw std::invalid_argument("Sorting disabled");
            w->sortItems(integer(e, "column", 0, w->columnCount() - 1),
                         Ui::SortOrder(integer(e, "order", 0, 1)));
            return;
        }
        if (name == "columnWidth")
        {
            w->setColumnWidth(integer(e, "column", 0, w->columnCount() - 1),
                              integer(e, "width", 16, 10000));
            return;
        }
        if (name == "selection")
        {
            if (e.isMember("all") && !e["all"].isBool())
                throw std::invalid_argument("Invalid selection mode");
            TableWidget::Selection cells;
            auto add = [&](int r, int c)
            {
                auto *item = w->item(r, c);
                if (item && (item->flags() & Ui::ItemIsSelectable) &&
                    (item->flags() & Ui::ItemIsEnabled))
                    cells.emplace(r, c);
            };
            if (e.get("all", false).asBool())
                for (int r = 0; r < w->rowCount(); ++r)
                    for (int c = 0; c < w->columnCount(); ++c)
                        add(r, c);
            else if (e["rows"].isArray() &&
                     w->selectionBehavior() == AbstractItemView::SelectRows)
                for (auto &row : e["rows"])
                {
                    if (!row.isInt() || row.asInt() < 0 ||
                        row.asInt() >= w->rowCount())
                        throw std::invalid_argument("Invalid row");
                    for (int c = 0; c < w->columnCount(); ++c)
                        add(row.asInt(), c);
                }
            else if (e["columns"].isArray() &&
                     w->selectionBehavior() == AbstractItemView::SelectColumns)
                for (auto &column : e["columns"])
                {
                    if (!column.isInt() || column.asInt() < 0 ||
                        column.asInt() >= w->columnCount())
                        throw std::invalid_argument("Invalid column");
                    for (int r = 0; r < w->rowCount(); ++r)
                        add(r, column.asInt());
                }
            else if (!e["cells"].isArray() || e["cells"].size() > 1000000)
                throw std::invalid_argument("Invalid selection");
            if (e["cells"].isArray())
                for (auto &pair : e["cells"])
                {
                    if (!pair.isArray() || pair.size() != 2 ||
                        !pair[0].isInt() || !pair[1].isInt())
                        throw std::invalid_argument("Invalid cell");
                    int r = pair[0].asInt(), c = pair[1].asInt();
                    auto *item = w->item(r, c);
                    if (!item || !(item->flags() & Ui::ItemIsSelectable) ||
                        !(item->flags() & Ui::ItemIsEnabled))
                        continue;
                    cells.emplace(r, c);
                }
            if (w->selectionMode() == AbstractItemView::NoSelection)
                cells.clear();
            if (w->selectionMode() == AbstractItemView::SingleSelection &&
                !cells.empty())
            {
                auto first = *cells.begin();
                for (auto it = cells.begin(); it != cells.end();)
                    if ((w->selectionBehavior() == AbstractItemView::SelectRows
                             ? it->first != first.first
                         : w->selectionBehavior() ==
                                 AbstractItemView::SelectColumns
                             ? it->second != first.second
                             : *it != first))
                        it = cells.erase(it);
                    else
                        ++it;
            }
            w->setSelectedCells(cells, true);
            return;
        }
        int r = integer(e, "row", 0, w->rowCount() - 1),
            c = integer(e, "column", 0, w->columnCount() - 1);
        auto *item = w->item(r, c);
        if (!item || !(item->flags() & Ui::ItemIsEnabled))
            throw std::invalid_argument("Disabled cell");
        if (name == "itemClicked")
        {
            if (!w->signalsBlocked())
                w->itemClicked(item, c);
            return;
        }
        if (name == "edit")
        {
            if (!(item->flags() & Ui::ItemIsEditable))
                throw std::invalid_argument("Read-only cell");
            item->setText(string(e), true);
            return;
        }
        if (name == "check")
        {
            if (!(item->flags() & Ui::ItemIsUserCheckable))
                throw std::invalid_argument("Cell has no checkbox");
            item->setCheckState(Ui::CheckState(integer(e, "state", 0, 2)),
                                true);
            return;
        }
    }
    else if (auto *w = dynamic_cast<TreeWidget *>(widget_))
    {
        if (!e["modelVersion"].isUInt() ||
            e["modelVersion"].asUInt() != modelVersion_)
            throw std::invalid_argument(
                "Tree changed; retry with current items");
        if (name == "sort")
        {
            if (!w->isSortingEnabled())
                throw std::invalid_argument("Sorting disabled");
            w->sortItems(integer(e, "column", 0, w->columnCount() - 1),
                         Ui::SortOrder(integer(e, "order", 0, 1)));
            return;
        }
        if (name == "selection")
        {
            if (!e["paths"].isArray() || e["paths"].size() > 100000)
                throw std::invalid_argument("Invalid selection");
            std::vector<TreeWidgetItem *> items;
            for (auto &path : e["paths"])
            {
                if (!path.isString())
                    throw std::invalid_argument("Invalid path");
                items.push_back(treeItem(w, path.asString()));
            }
            if (w->selectionMode() == AbstractItemView::NoSelection)
                items.clear();
            if (w->selectionMode() == AbstractItemView::SingleSelection &&
                items.size() > 1)
                items.resize(1);
            w->setSelectedItems(items, true);
            return;
        }
        if (!e["path"].isString())
            throw std::invalid_argument("Missing tree path");
        auto *item = treeItem(w, e["path"].asString());
        int c = integer(e, "column", 0, w->columnCount() - 1);
        if (name == "itemClicked")
        {
            if (!w->signalsBlocked())
                w->itemClicked(item, c);
            return;
        }
        if (name == "itemDoubleClicked")
        {
            if (!w->signalsBlocked())
                w->itemDoubleClicked(item, c);
            return;
        }
        if (name == "edit")
        {
            if (!item->isEditable())
                throw std::invalid_argument("Read-only item");
            item->setText(c, string(e), true);
            return;
        }
        if (name == "check")
        {
            if (!item->isCheckable(c))
                throw std::invalid_argument("Item has no checkbox");
            item->setCheckState(c, integer(e, "state", 0, 2), true);
            return;
        }
    }
    throw std::invalid_argument("Unknown control event");
}
