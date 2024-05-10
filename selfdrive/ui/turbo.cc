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
  timer->start(1000 / UI_FREQ);
}

// g29State singleton
G29State *g29State() {
  static G29State g29_state;
  return &g29_state;
}

void G29State::update() {
  watchdog_kick(nanos_monotonic());
  sm->update(0);
  emit g29Update(*this);
}

TurboWindow::TurboWindow(QWidget* parent) : QWidget(parent) {
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(0);

  mainCamera = new CameraWidget("camerad", VISION_STREAM_WIDE_ROAD, false);
  layout->addWidget(mainCamera);

  overlayCamera = new CameraWidget("camerad", VISION_STREAM_DRIVER, false);
  // Ensure overlayCamera is a child of the main window to move freely over it
  overlayCamera->setParent(this); overlayCamera->setFixedSize(rearViewWidth, rearViewHeight);

  // Connect g29Update to updateState
  QObject::connect(g29State(), &G29State::g29Update, this, &TurboWindow::updateState);
}

void TurboWindow::showRearView() {
  int margin = 20;
  overlayCamera->move(this->width() - overlayCamera->width() - margin, 
                      this->height() - overlayCamera->height() - margin);
  overlayCamera->setVisible(true);
}

void TurboWindow::resizeRearView(int w, int h) {
  std::cout << "Resizing rear view to " << w << "x" << h << std::endl;
  overlayCamera->setFixedSize(w, h);
  showRearView();
}

void TurboWindow::updateState(const G29State &s) {
  const SubMaster &sm = *(s.sm);
  if (sm.updated("g29")) {
    auto g = sm["g29"].getG29();
    steering = g.getSteering();
    throttle = g.getThrottle();
    brake = g.getBrake();
    auto dial = g.getButtons().getDial();
    int w = rearViewWidth;
    int h = rearViewHeight;
    if (dial > 0) {
      w = 482 + (dial * 10);
      h = 302 + (dial * 6);
    }
    if (w != rearViewWidth || h != rearViewHeight) {
      rearViewWidth = w;
      rearViewHeight = h;
      resizeRearView(w, h);
    }
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
