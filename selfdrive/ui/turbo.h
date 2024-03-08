#pragma once

#include "selfdrive/ui/qt/widgets/cameraview.h"
#include "cereal/messaging/messaging.h"

#include <QtWidgets>

class G29State : public QObject {
  Q_OBJECT
public:
  G29State(QObject *parent = 0);
  std::unique_ptr<SubMaster> sm;

signals:
  void g29Update(const G29State &s);

private slots:
  void update();

private:
  QTimer *timer;
};

class TurboWindow : public QWidget {
  Q_OBJECT

public:
  TurboWindow(QWidget* parent = 0);
  void showRearView();
  void resizeRearView(int w, int h);

private:
  CameraWidget *mainCamera;
  CameraWidget *overlayCamera;
  bool g29Connected = false;
  float steering = 0.0;
  float throttle = 0.0;
  float brake = 0.0;
  int rearViewWidth = 482;
  int rearViewHeight = 302;

private slots:
  void updateState(const G29State &s);
};
