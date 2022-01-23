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

/** @file PluginHeightMap.cpp */

#include <ColorPalette.hpp>
#include <Editor.hpp>
#include <PluginHeightMap.hpp>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDebug>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QProgressDialog>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <Time.hpp>

#define PLUGIN_HEIGHT_MAP_NAME "Heightmap"
#define PLUGIN_HEIGHT_MAP_COLORMAP_MATLAB_JET "Matlab Jet"
#define PLUGIN_HEIGHT_MAP_COLORMAP_VTK "VTK"
#define PLUGIN_HEIGHT_MAP_COLORMAP_GRAY "Gray"
#define PLUGIN_HEIGHT_MAP_COLORMAP_WIN_XP "Windows XP"
#define PLUGIN_HEIGHT_MAP_COLORMAP_DEFAULT PLUGIN_HEIGHT_MAP_COLORMAP_MATLAB_JET
#define PLUGIN_HEIGHT_MAP_COLORS_MAX 65536
#define PLUGIN_HEIGHT_MAP_COLORS_DEFAULT 256

#if 1 /* Allow to collapse this class */
PluginHeightMapFilter::PluginHeightMapFilter()
    : editor_(nullptr),
      previewEnabled_(false)
{
}

void PluginHeightMapFilter::initialize(Editor *editor)
{
    editor_ = editor;
    colormap_ = createColormap(PLUGIN_HEIGHT_MAP_COLORMAP_DEFAULT,
                               PLUGIN_HEIGHT_MAP_COLORS_DEFAULT);
}

void PluginHeightMapFilter::setColormap(const QString &name, int colorCount)
{
    bool previewEnabled;
    mutex_.lock();
    previewEnabled = previewEnabled_;
    colormap_ = createColormap(name, colorCount);
    mutex_.unlock();

    if (previewEnabled)
    {
        setPreviewEnabled(previewEnabled);
    }
}

void PluginHeightMapFilter::setPreviewEnabled(bool enabled,
                                              bool update,
                                              bool reload)
{
    if (update)
    {
        editor_->cancelThreads();

        mutex_.lock();
        previewEnabled_ = enabled;
        mutex_.unlock();

        editor_->lock();
        if (reload)
        {
            editor_->viewports().setState(EditorPage::STATE_READ);
        }
        else
        {
            editor_->viewports().setState(EditorPage::STATE_FILTER);
        }
        editor_->unlock();

        editor_->restartThreads();
    }
    else
    {
        mutex_.lock();
        previewEnabled_ = enabled;
        mutex_.unlock();
    }
}

bool PluginHeightMapFilter::isPreviewEnabled()
{
    bool ret;
    mutex_.lock();
    ret = previewEnabled_;
    mutex_.unlock();
    return ret;
}

void PluginHeightMapFilter::filterPage(EditorPage *page)
{
    mutex_.lock();

    double zMin = editor_->clipBoundary().min(2);
    double zLen = editor_->clipBoundary().max(2) - zMin;
    double colorDelta = 1.0 / static_cast<double>(colormap_.size() - 1);

    double zLenInv = 0;
    if (zLen > 0)
    {
        zLenInv = 1.0 / zLen;
    }

    const std::vector<uint32_t> &selection = page->selection;

    for (size_t i = 0; i < selection.size(); i++)
    {
        size_t row = selection[i];

        double z = page->position[3 * row + 2];
        double zNorm = (z - zMin) * zLenInv;

        size_t colorIndex = static_cast<size_t>(zNorm / colorDelta);

        page->renderColor[row * 3 + 0] *= colormap_[colorIndex][0];
        page->renderColor[row * 3 + 1] *= colormap_[colorIndex][1];
        page->renderColor[row * 3 + 2] *= colormap_[colorIndex][2];
    }

    mutex_.unlock();
}

void PluginHeightMapFilter::apply(QWidget *widget)
{
    editor_->cancelThreads();

    EditorQuery query(editor_);
    query.selectBox(editor_->clipBoundary());
    query.exec();

    int maximum = static_cast<int>(query.pageSizeEstimate());

    QProgressDialog progressDialog(widget);
    progressDialog.setCancelButtonText(QObject::tr("&Cancel"));
    progressDialog.setRange(0, maximum);
    progressDialog.setWindowTitle(QObject::tr(PLUGIN_HEIGHT_MAP_NAME));
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setMinimumDuration(0);
    progressDialog.show();

    for (int i = 0; i < maximum; i++)
    {
        // Update progress i
        progressDialog.setValue(i + 1);
        progressDialog.setLabelText(
            QObject::tr("Processing %1 of %n...", nullptr, maximum).arg(i + 1));

        QCoreApplication::processEvents();
        if (progressDialog.wasCanceled())
        {
            break;
        }

        // Process step i
        editor_->lock();
        if (query.nextPage())
        {
            editor_->applyFilters(query.page());
            // editor_->flush(query.page());
        }
        editor_->unlock();
    }
    progressDialog.setValue(progressDialog.maximum());

    editor_->restartThreads();
}

std::vector<Vector3<float>> PluginHeightMapFilter::createColormap(
    const QString &name,
    int colorCount)
{
    size_t n = static_cast<size_t>(colorCount);

    if (name == PLUGIN_HEIGHT_MAP_COLORMAP_MATLAB_JET)
    {
        return ColorPalette::blueCyanYellowRed(n);
    }
    else if (name == PLUGIN_HEIGHT_MAP_COLORMAP_VTK)
    {
        return ColorPalette::blueCyanGreenYellowRed(n);
    }
    else if (name == PLUGIN_HEIGHT_MAP_COLORMAP_GRAY)
    {
        return ColorPalette::gray(n);
    }
    else if (name == PLUGIN_HEIGHT_MAP_COLORMAP_WIN_XP)
    {
        return ColorPalette::WindowsXp32;
    }
    else
    {
        // White
        std::vector<Vector3<float>> colormap;
        colormap.resize(n);

        for (size_t i = 0; i < n; i++)
        {
            colormap[i].set(1.0F, 1.0F, 1.0F);
        }

        return colormap;
    }
}
#endif /* Allow to collapse this class */
// -----------------------------------------------------------------------------
#if 1 /* Allow to collapse this class */
PluginHeightMapWindow::PluginHeightMapWindow(QMainWindow *parent,
                                             PluginHeightMapFilter *filter)
    : WindowDock(parent),
      filter_(filter)
{
    // Widgets colormap
    colorCountSpinBox_ = new QSpinBox;
    colorCountSpinBox_->setRange(1, PLUGIN_HEIGHT_MAP_COLORS_MAX);
    colorCountSpinBox_->setValue(PLUGIN_HEIGHT_MAP_COLORS_DEFAULT);
    colorCountSpinBox_->setSingleStep(1);

    connect(colorCountSpinBox_,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(colorCountChanged(int)));

    colormapComboBox_ = new QComboBox;
    colormapComboBox_->addItem(PLUGIN_HEIGHT_MAP_COLORMAP_MATLAB_JET);
    colormapComboBox_->addItem(PLUGIN_HEIGHT_MAP_COLORMAP_VTK);
    colormapComboBox_->addItem(PLUGIN_HEIGHT_MAP_COLORMAP_GRAY);
    colormapComboBox_->addItem(PLUGIN_HEIGHT_MAP_COLORMAP_WIN_XP);
    colormapComboBox_->setCurrentText(PLUGIN_HEIGHT_MAP_COLORMAP_DEFAULT);

    connect(colormapComboBox_,
            SIGNAL(activated(QString)),
            this,
            SLOT(colormapChanged(QString)));

    // Widgets apply
    // connect() with '&' has less features than classic SIGNAL()/SLOT()
    // in the current Qt version but it is easier to write.
    previewCheckBox_ = new QCheckBox;
    connect(previewCheckBox_,
            &QCheckBox::stateChanged,
            this,
            &PluginHeightMapWindow::previewChanged);

    applyButton_ = new QPushButton(tr("Apply and save"));
    applyButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(applyButton_,
            &QAbstractButton::clicked,
            this,
            &PluginHeightMapWindow::apply);

    // Layout
    QGridLayout *groupBoxLayout = new QGridLayout;
    groupBoxLayout->addWidget(new QLabel(tr("N colors")), 0, 0);
    groupBoxLayout->addWidget(colorCountSpinBox_, 0, 1);
    groupBoxLayout->addWidget(new QLabel(tr("Colormap")), 1, 0);
    groupBoxLayout->addWidget(colormapComboBox_, 1, 1);
    groupBoxLayout->setColumnStretch(1, 1);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(previewCheckBox_);
    hbox->addWidget(new QLabel(tr("Preview")));
    hbox->addStretch();
    hbox->addWidget(applyButton_, 0, Qt::AlignRight);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addLayout(groupBoxLayout);
    vbox->addSpacing(10);
    vbox->addLayout(hbox);
    vbox->addStretch();

    // Dock
    widget_ = new QWidget;
    widget_->setLayout(vbox);
    widget_->setFixedHeight(100);
    setWidget(widget_);
}

void PluginHeightMapWindow::colorCountChanged(int i)
{
    (void)i;
    filter_->setColormap(colormapComboBox_->currentText(),
                         colorCountSpinBox_->value());
}

void PluginHeightMapWindow::colormapChanged(const QString &name)
{
    (void)name;
    filter_->setColormap(colormapComboBox_->currentText(),
                         colorCountSpinBox_->value());
}

void PluginHeightMapWindow::previewChanged(int index)
{
    (void)index;
    filter_->setPreviewEnabled(previewCheckBox_->isChecked());
}

void PluginHeightMapWindow::apply()
{
    // Filter is active during proccesing
    filter_->setPreviewEnabled(true, false);
    filter_->apply(mainWindow());
    filter_->setPreviewEnabled(previewCheckBox_->isChecked(), true, true);
}

void PluginHeightMapWindow::closeEvent(QCloseEvent *event)
{
    if (previewCheckBox_->isChecked())
    {
        previewCheckBox_->setChecked(false);
        filter_->setPreviewEnabled(false);
    }
    event->accept();
}
#endif /* Allow to collapse this class */
// -----------------------------------------------------------------------------
PluginHeightMap::PluginHeightMap() : window_(nullptr)
{
}

void PluginHeightMap::initialize(QMainWindow *parent, Editor *editor)
{
    // Do not create GUI when this plugin is loaded
    (void)parent;

    filter_.initialize(editor);
}

void PluginHeightMap::show(QMainWindow *parent)
{
    // Create GUI only when this plugin is used for the first time
    if (!window_)
    {
        window_ = new PluginHeightMapWindow(parent, &filter_);
        window_->setWindowTitle(windowTitle());
        window_->setWindowIcon(icon());
        window_->setFloating(true);
        window_->setAllowedAreas(Qt::RightDockWidgetArea);
        parent->addDockWidget(Qt::RightDockWidgetArea, window_);
    }

    window_->show();
    window_->raise();
    window_->activateWindow();
}

QAction *PluginHeightMap::toggleViewAction() const
{
    if (window_)
    {
        return window_->toggleViewAction();
    }

    return nullptr;
}

QString PluginHeightMap::windowTitle() const
{
    return tr(PLUGIN_HEIGHT_MAP_NAME);
}

QString PluginHeightMap::buttonText() const
{
    return tr("Height\nmap");
}

QString PluginHeightMap::toolTip() const
{
    return tr("Compute height map");
}

QPixmap PluginHeightMap::icon() const
{
    return QPixmap(":/histogram-ios-50.png");
}

bool PluginHeightMap::isFilterEnabled()
{
    return filter_.isPreviewEnabled();
}

void PluginHeightMap::filterPage(EditorPage *page)
{
    filter_.filterPage(page);
}
