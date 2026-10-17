import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

Rectangle {
    width: 250
    height: 400
    id: main
    //anchors.fill: parent
    //color:  "gray"
    Image {
        id: splash_image
        source: "qrc:/iland_splash.png"
        visible: rulercolors.caption == '';
        fillMode: Image.PreserveAspectFit
        anchors.fill: parent
    }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
        visible: rulercolors.caption != '';


        ColumnLayout{
            Layout.fillWidth: true

            Text {
                id: rulerCaption
                text: rulercolors.caption
                font.pixelSize: 16
            }
            Text {
                id: rulerDesc
                text: rulercolors.description
                wrapMode: Text.WordWrap
                width: 100
                Layout.margins: 10
                Layout.bottomMargin: 0
                //anchors.top: rulerCaption.bottom
                //anchors.topMargin: 10
                //anchors.leftMargin: 10
                //anchors.rightMargin: 10

            }
            Rectangle { height: 10}

        }
        Rectangle {
            //color: "#ea6b6b"
            Layout.fillHeight: true
            Layout.fillWidth: true



            Rectangle {
                visible: !rulercolors.hasFactors
                anchors.margins: 5
                anchors.leftMargin: 20
                anchors.topMargin: 10
                anchors.fill: parent


                ColumnLayout {
                    id: rulerDetailsLayout

                    CheckBox {
                        id: showRulerDetails
                        text: "Show details"
                    }

                    GroupBox {
                        id: details
                        flat: true
                        visible: showRulerDetails.checked
                        //anchors.top: showRulerDetails.bottom
                        height: visible?50:0

                        // anchors.topMargin: 10
                        Layout.topMargin: 10
                        SpinBox {
                            id: minValueSpin
                            enabled: !rangeAuto.checked
                            decimals: 2
                            minimumValue: -10000
                            maximumValue: 1000000
                            width: 80
                            value: rulercolors.minValue
                            onValueChanged: rulercolors.minValue = value
                        }
                        SpinBox {
                            id: maxValueSpin
                            enabled: !rangeAuto.checked
                            decimals: 2
                            width: 80
                            minimumValue: -10000
                            maximumValue: 1000000
                            value: rulercolors.maxValue
                            anchors.left: minValueSpin.right
                            anchors.leftMargin: 10
                            onValueChanged: rulercolors.maxValue = value
                        }
                        CheckBox {
                            id: rangeAuto
                            anchors.left: maxValueSpin.right
                            anchors.leftMargin: 5
                            text: "Auto"
                            checked: rulercolors.autoScale
                            onClicked: rulercolors.autoScale=rangeAuto.checked
                        }
                    }
                    GroupBox {
                        //anchors.top: details.bottom
                        //anchors.topMargin: 10
                        Layout.topMargin: 10
                        flat: true
                        Column {
                            id: colorRamp
                            anchors.topMargin: 10


                            Repeater {
                                //model: ["yellow", "red", "green", "darkgrey", "blue","yellow", "red", "green", "darkgrey", "blue", "darkgrey", "blue"]
                                model: rulercolors.colors
                                Rectangle {
                                    width: 60; height: 150 / rulercolors.count
                                    color: modelData
                                }
                            }
                        }
                        Text {
                            id: maxValue
                            text: rulercolors.labels[4]
                            anchors.left: colorRamp.right
                            anchors.top: colorRamp.top
                            anchors.topMargin: -height/2
                            anchors.leftMargin: 5
                        }
                        Text {
                            id: upperQuartileValue
                            text: rulercolors.labels[3]
                            anchors.left: colorRamp.right
                            anchors.top:  colorRamp.top
                            anchors.topMargin: colorRamp.height/4 - height/2
                            anchors.leftMargin: 5
                            visible: colorRamp.height>100
                        }
                        Text {
                            id: centerValue
                            text: rulercolors.labels[2]
                            anchors.left: colorRamp.right
                            anchors.verticalCenter:  colorRamp.verticalCenter
                            anchors.topMargin: height/2
                            anchors.leftMargin: 5
                        }
                        Text {
                            id: lowerQuartileValue
                            text: rulercolors.labels[1]
                            anchors.left: colorRamp.right
                            anchors.top:  colorRamp.top
                            anchors.topMargin: colorRamp.height*3/4 - height/2
                            anchors.leftMargin: 5
                            visible: colorRamp.height>100
                        }
                        Text {
                            id: minValue
                            text: rulercolors.labels[0]
                            anchors.left: colorRamp.right
                            anchors.bottom: colorRamp.bottom
                            anchors.topMargin: height/2
                            anchors.leftMargin: 5
                        }
                    }                }


            }
            ScrollView {
                visible: rulercolors.hasFactors
                id: palFactorsList

                anchors.fill: parent
                anchors.leftMargin: 20

                ListView {
                    anchors.fill: parent
                    model: rulercolors.factorLabels
                    delegate: Rectangle {
                        height: 25
                        width: 200
                        Rectangle {
                            id: delColorRect
                            height: 20; width: 50
                            color: rulercolors.colors[index]
                        }

                        Text { text: modelData
                            anchors.top: delColorRect.top;
                            anchors.left: delColorRect.right;
                            anchors.verticalCenter: delColorRect.verticalCenter;
                            anchors.leftMargin: 5
                        }
                    }
                }
            }
        }

        Rectangle {

            id: scale
            height: 40

            //color: "grey"
            width: parent.width
            Text {
                //text: "Meter/px:" + rulercolors.meterPerPixel
                text: "0m"
                height: 20
                anchors.top: scale.top
                anchors.topMargin: 30
            }


            Row{

                anchors.top: scale.top
                anchors.topMargin: 10


                Repeater {
                    id: scaleRep
                    width: parent.width
                    model: 4
                    property real cellWidth
                    cellWidth: { var n = rulercolors.meterPerPixel*main.width/5;
                        var sig=1;
                        var mult = Math.pow(10, sig - Math.floor(Math.log(n) / Math.LN10) - 1);
                        var s= Math.round(n * mult) / mult;
                        //console.log("n: " + n + " s: " + s);
                        return s / rulercolors.meterPerPixel;
                    }
                    Item {
                        width: scaleRep.cellWidth
                        height: 30
                        Rectangle {
                            width: scaleRep.cellWidth
                            height: 20
                            border.width: 1
                            border.color: "#808080"
                            color: index % 2==1?"#808080":"#a0a0a0"
                        }
                        Text {
                            text: Math.round(parent.width * (modelData+1) * rulercolors.meterPerPixel)
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.topMargin: 20
                            anchors.leftMargin: scaleRep.cellWidth-contentWidth/2

                        }
                    }
                }
            }

        }
    }

}
