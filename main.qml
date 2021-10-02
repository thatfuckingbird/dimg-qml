import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import DImgViewer 1.0

Window {
    id: mainWindow
    visible: true
    width: 1600
    height: 1000
    title: "dimg sample"

    RowLayout {
        DImgViewer {
            width: 800
            height: 1000
            source: sourceEdit.text
            onSourceChanged: fitToSize()
        }
        ColumnLayout {
            width: 800
            TextField {
                id: sourceEdit
                Layout.fillWidth: true
            }
        }
    }
}
