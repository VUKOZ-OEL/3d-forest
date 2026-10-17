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

/** @file ComputeHeightMapModifier.cpp */

// Include 3D Forest.
#include <Application.hpp>
#include <ColorPalette.hpp>
#include <ComputeHeightMapModifier.hpp>
#include <Editor.hpp>
#include <ProgressDialog.hpp>
#include <Time.hpp>

// Include local.
#define LOG_MODULE_NAME "ComputeHeightMapModifier"
#include <Log.hpp>

#define PLUGIN_COMPUTE_HEIGHT_MAP_NAME "Heightmap"
#define PLUGIN_COMPUTE_HEIGHT_MAP_COLORMAP_MATLAB_JET "Matlab Jet"
#define PLUGIN_COMPUTE_HEIGHT_MAP_COLORMAP_VTK "VTK"
#define PLUGIN_COMPUTE_HEIGHT_MAP_COLORMAP_GRAY "Gray"
#define PLUGIN_COMPUTE_HEIGHT_MAP_COLORMAP_WIN_XP "Windows XP"
#define PLUGIN_COMPUTE_HEIGHT_MAP_COLORMAP_DEFAULT                             \
    PLUGIN_COMPUTE_HEIGHT_MAP_COLORMAP_MATLAB_JET
#define PLUGIN_COMPUTE_HEIGHT_MAP_COLORS_DEFAULT 256

ComputeHeightMapModifier::ComputeHeightMapModifier()
    : app_(nullptr),
      editor_(nullptr),
      previewEnabled_(false),
      source_(SOURCE_Z_POSITION)
{
}

void ComputeHeightMapModifier::initialize(Application *app)
{
    app_ = app;
    editor_ = &app->editor();
    colormap_ = createColormap(PLUGIN_COMPUTE_HEIGHT_MAP_COLORMAP_DEFAULT,
                               PLUGIN_COMPUTE_HEIGHT_MAP_COLORS_DEFAULT);
}

void ComputeHeightMapModifier::setSource(Source source)
{
    bool previewEnabled;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        source_ = source;
        previewEnabled = previewEnabled_;
    }

    if (previewEnabled)
    {
        setPreviewEnabled(previewEnabled);
    }
}

void ComputeHeightMapModifier::setColormap(const std::string &name,
                                           int colorCount)
{
    bool previewEnabled;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        previewEnabled = previewEnabled_;
        colormap_ = createColormap(name, colorCount);
    }

    if (previewEnabled)
    {
        setPreviewEnabled(previewEnabled);
    }
}

void ComputeHeightMapModifier::setPreviewEnabled(bool enabled,
                                                 bool update,
                                                 bool reload)
{
    if (update)
    {
        app_->suspendThreads();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            previewEnabled_ = enabled;
        }

        {
            std::unique_lock<std::mutex> mutexlock(editor_->editorMutex_);
            if (reload)
            {
                editor_->viewports().setState(Page::STATE_READ);
            }
            else
            {
                editor_->viewports().setState(Page::STATE_RUN_MODIFIERS);
            }
        }

        app_->resumeThreads();
    }
    else
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            previewEnabled_ = enabled;
        }
    }
}

bool ComputeHeightMapModifier::previewEnabled()
{
    bool ret;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        ret = previewEnabled_;
    }

    return ret;
}

void ComputeHeightMapModifier::applyModifier(Page *page)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // Colormap range step in normalized colormap range.
    double colormapStep = 1.0 / static_cast<double>(colormap_.size() - 1);

    // Minimum and maximum height range.
    double heightMinimum;
    double heightRange;

    if (source_ == SOURCE_Z_POSITION)
    {
        heightMinimum = editor_->clipBoundary().min(2);
        heightRange = editor_->clipBoundary().max(2) - heightMinimum;
    }
    else
    {
        heightMinimum = editor_->elevationFilter().minimum();
        heightRange = editor_->elevationFilter().maximum() - heightMinimum;
    }

    // Height range step in normalized height range.
    double heightStep = 0;

    if (heightRange > 0)
    {
        heightStep = 1.0 / heightRange;
    }

    // Process selected points in this page.
    const std::vector<uint32_t> &selection = page->selection;

    for (size_t i = 0; i < page->selectionSize; i++)
    {
        // Index of next selected point in this page.
        size_t row = selection[i];

        // Calculate normalized height <0, 1>.
        double height;

        if (source_ == SOURCE_Z_POSITION)
        {
            height = page->position[3 * row + 2]; // z position from xyz.
        }
        else
        {
            height = page->elevation[row];
        }

        double heightNorm = (height - heightMinimum) * heightStep;

        // Get color by mapping height to colormap range.
        size_t colorIndex = static_cast<size_t>(heightNorm / colormapStep);

        // Output.
        page->renderColor[row * 3 + 0] *=
            static_cast<float>(colormap_[colorIndex][0]);
        page->renderColor[row * 3 + 1] *=
            static_cast<float>(colormap_[colorIndex][1]);
        page->renderColor[row * 3 + 2] *=
            static_cast<float>(colormap_[colorIndex][2]);
    }
}

void ComputeHeightMapModifier::apply(Application *app)
{
    app_->suspendThreads();

    Query query(editor_);
    query.where().setBox(editor_->clipBoundary());
    query.exec();

    int maximum = static_cast<int>(query.pageSizeEstimate());

    ProgressDialog progressDialog(app_);
    // progressDialog.setCancelButtonText(tr("&Cancel"));
    progressDialog.setRange(0, maximum);
    progressDialog.setWindowTitle(tr(PLUGIN_COMPUTE_HEIGHT_MAP_NAME));
    // progressDialog.setWindowModality(Ui::WindowModal);
    // progressDialog.setMinimumDuration(0);
    progressDialog.show();

    for (int i = 0; i < maximum; i++)
    {
        // Update progress i
        progressDialog.setValue(i + 1);
        progressDialog.setLabelText(tr("Processing")); // i + 1 of maximum

        app_->processEvents();
        if (progressDialog.wasCanceled())
        {
            break;
        }

        // Process step i.
        {
            std::unique_lock<std::mutex> mutexlock(editor_->editorMutex_);
            if (query.nextPage())
            {
                // editor_->applyFilters(query.page());
                // editor_->flush(query.page());
            }
        }
    }
    progressDialog.setValue(progressDialog.maximum());

    app_->resumeThreads();
}

std::vector<Vector3<double>> ComputeHeightMapModifier::createColormap(
    const std::string &name,
    int colorCount)
{
    size_t n = static_cast<size_t>(colorCount);

    if (name == PLUGIN_COMPUTE_HEIGHT_MAP_COLORMAP_MATLAB_JET)
    {
        return ColorPalette::blueCyanYellowRed(n);
    }
    else if (name == PLUGIN_COMPUTE_HEIGHT_MAP_COLORMAP_VTK)
    {
        return ColorPalette::blueCyanGreenYellowRed(n);
    }
    else if (name == PLUGIN_COMPUTE_HEIGHT_MAP_COLORMAP_GRAY)
    {
        return ColorPalette::gray(n);
    }
    else if (name == PLUGIN_COMPUTE_HEIGHT_MAP_COLORMAP_WIN_XP)
    {
        return ColorPalette::WindowsXp32;
    }
    else
    {
        // White.
        std::vector<Vector3<double>> colormap;
        colormap.resize(n);

        for (size_t i = 0; i < n; i++)
        {
            colormap[i].set(1.0, 1.0, 1.0);
        }

        return colormap;
    }
}
