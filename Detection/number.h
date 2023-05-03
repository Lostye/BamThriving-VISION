#ifndef NUMBER_H
#define NUMBER_H
#pragma once
#include<iostream>
#include<opencv2/opencv.hpp>
#include<sstream>
#include<string>
#include"../configer.h"

using namespace std;
using namespace cv;
using namespace cv::ml;
///
/// \brief The number class
///
class number
{
public:
    number(Base &base,vector<RotatedRect> rect_choose ,vector<Robot> &robots);              //数字检测总函数

private:
    bool get_ROI(RotatedRect rect);                                 //提取感兴趣区域
    bool find_ROI_contour();                                       //确定装甲板中是否有图像
    bool detect_num(Base base);                                   //数字检测

    double num_lowst_thresh=1;

    vector<RotatedRect> armor;
    int exposure;
    bool Switch;
    int num=0;               //装甲版序号
    Mat src;               //原图像
    Mat ROI;               //感兴趣区域
    Rect rect;             //矩形框数据

};

#endif // NUMBER_H
