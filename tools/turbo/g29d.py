import os
import time

from g29py import G29
import cereal.messaging as messaging

STEERING_RANGE = (90, 60, 120) # (center, min, max)
THROTTLE_RANGE = (60, 90, 130) # (neutral, min, max)
NEUTRAL_RANGE = 7

# the pedals to not mechanically sit a 0
# so we add small threshold trigger value
THROTTLE_THRESHOLD = 0.05
BREAKING_THRESHOLD = 0.05

def normalize_steering(val):
	return round(STEERING_RANGE[0] + val * ((STEERING_RANGE[2] - STEERING_RANGE[1])/2))

def format_msg(prefix, val):
	str_msg = "{}{:03d}{}".format(prefix, val, "#")
	return bytes(str_msg, 'UTF-8')

def normalize_pedal(val):
	start = THROTTLE_RANGE[1]
	return round(start + val * ((THROTTLE_RANGE[2] + NEUTRAL_RANGE - THROTTLE_RANGE[1])/2))

def normalize_braking(val):
    start = THROTTLE_RANGE[1]
    return round(start - val * ((THROTTLE_RANGE[1] - THROTTLE_RANGE[0])/2))

def normalize_long(throttle_val, breaking_val):
    # throttle. range(-0.99, 1) -> (0, 2)
    throttle_val = throttle_val+1 # shift to 0 to 2
    
    # breaking. range(0.999, -1) -> (0, 2)
    breaking_val = breaking_val*-1 # flip to match throttle
    breaking_val = breaking_val+1
    
    if breaking_val > BREAKING_THRESHOLD:
        return normalize_braking(breaking_val)
    elif throttle_val > THROTTLE_THRESHOLD:
        return normalize_pedal(throttle_val)
    else:
        return THROTTLE_RANGE[1]
    
def main():
    # connect to wheel/pedal
    g29 = G29()
    
    # reset ??? why is this breaking
    # set autocenter
    g29.set_range(500)
    g29.set_autocenter(0.25, 0.25)
    
    # create pub socket (msgq)
    messaging.context = messaging.Context()
    g29_sock = messaging.pub_sock("g29")
    controls_sock = messaging.pub_sock("controlsMsg")

    g29.start_pumping()
    while 1:
        # 50 hz loop
        time.sleep(0.04)
        state = g29.get_state()
        
        controls_data = messaging.new_message("controlsMsg")
        steering = normalize_steering(state["steering"])
        throttle = normalize_long(state["accelerator"], state["brake"]*-1.0)
        controls_data.controlsMsg.steering = format_msg("s", normalize_steering(float(state["steering"])))
        controls_data.controlsMsg.throttle = format_msg("t", normalize_long(float(state["accelerator"]), float(state["brake"]*-1.0)))
        controls_sock.send(controls_data.to_bytes())  
        
        g29_data = messaging.new_message("g29")
        g29_data.g29.steering = state["steering"]
        g29_data.g29.throttle = state["accelerator"]
        g29_data.g29.brake = state["brake"]
        g29_data.g29.clutch = state["clutch"]
        g29_data.g29.buttons = {
            "dial": state["buttons"]["misc2"]["dial"],
        }
        g29_sock.send(g29_data.to_bytes())

if __name__ == "__main__":
    main()