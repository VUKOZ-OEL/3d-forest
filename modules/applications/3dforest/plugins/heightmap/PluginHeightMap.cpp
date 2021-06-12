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

/**
    @file PluginHeightMap.cpp
*/

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
#define PLUGIN_HEIGHT_MAP_COLORMAP_JET "Jet"
#define PLUGIN_HEIGHT_MAP_COLORMAP_HUE "Hue"
#define PLUGIN_HEIGHT_MAP_COLORMAP_GRAY "Gray"
#define PLUGIN_HEIGHT_MAP_COLORMAP_DEFAULT PLUGIN_HEIGHT_MAP_COLORMAP_JET
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

    if (tile->view.rgb.size() != tile->xyz.size())
    {
        tile->view.rgb.resize(tile->xyz.size());
    }

    double colorDelta = 1.0 / static_cast<double>(colormap_.size());

    const std::vector<unsigned int> &indices = tile->indices;
    for (size_t i = 0; i < indices.size(); i++)
    {
        size_t row = indices[i];
        double z = tile->xyz[row * 3 + 2];
        double h = (z - zMin) / zLen;
        size_t c = static_cast<size_t>(h / colorDelta);

        tile->view.rgb[row * 3 + 0] = static_cast<float>(colormap_[c].redF());
        tile->view.rgb[row * 3 + 1] = static_cast<float>(colormap_[c].greenF());
        tile->view.rgb[row * 3 + 2] = static_cast<float>(colormap_[c].blueF());
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

        msleep(10); // TBD debug

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

std::vector<QColor> PluginHeightMapFilter::createColormap(const QString &name,
                                                          int colorCount)
{
    std::vector<QColor> colormap;

    size_t n = static_cast<size_t>(colorCount);
    colormap.resize(n);

    if (name == PLUGIN_HEIGHT_MAP_COLORMAP_JET)
    {
        size_t n1 = n / 3;
        float delta = 1.0F / static_cast<float>(n1);
        for (size_t i = 0; i < n1; i++)
        {
            float v = delta * static_cast<float>(i);
            colormap[i].setRgbF(0, v, 1.0F);
        }

        size_t n2 = n1 * 2;
        delta = 1.0F / static_cast<float>(n2 - n1);
        for (size_t i = n1; i < n2; i++)
        {
            float v = delta * static_cast<float>(i - n1);
            colormap[i].setRgbF(v, 1.0F, 1.0F - v);
        }

        delta = 1.0F / static_cast<float>(n - n2);
        for (size_t i = n2; i < n; i++)
        {
            float v = delta * static_cast<float>(i + 1 - n2);
            colormap[i].setRgbF(1.0F, 1.0F - v, 0);
        }
    }
    else if (name == PLUGIN_HEIGHT_MAP_COLORMAP_HUE)
    {
        float delta = 1.0F / static_cast<float>(n);
        for (size_t i = 0; i < n; i++)
        {
            float v = delta * static_cast<float>(i + 1);
            colormap[i].setHsvF(v, 1.0F, 1.0F);
        }
    }
    else if (name == PLUGIN_HEIGHT_MAP_COLORMAP_GRAY)
    {
        float delta = 1.0F / static_cast<float>(n);
        for (size_t i = 0; i < n; i++)
        {
            float v = delta * static_cast<float>(i + 1);
            colormap[i].setRgbF(v, v, v);
        }
    }
    else
    {
        for (size_t i = 0; i < n; i++)
        {
            colormap[i].setRgb(255, 255, 255);
        }
    }

    return colormap;
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
    colormapComboBox_->addItem(PLUGIN_HEIGHT_MAP_COLORMAP_JET);
    colormapComboBox_->addItem(PLUGIN_HEIGHT_MAP_COLORMAP_HUE);
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
    }

    window_->show();
    window_->raise();
    window_->activateWindow();
}

QString PluginHeightMap::windowTitle() const
{
    return tr(PLUGIN_HEIGHT_MAP_NAME);
}

bool PluginHeightMap::isFilterEnabled()
{
    return filter_.isPreviewEnabled();
}

void PluginHeightMap::filterTile(EditorTile *tile)
{
    filter_.filterTile(tile);
}
