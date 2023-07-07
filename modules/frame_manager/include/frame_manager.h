#ifndef FRAME_MANAGER_H
#define FRAME_MANAGER_H


#include "vector"

#include "opencv2/opencv.hpp"

#include "seekframe/seekframe.h"

#include "pattern/include/observer.h"
#include "device_cfg/include/device_cfg.h"
#include "driver/include/camera.h"

enum CrosshairType{
    CROSSHAIR_STATIC,
    CROSSHAIR_DYNAMIC
};
extern void ScreenUserEvent(int event, int x, int y, int flags, void* userdata);
class FrameManager : public IObserver<seekframe_t*>
{
    public:
        cv::Mat* cameraFrame = new cv::Mat(CAMERA_HEIGHT, CAMERA_WIDTH, CV_8UC4);
        cv::Mat* displayFrame = new cv::Mat(DISPLAY_HEIGHT, DISPLAY_WIDTH, CV_8UC4);
        bool source_set = false;

        FrameManager(Camera &subject) : subject_(subject){
            this->subject_.Attach(this);
            bgsubtractor->setVarThreshold(2);
            cv::namedWindow("Display", cv::WINDOW_NORMAL);
            //set the callback function for any mouse event
            cv::setMouseCallback("Display", ScreenUserEvent, this);
        #ifdef DISPLAY_FULLSCREEN
            cv::setWindowProperty("Display", cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);
        #endif
            //cv::imshow("Display", *(this->displayFrame));
        }

        ~FrameManager(){  
            this->removeFromSubject();
            delete(cameraFrame);
            delete(displayFrame);
        };
        void Update(seekframe_t*) override;
        void convertFrameToCV(seekframe_t*);
        void removeFromSubject();
        void drawCorsshair(cv::Mat &_img, CrosshairType _crosshair_type, int);
        void pictureInPicture(cv::Mat &_src, cv::Rect _roi);
        int _zoom_level = MAX_ZOOM_LEVEL;
        bool _pic_in_pic_enabled = false;
        Camera &subject_;
    private:
        cv::Ptr<cv::BackgroundSubtractorMOG2> bgsubtractor= cv::createBackgroundSubtractorMOG2();
        bool update_bg_model = true;

        std::vector<cv::Mat> _frame_buff;
};

#endif