import QtQuick 2.4
import QtQuick.Controls 2.4

import tlQtQuick 1.0

ApplicationWindow {
    id: window
    width: 1280
    height: 720
    visible: true
    
    GLFramebufferObject {
        id: fbo
        anchors.fill: parent

        Connections {
            target: timelinePlayer
            function onCurrentVideoChanged(video) { fbo.setVideo(video) }
        }
    }
}
