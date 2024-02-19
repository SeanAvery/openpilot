#include <QApplication>
#include <QtWidgets>

#include "selfdrive/ui/qt/qt_window.h"
#include "selfdrive/ui/qt/util.h"
#include "selfdrive/ui/qt/widgets/cameraview.h"

int main(int argc, char *argv[]) {
  initApp(argc, argv);

  QApplication a(argc, argv);
  QWidget w;
  setMainWindow(&w);

  QVBoxLayout *layout = new QVBoxLayout(&w);
  layout->setMargin(0);
  layout->setSpacing(0);

  // Fullscreen CameraWidget for VISION_STREAM_WIDE_ROAD
  CameraWidget *mainCamera = new CameraWidget("camerad", VISION_STREAM_WIDE_ROAD, false);
  layout->addWidget(mainCamera);

  // Overlay CameraWidget for VISION_STREAM_DRIVER
  CameraWidget *overlayCamera = new CameraWidget("camerad", VISION_STREAM_DRIVER, false);

  // Ensure overlayCamera is a child of the main window to move freely over it
  overlayCamera->setParent(&w); overlayCamera->setFixedSize(482, 302);

  // Position the overlayCamera at the bottom right
  int margin = 20;
  overlayCamera->move(w.width() - overlayCamera->width() - margin, 
                      w.height() - overlayCamera->height() - margin);

  w.showFullScreen();
  overlayCamera->setVisible(true);

  return a.exec();
}
