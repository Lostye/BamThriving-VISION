#ifndef DETECT_H
#define DETECT_H
#pragma once
#include<iostream>
#include<opencv2/opencv.hpp>
#include"../configer.h"
#include"number.h"

using namespace std;
using namespace cv;

///
/// \brief The detect class
///

class detect
{
public:
    detect(Base &base ,vector<Robot> &robots);           //识别中函数

private:
    bool singlelight();             //单灯条搜寻
    bool w_h_Ratio(RotatedRect rect);              //判断灯条宽高
    bool angle(RotatedRect rect);		           //判断灯条角度
    void adaptthresh();           //自适应二值化值域值

    int color;
    Mat src;                    //原图像
    Mat gray;                   //灰度图
    int single_light_lowest_threshold=120;              //灯条最低二值化值域
    vector<RotatedRect> LED_SINGLE_CHOOSE;		       //正确的单个灯条组


    bool form_armor();                        //形成装甲版矩形框
    bool double_x(RotatedRect lightL, RotatedRect lightR);           //判断两灯条x轴距离的长度
    bool double_angle(RotatedRect lightL, RotatedRect lightR);      //判断两灯条角度
    bool double_y(RotatedRect lightL, RotatedRect lightR);         //判断两灯条y轴距离的长度
    float angle_double_threshould=45;


    vector<RotatedRect> rect_choose;		//正确的两灯条组

};

#endif // DETECT_H
