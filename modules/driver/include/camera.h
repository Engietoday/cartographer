#ifndef CAMERA_H
#define CAMERA_H

// C includes
#include <cstring>

// C++ includes
#include <atomic>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <list>


// Seek SDK includes
#include "seekcamera/seekcamera.h"
#include "seekcamera/seekcamera_manager.h"
#include "seekframe/seekframe.h"

// User
#include "device_cfg/include/device_cfg.h"
#include "pattern/include/observer.h"

struct seekrenderer_t
{
	seekcamera_t* camera{};

	// // Rendering data
	// SDL_Window* window{};
	// SDL_Renderer* renderer{};
	// SDL_Texture* texture{};

	// Synchronization data
	std::atomic<bool> is_active;
	std::atomic<bool> is_dirty;
	seekcamera_frame_t* frame{};
};

class Camera : public ISubject<seekframe_t*>{
    public:
        seekrenderer_t camera_state;
        seekcamera_manager_t* manager = nullptr;
        std::map<seekcamera_t*, seekrenderer_t*> g_renderers;     // Tracks all renderers.
        std::atomic<bool> g_exit_requested;                       // Controls application shutdown.
        seekframe_t* frame = nullptr;

        Camera()
        {
            this->g_exit_requested.store(false);
        }
        bool switch_color_pallete(){
            return this->seekrenderer_switch_color_palette();
        };
        bool seekrenderer_switch_color_palette();
        void seekrenderer_close_window(seekrenderer_t* renderer);

        uint8_t init();

        virtual ~Camera(){
            seekrenderer_close_window(&(this->camera_state));
            // Teardown the camera manager.
            if(this->manager != nullptr)
                seekcamera_manager_destroy(&manager);
        };

        void Attach(IObserver<seekframe_t*> *observer) override {
            list_observer_.push_back(observer);
        }

        void Detach(IObserver<seekframe_t*> *observer) override {
            list_observer_.remove(observer);
        }
        
        void Notify() override {
            std::list<IObserver<seekframe_t*> *>::iterator iterator = list_observer_.begin();
            while(iterator != list_observer_.end()) {
                (*iterator)->Update(this->frame);
                ++iterator;
            }
        }
        // Switches the current color palette.
        // Settings will be refreshed between frames.
        bool seekrenderer_switch_color_palette(seekrenderer_t* renderer);

        void getFrame();
        // void handle_camera_frame_available(seekcamera_t* camera, seekcamera_frame_t* camera_frame, void* user_data);
        // void handle_camera_connect(seekcamera_t* camera, seekcamera_error_t event_status, void* user_data);
        // void handle_camera_disconnect(seekcamera_t* camera, seekcamera_error_t event_status, void* user_data);
        // void handle_camera_error(seekcamera_t* camera, seekcamera_error_t event_status, void* user_data);
        // void handle_camera_ready_to_pair(seekcamera_t* camera, seekcamera_error_t event_status, void* user_data);
        // void camera_event_callback(seekcamera_t* camera, seekcamera_manager_event_t event, seekcamera_error_t event_status, void* user_data);
    private:
        std::list<IObserver<seekframe_t*> *> list_observer_;
};

#endif