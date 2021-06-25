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
#include <QLabel>
#include <QProgressDialog>
#include <QPushButton>
#include <QSpinBox>
#include <Time.hpp>

#define PLUGIN_HEIGHT_MAP_NAME "Heightmap"
#define PLUGIN_HEIGHT_MAP_COLORMAP_VTK "VTK"
#define PLUGIN_HEIGHT_MAP_COLORMAP_GRAY "Gray"
#define PLUGIN_HEIGHT_MAP_COLORMAP_DEFAULT PLUGIN_HEIGHT_MAP_COLORMAP_VTK
#define PLUGIN_HEIGHT_MAP_COLORS_MAX 65536
#define PLUGIN_HEIGHT_MAP_COLORS_DEFAULT 256

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
    editor_->cancelThreads();
    editor_->lock();

    std::vector<FileIndex::Selection> selectionL1;
    editor_->select(selectionL1);

    int maximum = static_cast<int>(selectionL1.size());

    QProgressDialog progressDialog(widget->parentWidget());
    progressDialog.setCancelButtonText(QObject::tr("&Cancel"));
    progressDialog.setRange(0, static_cast<int>(maximum));
    progressDialog.setWindowTitle(QObject::tr(PLUGIN_HEIGHT_MAP_NAME));
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setMinimumDuration(100);

    for (int i = 0; i < maximum; i++)
    {
        // Update progress
        progressDialog.setValue(i);
        progressDialog.setLabelText(
            QObject::tr("Processing %1 of %n...", nullptr, maximum).arg(i));
        QCoreApplication::processEvents();

        if (progressDialog.wasCanceled())
        {
            break;
        }

        msleep(10); /**< @todo Remove. */

        // Process step i
        // FileIndex::Selection &sel = selectionL1[static_cast<size_t>(i)];
        // EditorTile *tile = editor_->tile(sel.id, sel.idx);
        // if (tile)
        // {
        //     filterTile(tile);
        // }
    }

    editor_->unlock();
    editor_->restartThreads();
}

std::vector<Vector3<float>> PluginHeightMapFilter::createColormap(
    const QString &name,
    int colorCount)
{
    size_t n = static_cast<size_t>(colorCount);

    if (name == PLUGIN_HEIGHT_MAP_COLORMAP_VTK)
    {
        return ColorPalette::blueCyanGreenYellowRed(n);
    }
    else if (name == PLUGIN_HEIGHT_MAP_COLORMAP_GRAY)
    {
        return ColorPalette::gray(n);
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
// -----------------------------------------------------------------------------
PluginHeightMapWindow::PluginHeightMapWindow(QWidget *parent,
                                             PluginHeightMapFilter *filter)
    : QDialog(parent),
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
    colormapComboBox_->addItem(PLUGIN_HEIGHT_MAP_COLORMAP_VTK);
    colormapComboBox_->addItem(PLUGIN_HEIGHT_MAP_COLORMAP_GRAY);
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

    applyButton_ = new QPushButton(tr("&Apply"), this);
    connect(applyButton_,
            &QAbstractButton::clicked,
            this,
            &PluginHeightMapWindow::apply);

    // Layout
    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(new QLabel(tr("N colors")), 0, 0);
    layout->addWidget(colorCountSpinBox_, 0, 1, 1, 2);

    layout->addWidget(new QLabel(tr("Colormap")), 1, 0);
    layout->addWidget(colormapComboBox_, 1, 1, 1, 2);

    layout->addWidget(new QLabel(tr("Preview")), 2, 0);
    layout->addWidget(previewCheckBox_, 2, 1);
    layout->addWidget(applyButton_, 2, 2, Qt::AlignRight);

    layout->setRowMinimumHeight(2, 50);
    layout->setColumnStretch(1, 1);

    // Window
    setWindowTitle(tr(PLUGIN_HEIGHT_MAP_NAME));
    setFixedWidth(200);
    setFixedHeight(110);
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
    filter_->applyToTiles(this);
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
// -----------------------------------------------------------------------------
PluginHeightMap::PluginHeightMap() : window_(nullptr)
{
}

void PluginHeightMap::initialize(QWidget *parent, Editor *editor)
{
    // Do not create GUI when this plugin is loaded
    (void)parent;

    filter_.initialize(editor);
}

void PluginHeightMap::show(QWidget *parent)
{
    // Create GUI only when this plugin is used for the first time
    if (!window_)
    {
        window_ = new PluginHeightMapWindow(parent, &filter_);
        window_->setWindowIcon(icon());
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
