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

class FrameManager : public IObserver<seekframe_t*>
{
    public:
        cv::Mat* cameraFrame = new cv::Mat(CAMERA_HEIGHT, CAMERA_WIDTH, CV_8UC4);
        cv::Mat* displayFrame = new cv::Mat(DISPLAY_HEIGHT, DISPLAY_WIDTH, CV_8UC4);
        bool source_set = false;

        FrameManager(Camera &subject) : subject_(subject){
            this->subject_.Attach(this);
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
        Camera &subject_;
    private:
        

        std::vector<cv::Mat> _frame_buff;
};

#endif