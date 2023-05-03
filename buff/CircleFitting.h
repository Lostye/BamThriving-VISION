#ifndef CIRCLEFITTING_H
#define CIRCLEFITTING_H

#endif // CIRCLEFITTING_H
#include<iostream>
#include<opencv4/opencv2/opencv.hpp>

///
/// \brief CircleFitting    用二分法拟合圆
/// \param pts              储存轨迹的向量
/// \param center           输出的圆心坐标
/// \param radius           输出的半径
///
unsigned char CircleFitting(std::vector<cv::Point> &pts,
//    cv::Point2f& center,
    float &radius);
