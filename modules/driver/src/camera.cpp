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
	auto* renderer = (seekrenderer_t*) &(((Camera*)user_data)->camera_state);

	// Lock the seekcamera frame for safe use outside of this callback.
	seekcamera_frame_lock(camera_frame);
	renderer->is_dirty.store(true);

	// Note that this will always render the most recent frame. There could be better buffering here but this is a simple example.
	renderer->frame = camera_frame;
}

// Handles camera connect events.
void handle_camera_connect(seekcamera_t* camera, seekcamera_error_t event_status, void* user_data)
{
	(void)event_status;
	//(void)user_data;
	seekrenderer_t* renderer = ((Camera*)user_data)->g_renderers[camera] == nullptr ? new seekrenderer_t() : ((Camera*)user_data)->g_renderers[camera];
	renderer->is_active.store(true);
	renderer->camera = camera;

	// Register a frame available callback function.
	seekcamera_error_t status = seekcamera_register_frame_available_callback(camera, handle_camera_frame_available, (void*)renderer);
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

	// Handle the event type.
	switch(event)
	{
		case SEEKCAMERA_MANAGER_EVENT_CONNECT:
			handle_camera_connect(camera, event_status, user_data);
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
            //seekframe_t* frame = nullptr;
            status = seekcamera_frame_get_frame_by_format(renderer->frame, SEEKCAMERA_FRAME_FORMAT_COLOR_ARGB8888, &frame);
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

            seekframe_get_data(frame);

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
