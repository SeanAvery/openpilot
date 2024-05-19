#!/usr/bin/env python3
from openpilot.common.params import Params, ParamKeyType, UnknownKeyName
from openpilot.system.hardware import PC, TICI
import sys

if __name__ == "__main__":
    params = Params()
    if "-down" in sys.argv:
        print("Turbo disabled")
        params.put("TurboPC", "0")
        params.put("SmallCar", "0")
        params.put("TurboLog", "0")
        sys.exit()
    if "-log" in sys.argv:
        print("Logging enabled")
        params.put("TurboLog", "1")
    if PC:
        params.put("TurboPC", "1")
    elif TICI:
        params.put("SmallCar", "1")
