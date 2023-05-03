#ifndef PREDICTOR_H
#define PREDICTOR_H
#pragma once

#include "../configer.h"
#include <iostream>
#include <Eigen/Dense>

using namespace std;
using Eigen::MatrixXd;

///
/// \brief The predictor class
///

class predictor
{
public:
    predictor();
    void prediction(Robot robot_last,Robot& robot,bool flag, double t,double BULLED_SPEED);
    Robot pre_robot;     //伪预测缓存的
    Robot pre_robot_last;

private:
    void time_corrector(double t,double BULLED_SPEED);        //时间矫正
    void v_and_a(bool flag);            //计算速度和时间
    void get_MatrixXd();                //初始化矩阵
    void reset();                       //重置
    void predict();                     //预测
    void update();                      //更新


   //缓存参数

    Robot robot_last;    //预测缓存
    Robot robot;
    double v_p_l;       //上一帧v_pitch
    double v_Y_l;       //上一帧v_yaw

   //物理参数
    double a_pitch;     //加速度
    double a_yaw;
    double v_pitch;     //速度
    double v_yaw;
    double t;           //时间
//    double fire_t=0.9;
    double fire_t=0.75;
    double t_V;

   //预测参数
    MatrixXd x;         //状态向量
    MatrixXd u;         //控制向量
    MatrixXd z;         //观测向量
    MatrixXd A;         //状态转移矩阵
    MatrixXd B;         //控制矩阵
   //更新参数
    MatrixXd K;         //卡尔曼增益
    MatrixXd Q;         //预测协方差
    MatrixXd P;         //协方差矩阵
    MatrixXd R;         //观测噪声协方差
    MatrixXd H;         //
};

#endif // PREDICTOR_H
