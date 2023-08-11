#ifndef BUTTON_H
#define BUTTON_H

// User
#include "device_cfg/include/device_cfg.h"

class ButtonManager {
    public:
        ButtonManager(){

        };

        ~ButtonManager(){

        };

        int init();
        int getEventId();

    private:
        int timePressed[NUM_OF_BUTTONS];
        int buttonPressed[NUM_OF_BUTTONS];
        int event_id;
};

#endif