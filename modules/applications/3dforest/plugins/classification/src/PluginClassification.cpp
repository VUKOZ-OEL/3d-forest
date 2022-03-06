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

/** @file PluginClassification.cpp */

// Ignore compiler warnings from Eigen 3rd party library.
#if ((__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)) ||               \
     defined(__clang__))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#include <ColorPalette.hpp>
#include <Editor.hpp>
#include <Log.hpp>
#include <PluginClassification.hpp>
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
#include <QMutex>
#include <QProgressDialog>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <Time.hpp>
#include <WindowDock.hpp>
#include <delaunator.hpp>
#include <igl/writeOBJ.h>

#define PLUGIN_CLASSIFICATION_NAME "Classification"

/** Plugin Classification Window. */
class PluginClassificationWindow : public WindowDock
{
    Q_OBJECT

public:
    PluginClassificationWindow(QMainWindow *parent, Editor *editor);
    ~PluginClassificationWindow() = default;

protected slots:
    void apply();
    void writeGroundMesh();

protected:
    Editor *editor_;
    QWidget *widget_;
    QSpinBox *nPointsSpinBox_;
    QSpinBox *lengthSpinBox_;
    QSpinBox *rangeSpinBox_;
    QSpinBox *angleSpinBox_;
    QCheckBox *liveCheckBox_;
    QPushButton *applyButton_;
    QPushButton *meshButton_;

    void update();
};

PluginClassificationWindow::PluginClassificationWindow(QMainWindow *parent,
                                                       Editor *editor)
    : WindowDock(parent),
      editor_(editor)
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

    meshButton_ = new QPushButton(tr("Export Ground"));
    meshButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(meshButton_, SIGNAL(clicked()), this, SLOT(writeGroundMesh()));

    applyButton_ = new QPushButton(tr("Classify"));
    applyButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(applyButton_, SIGNAL(clicked()), this, SLOT(apply()));

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
    hbox->addWidget(meshButton_, 0, Qt::AlignRight);
    hbox->addWidget(applyButton_, 0, Qt::AlignRight);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addLayout(groupBoxLayout);
    vbox->addSpacing(10);
    vbox->addLayout(hbox);
    vbox->addStretch();

    // Dock
    widget_ = new QWidget;
    widget_->setLayout(vbox);
    widget_->setFixedHeight(150);
    setWidget(widget_);
}

void PluginClassificationWindow::apply()
{
    editor_->cancelThreads();

    size_t pointsPerCell = static_cast<size_t>(nPointsSpinBox_->value());
    double cellLengthMin = static_cast<double>(lengthSpinBox_->value());
    double groundErrorPercent = static_cast<double>(rangeSpinBox_->value());
    double angle = static_cast<double>(angleSpinBox_->value());
    angle = 90.0 - angle; // Ground plane angle to inverted angle for selection.

    double zMax = editor_->clipBoundary().max(2);
    double zMin = editor_->clipBoundary().min(2);
    double zMinCell;
    double zMaxGround;

    size_t nPointsGroundGrid;

    EditorQuery queryPoint(editor_);
    EditorQuery query(editor_);
    query.setGrid(pointsPerCell, cellLengthMin);

    int maximum = static_cast<int>(query.gridSize());
    int i = 0;

    QProgressDialog progressDialog(mainWindow());
    progressDialog.setCancelButtonText(QObject::tr("&Cancel"));
    progressDialog.setRange(0, maximum);
    progressDialog.setWindowTitle(QObject::tr(PLUGIN_CLASSIFICATION_NAME));
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

        editor_->lock();

        // Select grid cell.
        query.selectBox(query.gridCell());
        query.exec();

        // Find local minimum.
        /// @todo Ignore outliers.
        zMinCell = zMax;
        while (query.nextPoint())
        {
            if (query.z() < zMinCell)
            {
                zMinCell = query.z();
            }
        }
        zMaxGround = zMinCell + (((zMax - zMin) * 0.01) * groundErrorPercent);

        // Set classification to 'ground' or 'unassigned'.
        nPointsGroundGrid = 0;
        query.reset();
        while (query.nextPoint())
        {
            if (query.z() > zMaxGround)
            {
                // unassigned (could be a roof)
                query.classification() = FileLas::CLASS_UNASSIGNED;
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

                if (queryPoint.nextPoint())
                {
                    // unassigned (has some points below, inside the cone)
                    query.classification() = FileLas::CLASS_UNASSIGNED;
                }
                else
                {
                    // ground
                    query.classification() = FileLas::CLASS_GROUND;

                    nPointsGroundGrid++;
                }
            }

            query.setModified();
        }

        // Ground surface.
#if 0
        if (nPointsGroundGrid > 0)
        {
            groundXY.resize(nPointsGroundGrid * 2);
            nPointsGroundGrid = 0;
            query.reset();
            while (query.nextPoint())
            {
                if (query.classification() == FileLas::CLASS_GROUND)
                {
                    groundXY[2 * nPointsGroundGrid] = query.x();
                    groundXY[2 * nPointsGroundGrid + 1] = query.y();
                    nPointsGroundGrid++;
                }
            }

            delaunator::Delaunator d(groundXY);

            std::cout << d.triangles.size() << " ground triangles\n";
        }
#endif

        editor_->unlock();
    }

    query.flush();

    progressDialog.setValue(progressDialog.maximum());

    update();

    editor_->restartThreads();
}

void PluginClassificationWindow::update()
{
    editor_->lock();
    editor_->viewports().setState(EditorPage::STATE_READ);
    editor_->unlock();
}

void PluginClassificationWindow::writeGroundMesh()
{
    editor_->cancelThreads();
    editor_->lock();

    // Obtain the number of ground points.
    size_t nPointsGround = 0;

    EditorQuery queryGround(editor_);
    queryGround.selectClassifications({FileLas::CLASS_GROUND});
    queryGround.selectBox(editor_->clipBoundary());
    queryGround.exec();
    while (queryGround.nextPoint())
    {
        nPointsGround++;
    }

    if (nPointsGround == 0)
    {
        // std::cout << "ground points not found\n";
        return;
    }

    // Collect 2D and 3D point coordinates.
    Eigen::MatrixXd V;
    V.resize(nPointsGround, 3);

    std::vector<double> XY;
    XY.resize(nPointsGround * 2);

    nPointsGround = 0;
    queryGround.reset();
    while (queryGround.nextPoint())
    {
        XY[2 * nPointsGround] = queryGround.x();
        XY[2 * nPointsGround + 1] = queryGround.y();

        V(nPointsGround, 0) = queryGround.x();
        V(nPointsGround, 1) = queryGround.z();  // swap for Windows 3D Viewer
        V(nPointsGround, 2) = -queryGround.y(); // swap for Windows 3D Viewer

        nPointsGround++;
    }

    // Create triangle mesh.
    delaunator::Delaunator delaunay(XY);

    // Convert to igl triangles.
    size_t nTriangles = delaunay.triangles.size() / 3;

    Eigen::MatrixXi F;
    F.resize(nTriangles, 3);

    for (size_t i = 0; i < nTriangles; i++)
    {
        F(i, 0) = delaunay.triangles[i * 3];
        F(i, 1) = delaunay.triangles[i * 3 + 1];
        F(i, 2) = delaunay.triangles[i * 3 + 2];
    }

    // Write output file.
    igl::writeOBJ("ground.obj", V, F);

    editor_->unlock();
    editor_->restartThreads();
}

PluginClassification::PluginClassification()
    : window_(nullptr),
      editor_(nullptr)
{
}

void PluginClassification::initialize(QMainWindow *parent, Editor *editor)
{
    // Do not create GUI when this plugin is loaded
    (void)parent;

    editor_ = editor;
}

void PluginClassification::show(QMainWindow *parent)
{
    // Create GUI only when this plugin is used for the first time
    if (!window_)
    {
        window_ = new PluginClassificationWindow(parent, editor_);
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

QAction *PluginClassification::toggleViewAction() const
{
    if (window_)
    {
        return window_->toggleViewAction();
    }

    return nullptr;
}

QString PluginClassification::windowTitle() const
{
    return tr(PLUGIN_CLASSIFICATION_NAME);
}

QString PluginClassification::buttonText() const
{
    return tr("Classifi\ncation");
}

QString PluginClassification::toolTip() const
{
    return tr("Classify Points");
}

QPixmap PluginClassification::icon() const
{
    return QPixmap(":/deviation-ios-50.png");
}

#include "PluginClassification.moc"
