/********************************************************************************
** Form generated from reading UI file 'ribbontabcontent.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RIBBONTABCONTENT_H
#define UI_RIBBONTABCONTENT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_RibbonTabContent
{
public:
    QGridLayout *gridLayout;
    QScrollArea *ribbonTabScrollArea;
    QWidget *ribbonTabScrollAreaContent;
    QGridLayout *gridLayout_2;
    QWidget *spacer;
    QHBoxLayout *ribbonHorizontalLayout;

    void setupUi(QWidget *RibbonTabContent)
    {
        if (RibbonTabContent->objectName().isEmpty())
            RibbonTabContent->setObjectName(QString::fromUtf8("RibbonTabContent"));
        RibbonTabContent->resize(400, 90);
        gridLayout = new QGridLayout(RibbonTabContent);
        gridLayout->setSpacing(0);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        ribbonTabScrollArea = new QScrollArea(RibbonTabContent);
        ribbonTabScrollArea->setObjectName(QString::fromUtf8("ribbonTabScrollArea"));
        ribbonTabScrollArea->setFrameShape(QFrame::NoFrame);
        ribbonTabScrollArea->setFrameShadow(QFrame::Plain);
        ribbonTabScrollArea->setLineWidth(0);
        ribbonTabScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ribbonTabScrollArea->setWidgetResizable(true);
        ribbonTabScrollAreaContent = new QWidget();
        ribbonTabScrollAreaContent->setObjectName(QString::fromUtf8("ribbonTabScrollAreaContent"));
        ribbonTabScrollAreaContent->setGeometry(QRect(0, 0, 400, 90));
        gridLayout_2 = new QGridLayout(ribbonTabScrollAreaContent);
        gridLayout_2->setSpacing(0);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        spacer = new QWidget(ribbonTabScrollAreaContent);
        spacer->setObjectName(QString::fromUtf8("spacer"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(spacer->sizePolicy().hasHeightForWidth());
        spacer->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(spacer, 0, 1, 1, 1);

        ribbonHorizontalLayout = new QHBoxLayout();
        ribbonHorizontalLayout->setSpacing(0);
        ribbonHorizontalLayout->setObjectName(QString::fromUtf8("ribbonHorizontalLayout"));
        ribbonHorizontalLayout->setContentsMargins(0, 3, 0, 0);

        gridLayout_2->addLayout(ribbonHorizontalLayout, 0, 0, 1, 1);

        ribbonTabScrollArea->setWidget(ribbonTabScrollAreaContent);

        gridLayout->addWidget(ribbonTabScrollArea, 0, 0, 1, 1);


        retranslateUi(RibbonTabContent);

        QMetaObject::connectSlotsByName(RibbonTabContent);
    } // setupUi

    void retranslateUi(QWidget *RibbonTabContent)
    {
        RibbonTabContent->setWindowTitle(QCoreApplication::translate("RibbonTabContent", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class RibbonTabContent: public Ui_RibbonTabContent {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RIBBONTABCONTENT_H
