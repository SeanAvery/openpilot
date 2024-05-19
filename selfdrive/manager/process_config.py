import os

from cereal import car
from openpilot.common.params import Params
from openpilot.system.hardware import PC, TICI
from openpilot.selfdrive.manager.process import PythonProcess, NativeProcess, DaemonProcess

WEBCAM = os.getenv("USE_WEBCAM") is not None
GCS_IP = os.getenv("GCS_IP", '127.0.0.1') # temp until better way to pass in IP
VENDOR_ID = os.getenv("VENDOR_ID", '0x2b24') # turbu mcu venddr id
PRODUCT_ID = os.getenv("PRODUCT_ID", '0x0001') # turbo mcu product id

def driverview(started: bool, params: Params, CP: car.CarParams) -> bool:
  return started or params.get_bool("IsDriverViewEnabled")

def notcar(started: bool, params: Params, CP: car.CarParams) -> bool:
  return started and CP.notCar

def smallcar(started: bool, params: Params, CP: car.CarParams) -> bool:
  return params.get_bool("SmallCar")

def turbolog(started: bool, params: Params, CP: car.CarParams) -> bool:
  return params.get_bool("TurboLog")

def gcs(started: bool, params: Params, CP: car.CarParams) -> bool:
  return params.get_bool("TurboPC")

def iscar(started: bool, params: Params, CP: car.CarParams) -> bool:
  return started and not CP.notCar

def logging(started, params, CP: car.CarParams) -> bool:
  run = (not CP.notCar) or not params.get_bool("DisableLogging")
  return started and run

def ublox_available() -> bool:
  return os.path.exists('/dev/ttyHS0') and not os.path.exists('/persist/comma/use-quectel-gps')

def ublox(started, params, CP: car.CarParams) -> bool:
  use_ublox = ublox_available()
  if use_ublox != params.get_bool("UbloxAvailable"):
    params.put_bool("UbloxAvailable", use_ublox)
  return started and use_ublox

def qcomgps(started, params, CP: car.CarParams) -> bool:
  return started and not ublox_available()

def always_run(started, params, CP: car.CarParams) -> bool:
  return True

def only_onroad(started: bool, params, CP: car.CarParams) -> bool:
  return started

def only_offroad(started, params, CP: car.CarParams) -> bool:
  return not started

procs = [
  DaemonProcess("manage_athenad", "selfdrive.athena.manage_athenad", "AthenadPid"),

  NativeProcess("camerad", "system/camerad", ["./camerad"], driverview),
  NativeProcess("logcatd", "system/logcatd", ["./logcatd"], only_onroad),
  NativeProcess("proclogd", "system/proclogd", ["./proclogd"], only_onroad),
  PythonProcess("logmessaged", "system.logmessaged", always_run),
  PythonProcess("micd", "system.micd", iscar),
  PythonProcess("timed", "system.timed", always_run, enabled=not PC),

  PythonProcess("dmonitoringmodeld", "selfdrive.modeld.dmonitoringmodeld", driverview, enabled=(not PC or WEBCAM)),
  NativeProcess("encoderd", "system/loggerd", ["./encoderd"], only_onroad),
  NativeProcess("stream_encoderd", "system/loggerd", ["./encoderd", "--stream"], notcar),
  NativeProcess("loggerd", "system/loggerd", ["./loggerd"], logging),
  NativeProcess("modeld", "selfdrive/modeld", ["./modeld"], only_onroad),
  # NativeProcess("mapsd", "selfdrive/navd", ["./mapsd"], only_onroad),
  NativeProcess("mapsd", "selfdrive/navd", ["./mapsd"], smallcar),
  PythonProcess("navmodeld", "selfdrive.modeld.navmodeld", only_onroad),
  # NativeProcess("sensord", "system/sensord", ["./sensord"], only_onroad, enabled=not PC),
  NativeProcess("sensord", "system/sensord", ["./sensord"], smallcar, enabled=not PC),
  NativeProcess("ui", "selfdrive/ui", ["./ui"], always_run, watchdog_max_dt=(5 if not PC else None)),
  PythonProcess("soundd", "selfdrive.ui.soundd", only_onroad),
  # NativeProcess("locationd", "selfdrive/locationd", ["./locationd"], only_onroad),
  NativeProcess("locationd", "selfdrive/locationd", ["./locationd"], smallcar),
  NativeProcess("boardd", "selfdrive/boardd", ["./boardd"], always_run, enabled=False),
  # PythonProcess("calibrationd", "selfdrive.locationd.calibrationd", only_onroad),
  PythonProcess("calibrationd", "selfdrive.locationd.calibrationd", smallcar),
  PythonProcess("torqued", "selfdrive.locationd.torqued", only_onroad),
  PythonProcess("controlsd", "selfdrive.controls.controlsd", only_onroad),
  PythonProcess("deleter", "system.loggerd.deleter", always_run),
  PythonProcess("dmonitoringd", "selfdrive.monitoring.dmonitoringd", driverview, enabled=(not PC or WEBCAM)),
  PythonProcess("qcomgpsd", "system.qcomgpsd.qcomgpsd", qcomgps, enabled=TICI),
  PythonProcess("navd", "selfdrive.navd.navd", only_onroad),
  PythonProcess("pandad", "selfdrive.boardd.pandad", always_run),
  PythonProcess("paramsd", "selfdrive.locationd.paramsd", only_onroad),
  # NativeProcess("ubloxd", "system/ubloxd", ["./ubloxd"], ublox, enabled=TICI),
  NativeProcess("ubloxd", "system/ubloxd", ["./ubloxd"], smallcar, enabled=TICI),
  PythonProcess("pigeond", "system.sensord.pigeond", ublox, enabled=TICI),
  PythonProcess("plannerd", "selfdrive.controls.plannerd", only_onroad),
  PythonProcess("radard", "selfdrive.controls.radard", only_onroad),
  PythonProcess("thermald", "selfdrive.thermald.thermald", always_run),
  PythonProcess("tombstoned", "selfdrive.tombstoned", always_run, enabled=not PC),
  PythonProcess("updated", "selfdrive.updated", only_offroad, enabled=not PC),
  PythonProcess("uploader", "system.loggerd.uploader", always_run),
  PythonProcess("statsd", "selfdrive.statsd", always_run),

  # debug procs
  NativeProcess("bridge", "cereal/messaging", ["./bridge"], notcar),
  PythonProcess("webrtcd", "system.webrtc.webrtcd", notcar),
  PythonProcess("webjoystick", "tools.bodyteleop.web", notcar),

  # turbo ugv
  NativeProcess("turbo_encoderd", "system/loggerd", ["./encoderd", "--turbo"], smallcar),
  NativeProcess("turbo_camerad", "system/camerad", ["./camerad"], smallcar),
  NativeProcess("turbo_bridge_client", "cereal/messaging", ["./bridge_client", GCS_IP, "wideRoadEncodeData,driverEncodeData,mapEncodeData"], smallcar),
  NativeProcess("turbo_arduinod", "tools/turbo/arduinod", ["./arduinod", VENDOR_ID, PRODUCT_ID, GCS_IP], smallcar, watchdog_max_dt=5),
  NativeProcess("turbo_log", "system/loggerd", ["./loggerd"], turbolog),

  # turbo gcs
  NativeProcess("turbo_camerastream", "tools/camerastream", ["./compressed_vipc.py", "0.0.0.0", "--cams=1,2,3"], gcs),
  NativeProcess("turbo_ui", "selfdrive/ui", ["./turbo"], gcs, watchdog_max_dt=5),
  NativeProcess("turbo_bridge_server", "cereal/messaging", ["./bridge", "controlsMsg"], gcs),
  PythonProcess("turbo_g29", "tools.turbo.g29d", gcs, watchdog_max_dt=5)
]

managed_processes = {p.name: p for p in procs}
