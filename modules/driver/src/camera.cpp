#include "driver/include/camera.h"

void Camera::seekrenderer_close_window(seekrenderer_t* renderer){
    if(renderer->is_active.load())
        seekcamera_capture_session_stop(renderer->camera);

    renderer->is_active.store(false);
    renderer->is_dirty.store(false);
    renderer->frame = nullptr;
    renderer->camera = nullptr;
}

// Handles frame available events.
void handle_camera_frame_available(seekcamera_t* camera, seekcamera_frame_t* camera_frame, void* user_data)
{
	(void)camera;
	seekrenderer_t* renderer = ((Camera*)user_data)->g_renderers[camera];

	// Lock the seekcamera frame for safe use outside of this callback.
	seekcamera_frame_lock(camera_frame);
	renderer->is_dirty.store(true);
	// Note that this will always render the most recent frame. There could be better buffering here but this is a simple example.
	renderer->frame = camera_frame;
}

// Handles camera connect events.
void handle_camera_connect(seekcamera_t* camera, seekcamera_error_t event_status, void* user_data)
{

#ifdef DEBUG_CFG
	std::cout << "Camera Connected" << std::endl;
#endif

	(void)event_status;
	//(void)user_data;
	seekrenderer_t* renderer = ((Camera*)user_data)->g_renderers[camera] == nullptr ? new seekrenderer_t() : ((Camera*)user_data)->g_renderers[camera];
	renderer->is_active.store(true);
	renderer->camera = camera;

	// Register a frame available callback function.
	seekcamera_error_t status = seekcamera_register_frame_available_callback(camera, handle_camera_frame_available, user_data);
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to register frame callback: " << seekcamera_error_get_str(status) << std::endl;
		renderer->is_active.store(false);
		return;
	}

	// Start the capture session.
	status = seekcamera_capture_session_start(camera, SEEKCAMERA_FRAME_FORMAT_COLOR_ARGB8888);
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to start capture session: " << seekcamera_error_get_str(status) << std::endl;
		renderer->is_active.store(false);
		return;
	}
	((Camera*)user_data)->g_renderers[camera] = renderer;
}

// Handles camera disconnect events.
void handle_camera_disconnect(seekcamera_t* camera, seekcamera_error_t event_status, void* user_data)
{
#ifdef DEBUG_CFG
	std::cout << "Camera Disconnected" << std::endl;
#endif

	(void)event_status;
	//(void)user_data;
	seekrenderer_t* renderer = ((Camera*)user_data)->g_renderers[camera];
	renderer->is_active.store(false);
}

// Handles camera error events.
void handle_camera_error(seekcamera_t* camera, seekcamera_error_t event_status, void* user_data)
{
	(void)user_data;
	seekcamera_chipid_t cid{};
	seekcamera_get_chipid(camera, &cid);
	std::cerr << "unhandled camera error: (CID: " << cid << ")" << seekcamera_error_get_str(event_status) << std::endl;
}

// Handles camera ready to pair events
void handle_camera_ready_to_pair(seekcamera_t* camera, seekcamera_error_t event_status, void* user_data)
{
	// Attempt to pair the camera automatically.
	// Pairing refers to the process by which the sensor is associated with the host and the embedded processor.
	const seekcamera_error_t status = seekcamera_store_calibration_data(camera, nullptr, nullptr, nullptr);
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to pair device: " << seekcamera_error_get_str(status) << std::endl;
	}

	// Start imaging.
	handle_camera_connect(camera, event_status, user_data);
}


// Callback function for the camera manager; it fires whenever a camera event occurs.
void camera_event_callback(seekcamera_t* camera, seekcamera_manager_event_t event, seekcamera_error_t event_status, void* user_data)
{
	seekcamera_chipid_t cid{};
	seekcamera_get_chipid(camera, &cid);
	std::cout << seekcamera_manager_get_event_str(event) << " (CID: " << cid << ")" << std::endl;
	seekcamera_pipeline_mode_t current_mode;

	// Handle the event type.
	switch(event)
	{
		case SEEKCAMERA_MANAGER_EVENT_CONNECT:
			handle_camera_connect(camera, event_status, user_data);
			if(seekcamera_set_pipeline_mode(camera, SEEKCAMERA_IMAGE_SEEKVISION) == SEEKCAMERA_SUCCESS){
				seekcamera_get_pipeline_mode(camera, &current_mode);
				std::cout << "Filter pipeline mode: " << seekcamera_pipeline_mode_get_str(current_mode) << std::endl;
			}
			break;
		case SEEKCAMERA_MANAGER_EVENT_DISCONNECT:
			handle_camera_disconnect(camera, event_status, user_data);
			break;
		case SEEKCAMERA_MANAGER_EVENT_ERROR:
			handle_camera_error(camera, event_status, user_data);
			break;
		case SEEKCAMERA_MANAGER_EVENT_READY_TO_PAIR:
			handle_camera_ready_to_pair(camera, event_status, user_data);
			break;
		default:
			break;
	}
}

uint8_t Camera::init()
{
	// Create the camera manager.
	// This is the structure that owns all Seek camera devices.
	seekcamera_error_t status;

    status = seekcamera_manager_create(&(this->manager), SEEKCAMERA_IO_TYPE_USB);
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to create camera manager: " << seekcamera_error_get_str(status) << std::endl;
		return 1;
	}

	// Register an event handler for the camera manager to be called whenever a camera event occurs.
	status = seekcamera_manager_register_event_callback(this->manager, camera_event_callback, this);
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to register camera event callback: " << seekcamera_error_get_str(status) << std::endl;
		return 1;
	}
	return 0;
}

void Camera::getFrame()
{
    seekcamera_error_t status;

    for(auto& kvp : this->g_renderers)
    {
        seekrenderer_t* renderer = kvp.second;

        if(renderer == NULL)
            break;
        // Render frame if necessary
        if(renderer->is_dirty.load())
        {
            if(renderer->frame == NULL || !renderer->is_active.load())
                break;
            // Get the frame to draw.
			this->frame = nullptr;
            status = seekcamera_frame_get_frame_by_format(renderer->frame, SEEKCAMERA_FRAME_FORMAT_COLOR_ARGB8888, &(this->frame));
            if(status != SEEKCAMERA_SUCCESS)
            {
                std::cerr << "failed to get frame: " << seekcamera_error_get_str(status) << std::endl;
                seekcamera_frame_unlock(renderer->frame);
                break;
            }

            // Get the frame dimensions.
            const int frame_width = (int)seekframe_get_width(frame);
            const int frame_height = (int)seekframe_get_height(frame);
            const int frame_stride = (int)seekframe_get_line_stride(frame);

            //seekframe_get_data(frame);

#ifdef DEBUG_CFG
			//std::cout << "New frame from camera." << std::endl;
#endif
            // Unlock the camera frame.
            seekcamera_frame_unlock(renderer->frame);
            renderer->is_dirty.store(false);
            renderer->frame = nullptr;
            this->Notify();
        }

        // Close inactive windows
        if(!renderer->is_active.load())
            seekrenderer_close_window(renderer);
    }
}

// Switches the current color palette.
// Settings will be refreshed between frames.
bool Camera::seekrenderer_switch_color_palette()
{    
	seekcamera_color_palette_t current_palette;
	seekrenderer_t* renderer;
	for(auto& kvp : this->g_renderers)
    {
        renderer = kvp.second;
		//std::cout << "Attempt to switch color pallet" << std::endl;
		if(seekcamera_get_color_palette(renderer->camera, &current_palette) != SEEKCAMERA_SUCCESS)
			return false;

		// Not including the user palettes so we will cycle back to the beginning once GREEN is hit
		current_palette = (seekcamera_color_palette_t)((current_palette + 1) % SEEKCAMERA_COLOR_PALETTE_USER_0);
		//std::cout << "color palette: " << seekcamera_color_palette_get_str(current_palette) << std::endl;
	}
	return seekcamera_set_color_palette(renderer->camera, current_palette) == SEEKCAMERA_SUCCESS;
}

bool Camera::seekrenderer_switch_pipeline_mode()
{
	seekrenderer_t* renderer;
	// get value for current filter mode
	seekcamera_pipeline_mode_t current_mode;

	for(auto& kvp : this->g_renderers)
    {
        renderer = kvp.second;
		if(!renderer->is_active.load()){
			return false;
		}
		if(seekcamera_get_pipeline_mode(renderer->camera, &current_mode) != SEEKCAMERA_SUCCESS){
			return false;
		}

		// Rotate through smoothing filter values and cycle back to the beginning once SEEKVISION is hit
		current_mode = (seekcamera_pipeline_mode_t)((current_mode + 1) % SEEKCAMERA_IMAGE_LASTVALUE);
		std::cout << "Filter pipeline mode: " << seekcamera_pipeline_mode_get_str(current_mode) << std::endl;
	}
	return seekcamera_set_pipeline_mode(renderer->camera, current_mode) == SEEKCAMERA_SUCCESS;
}