#pragma once

#include "selfdrive/ui/qt/widgets/cameraview.h"

#include <QtWidgets>

class TurboWindow : public QWidget {
  Q_OBJECT

public:
   TurboWindow(QWidget* parent = 0);
   void showRearView();

private:
  CameraWidget *mainCamera;
  CameraWidget *overlayCamera;
  bool g29Connected = false;
  float steering = 0.0;
  float throttle = 0.0;
  float brake = 0.0;
};
