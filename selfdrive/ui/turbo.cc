#include <QApplication>
#include <QtWidgets>
#include <QTimer>
#include <iostream>
#include <memory>

#include "selfdrive/ui/qt/qt_window.h"
#include "selfdrive/ui/qt/util.h"
#include "selfdrive/ui/qt/widgets/cameraview.h"
#include "selfdrive/ui/turbo.h"

G29State::G29State(QObject *parent) : QObject(parent) {
  sm = std::make_unique<SubMaster, const std::initializer_list<const char *>>({
    "g29"
  });

  timer = new QTimer(this);
  QObject::connect(timer, &QTimer::timeout, this, &G29State::update);
  timer->start(1000 / 30);
}

// g29State singleton
G29State *g29State() {
  static G29State g29_state;
  return &g29_state;
}

void G29State::update() {
  sm->update();
  emit g29Update(*this);
}

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

    // Connect g29Update to updateState
    QObject::connect(g29State(), &G29State::g29Update, this, &TurboWindow::updateState);
}

void TurboWindow::showRearView() {
  int margin = 20;
  overlayCamera->move(this->width() - overlayCamera->width() - margin, 
                      this->height() - overlayCamera->height() - margin);
  overlayCamera->setVisible(true);
}

void TurboWindow::updateState(const G29State &s) {
  const SubMaster &sm = *(s.sm);
  if (sm.updated("g29")) {
    auto g = sm["g29"].getG29();
    steering = g.getSteering();
    throttle = g.getThrottle();
    brake = g.getBrake();
  }
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
