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
    @file Forest3dPluginToolExampleWindow.cpp
*/

#include <Forest3dEditor.hpp>
#include <Forest3dPluginToolExampleWindow.hpp>
#include <QCoreApplication>
#include <QDebug>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <Time.hpp>

Forest3dPluginToolExampleWindow::Forest3dPluginToolExampleWindow(
    QWidget *parent,
    Forest3dEditor *editor)
    : QDialog(parent), editor_(editor)
{
    setWindowTitle(tr("Example"));

    label_ = new QLabel(tr("Name"));

    button_ = new QPushButton(tr("&Run"), this);
    connect(button_,
            &QAbstractButton::clicked,
            this,
            &Forest3dPluginToolExampleWindow::run);

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->addWidget(label_, 0, 0);
    mainLayout->addWidget(button_, 0, 1, Qt::AlignRight);
}

Forest3dPluginToolExampleWindow::~Forest3dPluginToolExampleWindow()
{
}

QSize Forest3dPluginToolExampleWindow::minimumSizeHint() const
{
    return QSize(100, 50);
}

QSize Forest3dPluginToolExampleWindow::sizeHint() const
{
    return QSize(200, 50);
}

void Forest3dPluginToolExampleWindow::run()
{
    editor_->cancel();
    editor_->editor_.lock();

    std::vector<OctreeIndex::Selection> selectionL1;
    editor_->editor_.database().select(selectionL1);

    int maximum = static_cast<int>(selectionL1.size());

    QProgressDialog progressDialog(this);
    progressDialog.setCancelButtonText(tr("&Cancel"));
    progressDialog.setRange(0, static_cast<int>(maximum));
    progressDialog.setWindowTitle(tr("Example"));

    for (int i = 0; i < maximum; i++)
    {
        // Update progress
        progressDialog.setValue(i);
        progressDialog.setLabelText(
            tr("Processing %1 of %n...", nullptr, maximum).arg(i));
        QCoreApplication::processEvents();

        if (progressDialog.wasCanceled())
        {
            break;
        }

        // Process [i], TBD selectionL1[i], L2
        size_t idx = static_cast<size_t>(i);
        DatabaseCell *cell = editor_->editor_.database().get(0, idx);
        if (!cell)
        {
            continue;
        }

        msleep(1);
        double zMin = editor_->editor_.boundary().min(2);
        double zLen = editor_->editor_.boundary().max(2) - zMin;

        size_t nRows = cell->xyz.size() / 3;

        if (cell->view.rgb.size() != cell->xyz.size())
        {
            cell->view.rgb.resize(cell->xyz.size());
        }

        for (size_t row = 0; row < nRows; row++)
        {
            // selectionL1[i].partial == false, otherwise select point
            double z = cell->xyz[row * 3 + 2];
            double h = (z - zMin) / zLen;

            // cell->attrib[row].intensity = static_cast<uint16_t>(h * 65535.0);
            cell->view.rgb[row * 3 + 0] = static_cast<float>(h);
            cell->view.rgb[row * 3 + 1] = static_cast<float>(h);
            cell->view.rgb[row * 3 + 2] = static_cast<float>(h);
        }
    }

    editor_->editor_.unlock();
    editor_->render();
}
