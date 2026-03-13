#include "draghandler.h"
#include <QUrl>
#include <QFileInfo>
#include <QMessageBox>

DragHandler::DragHandler()
{
}

bool DragHandler::isValidFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.exists() && fileInfo.isFile();
}

void DragHandler::startDrag(const QString &filePath, QWidget *dragSource)
{
    if (!isValidFile(filePath)) {
        return;
    }

    // Create a drag object
    QDrag *drag = new QDrag(dragSource);

    // Set the file URL as mime data
    QMimeData *mimeData = new QMimeData();
    mimeData->setUrls({QUrl::fromLocalFile(filePath)});
    drag->setMimeData(mimeData);

    // Execute the drag operation
    drag->exec(Qt::CopyAction);
}
