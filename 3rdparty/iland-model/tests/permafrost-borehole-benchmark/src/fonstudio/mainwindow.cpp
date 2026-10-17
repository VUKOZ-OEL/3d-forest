#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtGui>
#include <QtXml>
#include <QMessageBox>
#include <imagestamp.h>
#include "lightroom.h"
#include "stampcontainer.h"
#include "speciesset.h"
#include "exception.h"
#include "paintarea.h"
#include  "globalsettings.h"
//GlobalSettings *GlobalSettings::mInstance = 0;
#include "debugtimer.h"

// global settings
QDomDocument xmldoc;
QDomNode xmlparams;

// global Object pointers
LightRoom *lightroom = 0;
StampContainer *stamp_container=0;
StampContainer *reader_stamp_container=0;
QList<Species*> tree_species;




double distance(const QPointF &a, const QPointF &b)
{
    return sqrt( (a.x()-b.x())*(a.x()-b.x()) + (a.y()-b.y())*(a.y()-b.y()) );
}

QString setting(const QString& paramname)
{
    if (!xmlparams.isNull())
        return xmlparams.firstChildElement(paramname).text();
    else
        return "ERROR";
}


double nrandom(const float& p1, const float& p2)
{
    return p1 + (p2-p1)*(rand()/float(RAND_MAX));
}

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
 {
     switch (type) {
     case QtDebugMsg:
         MainWindow::logSpace()->appendPlainText(msg);
         MainWindow::logSpace()->ensureCursorVisible();
         break;
     case QtWarningMsg:
         MainWindow::logSpace()->appendPlainText(QString("WARNING: %1").arg(msg));
         MainWindow::logSpace()->ensureCursorVisible();
         fprintf(stderr, "WARNING: %s\n", msg.toLocal8Bit().data());
         break;
     case QtCriticalMsg:
         fprintf(stderr, "Critical: %s\n", msg.toLocal8Bit().data());
         break;
     case QtFatalMsg:
         fprintf(stderr, "Fatal: %s\n", msg.toLocal8Bit().data());
         abort();
     }


 }

QPlainTextEdit *MainWindow::mLogSpace=NULL;

QPlainTextEdit* MainWindow::logSpace()
{
   return mLogSpace;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass)
{
    ui->setupUi(this);

    connect( ui->PaintWidget, SIGNAL(needsPainting(QPainter&)),
             this, SLOT(repaintArea(QPainter&)) );


    mLogSpace = ui->logOutput;

    qInstallMessageHandler(myMessageOutput);

    // load xml file
    xmldoc.clear();
    QString argText = QApplication::arguments().last();
    if (QApplication::arguments().count()>1 && !argText.isEmpty())
        ui->initFileName->setText(argText);

    QString xmlFile = Helper::loadTextFile(ui->initFileName->text());

    if (!xmlFile.isEmpty()) {
        ui->iniEdit->setPlainText(xmlFile);

        if (!xmldoc.setContent(xmlFile)) {
            QMessageBox::information(this, "title text", "Cannot set content of XML file " + ui->initFileName->text());
            return;
        }
    }

    on_actionEdit_XML_settings_triggered();
    // update

    qDebug() << "threadcount: " << QThread::idealThreadCount();

}

MainWindow::~MainWindow()
{
    delete ui;
    if (lightroom)
        delete lightroom;
}

void MainWindow::on_applyXML_clicked()
{
    xmldoc.clear();
    // load for edit
    QString xml = ui->iniEdit->toPlainText();
    QString errMsg;
    int errLine, errCol;
    if (!xmldoc.setContent(xml, &errMsg, &errLine, &errCol)) {
        QMessageBox::information(this, "Error applying XML",
                                 QString("Error applying xml line %1, col %2.\nMessage: %3").arg(errLine).arg(errCol).arg(errMsg));
        return;
    }
    qDebug() << "XML file read.";
}


void MainWindow::on_saveFile_clicked()
{
    QString content = ui->iniEdit->toPlainText();
    if (!content.isEmpty())
         Helper::saveToTextFile(ui->initFileName->text(), content);

}


void MainWindow::repaintArea(QPainter &)
{
    // select drawing type...
    switch (m_gfxtype) {
        case 0: // paint FON cells
            return;
        case 1:  // paint Lightroom - studio --- painting is done on the background image of PaintArea
            break;
        case 2: // paint Lightroom
        default: break; // no painting
    }
    // fix viewpoint
    //vp.setScreenRect(ui->PaintWidget->rect());
}

void MainWindow::on_actionLightroom_triggered()
{
    ui->editStack->setCurrentIndex(1);
    ui->headerCaption->setText("Lightroom");
    m_gfxtype = 1;
    ui->PaintWidget->update();
}

void MainWindow::on_actionEdit_XML_settings_triggered()
{

    ui->editStack->setCurrentIndex(0);

    ui->PaintWidget->update();
}

void MainWindow::on_actionFON_action_triggered()
{
    ui->editStack->setCurrentIndex(2);
    ui->headerCaption->setText("Field Of Neighborhood test environment");
    m_gfxtype = 0;
    ui->PaintWidget->update();

}

void MainWindow::on_pbCreateLightroom_clicked()
{


    if (xmldoc.isNull()) {
        Helper::msg("!!XML not loaded!!");
        return;
    }
    QDomElement docLightroom = xmldoc.documentElement(); // top element

    if (docLightroom.isNull()) MSGRETURN("cant find node 'lightroom' in xml");
    int x,y,z;
    x = docLightroom.firstChildElement("size").attribute("x").toInt();
    y = docLightroom.firstChildElement("size").attribute("y").toInt();
    z = docLightroom.firstChildElement("size").attribute("z").toInt();
    double cellsize = docLightroom.firstChildElement("size").attribute("cellsize").toDouble();

    int hemisize = docLightroom.firstChildElement("hemigrid").attribute("size").toInt();
    double lat = docLightroom.firstChildElement("hemigrid").attribute("latitude").toFloat();
    double diffus = docLightroom.firstChildElement("hemigrid").attribute("diffus").toFloat();

    // create a lightroom object...
    if (!lightroom)
        lightroom = new LightRoom();

    lightroom->setup(x,y,z,cellsize,
                    hemisize,lat,diffus);
    LightRoomObject *lro = new LightRoomObject();
    lro->setuptree(40., 20., "5*(1-x*x)");
    lightroom->setLightRoomObject(lro);

    qDebug() << "Lightroom setup complete";
}

void MainWindow::on_testLRO_clicked()
{
        // setup a lightroom object, and do some tests...
    double x = ui->lr_x->text().toDouble();
    double y = ui->lr_y->text().toDouble();
    double z = ui->lr_z->text().toDouble();
    double azi = ui->lrAzim->text().toDouble();
    double elev = ui->lrElevation->text().toDouble();

    LightRoomObject lro;
    lro.setuptree(40., 10., "5*(1-x*x)");
    qDebug()<<"x:"<<x<<"y:"<<y<<"z:"<<z<<"azimuth:"<<azi<<"elevation:"<<elev << lro.hittest(x,y,z,RAD(azi), RAD(elev));
    //qDebug()<<"-10,-10,0 - azimuth 42, elev: 45:" << lro.hittest(-10,-10,0,RAD(42), RAD(45));
}

void MainWindow::on_lroTestHemi_clicked()
{
    double x = double(ui->lrSliderX->value());// ui->lr_x->text().toDouble();
    double y = ui->lrSliderY->value();
    double z = ui->lrSliderZ->value();
    ui->lr_x->setText(QString::number(x));
    ui->lr_y->setText(QString::number(y));
    ui->lr_z->setText(QString::number(z));
    if (!lightroom)
        MSGRETURN("Lightroom NULL!");
    DebugTimer t("single point");
    double res = lightroom->calculateGridAtPoint(x,y,z);
    ui->lrHemiValue->setText(QString::number(res));
    // now paint...
    //ui->PaintWidget->drawImage();
    lightroom->shadowGrid().paintGrid(ui->PaintWidget->drawImage());
    ui->PaintWidget->update(); // repaint
    //qDebug() << lightroom->shadowGrid().dumpGrid();

}
void MainWindow::on_lrLightGrid_clicked()
{
    lightroom->solarGrid().paintGrid(ui->PaintWidget->drawImage());
    ui->PaintWidget->update(); // repaint
    qDebug() << lightroom->solarGrid().dumpGrid();
}

void MainWindow::on_lrCalcFullGrid_clicked()
{
    if (!lightroom)
        MSGRETURN("Lightroom NULL!");
    lightroom->calculateFullGrid();
    float maxvalue = lightroom->result().max();
    qDebug() << "maxvalue" << maxvalue;
    const FloatGrid &result = lightroom->result();
    QString res;
    for (int x=0;x<result.sizeX();x++){
        for (int y=0;y<result.sizeY();y++) {
            res+=QString::number(result.constValueAtIndex(QPoint(x,y))) + ";";
        }
        res+="\n";
    }
    qDebug()<< res;
}


void MainWindow::on_lrProcess_clicked()
{
    if (xmldoc.isNull()) {
        Helper::msg("!!XML not loaded!!");
        return;
    }

    QDomElement docElem = xmldoc.documentElement();

    QString output_file = docElem.firstChildElement("outputStamp").text();
    QDomElement tree = docElem.firstChildElement("trees").firstChildElement("tree");

    double cut_threshold = docElem.firstChildElement("cutvalue").text().toDouble();
    QString cut_mode = docElem.firstChildElement("cutoffMode").text();
    QString agg_mode = docElem.firstChildElement("aggregationMode").text();
    int mode=-1;
    if (agg_mode=="sum") {
        mode=1;
        qDebug() << "aggregation mode set to 'sum'";
    }
    if (agg_mode=="mean") {
        qDebug() << "aggregation mode set to 'mean'";
        mode=0;
    }
    if (mode==-1) {
        Helper::msg("Error: invalid or no aggregationMode specified!");
        return;
    }

    QString stamp_desc = docElem.firstChildElement("desc").text();
    QString binaryReaderStampFile =  docElem.firstChildElement("readerStamp").text();
    qDebug() << "cutting stamps when averaged absoulte value of rings is below"<<cut_threshold;
    qDebug() << "reading binary stamp reader file" << binaryReaderStampFile;

    if (!Helper::question(QString("Create writer stamps?\ntarget=%1, \nreader stamps=%2").arg(output_file, binaryReaderStampFile)))
        return;
    float crown, height, bhd;
    QString formula, name;

    LightRoomObject *lro = new LightRoomObject();
    lightroom->setLightRoomObject(lro);
    lightroom->setAggregationMode(mode);

    StampContainer readers;

    QFile infile(binaryReaderStampFile);
    infile.open(QIODevice::ReadOnly);
    QDataStream in(&infile);
    readers.load(in);
    infile.close();

    StampContainer container;
    container.useLookup(false); // disable lookup table (not necessary for creation)

    DebugTimer creation_time("total creation of stamps");

    while (!tree.isNull()) {
        name = tree.attribute("name");
        height = tree.attribute("h").toDouble();
        crown = tree.attribute("crown").toDouble();
        bhd = tree.attribute("bhd").toDouble();
        formula = tree.attribute("formula");
        ///////////////////////////
        lro->setuptree(height, crown, formula);
        qDebug() << "start" << name;
        lightroom->calculateFullGrid();

        // store as textfile:
        //result = gridToString( lightroom->result() );
        //Helper::saveToTextFile(path + "\\ " + name + ".txt", result);

        // save stamp as image:
        //QImage img = gridToImage( lightroom->result() );
        //img.save(path + "\\ " + name + ".jpg", "JPG", 100);

        // averaged copy: FloatGrid gr = lightroom->result().averaged(avg_cells);
        const FloatGrid &gr = lightroom->result();
        // calculate sums...
        QVector<double> sums; // stores sum per ring (rectangle)
        QVector<double> rel_sum; // stores sum/px
        QVector<double> max_vals; // stores values in north direction
        double sum;
        int ring_count;
        double total_sum = 0.;
        for (int o=0; o<gr.sizeX()/2; o++) {
            sum = 0; ring_count=0;
            // top and bottom
            for (int i=o; i<gr.sizeX()-o; i++) {
                sum += gr(i, o);
                sum += gr(i, gr.sizeX()-1-o);
                ring_count+=2;
            }
            // left, right :: do not calculate the edges two times....
            for (int i=o+1; i<gr.sizeX()-o-1; i++) {
                sum += gr(o, i);
                sum += gr(gr.sizeX()-1-o, i);
                ring_count+=2;
            }
            total_sum += sum;
            sums.push_back(sum);
            rel_sum.push_back(sum / double(ring_count) );
            max_vals.push_back(gr(gr.sizeX()/2, gr.sizeX()-1-o)); // bottom....
        }
        if (gr.sizeX()% 2) {
            total_sum += gr(gr.sizeX()/2, gr.sizeX()/2); // center pixel for unevenly sized grids
            sums.push_back(gr(gr.sizeX()/2, gr.sizeX()/2)); // center pixel for unevenly sized grids
            rel_sum.push_back(gr(gr.sizeX()/2, gr.sizeX()/2)); // center pixel for unevenly sized grids
            max_vals.push_back(gr(gr.sizeX()/2, gr.sizeX()/2)); // center pixel
        }
        int end_ring, target_grid_size;
        /* version < 20090905: average ring value
        for (end_ring=0;end_ring<rel_sum.count();end_ring++)
            if (rel_sum[end_ring]>cut_threshold)
                break;
        end_ring = rel_sum.count() - end_ring; // */

        // version > 20090905: based on total area
        if (cut_mode == "sum") {
            double rsum = 0;
            for (end_ring=0;end_ring<sums.count();end_ring++) {
                rsum += sums[end_ring];
                // threshold: sum of influence > threshold
                if (rsum>cut_threshold*total_sum)
                    break;
            }
        } else if (cut_mode == "north") {
            // 20100527: based on direction with maximum values
            for (end_ring=0;end_ring<sums.count();end_ring++) {
                if (max_vals[end_ring]>cut_threshold)
                   break;
            }

        } else {
           qDebug() << "ERROR: invalid cutoffMode (possible values: sum, north)!!! Exiting.";
           return;
        }
        end_ring = sums.count() - end_ring;
        if (end_ring<2) // minimum ring-count=2 (i.e. 9pixel)
            end_ring=2;

        target_grid_size = 2*end_ring - 1; // e.g. 3rd ring -> 5x5-matrix
        qDebug() << "break at ring" << end_ring << "with cutoff mode" << cut_mode;
        qDebug() << "circle sum relsum maxvals";
        for (int i=0;i<sums.count();i++)
            qDebug() << i << sums[i] << rel_sum[i] << max_vals[i];


        // test: use subpixel averages ....
        /*
        FloatGrid gr3x3 = lightroom->result().averaged(3);
        QImage img3x3 = gridToImage( gr3x3 );
        img3x3.save(path + "\\ " + name + "_3x3.jpg", "JPG", 100);
        Helper::saveToTextFile(path + "\\ " + name + "_3x3.txt", gridToString(gr3x3));
        result="";
        for (int x=0;x<3;x++)
            for (int y=0;y<3;y++) {
            FloatGrid gr3x3 = lightroom->result().averaged(3,x,y);
            result+="\r\n" + gridToString(gr3x3);

        }
        Helper::saveToTextFile(QString("%1\\%2_shift.txt").arg(path, name), result); */

        // store to container
        Stamp *stamp = stampFromGrid(gr, target_grid_size);
        if (!stamp)
            return;
        qDebug() << "created stamp with size (n x n)" << stamp->size() << "in an data-block of:" << stamp->dataSize();
        // process reading area
        double maxradius = lro->maxRadius();
        const Stamp *readerstamp = readers.readerStamp(maxradius);
        if (readerstamp) {
            int offset = stamp->offset() - readerstamp->offset();
            qDebug() << "fetching read-sum. offset stamp" << stamp->offset() << "offset readerstamp" << readerstamp->offset();
            double sum = 0;
            for (int x=0;x<readerstamp->size();x++)
                for (int y=0;y<readerstamp->size(); y++)
                    sum += *(readerstamp->data(x,y)) * *(stamp->data(x+offset, y+offset));
            qDebug() << "sum of reader-area over stamp" << sum;
        } else qDebug() << "!!! no readerstamp available!!!";

        double hd = qRound( height*100 / bhd );
        container.addStamp(stamp,bhd, hd, lro->maxRadius()); // 3rd param was: ,
        ///////////////////////////
        tree = tree.nextSiblingElement("tree");
    }
    qDebug() << "finished!!!";
    // write container to a file....
    QFile file(output_file);
    file.open(QIODevice::WriteOnly);
    container.setDescription(stamp_desc); // provided in xml...
    QDataStream out(&file);   // we will serialize the data into the file
    container.save(out);
    file.close();
    qDebug() << "current content of the container:";
    qDebug() << container.dump();
}


void MainWindow::on_lrLoadStamps_clicked()
{
    {
        QString fileName = Helper::fileDialog("Name for binary stamp file");
        if (fileName.isEmpty())
            return;
        QFile infile(fileName);
        infile.open(QIODevice::ReadOnly);
        QDataStream in(&infile);
        StampContainer container;
        container.load(in);
        infile.close();
        qDebug() << "Dumping content of Stamp-container" << fileName;
        qDebug() << container.dump();
        // and the reader-stamps....
    }
}


// create "reader" stamps....
void MainWindow::on_lrReadStamps_clicked()
{
    FloatGrid grid; // use copy ctor
    grid.setup(QRectF(-21., -21, 42, 42),2.); // 0/0 is the center of the center cell
    StampContainer container;
    int totcount=0;
    DebugTimer t;
    for (double radius=0.5; radius<=15; radius+=0.1) {
        qDebug() << "creation of reader stamp for radius" << radius;
        grid.initialize(0.);
        float x,y;
        int tested=0, yes=0;
        // the center point of the cell where the center of the tree is,
        // is at 1m/1m (at a cell width of 2m)
        for (x=-radius;x<=radius;x+=0.01)
            for (y=-radius;y<=radius;y+=0.01) {
                tested++;
                if ( x*x + y*y < radius*radius) {
                    grid.valueAt(x,y)++; yes++;
                }
            }
        qDebug() << "tested" << tested << "hit" << yes << "ratio" << 4.*yes/double(tested); // that should be pi, right?
        totcount+=tested;
        FloatGrid ngrid = grid.normalized(1.); // normalize with 1
        // create a stamp with a fitting size
        Stamp *stamp;
        int width=11;
        // radius: 0..<1: 1, >=1 <3: 3, >=3 < 5: 5, ...
        width = (((int)radius+1)/2)*2 + 1;

        stamp = stampFromGrid(ngrid,width);
        // save stamp
        container.addReaderStamp(stamp, radius);

    } // for (radius)
    qDebug() << "tested a total of" << totcount;
    QString targetFile = xmldoc.documentElement().firstChildElement("readerStamp").text();
    if (!Helper::question(QString("Save readerfile to %1?").arg(targetFile))) {
        qDebug() << container.dump();
        return;
    }
    QFile file(targetFile);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Error opening reader file" << targetFile << "with error:" << file.errorString();
    }
    QDataStream out(&file);   // we will serialize the data into the file
    container.setDescription("Reader-stamps for crown-radii ranging from 0.5m to 15m, stepwidth is 0.1m.");
    container.save(out);
    file.close();
    qDebug() << container.dump();


}



void MainWindow::on_openFile_clicked()
{
    QString fileName = Helper::fileDialog("select XML-ini file for FonStudio...");
    if (fileName.isEmpty())
        return;
    ui->initFileName->setText(fileName);
    QString xmlFile = Helper::loadTextFile(ui->initFileName->text());
    ui->iniEdit->setPlainText(xmlFile);
}


void MainWindow::on_reloadFile_clicked()
{
    QString xmlFile = Helper::loadTextFile(ui->initFileName->text());
    ui->iniEdit->setPlainText(xmlFile);
}
