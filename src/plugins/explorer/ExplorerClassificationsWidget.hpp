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

/** @file ExplorerClassificationsWidget.hpp */

#ifndef EXPLORER_CLASSIFICATIONS_WIDGET_HPP
#define EXPLORER_CLASSIFICATIONS_WIDGET_HPP

#include <Classifications.hpp>
#include <Editor.hpp>
#include <ExplorerWidgetInterface.hpp>
class MainWindow;

class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;

/** Explorer Classifications Widget. */
class ExplorerClassificationsWidget : public ExplorerWidgetInterface
{
    Q_OBJECT

public:
    ExplorerClassificationsWidget(MainWindow *mainWindow,
                                  const QIcon &icon,
                                  const QString &text);

    virtual bool hasColorSource() const { return true; }
    virtual SettingsView::ColorSource colorSource() const
    {
        return SettingsView::COLOR_SOURCE_CLASSIFICATION;
    }

    virtual bool hasFilter() const { return true; }
    virtual bool isFilterEnabled() const;
    virtual void setFilterEnabled(bool b);

public slots:
    void slotUpdate(void *sender, const QSet<Editor::Type> &target);

    void slotShow();
    void slotHide();
    void slotSelectAll();
    void slotSelectInvert();
    void slotSelectNone();

    void slotItemSelectionChanged();
    void slotItemChanged(QTreeWidgetItem *item, int column);

protected:
    /** Explorer Classifications Column. */
    enum Column
    {
        COLUMN_CHECKED,
        COLUMN_ID,
        COLUMN_LABEL,
        COLUMN_LAST,
    };

    QTreeWidget *tree_;
    QToolButton *showButton_;
    QToolButton *hideButton_;
    QToolButton *selectAllButton_;
    QToolButton *selectInvertButton_;
    QToolButton *selectNoneButton_;
    Classifications classifications_;
    QueryFilterSet filter_;
    bool updatesEnabled_;

    void setClassifications(const Classifications &classifications,
                            const QueryFilterSet &filter);
    void dataChanged();
    void filterChanged();

    size_t identifier(const QTreeWidgetItem *item);
    void updateTree();
    void block();
    void unblock();
    void addTreeItem(size_t index);
};

#endif /* EXPLORER_CLASSIFICATIONS_WIDGET_HPP */
