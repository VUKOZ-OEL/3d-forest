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

/** @file WindowSegmentation.cpp */

#include <ColorPalette.hpp>
#include <Editor.hpp>
#include <Log.hpp>
#include <QCheckBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <WindowSegmentation.hpp>

WindowSegmentation::WindowSegmentation(QWidget *parent) : QWidget(parent)
{
    // Widgets
    radiusSpinBox_ = new QDoubleSpinBox;
    radiusSpinBox_->setRange(0.1, 99999.0);
    radiusSpinBox_->setValue(10.0);

    deltaSpinBox_ = new QDoubleSpinBox;
    deltaSpinBox_->setRange(0.01, 1.0);
    deltaSpinBox_->setValue(0.1);

    runButton_ = new QPushButton(tr("&Run"), this);
    connect(runButton_,
            &QAbstractButton::clicked,
            this,
            &WindowSegmentation::run);

    // Layout
    QGridLayout *mainLayout = new QGridLayout();
    int row = 0;

    mainLayout->addWidget(new QLabel(tr("Radius")), row, 0);
    mainLayout->addWidget(radiusSpinBox_, row, 1, 1, 2);
    row++;

    mainLayout->addWidget(new QLabel(tr("Step")), row, 0);
    mainLayout->addWidget(deltaSpinBox_, row, 1, 1, 2);
    row++;

    mainLayout->addWidget(runButton_, row, 2, Qt::AlignRight);
    row++;

    mainLayout->setVerticalSpacing(0);
    mainLayout->setColumnStretch(1, 1);
    setLayout(mainLayout);

    // Window
    setFixedHeight(110);
}

WindowSegmentation::~WindowSegmentation()
{
}

void WindowSegmentation::setEditor(Editor *editor)
{
    editor_ = editor;
}

void WindowSegmentation::run()
{
    double radius = radiusSpinBox_->value();
    double delta = deltaSpinBox_->value();

    editor_->cancelThreads();

    segmentation(editor_, radius, delta);

    editor_->restartThreads();

    emit finished();
}

void WindowSegmentation::segmentation(Editor *editor,
                                      double radius,
                                      double delta)
{
    size_t nPoints = 0;
    size_t iPoint = 1;

    EditorQuery query(editor);
    query.selectClassifications({FileLas::CLASS_UNASSIGNED});
    query.selectBox(editor->clipBoundary());
    query.exec();

    while (query.nextPoint())
    {
        nPoints++;
    }

    LOG_WINDOW_SEGMENTATION("nPoints=" << nPoints);

    if (nPoints == 0)
    {
        return;
    }

    std::vector<Vector3<double>> clusterList;
    clusterList.reserve(nPoints);

    Vector3<double> p;
    Vector3<double> m;

    EditorQuery queryPoint(editor_);
    queryPoint.selectClassifications({FileLas::CLASS_UNASSIGNED});

    query.reset();

    while (query.nextPoint())
    {
        int i = 0;

        m.clear();
        p.set(query.x(), query.y(), query.z());
        // LOG_WINDOW_SEGMENTATION(iPoint << "/" << nPoints);
        //  LOG_WINDOW_SEGMENTATION(iPoint << "/" << nPoints << ", point=" <<
        //  p);

        while (i < 1000000)
        {
            size_t nPointsSelect = 0;

            queryPoint.selectSphere(p[0], p[1], p[2], radius);
            queryPoint.exec();

            while (queryPoint.nextPoint())
            {
                m[0] += queryPoint.x();
                m[1] += queryPoint.y();
                m[2] += queryPoint.z();

                nPointsSelect++;
            }

            if (nPointsSelect > 0)
            {
                m[0] /= nPointsSelect;
                m[1] /= nPointsSelect;
                m[2] /= nPointsSelect;
            }
            else
            {
                m = p;
            }

            double step = (m - p).length();
            if (step < delta * radius)
            {
                break;
            }

            p = m;
            i++;
        }

        // LOG_WINDOW_SEGMENTATION("peak=" << m);

        size_t clusterIdx;
        bool clusterFound = false;
        for (size_t idx = 0; idx < clusterList.size(); idx++)
        {
            double clusterDistance = (m - clusterList[idx]).length();
            if (clusterDistance < delta * radius)
            {
                clusterIdx = idx;
                clusterFound = true;
                break;
            }
        }

        if (!clusterFound)
        {
            clusterIdx = clusterList.size();
            clusterList.push_back(m);
        }

        query.layer() = clusterIdx;
        if (clusterIdx < 16)
        {
            Vector3<float> rgb = ColorPalette::Classification[clusterIdx];
            query.userRed() = rgb[0];
            query.userGreen() = rgb[1];
            query.userBlue() = rgb[2];
        }
        query.setModified();

        iPoint++;
    }

    query.flush();
}
