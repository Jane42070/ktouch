/*
 *  Copyright 2012  Sebastian Gottfried <sebastiangottfried@web.de>
 *  Copyright 2015  Sebastian Gottfried <sebastiangottfried@web.de>
 *  Copyright 2017  Sebastian Gottfried <sebastian.gottfried@posteo.de>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick 2.9
import QtQuick.Layouts 1.3
import org.kde.charts 0.1 as Charts
import org.kde.kcoreaddons 1.0
import ktouch 1.0

import "../common"

Item {
    id: root

    property Profile profile

    function update() {
        if (profile) {
            var isNewProfile = root.profile.id === -1
            profileForm.name = profile.name
            profileForm.skillLevel = profile.skillLevel
            profileForm.skillLevelSelectionEnabled = isNewProfile
            deleteConfirmationLabel.name = profile.name
            state = isNewProfile? "editor": "info"
        }
    }

    signal deletionRequest();

    onProfileChanged: update()

    SystemPalette {
        id: activePalette
        colorGroup: SystemPalette.Active
    }

    KColorScheme {
        id: colorScheme
        colorGroup: KColorScheme.Active
        colorSet: KColorScheme.View
    }


    Item {
        id: infoContainer
        width: parent.width
        height: childrenRect.height
        anchors.centerIn: parent

        Column {
            width: parent.width
            height: childrenRect.height
            spacing: 40

            LearningProgressModel {
                id: learningProgressModel
                profile: root.profile
            }


            Rectangle {

                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: 250
                color: activePalette.base

                border {
                    width: 1
                    color: activePalette.text
                }

                Column {
                    id: column
                    anchors {
                        fill: parent
                        topMargin: column.spacing + spacing + legend.height
                        leftMargin: column.spacing
                        rightMargin: column.spacing
                        bottomMargin: column.spacing
                    }

                    spacing: 20

                    width: parent.width
                    height: parent.height - legend.height - parent.spacing

                    LearningProgressChart {
                        id: learningProgressChart
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width
                        height: parent.height - legend.height - parent.spacing
                        backgroundColor: colorScheme.normalBackground
                        textColor: colorScheme.normalText
                        model: learningProgressModel
                    }

                    Row {
                        id: legend
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 20
                        Charts.LegendItem {
                            dimension: learningProgressChart.accuracy
                            textColor: colorScheme.normalText
                        }
                        Charts.LegendItem {
                            dimension: learningProgressChart.charactersPerMinute
                            textColor: colorScheme.normalText
                        }
                    }
                }
            }

            InformationTable {
                id: profileInfoTable
                width: parent.width
                property int trainedLessonCount: profile && profile.id !== -1? profileDataAccess.lessonsTrained(profile): 0
                property list<InfoItem> infoModel: [
                    InfoItem {
                        title: i18n("Lessons trained:")
                        text: profile && profile.id !== -1? profileInfoTable.trainedLessonCount: ""
                    },
                    InfoItem {
                        title: i18n("Total training time:")
                        text: profile && profile.id !== -1? Format.formatDuration(profileDataAccess.totalTrainingTime(profile)): ""
                    },
                    InfoItem {
                        title: i18n("Last trained:")
                        text: profile && profile.id !== -1 && profileInfoTable.trainedLessonCount > 0? profileDataAccess.lastTrainingSession(profile).toLocaleDateString(): i18n("Never")
                    }
                ]

                model: infoModel
            }
        }

        InlineToolbar {
            anchors {
                top: parent.top
                horizontalCenter: parent.horizontalCenter
                topMargin: 5
            }
            content: [
                IconToolButton {
                    iconName: "document-edit"
                    text: i18n("Edit")
                    onClicked: root.state = "editor"
                },
                IconToolButton {
                    iconName: "edit-delete"
                    text: i18n("Delete")
                    enabled: profileDataAccess.profileCount > 1
                    onClicked: root.state = "deleteConfirmation"
                }
            ]
        }
    }

    Item {
        id: editorContainer
        width: parent.width
        height: childrenRect.height
        anchors.centerIn: parent

        ProfileForm {
            id: profileForm
            width: parent.width
            showWelcomeLabel: false
            onDone: {
                root.profile.name = profileForm.name
                root.profile.skillLevel = profileForm.skillLevel
                if (root.profile.id === -1) {
                    profileDataAccess.addProfile(profile)
                }
                else {
                    profileDataAccess.updateProfile(profileDataAccess.indexOfProfile(root.profile))
                }
                root.update()
                root.state = "info"
            }
        }
    }

    Item {
        id: deleteConfirmationContainer
        width: parent.width
        height: childrenRect.height
        anchors.centerIn: parent
        Column {
            width: parent.width
            height: childrenRect.height
            spacing: 15

            Label {
                property string name
                id: deleteConfirmationLabel
                width: parent.width
                text: i18n("Do you really want to delete the profile \"<b>%1</b>\"?", name)
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
            }
            Row {
                spacing: 10
                anchors.horizontalCenter: parent.horizontalCenter
                width: childrenRect.width
                height: childrenRect.height
                IconButton {
                    iconName: "edit-delete"
                    text: i18n("Delete")
                    bgColor: colorScheme.negativeBackground
                    onClicked: root.deletionRequest()
                }
                IconButton {
                    text: i18n("Cancel")
                    onClicked: root.state = "info"
                }
            }
        }
    }

    states: [
        State {
            name: "info"
            PropertyChanges {
                target: infoContainer
                visible: true
            }
            PropertyChanges {
                target: editorContainer
                visible: false
            }
            PropertyChanges {
                target: deleteConfirmationContainer
                visible: false
            }
        },
        State {
            name: "editor"
            PropertyChanges {
                target: infoContainer
                visible: false
            }
            PropertyChanges {
                target: editorContainer
                visible: true
            }
            PropertyChanges {
                target: deleteConfirmationContainer
                visible: false
            }
        },
        State {
            name: "deleteConfirmation"
            PropertyChanges {
                target: infoContainer
                visible: false
            }
            PropertyChanges {
                target: editorContainer
                visible: false
            }
            PropertyChanges {
                target: deleteConfirmationContainer
                visible: true
            }
        }
    ]
}
