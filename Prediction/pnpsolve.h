#ifndef PNPSOLVE_H
#define PNPSOLVE_H
#pragma once
#include <iostream>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <opencv2/core/eigen.hpp>
#include "../configer.h"
using namespace Eigen;
using namespace std;
using namespace cv;

///
/// \brief The anglesolve class
///

class pnpsolve
{
public:
    pnpsolve(vector<Robot> &robots,Base base);                      //角度结算总函数


private:
    void setRobot_ArmorSize_3D();                         //设置机器人装甲板3D点
    void setTarget_Contour_2D(RotatedRect rect);            //设置机器人装甲板2D点
    void solvepnp(vector<double>& x_y_z);                                 //结算角度
    void PinHole_solver();                             //重新矫正结算
    void compensation();                            //人工矫正

    vector<Point2f> targetContour_2D;
    Point2f targetCenter;


    Mat CAMERA_MATRIX;                      //相机内参
    Mat DISTORTION_COEFF;                    //相机畸参


    vector<Point3f> SMALL_POINTS_3D;             //大装甲板目标3D点
    vector<Point3f> BIG_POINTS_3D;               //小装甲板目标3D点
    vector<Point3f> BUFF_POINTS_3D;              //风车装甲模块3D点

    double GUN_CAM_DISTANCE_Y=0;              //

    Mat rVec=Mat::zeros(3,1,CV_64FC1);
    Mat tVec=Mat::zeros(3,1,CV_64FC1);
    Eigen::Vector3f P_oc;                    //旋转矩阵转换得来的世界坐标

    double yaw;
    double pitch;
    double distance;
    int type;

};

#endif // ANGLESOLVE_H
