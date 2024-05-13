#!/usr/bin/env python3
from openpilot.common.params import Params, ParamKeyType, UnknownKeyName
from openpilot.system.hardware import PC, TICI
import sys

if __name__ == "__main__":
    params = Params()
    if "-log" in sys.argv:
        print("Logging enabled")
        params.put("TurboLog", "1")
    if PC:
        params.put("TurboPC", "1")
    elif TICI:
        params.put("SmallCar", "1")
