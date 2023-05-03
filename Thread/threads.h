#ifndef THREADS_H
#define THREADS_H
#include "cstdio"
#include "../configer.h"
#include <thread>
#include <sstream>
#include <chrono>
#include "../Number/svm.h"
#include "../serial/serial.h"
#include "../buff/BUFF.h"
#include "../dahua/dhua.h"
#include "../Prediction/predictor.h"



#define NoFind 0x0000//没找到装甲版不开火
#define NOFire 0x0077//找到装甲版不开火
#define Fire 0x0088//找到装甲版且开火

using namespace std::chrono;

using namespace std;
///
/// \brief The Threads class
///
class Threads
{
public:
    Threads();
    void camera();                //图像获取线程函数
    void image();                  //图像处理线程函数
    void T_serial();                 //串口线程
    double fps();                  //帧率计算
    void judge_robotype(vector<Robot> &robots);         //判断机器人装甲版类型
    Robot select_robot(vector<Robot> robots,Robot tobot_last);           //选择机器人
    Mat show_final(Robot robot,Robot robot_last,double fps);
    bool fire_command(Robot robot_pre, double robot_nopre);
    void pitch_compensation(double SPEED,double getPITCH,Robot &robot);

    Bam_SVM bam_svm;
    /**
     * 可调参
    */
    double count_flash=10;    //装甲板闪烁或丢失帧数
    int frame_rate=30;      //保存视频的帧率

    /**
     * 不可调参
    */
    size_t camera_count=0;
    size_t image_count=0;
    serial Serial;

    Base base;

    DHUA dahua;



    predictor pre;

    buff Buff;
    double pre_pitch=0;
    double pre_yaw=0;

    /**
     *数据缓存
    */
    int ss;                 //串口用
//    bool Switch=0;
    Robot robot_last;           //上一帧目标数据
    double t_;                //时间缓存
    double t__;
    double t_pre=0;            //帧数丢失时间累计
    double num;               //数字缓存
    double count=0;            //计数器
    double gravity_pitch=0;      //弹道补偿pitch缓存
    double serial_p=0;
    double serial_y=0;

    double serial_pitch=0;
    double serial_yaw=0;
    int fire_mode=0;

    bool serial_flag=0;

    double final_x=0;
    double final_y=0;



};


#endif // THREADS_H
