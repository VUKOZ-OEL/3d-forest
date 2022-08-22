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

/** @file ClassifyGroundPlugin.cpp */

#include <Eigen/Core>
#include <delaunator.hpp>
#include <igl/point_mesh_squared_distance.h>

#include <ClassifyGroundPlugin.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDockWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressDialog>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#define ICON(name) (ThemeIcon(":/classifyground/", name))
#define CLASSIFY_GROUND_PLUGIN_NAME "Classify Ground"
#define CLASSIFY_GROUND_BUFFER_SIZE 8192

/** Classify Ground Window. */
class ClassifyGroundWindow : public QDockWidget
{
    Q_OBJECT

public:
    ClassifyGroundWindow(MainWindow *mainWindow);

protected slots:
    void slotApply();

protected:
    MainWindow *mainWindow_;

    QWidget *widget_;
    QSpinBox *nPointsSpinBox_;
    QSpinBox *lengthSpinBox_;
    QSpinBox *rangeSpinBox_;
    QSpinBox *angleSpinBox_;
    QCheckBox *liveCheckBox_;
    QPushButton *applyButton_;
};

ClassifyGroundWindow::ClassifyGroundWindow(MainWindow *mainWindow)
    : QDockWidget(mainWindow),
      mainWindow_(mainWindow)
{
    // Widgets
    nPointsSpinBox_ = new QSpinBox;
    nPointsSpinBox_->setRange(1000, 1000000);
    nPointsSpinBox_->setValue(100000);
    nPointsSpinBox_->setSingleStep(1);

    lengthSpinBox_ = new QSpinBox;
    lengthSpinBox_->setRange(1, 100);
    lengthSpinBox_->setValue(1);
    lengthSpinBox_->setSingleStep(1);

    rangeSpinBox_ = new QSpinBox;
    rangeSpinBox_->setRange(1, 100);
    rangeSpinBox_->setValue(15);
    rangeSpinBox_->setSingleStep(1);

    angleSpinBox_ = new QSpinBox;
    angleSpinBox_->setRange(1, 89);
    angleSpinBox_->setValue(60);
    angleSpinBox_->setSingleStep(1);

    liveCheckBox_ = new QCheckBox;
    liveCheckBox_->setChecked(false);
    liveCheckBox_->setEnabled(false);

    applyButton_ = new QPushButton(tr("Classify"));
    applyButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(applyButton_, SIGNAL(clicked()), this, SLOT(slotApply()));

    // Layout
    QGridLayout *groupBoxLayout = new QGridLayout;
    groupBoxLayout->addWidget(new QLabel(tr("Points per cell")), 0, 0);
    groupBoxLayout->addWidget(nPointsSpinBox_, 0, 1);
    groupBoxLayout->addWidget(new QLabel(tr("Cell min length (%)")), 1, 0);
    groupBoxLayout->addWidget(lengthSpinBox_, 1, 1);
    groupBoxLayout->addWidget(new QLabel(tr("Ground level (%)")), 2, 0);
    groupBoxLayout->addWidget(rangeSpinBox_, 2, 1);
    groupBoxLayout->addWidget(new QLabel(tr("Ground angle (deg)")), 3, 0);
    groupBoxLayout->addWidget(angleSpinBox_, 3, 1);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(liveCheckBox_);
    hbox->addWidget(new QLabel(tr("Live")));
    hbox->addStretch();
    hbox->addWidget(applyButton_, 0, Qt::AlignRight);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addLayout(groupBoxLayout);
    vbox->addSpacing(10);
    vbox->addLayout(hbox);

    // Dock
    widget_ = new QWidget;
    widget_->setLayout(vbox);
    widget_->setFixedHeight(180);
    setWidget(widget_);
    setWindowTitle(QObject::tr(CLASSIFY_GROUND_PLUGIN_NAME));
    setFloating(true);
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mainWindow_->addDockWidget(Qt::RightDockWidgetArea, this);
}

void ClassifyGroundWindow::slotApply()
{
    mainWindow_->suspendThreads();

    size_t pointsPerCell = static_cast<size_t>(nPointsSpinBox_->value());
    double cellLengthMin = static_cast<double>(lengthSpinBox_->value());
    double groundErrorPercent = static_cast<double>(rangeSpinBox_->value());
    double angle = static_cast<double>(angleSpinBox_->value());
    angle = 90.0 - angle; // Ground plane angle to inverted angle for selection.

    double zMax = mainWindow_->editor().clipBoundary().max(2);
    double zMin = mainWindow_->editor().clipBoundary().min(2);
    double zMinCell;
    double zMaxGround;

    size_t nPointsGroundGrid;
    size_t nPointsAboveGrid;
    Eigen::MatrixXd P;      // Points above ground
    Eigen::MatrixXd V;      // Ground coordinates
    std::vector<double> XY; // Ground xy coordinates
    Eigen::MatrixXi F;      // Ground triangles
    Eigen::MatrixXd D;      // List of smallest squared distances
    Eigen::MatrixXi I;      // List of indices to smallest distances
    Eigen::MatrixXd C;      // 3 list of closest points

    P.resize(CLASSIFY_GROUND_BUFFER_SIZE, 3);
    V.resize(CLASSIFY_GROUND_BUFFER_SIZE, 3);
    XY.resize(CLASSIFY_GROUND_BUFFER_SIZE * 2);

    Query queryPoint(&mainWindow_->editor());
    Query query(&mainWindow_->editor());
    query.setGrid(pointsPerCell, cellLengthMin);

    int maximum = static_cast<int>(query.gridSize());
    int i = 0;

    QProgressDialog progressDialog(mainWindow_);
    progressDialog.setCancelButtonText(QObject::tr("&Cancel"));
    progressDialog.setRange(0, maximum);
    progressDialog.setWindowTitle(QObject::tr(CLASSIFY_GROUND_PLUGIN_NAME));
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setMinimumDuration(0);
    progressDialog.show();

    while (query.nextGrid())
    {
        // Update progress
        i++;
        progressDialog.setValue(i);
        progressDialog.setLabelText(
            QObject::tr("Processing %1 of %n...", nullptr, maximum).arg(i));

        QCoreApplication::processEvents();
        if (progressDialog.wasCanceled())
        {
            break;
        }

        // Select grid cell.
        query.selectBox(query.gridCell());
        query.exec();

        // Find local minimum.
        zMinCell = zMax;
        while (query.next())
        {
            if (query.z() < zMinCell)
            {
                zMinCell = query.z();
            }
        }
        zMaxGround = zMinCell + (((zMax - zMin) * 0.01) * groundErrorPercent);

        // Set classification to 'ground' or 'unassigned'.
        nPointsGroundGrid = 0;
        nPointsAboveGrid = 0;
        query.reset();
        while (query.next())
        {
            if (query.z() > zMaxGround)
            {
                // unassigned (could be a roof)
                query.classification() = LasFile::CLASS_UNASSIGNED;
            }
            else
            {
                queryPoint.setMaximumResults(1);

                queryPoint.selectCone(query.x(),
                                      query.y(),
                                      query.z(),
                                      zMinCell,
                                      angle);

                queryPoint.exec();

                if (queryPoint.next())
                {
                    // unassigned (has some points below, inside the cone)
                    query.classification() = LasFile::CLASS_UNASSIGNED;
                }
                else
                {
                    // ground
                    query.classification() = LasFile::CLASS_GROUND;
                }
            }

            if (query.classification() == LasFile::CLASS_GROUND)
            {
                if (static_cast<Eigen::Index>(nPointsGroundGrid) == V.rows())
                {
                    V.resize(static_cast<Eigen::Index>(nPointsGroundGrid) * 2,
                             3);
                    XY.resize(nPointsGroundGrid * 4);
                }

                V(static_cast<Eigen::Index>(nPointsGroundGrid), 0) = query.x();
                V(static_cast<Eigen::Index>(nPointsGroundGrid), 1) = query.y();
                V(static_cast<Eigen::Index>(nPointsGroundGrid), 2) = query.z();

                XY[2 * nPointsGroundGrid] = query.x();
                XY[2 * nPointsGroundGrid + 1] = query.y();

                nPointsGroundGrid++;
            }
            else
            {
                if (static_cast<Eigen::Index>(nPointsAboveGrid) == P.rows())
                {
                    P.resize(static_cast<Eigen::Index>(nPointsAboveGrid) * 2,
                             3);
                }

                P(static_cast<Eigen::Index>(nPointsAboveGrid), 0) = query.x();
                P(static_cast<Eigen::Index>(nPointsAboveGrid), 1) = query.y();
                P(static_cast<Eigen::Index>(nPointsAboveGrid), 2) = query.z();

                nPointsAboveGrid++;
            }

            query.elevation() = 0;

            query.setModified();
        }

        // Ground surface
        if (nPointsGroundGrid > 0)
        {
            XY.resize(nPointsGroundGrid * 2);

            delaunator::Delaunator delaunay(XY);

            // Convert to igl triangles
            size_t nTriangles = delaunay.triangles.size() / 3;

            F.resize(static_cast<Eigen::Index>(nTriangles), 3);

            for (size_t iTriangle = 0; iTriangle < nTriangles; iTriangle++)
            {
                // Swap the order of the vertices in triangle to 0, 2, 1.
                // This will affect the direction of the normal to face up
                // along z.
                F(static_cast<Eigen::Index>(iTriangle), 0) =
                    static_cast<int>(delaunay.triangles[iTriangle * 3]);
                F(static_cast<Eigen::Index>(iTriangle), 1) =
                    static_cast<int>(delaunay.triangles[iTriangle * 3 + 2]);
                F(static_cast<Eigen::Index>(iTriangle), 2) =
                    static_cast<int>(delaunay.triangles[iTriangle * 3 + 1]);
            }

            // Compute distances from a set of points P to a triangle mesh (V,F)
            igl::point_mesh_squared_distance(P, V, F, D, I, C);

            // Set elevation
            Eigen::Index idx = 0;
            query.reset();
            while (query.next())
            {
                if (query.classification() != LasFile::CLASS_GROUND)
                {
                    if (idx < D.rows() && D(idx) > 0.)
                    {
                        query.elevation() = ::sqrt(D(idx));
                    }

                    idx++;

                    query.setModified();
                }
            }
        } // Ground surface
    }     // Next grid

    query.flush();

    progressDialog.setValue(progressDialog.maximum());

    mainWindow_->editor().viewports().setState(Page::STATE_READ);

    mainWindow_->resumeThreads();
}

ClassifyGroundPlugin::ClassifyGroundPlugin()
    : mainWindow_(nullptr),
      dockWindow_(nullptr)
{
}

void ClassifyGroundPlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    mainWindow_->createAction(nullptr,
                              "Utilities",
                              "Utilities",
                              tr("Classify Ground"),
                              tr("Classify points to ground and unassigned"),
                              ICON("soil"),
                              this,
                              SLOT(slotPlugin()));
}

void ClassifyGroundPlugin::slotPlugin()
{
    // Create GUI only when this plugin is used for the first time
    if (!dockWindow_)
    {
        dockWindow_ = new ClassifyGroundWindow(mainWindow_);
    }

    dockWindow_->show();
    dockWindow_->raise();
    dockWindow_->activateWindow();
}

// Q_OBJECT is used in this cpp file
#include "ClassifyGroundPlugin.moc"
