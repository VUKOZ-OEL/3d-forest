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
#include <stdexcept>

// Include 3D Forest.
#include <MessageBox.hpp>
#include <QtApplication.hpp>
#include <QtCheckBox.hpp>
#include <QtComboBox.hpp>
#include <QtDialog.hpp>
#include <QtDoubleRangeSlider.hpp>
#include <QtDoubleSpinBox.hpp>
#include <QtGridLayout.hpp>
#include <QtGroupBox.hpp>
#include <QtHBoxLayout.hpp>
#include <QtLabel.hpp>
#include <QtLineEdit.hpp>
#include <QtMessageBox.hpp>
#include <QtProgressBar.hpp>
#include <QtProgressDialog.hpp>
#include <QtPushButton.hpp>
#include <QtRadioButton.hpp>
#include <QtSlider.hpp>
#include <QtSpinBox.hpp>
#include <QtSplitter.hpp>
#include <QtTableWidget.hpp>
#include <QtTextEdit.hpp>
#include <QtToolBar.hpp>
#include <QtToolButton.hpp>
#include <QtTreeWidget.hpp>
#include <QtVBoxLayout.hpp>
#include <QtViewer.hpp>
#include <QtWidget.hpp>

// Include Qt.
#include <QFileDialog>
#include <QMessageBox>
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

void QtApplication::processEvents()
{
    QCoreApplication::processEvents();
}

void QtApplication::initLayout()
{
    splitter_ = new QSplitter(Qt::Horizontal, &mainWindow_);
    splitter_->setHandleWidth(1);
    // splitter_->setStyleSheet("QSplitter::handle { background: #303030; }");

    // Left side bar area
    sidebar_ = new QtSidebar(&navigation(), this, splitter_);

    // Right: viewer above an optional plugin panel.
    rightSplitter_ = new QSplitter(Qt::Vertical, splitter_);
    rightSplitter_->setHandleWidth(1);
    rightSplitter_->setChildrenCollapsible(false);

    // Viewer.
    viewerContainer_ = new QWidget(rightSplitter_);

    viewerLayout_ = new QVBoxLayout(viewerContainer_);
    viewerLayout_->setContentsMargins(0, 0, 0, 0);
    viewerLayout_->setSpacing(0);

    // Plugin panels.
    bottomStack_ = new QStackedWidget(rightSplitter_);
    bottomStack_->setContentsMargins(0, 0, 0, 0);

    rightSplitter_->addWidget(viewerContainer_);
    rightSplitter_->addWidget(bottomStack_);

    rightSplitter_->setStretchFactor(0, 1);
    rightSplitter_->setStretchFactor(1, 1);

    bottomStack_->hide();
    bottomStack_->setObjectName("bottomPanel");

    // Splitter.
    splitter_->addWidget(sidebar_);
    splitter_->addWidget(rightSplitter_);

    // Left side does not stretch as much as the viewer.
    splitter_->setStretchFactor(0, 0);
    splitter_->setStretchFactor(1, 1);

    // Initial widths.
    splitter_->setSizes({280, 920});

    sidebar_->setMinimumWidth(220);
    sidebar_->setMaximumWidth(500);

    mainWindow_.setCentralWidget(splitter_);

    // Theme colors.
    updateTheme();

    QObject::connect(
        qapplication_.styleHints(),
        &QStyleHints::colorSchemeChanged,
        &mainWindow_,
        [this](Qt::ColorScheme) { updateTheme(); },
        Qt::QueuedConnection);
}

void QtApplication::updateTheme()
{
    const bool darkMode = QtThemeColors::isDesktopDarkMode(&qapplication_);

    themeColors_.setDarkMode(darkMode);
    sidebar_->setTheme(themeColors_);

    QString styleSheet = themeColors_.getStyleSheet();
    mainWindow_.setStyleSheet(styleSheet);

    Q_EMIT themeChanged(darkMode);
}

void QtApplication::bindTheme(QObject *receiver,
                              std::function<void(bool)> applyTheme)
{
    QObject::connect(this, &QtApplication::themeChanged, receiver, applyTheme);

    applyTheme(isDarkMode());
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

void QtApplication::showBottomWidget(Widget *widget)
{
    if (!widget)
    {
        return;
    }

    if (!bottomStack_ || !rightSplitter_)
    {
        throw std::logic_error("showBottomWidget requires initLayout() first.");
    }

    auto &qtWidget = bottomWidgets_[widget];

    if (!qtWidget)
    {
        qtWidget = createWidget(widget, bottomStack_.data());

        if (!qtWidget)
        {
            bottomWidgets_.erase(widget);

            throw std::runtime_error("Failed to create bottom widget.");
        }

        bottomStack_->addWidget(qtWidget.data());
    }

    const bool wasHidden = bottomStack_->isHidden();

    bottomStack_->setCurrentWidget(qtWidget.data());
    bottomStack_->show();

    if (wasHidden)
    {
#if 0        
        // Request equal heights when opening the bottom area.
        // Qt respects the widgets' minimum sizes.
        const int height = rightSplitter_->height();
        const int half = height > 2 ? height / 2 : 500;

        rightSplitter_->setSizes({half, half});
#else
        // Viewer: 75%, bottom panel: 25%.
        rightSplitter_->setSizes({900, 300});
#endif
    }
}

void QtApplication::hideBottomWidget()
{
    if (bottomStack_)
    {
        bottomStack_->hide();
    }
}

void QtApplication::toggleBottomWidget(Widget *widget)
{
    if (!widget)
    {
        return;
    }

    const auto it = bottomWidgets_.find(widget);

    if (bottomStack_ && !bottomStack_->isHidden() &&
        it != bottomWidgets_.end() && it->second &&
        bottomStack_->currentWidget() == it->second.data())
    {
        hideBottomWidget();
        return;
    }

    showBottomWidget(widget);
}

void QtApplication::removeBottomWidget(Widget *widget)
{
    const auto it = bottomWidgets_.find(widget);

    if (it == bottomWidgets_.end())
    {
        return;
    }

    const QPointer<QWidget> qtWidget = it->second;
    bottomWidgets_.erase(it);

    if (!bottomStack_)
    {
        return;
    }

    const bool wasCurrent =
        qtWidget && bottomStack_->currentWidget() == qtWidget.data();

    if (qtWidget)
    {
        bottomStack_->removeWidget(qtWidget.data());
        delete qtWidget.data();
    }

    if (wasCurrent || bottomStack_->count() == 0)
    {
        bottomStack_->hide();
    }
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
        return new QtLabel(w, this, parent);
    }

    if (auto *w = dynamic_cast<LineEdit *>(widget))
    {
        LOG_DEBUG(<< "Create line edit widget.");
        return new QtLineEdit(w, parent);
    }

    if (auto *w = dynamic_cast<ProgressBar *>(widget))
    {
        LOG_DEBUG(<< "Create progress bar widget.");
        return new QtProgressBar(w, parent);
    }

    if (auto *w = dynamic_cast<PushButton *>(widget))
    {
        LOG_DEBUG(<< "Create push button widget.");
        return new QtPushButton(w, this, parent);
    }

    if (auto *w = dynamic_cast<RadioButton *>(widget))
    {
        LOG_DEBUG(<< "Create radio button widget.");
        return new QtRadioButton(w, parent);
    }

    if (auto *w = dynamic_cast<ToolButton *>(widget))
    {
        LOG_DEBUG(<< "Create tool button widget.");
        return new QtToolButton(w, this, parent);
    }

    if (auto *w = dynamic_cast<Slider *>(widget))
    {
        LOG_DEBUG(<< "Create slider widget.");
        return new QtSlider(w, parent);
    }

    if (auto *w = dynamic_cast<DoubleRangeSlider *>(widget))
    {
        LOG_DEBUG(<< "Create double range slider widget.");
        return new QtDoubleRangeSlider(w, parent);
    }

    if (auto *w = dynamic_cast<SpinBox *>(widget))
    {
        LOG_DEBUG(<< "Create spin box widget.");
        return new QtSpinBox(w, parent);
    }

    if (auto *w = dynamic_cast<DoubleSpinBox *>(widget))
    {
        LOG_DEBUG(<< "Create double spin box widget.");
        return new QtDoubleSpinBox(w, parent);
    }

    if (auto *w = dynamic_cast<Splitter *>(widget))
    {
        LOG_DEBUG(<< "Create splitter widget.");
        return new QtSplitter(w, this, parent);
    }

    if (auto *w = dynamic_cast<TableWidget *>(widget))
    {
        LOG_DEBUG(<< "Create table widget.");
        return new QtTableWidget(w, parent);
    }

    if (auto *w = dynamic_cast<TextEdit *>(widget))
    {
        LOG_DEBUG(<< "Create text edit widget.");
        return new QtTextEdit(w, parent);
    }

    if (auto *w = dynamic_cast<ToolBar *>(widget))
    {
        LOG_DEBUG(<< "Create tool bar widget.");
        return new QtToolBar(w, this, parent);
    }

    if (auto *w = dynamic_cast<TreeWidget *>(widget))
    {
        LOG_DEBUG(<< "Create tree widget.");
        return new QtTreeWidget(w, parent);
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

    if (auto *hBoxLayout = dynamic_cast<HBoxLayout *>(layout))
    {
        LOG_DEBUG(<< "Create HBoxLayout.");
        return new QtHBoxLayout(hBoxLayout, this, parent);
    }

    if (auto *vBoxLayout = dynamic_cast<VBoxLayout *>(layout))
    {
        LOG_DEBUG(<< "Create VBoxLayout.");
        return new QtVBoxLayout(vBoxLayout, this, parent);
    }

    LOG_DEBUG(<< "Create null layout.");
    return nullptr;
}

QDialog *QtApplication::createDialog(Dialog &dialog, QWidget *parent)
{
    if (auto *progressDialog = dynamic_cast<ProgressDialog *>(&dialog))
    {
        return new QtProgressDialog(progressDialog, parent);
    }

    if (auto *messageBox = dynamic_cast<MessageBox *>(&dialog))
    {
        return new QtMessageBox(messageBox, parent);
    }

    return new QtDialog(&dialog, this, parent);
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

int QtApplication::showDialog(Dialog &dialog)
{
    QWidget *parent = QApplication::activeModalWidget();

    if (!parent)
    {
        parent = &mainWindow_;
    }

    std::unique_ptr<QDialog> qtDialog(createDialog(dialog, parent));

    const int result = qtDialog->exec();
    dialog.setResult(result);

    return result;
}

void QtApplication::openDialog(Dialog &dialog)
{
    // Remove entries whose Qt representation was destroyed.
    for (auto it = dialogs_.begin(); it != dialogs_.end();)
    {
        if (it->second.isNull())
        {
            it = dialogs_.erase(it);
        }
        else
        {
            ++it;
        }
    }

    auto &qtDialog = dialogs_[&dialog];

    if (!qtDialog)
    {
        QWidget *parent = QApplication::activeModalWidget();

        if (!parent)
        {
            parent = &mainWindow_;
        }

        qtDialog = createDialog(dialog, parent);

        const QPointer<QDialog> guard = qtDialog;

        dialog.hideRequested.connect(
            [guard]
            {
                if (guard)
                {
                    guard->hide();
                }
            });

        dialog.raiseRequested.connect(
            [guard]
            {
                if (guard)
                {
                    guard->raise();
                }
            });

        dialog.activateWindowRequested.connect(
            [guard]
            {
                if (guard)
                {
                    guard->activateWindow();
                }
            });

        // Destroy the Qt representation before its common data disappears.
        dialog.destroyed.connect(
            [guard]
            {
                if (guard)
                {
                    delete guard.data();
                }
            });
    }

    qtDialog->show();
}

Pixmap QtApplication::loadPixmap(const std::string &fileName) const
{
    LOG_DEBUG(<< "loadPixmap <" << fileName << ">.");

    QImage image;
    if (fileName.compare(0, 2, ":/") == 0)
    {
        const ResourceData bytes = ResourceRegistry::get(fileName);
        if (!bytes || bytes->empty() ||
            bytes->size() >
                static_cast<std::size_t>((std::numeric_limits<int>::max)()))
        {
            LOG_DEBUG(<< "Resource not registered <" << fileName << ">.");
            return {};
        }
        // Decode our registry's bytes. There is no Qt resource lookup here.
        image =
            QImage::fromData(bytes->data(), static_cast<int>(bytes->size()));
    }
    else
    {
        image.load(QString::fromStdString(fileName));
    }

    if (image.isNull())
    {
        LOG_DEBUG(<< "Image decoding failed <" << fileName << ">.");
        return {};
    }

    image = image.convertToFormat(QImage::Format_RGBA8888);
    if (image.isNull())
    {
        return {};
    }

    const std::size_t stride = static_cast<std::size_t>(image.width()) * 4;
    std::vector<Pixmap::Byte> rgba(stride *
                                   static_cast<std::size_t>(image.height()));
    for (int y = 0; y < image.height(); ++y)
    {
        std::memcpy(rgba.data() + static_cast<std::size_t>(y) * stride,
                    image.constScanLine(y),
                    stride);
    }

    return Pixmap(image.width(), image.height(), rgba);
}
