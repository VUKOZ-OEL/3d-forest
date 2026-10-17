#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QWidget>
#include <vector>

#include "grid.h"

#include "imagestamp.h"
#include "helper.h"


namespace Ui
{
    class MainWindowClass;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    static QPlainTextEdit* logSpace();
    ~MainWindow();

private:
    Ui::MainWindowClass *ui;

    ImageStamp mStamp;
    int m_gfxtype;
    float m_pixelpercell;
    static QPlainTextEdit *mLogSpace;
    void loadPicusIniFile(const QString &fileName);
    // painter functions

    //Viewport vp;


private slots:

    void on_reloadFile_clicked();
    void on_openFile_clicked();

    void on_lrReadStamps_clicked();

    void on_lrLoadStamps_clicked();
    void on_lrProcess_clicked(); // lightroom

    void on_lrLightGrid_clicked();
    void on_lrCalcFullGrid_clicked();
    void on_lroTestHemi_clicked();
    void on_testLRO_clicked();
    void on_pbCreateLightroom_clicked();
    void on_actionFON_action_triggered();
    void on_actionEdit_XML_settings_triggered();
    void on_actionLightroom_triggered();


    void on_applyXML_clicked();
    void on_saveFile_clicked();

    void repaintArea(QPainter &painter);


};


#endif // MAINWINDOW_H
