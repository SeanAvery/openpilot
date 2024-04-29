#!/usr/bin/env python3
from openpilot.common.params import Params, ParamKeyType, UnknownKeyName

if __name__ == "__main__":
    params = Params()
    # create params pubsocket
    params.put("SmallCar", "1")