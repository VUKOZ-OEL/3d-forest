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

/** @file WebFileDialog.cpp */

// Include 3D Forest.
#include <ComboBox.hpp>
#include <Dialog.hpp>
#include <HBoxLayout.hpp>
#include <Label.hpp>
#include <LineEdit.hpp>
#include <MessageBox.hpp>
#include <PushButton.hpp>
#include <TreeWidget.hpp>
#include <VBoxLayout.hpp>
#include <WebApplication.hpp>
#include <WebFileDialog.hpp>

// Include std.
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <map>
#include <sstream>

// Include local.
#define LOG_MODULE_NAME "WebFileDialog"
#include <Log.hpp>

namespace fs = std::filesystem;

namespace
{
bool wildcard(const std::string &pattern, const std::string &text)
{
    std::size_t p = 0, t = 0, star = std::string::npos, mark = 0;
    while (t < text.size())
    {
        if (p < pattern.size() &&
            (pattern[p] == '?' || std::tolower((unsigned char)pattern[p]) ==
                                      std::tolower((unsigned char)text[t])))
        {
            ++p;
            ++t;
        }
        else if (p < pattern.size() && pattern[p] == '*')
        {
            star = p++;
            mark = t;
        }
        else if (star != std::string::npos)
        {
            p = star + 1;
            t = ++mark;
        }
        else
            return false;
    }
    while (p < pattern.size() && pattern[p] == '*')
        ++p;
    return p == pattern.size();
}
class Picker : public Dialog
{
public:
    Picker(WebApplication &app,
           const std::string &title,
           const std::string &filter,
           bool multiple,
           bool save,
           const std::string &initial,
           const std::string &selectedFilter,
           int options)
        : Dialog(&app),
          app_(app),
          multiple_(multiple),
          save_(save),
          options_(options)
    {
        setWindowTitle(title.empty() ? (save ? "Save file" : "Open file")
                                     : title);
        setModal(true);
        setStandardButtons((save ? Save : Open) | Cancel);
        setDefaultButton(save ? Save : Open);
        auto *layout = new VBoxLayout;
        setLayout(layout);
        auto *row = new HBoxLayout;
        auto *up = new PushButton("Up");
        auto *folder = new PushButton("Open folder");
        path_ = new LineEdit;
        row->addWidget(up);
        row->addWidget(path_, 1);
        row->addWidget(folder);
        layout->addWidget(new Label("Server folder"));
        layout->addLayout(row);
        tree_ = new TreeWidget;
        tree_->setColumnCount(2);
        tree_->setHeaderLabels({"Name", "Type"});
        tree_->setSelectionMode(multiple ? AbstractItemView::ExtendedSelection
                                         : AbstractItemView::SingleSelection);
        layout->addWidget(tree_, 1);
        name_ = new LineEdit;
        layout->addWidget(
            new Label(save ? "File name" : "Selected file / path"));
        layout->addWidget(name_);
        filter_ = new ComboBox;
        std::size_t pos = 0;
        while (pos < filter.size())
        {
            auto end = filter.find(";;", pos);
            if (end == std::string::npos)
                end = filter.size();
            filter_->addItem(filter.substr(pos, end - pos));
            pos = end + 2;
        }
        if (!filter_->count())
            filter_->addItem("All files (*)");
        for (int i = 0; i < filter_->count(); ++i)
            if (filter_->itemText(i) == selectedFilter)
                filter_->setValue(i, false);
        layout->addWidget(filter_);
        error_ = new Label;
        layout->addWidget(error_);
        up->clicked.connect([this] { navigate(current_.parent_path()); });
        folder->clicked.connect(
            [this]
            {
                auto items = tree_->selectedItems();
                if (!items.empty() &&
                    fs::is_directory(paths_.at(items.front())))
                    navigate(paths_.at(items.front()));
            });
        path_->returnPressed.connect([this]
                                     { navigate(fs::u8path(path_->text())); });
        name_->returnPressed.connect([this] { accept(); });
        filter_->activated.connect([this](int) { navigate(current_); });
        tree_->itemClicked.connect(
            [this](TreeWidgetItem *item, int)
            {
                auto p = paths_.at(item);
                name_->setText(p.filename().u8string());
            });
        auto start = initial.empty() ? fs::current_path()
                                     : fs::absolute(fs::u8path(initial));
        if (fs::is_directory(start))
            navigate(start);
        else
        {
            navigate(start.parent_path());
            name_->setText(start.filename().u8string());
        }
    }
    Size sizeHint() const override { return Size(700, 500); }
    std::vector<std::string> resultPaths;
    std::string selectedFilter() const { return filter_->currentText(); }
    void accept() override
    {
        try
        {
            std::vector<fs::path> values;
            if (multiple_ && tree_->selectedItems().size() > 1)
                for (auto *item : tree_->selectedItems())
                    values.push_back(paths_.at(item));
            else if (!name_->text().empty())
            {
                auto p = fs::u8path(name_->text());
                values.push_back(p.is_absolute() ? p : current_ / p);
            }
            if (values.empty())
                return;
            if (values.size() == 1 && fs::is_directory(values.front()))
            {
                navigate(values.front());
                return;
            }
            for (auto &p : values)
            {
                p = fs::absolute(p).lexically_normal();
                if (!save_ && !fs::is_regular_file(p))
                    throw std::runtime_error("Select an existing file.");
                if (save_ && !fs::is_directory(p.parent_path()))
                    throw std::runtime_error(
                        "The destination folder does not exist.");
                if (save_ && fs::exists(p) &&
                    !(options_ & Ui::DontConfirmOverwrite))
                    if (MessageBox::question(
                            &app_,
                            "Replace file?",
                            "A file with that name already exists. Replace it?",
                            Yes | No,
                            No) != Yes)
                        return;
            }
            resultPaths.clear();
            for (auto &p : values)
                resultPaths.push_back(p.u8string());
            Dialog::accept();
        }
        catch (const std::exception &e)
        {
            error_->setText(e.what());
        }
    }

private:
    WebApplication &app_;
    bool multiple_, save_;
    int options_;
    fs::path current_;
    LineEdit *path_, *name_;
    TreeWidget *tree_;
    ComboBox *filter_;
    Label *error_;
    std::map<TreeWidgetItem *, fs::path> paths_;
    void navigate(const fs::path &path)
    {
        try
        {
            auto next = fs::canonical(path);
            std::vector<fs::directory_entry> entries;
            for (auto &entry : fs::directory_iterator(next))
                entries.push_back(entry);
            std::sort(entries.begin(),
                      entries.end(),
                      [](const auto &a, const auto &b)
                      {
                          if (a.is_directory() != b.is_directory())
                              return a.is_directory();
                          return a.path().filename() < b.path().filename();
                      });
            current_ = next;
            path_->setText(current_.u8string());
            name_->setText("");
            tree_->clear();
            paths_.clear();
            error_->setText("");
            auto f = filter_->currentText();
            // auto begin = f.find('('), end = f.rfind(')');
            auto begin = f.rfind('('), end = f.rfind(')');
            std::vector<std::string> patterns;
            if (begin != std::string::npos && end > begin)
            {
                std::istringstream in(f.substr(begin + 1, end - begin - 1));
                for (std::string s; in >> s;)
                    patterns.push_back(s);
            }
            for (auto &entry : entries)
            {
                auto name = entry.path().filename().u8string();
                bool directory = entry.is_directory();
                if (!directory && !patterns.empty() &&
                    !std::any_of(patterns.begin(),
                                 patterns.end(),
                                 [&](const auto &p)
                                 { return wildcard(p, name); }))
                    continue;
                auto *item = tree_->push_back(
                    TreeWidgetItem({name, directory ? "Folder" : "File"}));
                paths_[item] = entry.path();
            }
        }
        catch (const std::exception &e)
        {
            error_->setText(e.what());
        }
    }
};
} // namespace
std::vector<std::string> WebFileDialog::choose(WebApplication &app,
                                               const std::string &title,
                                               const std::string &filter,
                                               bool multiple,
                                               bool save,
                                               const std::string &initial,
                                               std::string *selectedFilter,
                                               int options)
{
    Picker dialog(app,
                  title,
                  filter,
                  multiple,
                  save,
                  initial,
                  selectedFilter ? *selectedFilter : std::string{},
                  options);
    if (dialog.exec() != Dialog::Accepted)
        return {};
    if (selectedFilter)
        *selectedFilter = dialog.selectedFilter();
    return dialog.resultPaths;
}
