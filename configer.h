#ifndef BASE_H
#define BASE_H
#pragma once
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <mutex>
#include "serial/serial.h"
#include "dahua/dhua.h"

using namespace std;
using namespace cv;
using namespace cv::ml;


#define BIG 1
#define SMALL 0
#define BUFF 3

/*   全局宏定义，控制终端和图像信息输出         */

#define SHOW_FINAL                //显示最终图像
//#define SAVE_VIDEO             //保存最终图像为视频

//#define SHOW_GRAY                  //显示灰度图
//#define SHOW_SINGLE_THRESH            //显示灯条二值化图像
//#define DRAW_LIGHTS                //显示单灯条图像
//#define OUTPUT_SINGLE_information       //输出灯条信息
//#define SHOW_DOUBLE_LIGHTS              //显示双灯条匹配图像
//#define SHOW_ROBOTS_INFORMATIONS       //显示机器人信息
//#define DRAW_ROI_RECT             //画ROI矩形
//#define SHOW_ROI_AERA             //显示ROI区域

#define SERIAL_OPEN             //串口开关

typedef struct base
{
    Ptr<KNearest> model_R = StatModel::load<KNearest>("../R.xml");

    Mat CAMERA_MATRIX = (Mat_<double>(3, 3) <<1.361985498410507e+03 , 0 , 6.237240454251589e+02 ,
                0 ,  1.315954307698096e+03 , 4.909753001718418e+02 , 0 , 0 , 1);
    Mat DISTORTION_COEFF = (Mat_<double>(4, 1) <<-0.070530658081439 ,  0.225302683933098 , 0 , 0);


    int enemy_team=1;           //enemy：0 for blue ,1 for red
    int exposure = 5000;
    bool Switch = 1;
    bool flag;                   //错误就跳过，减少运行错误和时间
    Mat src;
}Base;



typedef struct robot
{
    RotatedRect roborect;                     //机器人装甲板旋转矩形信息
    double robonum=0;                         //机器人序号
    int robot_type;                        //装甲版类型
    Mat ROI;                              //ROI
    double pitch=0;
    double yaw=0;
    double distance=0;
    /*   pnp平移向量结算坐标系     */
    double value_x=0;
    double value_y=0;
    double value_z=0;
//测试
    Mat src;
}Robot;



#endif // BASE_H
