#ifndef KALMAN_H
#define KALMAN_H
#include<opencv4/opencv2/opencv.hpp>


#define TEXT
class kalman
{
public:
    kalman();//在构造函数里初始化卡尔曼
    void kalman_init();
    cv::Point2f kalmanRun(cv::Mat src,cv::Point2f measurePt ,cv::Point2f center,double R,bool color);//运行卡尔曼迭代

//private:
    cv::KalmanFilter KF;//cv自带的卡尔曼滤波器
    cv::Point2f center;//输入的圆心
    cv::Point2f measurePt;//测量的点
    cv::Mat measurement;//测量值
    double R;//输入的半径
    double limit_a=0.03;
    bool color;
    float last_angle=0,diff=0,real_angle;
//private:
    inline cv::Point2f PreAngle(cv::Point2f center, double  R, float* angles);
    inline float Point2angle(cv::Point2f point ,cv::Point2f center);

};

#endif // KALMAN_H
