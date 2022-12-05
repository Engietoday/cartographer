#include <iostream>
#include "driver/include/camera.h"
#include "frame_manager/include/frame_manager.h"

Camera* g_cam = new Camera();
FrameManager* g_frameManager = new FrameManager(*g_cam);

int main(int argc, char *argv[])
{
    bool run_camera = true;
    g_cam->init();
    while(run_camera){
        g_cam->getFrame();
        for(auto kvp : g_cam->g_renderers){
            auto renderer = kvp.second;
            run_camera = renderer->is_active.load();
            if(!run_camera) break;
        }
    }
    delete(g_frameManager);
    delete(g_cam);
    return 0;
}