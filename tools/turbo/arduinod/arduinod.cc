#include <iostream>
#include <csignal>
#include <unistd.h>
#include <memory>
#include <cstdlib>
#include "messaging/messaging.h"
#include "common/watchdog.h"
#include "common/timing.h"
#include "ch340.h"

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received." << std::endl;
    exit(signum);
}

int main(int argc, char *argv[]) {
    setenv("ZMQ", "1", 1);
    int err = 0;
    if (argc < 4) {
        // usage product vendor
        std::cout << "usage: ./arduinod <vendor> <product> <ip>" << std::endl;
        return -1;
    }

    // exit handler
    signal(SIGINT, signalHandler);

    // connect to usb device
    CH340 mcu;
    int vendor = std::stoul(argv[1], nullptr, 16);
    int product = std::stoul(argv[2], nullptr, 16);
    std::string ip = argv[3];
    std::cout << "product: " << product << std::endl;
    std::cout << "vendor: " << vendor << std::endl;
    err = mcu.init(vendor, product);
    if (err != 0) {
        std::cout << "mcu init error" << std::endl;
        return -1;
    }

    usleep(100000); // 100 ms


    // create subscriber
    std::cout << "creating subsocket" << std::endl << "connecting to pubsocket" << std::endl;
    Context *context = Context::create();
    SubSocket *sub = SubSocket::create(context, "controlsMsg", ip, true, true);;
    AlignedBuffer aligned_buf;

    while(1) {
        usleep(10000); // 100 hz
        watchdog_kick(nanos_since_boot());

        std::unique_ptr<Message> msg(sub->receive());
        if (!msg) {
            std::cout << "no message" << std::endl;
            continue;
        }

        // parse zmq message into ControlsMsg type
        capnp::FlatArrayMessageReader cmsg(aligned_buf.align(msg.get()));
        cereal::Event::Reader event = cmsg.getRoot<cereal::Event>();
        auto controlsMsg = event.getControlsMsg();

        std::vector<unsigned char *> packets;

        if (controlsMsg.hasSteering()) {
            const capnp::Data::Reader& steeringData = controlsMsg.getSteering();
            const unsigned char *steering = reinterpret_cast<const unsigned char*>(steeringData.asBytes().begin());
            std::cout << steering << std::endl;
            packets.push_back((unsigned char *)steering);
        }

        if (controlsMsg.hasThrottle()) {
            const capnp::Data::Reader& throttleData = controlsMsg.getThrottle();
            const unsigned char *throttle = reinterpret_cast<const unsigned char*>(throttleData.asBytes().begin());
            packets.push_back((unsigned char *)throttle);
        }

        // send packets to MCU one at a time
        for (auto packet : packets) {
            // send packet
            err = mcu.bulk_write(EP_DATA_OUT, packet, 5, 100);
            if (err < 0) {
                if (err == LIBUSB_ERROR_NO_DEVICE) {
                    std::cerr << "Device has disconnected." << std::endl;
                    return -1;
                }
            }
        }
    }

    return 0;
}