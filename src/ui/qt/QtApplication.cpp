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

/** @file QtApplication.cpp */

// Include std.

// Include 3D Forest.
#include <MessageBox.hpp>
#include <QtApplication.hpp>
#include <QtCheckBox.hpp>
#include <QtComboBox.hpp>
#include <QtGridLayout.hpp>
#include <QtGroupBox.hpp>
#include <QtLabel.hpp>
#include <QtSlider.hpp>
#include <QtVBoxLayout.hpp>
#include <QtViewer.hpp>
#include <QtWidget.hpp>

// Include Qt.
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPalette>
#include <QStyleHints>

// Include local.
#define LOG_MODULE_NAME "QtApplication"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

QtApplication::QtApplication(QApplication &qapplication)
    : qapplication_(qapplication)
{
}

QtApplication::~QtApplication()
{
}

void QtApplication::init()
{
    initLayout();

    load();

    connect(
        this,
        &QtApplication::wakeUpRequested,
        this,
        [this] { processRenderRequest(); },
        Qt::QueuedConnection);
}

void QtApplication::wakeUp()
{
    emit wakeUpRequested();
}

void QtApplication::setOrganizationName(const std::string &str)
{
}

void QtApplication::setApplicationName(const std::string &str)
{
}

void QtApplication::setApplicationVersion(const std::string &str)
{
}

void QtApplication::setWindowIcon(const QIcon &icon)
{
    qapplication_.setWindowIcon(icon);
    mainWindow_.setWindowIcon(icon);
}

int QtApplication::exec()
{
    mainWindow_.resize(800, 600);
    mainWindow_.show();
    return qapplication_.exec();
}

void QtApplication::initLayout()
{
    splitter_ = new QSplitter(Qt::Horizontal, &mainWindow_);
    splitter_->setHandleWidth(1);
    splitter_->setStyleSheet("QSplitter::handle {"
                             "    background: #303030;"
                             "}");

    // Left side bar area
    sidebar_ = new QtSidebar(&navigation(), this, splitter_);

    // Right viewer area, initially empty
    viewerContainer_ = new QWidget(splitter_);

    viewerLayout_ = new QVBoxLayout(viewerContainer_);
    viewerLayout_->setContentsMargins(0, 0, 0, 0);
    viewerLayout_->setSpacing(0);

    // Splitter.
    splitter_->addWidget(sidebar_);
    splitter_->addWidget(viewerContainer_);

    // Left side does not stretch as much as the viewer.
    splitter_->setStretchFactor(0, 0);
    splitter_->setStretchFactor(1, 1);

    // Initial widths.
    splitter_->setSizes({300, 900});

    sidebar_->setMinimumWidth(220);
    sidebar_->setMaximumWidth(500);

    mainWindow_.setCentralWidget(splitter_);

    // Theme.
    updateTheme();

    QObject::connect(qapplication_.styleHints(),
                     &QStyleHints::colorSchemeChanged,
                     &mainWindow_,
                     [this](Qt::ColorScheme) { updateTheme(); });
}

void QtApplication::setViewer(Widget *widget)
{
    if (commonViewer_ == widget)
    {
        return;
    }

    if (commonViewer_)
    {
        removeViewer(commonViewer_);
    }

    if (!widget)
    {
        return;
    }

    QWidget *qtWidget = createWidget(widget, viewerContainer_);

    if (!qtWidget)
    {
        return;
    }

    commonViewer_ = widget;
    qtViewer_ = qtWidget;

    viewerLayout_->addWidget(qtViewer_);
}

void QtApplication::removeViewer(Widget *widget)
{
    if (commonViewer_ != widget)
    {
        return;
    }

    if (qtViewer_)
    {
        viewerLayout_->removeWidget(qtViewer_);

        // Deletes Viewer before the generic Viewer.
        delete qtViewer_;
    }

    qtViewer_ = nullptr;
    commonViewer_ = nullptr;
}

QWidget *QtApplication::createWidget(Widget *widget, QWidget *parent)
{
    if (!widget)
    {
        LOG_WARNING(<< "Could not create Qt widget from null.");
        return nullptr;
    }

    if (auto *w = dynamic_cast<CheckBox *>(widget))
    {
        LOG_DEBUG(<< "Create checkbox widget.");
        return new QtCheckBox(w, parent);
    }

    if (auto *w = dynamic_cast<ComboBox *>(widget))
    {
        LOG_DEBUG(<< "Create combobox widget.");
        return new QtComboBox(w, parent);
    }

    if (auto *w = dynamic_cast<GroupBox *>(widget))
    {
        LOG_DEBUG(<< "Create groupbox widget.");
        return new QtGroupBox(w, this, parent);
    }

    if (auto *w = dynamic_cast<Label *>(widget))
    {
        LOG_DEBUG(<< "Create label widget.");
        return new QtLabel(w, parent);
    }

    if (auto *w = dynamic_cast<Slider *>(widget))
    {
        LOG_DEBUG(<< "Create slider widget.");
        return new QtSlider(w, parent);
    }

    if (auto *w = dynamic_cast<Viewer *>(widget))
    {
        LOG_DEBUG(<< "Create viewer widget.");
        return new QtViewer(w, this, parent);
    }

    LOG_DEBUG(<< "Create default widget.");
    return new QtWidget(widget, this, parent);
}

QLayout *QtApplication::createLayout(Layout *layout, QWidget *parent)
{
    if (auto *gridLayout = dynamic_cast<GridLayout *>(layout))
    {
        LOG_DEBUG(<< "Create GridLayout.");
        return new QtGridLayout(gridLayout, this, parent);
    }

    if (auto *vBoxLayout = dynamic_cast<VBoxLayout *>(layout))
    {
        LOG_DEBUG(<< "Create VBoxLayout.");
        return new QtVBoxLayout(vBoxLayout, this, parent);
    }

    LOG_DEBUG(<< "Create null layout.");
    return nullptr;
}

bool QtApplication::isDarkMode() const
{
    const Qt::ColorScheme scheme = qapplication_.styleHints()->colorScheme();

    if (scheme == Qt::ColorScheme::Dark)
    {
        return true;
    }

    if (scheme == Qt::ColorScheme::Light)
    {
        return false;
    }

    const int lightness =
        qapplication_.palette().color(QPalette::Window).lightness();

    return lightness < 128;
}

void QtApplication::updateTheme()
{
    sidebar_->setDarkMode(isDarkMode());
}

std::string QtApplication::getOpenFileName(const std::string &dialogTitle,
                                           const std::string &filter)
{
    const QString filePath =
        QFileDialog::getOpenFileName(&mainWindow_,
                                     QString::fromStdString(dialogTitle),
                                     QString(),
                                     QString::fromStdString(filter));

    return filePath.toStdString();
}

std::vector<std::string> QtApplication::getOpenFileNames(
    const std::string &dialogTitle,
    const std::string &filter)
{
    std::vector<std::string> list;

    QFileDialog fileDialog(&mainWindow_, QString::fromStdString(dialogTitle));
    fileDialog.setNameFilter(QString::fromStdString(filter));
    fileDialog.setFileMode(QFileDialog::ExistingFiles);

    if (fileDialog.exec() == QDialog::Rejected)
    {
        LOG_DEBUG(<< "Canceled opening files from the dialog.");
        return list;
    }

    QStringList files = fileDialog.selectedFiles();
    for (auto const &file : files)
    {
        if (file.length() > 0)
        {
            list.push_back(file.toStdString());
        }
    }

    return list;
}

std::string QtApplication::getSaveFileName(const std::string &caption,
                                           const std::string &dir,
                                           const std::string &filter,
                                           std::string *selectedFilter,
                                           int options)
{
    QFileDialog::Options qoptions;
    if (options & Ui::FileDialogOption::DontConfirmOverwrite)
    {
        qoptions = QFlag(QFileDialog::DontConfirmOverwrite);
    }

    QString qselectedFilter;

    QString fileName =
        QFileDialog::getSaveFileName(&mainWindow_,
                                     QString::fromStdString(caption),
                                     QString::fromStdString(dir),
                                     QString::fromStdString(filter),
                                     &qselectedFilter,
                                     qoptions);

    return fileName.toStdString();
}

int QtApplication::showMessageBox(const MessageBox &box)
{
    QMessageBox dialog(&mainWindow_);

    dialog.setWindowTitle(QString::fromStdString(box.title()));
    dialog.setText(QString::fromStdString(box.text()));
    dialog.setInformativeText(QString::fromStdString(box.informativeText()));

    const auto toQt = [](MessageBox::StandardButton button)
    {
        switch (button)
        {
            case MessageBox::Ok:
                return QMessageBox::Ok;
            case MessageBox::Save:
                return QMessageBox::Save;
            case MessageBox::Discard:
                return QMessageBox::Discard;
            case MessageBox::Cancel:
                return QMessageBox::Cancel;
            default:
                return QMessageBox::NoButton;
        }
    };

    QMessageBox::StandardButtons buttons = QMessageBox::NoButton;

    for (auto button : {MessageBox::Ok,
                        MessageBox::Save,
                        MessageBox::Discard,
                        MessageBox::Cancel})
    {
        if (box.standardButtons() & button)
        {
            buttons |= toQt(button);
        }
    }

    dialog.setStandardButtons(buttons);
    dialog.setDefaultButton(toQt(box.defaultButton()));

    const int result = dialog.exec();

    for (auto button : {MessageBox::Ok,
                        MessageBox::Save,
                        MessageBox::Discard,
                        MessageBox::Cancel})
    {
        if (result == toQt(button))
        {
            return button;
        }
    }

    return MessageBox::NoButton;
}
