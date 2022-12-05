#include "frame_manager/include/frame_manager.h"

void FrameManager::removeFromSubject(){
    this->subject_.Detach(this);
}

void ScreenUserEvent(int event, int x, int y, int flags, void* userdata)
{
    if( event == cv::EVENT_LBUTTONDOWN )
    {
        ((FrameManager*) userdata)->subject_.switch_color_pallete();
        std::cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << std::endl;
    }
    // if( event == cv::EVENT_RBUTTONDOWN )
    // {
    //     std::cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << std::endl;
    // }
    // if( event == cv::EVENT_MBUTTONDOWN )
    // {
    //     std::cout << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")" << std::endl;
    // }
    // if( event == cv::EVENT_MOUSEMOVE )
    // {
    //     std::cout << "Mouse move over the window - position (" << x << ", " << y << ")" << std::endl;
    // }
}

void FrameManager::Update(seekframe_t* frame){
    this->cameraFrame->data = (uchar*) seekframe_get_data(frame);
    cv::resize(*(this->cameraFrame), *displayFrame, cv::Size(DISPLAY_WIDTH, DISPLAY_HEIGHT), 0, 0, cv::INTER_LINEAR);

    #ifdef DEBUG_CFG
        //std::cout << "Frame Manager recieved a frame." << std::endl;
        cv::Rect roi = cv::Rect((DISPLAY_WIDTH -ZOOM_LEVEL)/2, (DISPLAY_HEIGHT-ZOOM_LEVEL)/2, ZOOM_LEVEL, ZOOM_LEVEL);
        this->pictureInPicture(*(this->displayFrame), roi);
        this->drawCorsshair(*(this->displayFrame), CROSSHAIR_DYNAMIC, 5);
        cv::imshow("Test", *(this->displayFrame));
        //set the callback function for any mouse event
        cv::setMouseCallback("Test", ScreenUserEvent, this);

        cv::waitKey(1);
    #endif
}

void FrameManager::drawCorsshair(cv::Mat &_img, CrosshairType _crosshair_type, int offset_){
    cv::Scalar color = cv::Scalar CROSSAIR_COLOR;
    int _crosshair_size = CROSSAIR_SIZE;
    int _crosshair_thickness = CROSSAIR_THICKNESS;

    int _width_mid = DISPLAY_WIDTH/2;
    int _height_mid = DISPLAY_HEIGHT/2;

    cv::Point _img_midpoint = cv::Point(_width_mid, _height_mid);

    cv::Point _crosshair_hl = cv::Point(_width_mid-_crosshair_size, _height_mid);
    cv::Point _crosshair_hr = cv::Point(_width_mid+_crosshair_size, _height_mid);
    cv::Point _crosshair_vt = cv::Point(_width_mid, _height_mid - _crosshair_size);
    cv::Point _crosshair_vb = cv::Point(_width_mid, _height_mid + _crosshair_size);

    cv::Point _crosshair_delta_h = cv::Point(offset_, 0);
    cv::Point _crosshair_delta_v = cv::Point(0, offset_);

    switch(_crosshair_type)
    {
        case CROSSHAIR_DYNAMIC:
            cv::line(_img, _crosshair_hr+_crosshair_delta_h, _img_midpoint+_crosshair_delta_h, color, _crosshair_thickness);  //crosshair horizontal left

            cv::line(_img, _crosshair_hl-_crosshair_delta_h, _img_midpoint-_crosshair_delta_h, color, _crosshair_thickness);  //crosshair horizontal right

            cv::line(_img, _crosshair_vt-_crosshair_delta_v, _img_midpoint-_crosshair_delta_v, color, _crosshair_thickness);  //crosshair vertical top

            cv::line(_img, _crosshair_vb+_crosshair_delta_v, _img_midpoint+_crosshair_delta_v, color, _crosshair_thickness);  //crosshair vertical bottom
            cv::drawMarker(_img, cv::Point(_width_mid, _height_mid), color, cv::MARKER_CROSS, CROSSAIR_DOT_SIZE, _crosshair_thickness);
            break;
        case CROSSHAIR_STATIC:
        default:
            cv::drawMarker(_img, cv::Point(_width_mid, _height_mid), color, cv::MARKER_CROSS, _crosshair_size, _crosshair_thickness);
            break;
    }
}


void FrameManager::pictureInPicture(cv::Mat &_src, cv::Rect _roi){
    cv::Mat subView = _src(_roi);
    cv::resize(subView, subView, cv::Size(DISPLAY_WIDTH*.25 - 2, DISPLAY_HEIGHT*.25 - 1), 0, 0);
    cv::drawMarker(subView, cv::Point((DISPLAY_WIDTH*.25)/2, (DISPLAY_HEIGHT*.25)/2),  cv::Scalar(0, 0, 255), cv::MARKER_CROSS, 15, 1);
    this->drawCorsshair(subView, CROSSHAIR_STATIC, 5);
    cv::copyMakeBorder(subView,subView,0,1,1,1,cv::BORDER_CONSTANT,cv::Scalar(0, 0, 255));
    subView.copyTo(_src(cv::Rect((DISPLAY_WIDTH - DISPLAY_WIDTH*.25)/2, 0, DISPLAY_WIDTH*.25, DISPLAY_HEIGHT*.25)));
}

void FrameManager::convertFrameToCV(seekframe_t* _in){

}