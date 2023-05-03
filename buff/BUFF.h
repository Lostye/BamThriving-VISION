#ifndef BUFF_H
#define BUFF_H
#include<opencv4/opencv2/opencv.hpp>
#include"kalman.h"
#include"../configer.h"
#include"CircleFitting.h"

#define SHOW_ALL

class buff
{
public:
    buff();
    cv::Point2f point_find;
    bool buff_flag;
    float angle;
    float c;

    float R=126.39;
    std::vector<cv::Point>track;//用于拟合圆的点集
    cv::Point2f Last_center,Last_point_find;//上一帧的数据，丢失目标后发送上一帧数据
//    kalman KF;
public:
    bool find_R(cv::Mat src,bool color,Point2f& R_center,Ptr<KNearest> model_R);
    void findSquares(const cv::Mat& image,bool color,cv::Point2f circle_center);//红1蓝0
private:
    void draw(cv::Mat dst, std::vector<std::vector<cv::Point>> &aim);
    void draw(cv::Mat dst,cv::RotatedRect aim);

    kalman a;

};

#endif // BUFF_H
