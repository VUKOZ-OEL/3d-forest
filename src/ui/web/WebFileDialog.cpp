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
#include <ThemeIcon.hpp>
#include <TreeWidget.hpp>
#include <VBoxLayout.hpp>
#include <WebApplication.hpp>
#include <WebFileDialog.hpp>

// Include std.
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <map>
#include <sstream>

// Include local.
#define LOG_MODULE_NAME "WebFileDialog"
#include <Log.hpp>

namespace fs = std::filesystem;

namespace
{
fs::path projectsDirectory()
{
    auto path = fs::current_path() / "projects";
    fs::create_directories(path);
    return fs::canonical(path);
}

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

std::string modifiedTime(const fs::file_time_type &time)
{
    const auto systemTime = std::chrono::time_point_cast<
        std::chrono::system_clock::duration>(
        time - fs::file_time_type::clock::now() +
        std::chrono::system_clock::now());
    const std::time_t value = std::chrono::system_clock::to_time_t(systemTime);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &value);
#else
    localtime_r(&value, &local);
#endif
    std::ostringstream out;
    out << std::put_time(&local, "%Y-%m-%d %H:%M");
    return out.str();
}

std::string fileSize(std::uintmax_t bytes)
{
    std::ostringstream out;
    if (bytes < 1024)
        out << bytes << " B";
    else if (bytes < 1024 * 1024)
        out << std::fixed << std::setprecision(1)
            << (double(bytes) / 1024.0) << " KB";
    else
        out << std::fixed << std::setprecision(1)
            << (double(bytes) / (1024.0 * 1024.0)) << " MB";
    return out.str();
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
          options_(options),
          root_(projectsDirectory())
    {
        setWindowTitle(title.empty() ? (save ? "Save file" : "Open file")
                                     : title);
        setModal(true);
        setStandardButtons((save ? Save : Open) | Cancel);
        setDefaultButton(save ? Save : Open);
        auto *layout = new VBoxLayout;
        setLayout(layout);
        auto *row = new HBoxLayout;
        back_ = new PushButton("Back");
        forward_ = new PushButton("Forward");
        up_ = new PushButton("Up");
        auto *refresh = new PushButton("Refresh");
        back_->setToolTip("Back");
        forward_->setToolTip("Forward");
        up_->setToolTip("Up one folder");
        refresh->setToolTip("Refresh");
        back_->setIcon(ThemeIcon(&app, ":/ApplicationResources/misc", "back"));
        forward_->setIcon(
            ThemeIcon(&app, ":/ApplicationResources/misc", "forward"));
        up_->setIcon(ThemeIcon(&app, ":/ApplicationResources/misc", "up"));
        refresh->setIcon(
            ThemeIcon(&app, ":/ApplicationResources/misc", "refresh"));
        path_ = new LineEdit;
        path_->setPlaceholderText("Folder path");
        search_ = new LineEdit;
        search_->setPlaceholderText("Search projects");
        search_->setEnabled(false);
        searchButton_ = new PushButton("Search");
        searchButton_->setIcon(
            ThemeIcon(&app, ":/ApplicationResources/misc", "search"));
        searchButton_->setEnabled(false);
        row->addWidget(back_);
        row->addWidget(forward_);
        row->addWidget(up_);
        row->addWidget(path_, 3);
        row->addWidget(refresh);
        row->addWidget(searchButton_);
        row->addWidget(search_, 2);
        layout->addLayout(row);
        tree_ = new TreeWidget;
        tree_->setColumnCount(5);
        tree_->setHeaderLabels(
            {"Name", "Date modified", "Type", "Size", "Tags"});
        tree_->setSortingEnabled(true);
        tree_->sortItems(2, Ui::AscendingOrder);
        tree_->setSelectionMode(multiple ? AbstractItemView::ExtendedSelection
                                         : AbstractItemView::SingleSelection);
        layout->addWidget(tree_, 1);
        name_ = new LineEdit;
        auto *fileRow = new HBoxLayout;
        fileRow->addWidget(new Label("File name:"));
        fileRow->addWidget(name_, 1);
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
        fileRow->addWidget(filter_);
        layout->addLayout(fileRow);
        error_ = new Label;
        layout->addWidget(error_);
        back_->clicked.connect([this] { navigateHistory(-1); });
        forward_->clicked.connect([this] { navigateHistory(1); });
        up_->clicked.connect([this] { navigate(current_.parent_path()); });
        refresh->clicked.connect([this] { navigate(current_); });
        path_->returnPressed.connect([this]
                                     { navigate(fs::u8path(path_->text())); });
        search_->textChanged.connect([this](const std::string &)
                                     { populateDirectory(); });
        searchButton_->clicked.connect([this] { populateDirectory(); });
        search_->returnPressed.connect([this] { populateDirectory(); });
        name_->returnPressed.connect([this] { accept(); });
        filter_->activated.connect([this](int) { navigate(current_); });
        tree_->itemClicked.connect(
            [this](TreeWidgetItem *item, int)
            {
                auto p = paths_.at(item);
                name_->setText(p.filename().u8string());
            });
        tree_->itemDoubleClicked.connect(
            [this](TreeWidgetItem *item, int)
            {
                const auto p = paths_.at(item);
                if (fs::is_directory(p))
                    navigate(p);
                else
                {
                    name_->setText(p.filename().u8string());
                    accept();
                }
            });
        auto start = initial.empty() ? root_ : fs::absolute(fs::u8path(initial));
        if (!isWithinRoot(fs::is_directory(start) ? start
                                                  : start.parent_path()))
            start = root_;
        if (fs::is_directory(start))
            navigate(start);
        else
        {
            navigate(start.parent_path());
            name_->setText(start.filename().u8string());
        }
    }
    Size sizeHint() const override { return Size(840, 560); }
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
                const auto checkedPath =
                    save_ && !fs::exists(p) ? p.parent_path() : p;
                if (!isWithinRoot(checkedPath))
                    throw std::runtime_error(
                        "Select a file inside the projects folder.");
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
    fs::path root_;
    fs::path current_;
    std::vector<fs::path> history_;
    std::size_t historyIndex_{0};
    PushButton *back_{nullptr}, *forward_{nullptr}, *up_{nullptr},
        *searchButton_{nullptr};
    LineEdit *path_, *search_, *name_;
    TreeWidget *tree_;
    ComboBox *filter_;
    Label *error_;
    std::map<TreeWidgetItem *, fs::path> paths_;
    void navigateHistory(int offset)
    {
        const auto next = static_cast<long long>(historyIndex_) + offset;
        if (next < 0 || next >= static_cast<long long>(history_.size()))
            return;
        historyIndex_ = static_cast<std::size_t>(next);
        navigate(history_[historyIndex_], false);
    }
    bool isWithinRoot(const fs::path &path) const
    {
        try
        {
            const auto canonical = fs::canonical(path);
            const auto relative = canonical.lexically_relative(root_);
            return !relative.empty() && *relative.begin() != ".." &&
                   !relative.is_absolute();
        }
        catch (...)
        {
            return false;
        }
    }
    void navigate(const fs::path &path, bool addHistory = true)
    {
        try
        {
            auto requested = path;
            if (requested.is_relative())
            {
                auto first = requested.begin();
                if (first != requested.end() && *first == "projects")
                    requested = requested.lexically_relative("projects");
                requested = root_ / requested;
            }
            auto next = fs::canonical(requested);
            if (!isWithinRoot(next))
                throw std::runtime_error(
                    "Navigation is limited to the projects folder.");
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
            const bool changed = current_ != next;
            current_ = next;
            const auto relative = current_.lexically_relative(root_);
            path_->setText(relative == "."
                               ? "projects"
                               : (fs::path("projects") / relative).u8string());
            if (changed)
                search_->clear();
            if (addHistory &&
                (history_.empty() || history_[historyIndex_] != current_))
            {
                history_.resize(historyIndex_ + (history_.empty() ? 0 : 1));
                history_.push_back(current_);
                historyIndex_ = history_.size() - 1;
            }
            back_->setEnabled(historyIndex_ > 0);
            forward_->setEnabled(historyIndex_ + 1 < history_.size());
            up_->setEnabled(current_ != root_);
            if (changed)
                name_->setText("");
            populateDirectory();
        }
        catch (const std::exception &e)
        {
            error_->setText(e.what());
        }
    }
    void populateDirectory()
    {
        try
        {
            std::vector<fs::directory_entry> entries;
            for (auto &entry : fs::directory_iterator(current_))
                entries.push_back(entry);
            std::sort(entries.begin(),
                      entries.end(),
                      [](const auto &a, const auto &b)
                      {
                          if (a.is_directory() != b.is_directory())
                              return a.is_directory();
                          return a.path().filename() < b.path().filename();
                      });
            tree_->clear();
            paths_.clear();
            error_->setText("");
            auto filter = filter_->currentText();
            auto begin = filter.rfind('('), end = filter.rfind(')');
            std::vector<std::string> patterns;
            if (begin != std::string::npos && end > begin)
            {
                std::istringstream in(filter.substr(begin + 1, end - begin - 1));
                for (std::string pattern; in >> pattern;)
                    patterns.push_back(pattern);
            }
            std::string query = search_->text();
            std::transform(query.begin(), query.end(), query.begin(),
                           [](unsigned char c) { return char(std::tolower(c)); });
            for (auto &entry : entries)
            {
                auto name = entry.path().filename().u8string();
                auto lowerName = name;
                std::transform(lowerName.begin(), lowerName.end(),
                               lowerName.begin(), [](unsigned char c)
                               { return char(std::tolower(c)); });
                if (!query.empty() && lowerName.find(query) == std::string::npos)
                    continue;
                bool directory = entry.is_directory();
                if (!directory && !patterns.empty() &&
                    !std::any_of(patterns.begin(), patterns.end(),
                                 [&](const auto &pattern)
                                 { return wildcard(pattern, name); }))
                    continue;
                auto type = directory ? std::string("Folder")
                                      : fs::u8path(name).extension().u8string();
                if (!directory)
                {
                    if (!type.empty() && type.front() == '.')
                        type.erase(type.begin());
                    if (type.empty())
                        type = "File";
                    else
                    {
                        std::transform(type.begin(),
                                       type.end(),
                                       type.begin(),
                                       [](unsigned char c)
                                       { return char(std::toupper(c)); });
                        type += " File";
                    }
                }
                const auto modified = modifiedTime(entry.last_write_time());
                const auto size = directory ? std::string{}
                                            : fileSize(entry.file_size());
                const std::string entryIcon = directory ? "\xf0\x9f\x93\x81 "
                                                         : "\xf0\x9f\x93\x84 ";
                auto *item = tree_->push_back(TreeWidgetItem(
                    {entryIcon + name, modified, type, size, ""}));
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
