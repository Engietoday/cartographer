#include "frame_manager/include/frame_manager.h"

void FrameManager::removeFromSubject(){
    this->subject_.Detach(this);
}

void ScreenUserEvent(int event, int x, int y, int flags, void* userdata)
{
    switch(event){
        case cv::EVENT_LBUTTONDOWN:
            ((FrameManager*) userdata)->subject_.switch_color_pallete();
            std::cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << std::endl;
            break;
        case cv::EVENT_RBUTTONDOWN:
            ((FrameManager*) userdata)->subject_.seekrenderer_switch_pipeline_mode();
            std::cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << std::endl;
            break;
        case cv::EVENT_MBUTTONDOWN:
            ((FrameManager*) userdata)->_zoom_level -= DEL_ZOOM_LEVEL;
            std::cout << "Zoom Level: " << ((FrameManager*) userdata)->_zoom_level << std::endl;

            if(((FrameManager*) userdata)->_zoom_level <= 0){
                ((FrameManager*) userdata)->_zoom_level = MAX_ZOOM_LEVEL;
                ((FrameManager*) userdata)->_pic_in_pic_enabled = false;
            }else{
                ((FrameManager*) userdata)->_pic_in_pic_enabled = true;
            }
            std::cout << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")" << std::endl;
            break;
        default:
            break;
    }
}


#ifdef DEBUG_CFG
static void refineSegments(const cv::Mat& img, cv::Mat& mask, cv::Mat& dst)
{
    int niters = 3;
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::Mat temp;
    cv::dilate(mask, temp, cv::Mat(), cv::Point(-1,-1), niters);
    cv::erode(temp, temp, cv::Mat(), cv::Point(-1,-1), niters*2);
    cv::dilate(temp, temp, cv::Mat(), cv::Point(-1,-1), niters);
    cv::findContours( temp, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);
    dst = cv::Mat::zeros(img.size(), CV_8UC3);
    if( contours.size() == 0 )
        return;
    // iterate through all the top-level contours,
    // draw each connected component with its own random color
    int idx = 0, largestComp = 0;
    double maxArea = 0;
    for( ; idx >= 0; idx = hierarchy[idx][0] )
    {
        const std::vector<cv::Point>& c = contours[idx];
        double area = fabs(cv::contourArea(cv::Mat(c)));
        if( area > maxArea )
        {
            maxArea = area;
            largestComp = idx;
        }
    }
    cv::Scalar color( 0, 0, 255 );
    drawContours( dst, contours, largestComp, color, cv::FILLED, cv::LINE_8, hierarchy );
}


#endif
void FrameManager::Update(seekframe_t* frame){
    this->cameraFrame->data = (uchar*) seekframe_get_data(frame);
    cv::resize(*(this->cameraFrame), *displayFrame, cv::Size(DISPLAY_WIDTH, DISPLAY_HEIGHT), 0, 0, cv::INTER_LINEAR);

    #ifdef DEBUG_CFG
        //std::cout << "Frame Manager recieved a frame." << std::endl;
        if(_pic_in_pic_enabled){
            cv::Rect roi = cv::Rect((DISPLAY_WIDTH - _zoom_level)/2, (DISPLAY_HEIGHT - _zoom_level)/2, _zoom_level, _zoom_level);
            this->pictureInPicture(*(this->displayFrame), roi);
        }
        this->drawCorsshair(*(this->displayFrame), CROSSHAIR_DYNAMIC, 5);
    #ifdef ROTATE_IMG
        cv::rotate(*(this->displayFrame), *(this->displayFrame), cv::RotateFlags::ROTATE_180);
    #endif
        cv::imshow("Display", *(this->displayFrame));
        cv::waitKey(1);
    // if((this->displayFrame)->empty())
    // {
    //     printf("can not read data from the video source\n");
    //     return;
    // }
    // cv::namedWindow("video", 1);
    // cv::setMouseCallback("video", ScreenUserEvent, this);
    // cv::namedWindow("segmented", 1);
    // cv::Mat bgmask, out_frame;

    // if(!(this->displayFrame)->empty())
    // {
    //     bgsubtractor->apply(*(this->displayFrame), bgmask, update_bg_model ? -1 : 0);
    //     refineSegments(*(this->displayFrame), bgmask, out_frame);
    //     cv::imshow("video", *(this->displayFrame));
    //     cv::imshow("segmented", out_frame);
    //     char keycode = (char)cv::waitKey(1);
    //     if( keycode == 27 )
    //         return;
    //     if( keycode == ' ' )
    //     {
    //         update_bg_model = !update_bg_model;
    //         printf("Learn background is in state = %d\n",update_bg_model);
    //     }

    // }
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
    cv::Scalar color = cv::Scalar CROSSAIR_COLOR;
    cv::Mat subView = _src(_roi);
    cv::resize(subView, subView, cv::Size(DISPLAY_WIDTH*.25 - 2, DISPLAY_HEIGHT*.25 - 1), 0, 0);
    cv::drawMarker(subView, cv::Point((DISPLAY_WIDTH*.25)/2, (DISPLAY_HEIGHT*.25)/2),  color, cv::MARKER_CROSS, 15, 1);
    this->drawCorsshair(subView, CROSSHAIR_STATIC, 5);
    cv::copyMakeBorder(subView,subView,0,1,1,1,cv::BORDER_CONSTANT,cv::Scalar(0, 0, 255));
    subView.copyTo(_src(cv::Rect((DISPLAY_WIDTH - DISPLAY_WIDTH*.25)/2, 0, DISPLAY_WIDTH*.25, DISPLAY_HEIGHT*.25)));
}

void FrameManager::convertFrameToCV(seekframe_t* _in){

}