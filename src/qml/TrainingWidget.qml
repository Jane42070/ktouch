import QtQuick 1.0
import ktouch 1.0
import org.kde.plasma.components 0.1 as PlasmaComponents

FocusScope {
    id: trainer
    property int fontSize: 20
    property int position: -1
    property int lineHeight: 2 * fontSize
    property int margin: 30
    signal done
    signal keyPressed(variant event)
    signal keyReleased(variant event)

    Component.onCompleted: {
        trainer.position = 0
        trainingLine.forceActiveFocus()
    }

    Flickable
    {
        id: sheetFlick
        anchors.fill: parent
        contentWidth: parent.width
        contentHeight: sheet.height + 60
        clip: true
        flickableDirection: Flickable.VerticalFlick

        function targetScrollPosition()
        {
            var position = trainingLine.y + sheet.y + (trainingLine.height / 2)
            return Math.max(Math.min((position - (height / 2)), contentHeight - height), 0)
        }

        function scrollToTrainingLine() {
            scrollAnimation.to = targetScrollPosition()
            scrollAnimation.start()
        }

        onHeightChanged: {
            contentY = targetScrollPosition()
        }

        NumberAnimation
        {
            target: sheetFlick
            id: scrollAnimation
            duration: 150
            property: "contentY"
        }

        Rectangle
        {
            id: sheet
            color: "#fff"
            anchors.centerIn: parent
            width: childrenRect.width
            height: childrenRect.height

            border {
                width: 1
                color: "#000"
            }

            Column {
                width: childrenRect.width
                height: childrenRect.height
                Item {
                    height: margin
                    width: 1
                }
                Item {
                    height: fontSize * 2
                    width: titleText.width + 2 * margin
                    Text {
                        id: titleText
                        anchors.centerIn: parent
                        font.pixelSize: fontSize * 1.7
                        text: lesson.title
                    }
                }
                Item {
                    height: 15
                    width: 1
                }
                Repeater {
                    id: lines
                    model: lesson.lineCount
                    Item {
                        property bool isDone: false
                        width: text.width + 2 * margin
                        height: lineHeight
                        Text {
                            id: text
                            anchors.centerIn: parent
                            color: isDone? "#000": "#888"
                            text: lesson.line(index).value
                            font.family: "monospace"
                            font.pixelSize: fontSize
                            opacity: trainer.position == index? 0: 1
                        }
                    }
                }
                Item {
                    height: margin
                    width: 1
                }
            }

            TrainingLine {
                id: trainingLine
                property Item target: lines.itemAt(trainer.position)
                onDone: {
                    lines.itemAt(trainer.position).isDone = true;
                    if (trainer.position < lesson.lineCount - 1)
                    {
                        trainer.position++
                        sheetFlick.scrollToTrainingLine()
                    }
                    else
                    {
                        trainer.done();
                    }
                }
                onKeyPressed: trainer.keyPressed(event)
                onKeyReleased: trainer.keyReleased(event)
                y: target? target.y: 0
                x: 0
                width: target? target.width: 0
                height: lineHeight
                text: trainer.position >= 0 && trainer.position < lesson.lineCount? lesson.line(trainer.position).value: ""
            }
        }
        MouseArea {
            anchors.fill: parent
            onClicked: trainingLine.forceActiveFocus()
        }
    }
    PlasmaComponents.ScrollBar {
        flickableItem: sheetFlick
    }
}

