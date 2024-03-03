#include <QApplication>
#include <QtWidgets>

#include "selfdrive/ui/qt/qt_window.h"
#include "selfdrive/ui/qt/util.h"
#include "selfdrive/ui/qt/widgets/cameraview.h"
#include "selfdrive/ui/turbo.h"


TurboWindow::TurboWindow(QWidget* parent) : QWidget(parent) {
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    mainCamera = new CameraWidget("camerad", VISION_STREAM_WIDE_ROAD, false);
    layout->addWidget(mainCamera);

    overlayCamera = new CameraWidget("camerad", VISION_STREAM_DRIVER, false);
    overlayCamera->setParent(this); overlayCamera->setFixedSize(482, 302);

    // Ensure overlayCamera is a child of the main window to move freely over it
    overlayCamera->setParent(this); overlayCamera->setFixedSize(482, 302);
}

void TurboWindow::showRearView() {
  int margin = 20;
  overlayCamera->move(this->width() - overlayCamera->width() - margin, 
                      this->height() - overlayCamera->height() - margin);
  overlayCamera->setVisible(true);
}

int main(int argc, char *argv[]) {
  initApp(argc, argv);

  QApplication a(argc, argv);
  TurboWindow w;
  setMainWindow(&w);
  w.showFullScreen();
  w.showRearView();

  return a.exec();
}
