pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import com.LFWQSP2641.qmlcomponents

ApplicationWindow {
    id: window
    visible: true
    width: 800
    height: 600
    title: "Star"

    readonly property bool portraitMode: width < height

    Shortcut {
        sequences: ["Esc", "Back"]
        enabled: stackView.depth > 1
        onActivated: navigateBackAction.trigger()
    }

    Action {
        id: navigateBackAction
        text: stackView.depth > 1 ? qsTr("Back") : ( window.portraitMode ?  qsTr("Menu") : "" )
        icon.source: stackView.depth > 1 ? "qrc:/quick/icon/angle-left.svg" : ( window.portraitMode ?  "qrc:/quick/icon/menu-burger.svg" : "" )
        onTriggered: {
            if (stackView.depth > 1) {
                stackView.pop()
                listView.currentIndex = -1
            } else if (window.portraitMode) {
                drawer.open()
            }
        }
    }

    header: ToolBar {
        ToolButton {
            anchors.left: parent.left
            anchors.leftMargin: window.portraitMode ? 0 : drawer.width
            anchors.verticalCenter: parent.verticalCenter
            action: navigateBackAction
            visible: window.portraitMode || stackView.depth > 1
        }

        Label {
            id: titleLabel
            anchors.centerIn: parent
            text: qsTr("Star")
            font.pixelSize: 20
            elide: Label.ElideRight
        }

    }

    Drawer {
        id: drawer

        width: listView.maxWidth + 4
        height: window.height
        modal: window.portraitMode
        interactive: window.portraitMode ? (stackView.depth === 1) : false
        position: window.portraitMode ? 0 : 1
        visible: !window.portraitMode

        ListView {
            id: listView

            property double maxWidth: 0

            focus: true
            currentIndex: -1
            anchors.fill: parent

            model: ListModel {
                ListElement { title: qsTr("Three Body Motion (C++)"); source: "item:cppThreeBodyMotionComponent" }
                ListElement { title: qsTr("Three Body Motion"); source: "qrc:/quick/qml/ThreeBodyMotion_qml.qml" }
                ListElement { title: qsTr("Launching Ball"); source: "qrc:/quick/qml/LaunchingBall.qml" }
            }

            delegate: ItemDelegate {
                id: delegateItem
                text: title
                highlighted: ListView.isCurrentItem

                required property int index
                required property var model
                required property string title
                required property string source

                onClicked: {
                    parent.currentIndex = index
                    if (window.portraitMode)
                    {
                        drawer.close()
                    }
                    else
                    {
                        listView.currentIndex = -1
                        stackView.pop(initialPage, StackView.Immediate)
                    }
                    if(source === "item:cppThreeBodyMotionComponent")
                    {
                        stackView.push(cppThreeBodyMotionComponent)
                    }
                    else
                    {
                        stackView.push(source)
                    }
                }

                Component.onCompleted: {
                    listView.maxWidth = Math.max(delegateItem.width, listView.maxWidth)
                }
            }

            ScrollIndicator.vertical: ScrollIndicator { }
        }
    }

    StackView {
        id: stackView

        anchors.fill: parent
        anchors.leftMargin: !window.portraitMode ? drawer.width : undefined

        initialItem: Pane {
            id: initialPage

            Label {
                text: qsTr("Select an item from the menu")
                anchors.centerIn: parent
            }

            Text {
                id: arrow
                text: "<"
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                visible: window.portraitMode
            }
        }
    }

    Component {
        id: cppThreeBodyMotionComponent
        ThreeBodyMotion {
            id: cppThreeBodyMotion
            Component.onCompleted: {
                cppThreeBodyMotion.initialize()
            }
        }
    }
}
