/********************************************************************************
** Form generated from reading UI file 'ribbonbuttongroup.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RIBBONBUTTONGROUP_H
#define UI_RIBBONBUTTONGROUP_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_RibbonButtonGroup
{
public:
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    //QFrame *horizontalLine;
    //QLabel *label;
    QFrame *verticalLine;

    void setupUi(QWidget *RibbonButtonGroup)
    {
        if (RibbonButtonGroup->objectName().isEmpty())
            RibbonButtonGroup->setObjectName(QString::fromUtf8("RibbonButtonGroup"));
        RibbonButtonGroup->resize(150, 37);
        gridLayout = new QGridLayout(RibbonButtonGroup);
        gridLayout->setSpacing(0);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        //verticalLayout->setContentsMargins(2, 2, 2, 2);
        verticalLayout->setContentsMargins(0, 0, 0, 0);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);

        verticalLayout->addLayout(horizontalLayout);

        // horizontalLine = new QFrame(RibbonButtonGroup);
        // horizontalLine->setObjectName(QString::fromUtf8("horizontalLine"));
        // horizontalLine->setStyleSheet(QString::fromUtf8("color: #c0c0c0;"));
        // horizontalLine->setFrameShadow(QFrame::Plain);
        // horizontalLine->setFrameShape(QFrame::HLine);

        // verticalLayout->addWidget(horizontalLine);

        // label = new QLabel(RibbonButtonGroup);
        // label->setObjectName(QString::fromUtf8("label"));
        // label->setAlignment(Qt::AlignCenter);

        // verticalLayout->addWidget(label);


        gridLayout->addLayout(verticalLayout, 0, 0, 1, 1);

        verticalLine = new QFrame(RibbonButtonGroup);
        verticalLine->setObjectName(QString::fromUtf8("verticalLine"));
        verticalLine->setStyleSheet(QString::fromUtf8("color: #c0c0c0;"));
        verticalLine->setFrameShadow(QFrame::Plain);
        verticalLine->setFrameShape(QFrame::VLine);

        gridLayout->addWidget(verticalLine, 0, 1, 1, 1);


        retranslateUi(RibbonButtonGroup);

        QMetaObject::connectSlotsByName(RibbonButtonGroup);
    } // setupUi

    void retranslateUi(QWidget *RibbonButtonGroup)
    {
        RibbonButtonGroup->setWindowTitle(QCoreApplication::translate("RibbonButtonGroup", "Form", nullptr));
        //label->setText(QCoreApplication::translate("RibbonButtonGroup", "ToolBar", nullptr));
    } // retranslateUi

};

namespace Ui {
    class RibbonButtonGroup: public Ui_RibbonButtonGroup {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RIBBONBUTTONGROUP_H
