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

void PluginHeightMapFilter::setPreviewEnabled(bool enabled)
{
    editor_->cancelThreads();

    mutex_.lock();
    previewEnabled_ = enabled;
    mutex_.unlock();

    editor_->lock();
    editor_->tileViewClear();
    editor_->unlock();

    editor_->restartThreads();
}

bool PluginHeightMapFilter::isPreviewEnabled()
{
    bool ret;
    mutex_.lock();
    ret = previewEnabled_;
    mutex_.unlock();
    return ret;
}

void PluginHeightMapFilter::filterTile(EditorTile *tile)
{
    mutex_.lock();

    double zMin = editor_->boundary().min(2);
    double zLen = editor_->boundary().max(2) - zMin;
    double colorDelta = 1.0 / static_cast<double>(colormap_.size() - 1);

    double zLenInv = 0;
    if (zLen > 0)
    {
        zLenInv = 1.0 / zLen;
    }

    const std::vector<unsigned int> &indices = tile->indices;

    for (size_t i = 0; i < indices.size(); i++)
    {
        size_t row = indices[i];

        double z = tile->xyz[row * 3 + 2];
        double zNorm = (z - zMin) * zLenInv;

        size_t colorIndex = static_cast<size_t>(zNorm / colorDelta);

        tile->view.rgb[row * 3 + 0] *= colormap_[colorIndex][0];
        tile->view.rgb[row * 3 + 1] *= colormap_[colorIndex][1];
        tile->view.rgb[row * 3 + 2] *= colormap_[colorIndex][2];
    }

    mutex_.unlock();
}

void PluginHeightMapFilter::applyToTiles(QWidget *widget)
{
    editor_->attach();

    std::vector<FileIndex::Selection> selection;
    editor_->select(selection);

    int maximum = static_cast<int>(selection.size());

    QProgressDialog progressDialog(widget);
    progressDialog.setCancelButtonText(QObject::tr("&Cancel"));
    progressDialog.setRange(0, maximum);
    progressDialog.setWindowTitle(QObject::tr(PLUGIN_HEIGHT_MAP_NAME));
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setMinimumDuration(100);

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
        FileIndex::Selection &sel = selection[static_cast<size_t>(i)];
        EditorTile *tile = editor_->tile(sel.id, sel.idx);
        if (tile)
        {
            filterTile(tile);
        }
    }
    progressDialog.setValue(progressDialog.maximum());

    editor_->detach();
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

    applyButton_ = new QPushButton(tr("&Apply"));
    applyButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(applyButton_,
            &QAbstractButton::clicked,
            this,
            &PluginHeightMapWindow::apply);

    // Layout
    QGroupBox *groupBox = new QGroupBox;
    QGridLayout *groupBoxLayout = new QGridLayout;
    groupBoxLayout->addWidget(new QLabel(tr("N colors")), 0, 0);
    groupBoxLayout->addWidget(colorCountSpinBox_, 0, 1);
    groupBoxLayout->addWidget(new QLabel(tr("Colormap")), 1, 0);
    groupBoxLayout->addWidget(colormapComboBox_, 1, 1);
    groupBoxLayout->setColumnStretch(1, 1);
    groupBox->setLayout(groupBoxLayout);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(previewCheckBox_);
    hbox->addWidget(new QLabel(tr("Preview")));
    hbox->addStretch();
    hbox->addWidget(applyButton_, 0, Qt::AlignRight);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(groupBox);
    vbox->addSpacing(10);
    vbox->addLayout(hbox);
    vbox->addStretch();

    // Dock
    widget_ = new QWidget;
    widget_->setLayout(vbox);
    widget_->setFixedHeight(120);
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
    filter_->applyToTiles(mainWindow());
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
        window_->setAllowedAreas(Qt::LeftDockWidgetArea |
                                 Qt::RightDockWidgetArea);
        parent->addDockWidget(Qt::RightDockWidgetArea, window_);
    }

    window_->show();
    window_->raise();
    window_->activateWindow();
}

QString PluginHeightMap::windowTitle() const
{
    return tr(PLUGIN_HEIGHT_MAP_NAME);
}

QString PluginHeightMap::toolTip() const
{
    return tr("Compute height map");
}

QIcon PluginHeightMap::icon() const
{
    return QIcon(":/icons8-histogram-40.png");
}

bool PluginHeightMap::isFilterEnabled()
{
    return filter_.isPreviewEnabled();
}

void PluginHeightMap::filterTile(EditorTile *tile)
{
    filter_.filterTile(tile);
}
