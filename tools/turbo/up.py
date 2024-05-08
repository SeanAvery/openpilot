#!/usr/bin/env python3
from openpilot.common.params import Params, ParamKeyType, UnknownKeyName
from openpilot.system.hardware import PC, TICI

if __name__ == "__main__":
    params = Params()
    if PC:
        params.put("TurboPC", "1")
    elif TICI:
        params.put("SmallCar", "1")
