
#include "predictor.h"

predictor::predictor()
{

}


void predictor::time_corrector(double t,double BULLED_SPEED){
    if(t>0.02)
        t=0.01;
    int ek=(robot.distance*1000.0)/BULLED_SPEED;
    double es=ek/1000000.0;
    this->t_V=t;
//    this->t_V=1.0;
    this->t=t+es+fire_t;
//    cout<<"debug_es"<<es<<endl;
//    this->t=0.05;
//    cout<<"pre_t:   "<<this->t<<endl;
}

void predictor:: v_and_a(bool flag){

    v_p_l=v_pitch;
    v_Y_l=v_yaw;

    v_pitch=(robot.pitch-robot_last.pitch)/t_V;
    v_yaw=(robot.yaw-robot_last.yaw)/t_V;
//    cout<<"debug_some   "<<robot.pitch-robot_last.pitch<<endl;
//    cout<<"debug_some1   "<<t_V<<endl;

    cout<<"V_Y   "<<v_yaw<<endl;
    cout<<"V_P   "<<v_pitch<<endl;
    cout<<"t_V   "<<t_V<<endl;
 if(flag){
       a_pitch=(v_pitch-v_p_l)/t_V;
       a_yaw=(v_yaw-v_Y_l)/t_V;
//       cout<<"a______________________"<<a_yaw<<endl;
   }
   else{
     a_pitch=0;
     a_yaw=0;
 }
}

void predictor::get_MatrixXd(){
    x=MatrixXd(4,1);
    x<<robot.pitch,
       robot.yaw,
       v_pitch,
       v_yaw;
    u=MatrixXd(2,1);
    u<<a_pitch,
       a_yaw;
    z=MatrixXd(4,1);
    z<<robot.pitch,
       robot.yaw,
       v_pitch,
       v_yaw;
    A=MatrixXd(4,4);
    A<<1,0,t,0,
       0,1,0,t,
       0,0,1,0,
       0,0,0,1;
    B=MatrixXd(4,2);
    B<<(t_V*t)/2,0,
        0,(t_V*t)/2,
        t,0,
        0,t;
    /*
     * 风车
     */
//    R=MatrixXd(4,4);       //观测噪声协方差(信任估计值调大，信任观测值调小)
//    R<<3.5,0,0,0,
//       0,5.1,0,0,
//       0,0,3.2,0,
//       0,0,0,4.8;

//    Q=MatrixXd(4,4);       //预测协方差(信任估计值调大，临界点未知)
//    Q<<45,0,0,0,
//       0,50,0,0,
//       0,0,50,0,
//       0,0,0,45;

    R=MatrixXd(4,4);       //观测噪声协方差(信任估计值调大，信任观测值调小)
    R<<1,0,0,0,
       0,4.8,0,0,
       0,0,1,0,
       0,0,0,12;

    Q=MatrixXd(4,4);       //预测协方差(信任估计值调大，临界点未知)
    Q<<1,0,0,0,
       0,55,0,0,
       0,0,1,0,
       0,0,0,145;

    H=MatrixXd(4,4);
    H<<1,0,0,0,
       0,1,0,0,
       0,0,1,0,
       0,0,0,1;
}

void predictor::reset(){
    P=MatrixXd(4,4);
    P<<1,0,0,0,
       0,1,0,0,
       0,0,1,0,
       0,0,0,1;
    K=MatrixXd(4,4);
    K<<1,0,0,0,
       0,1,0,0,
       0,0,1,0,
       0,0,0,1;
    x=MatrixXd(4,1);
    x<<robot.pitch,
       robot.yaw,
       v_pitch,
       v_yaw;
}

void predictor::predict(){
//    x=A*x+B*u;
    x=A*x;
    P=A*P*A.transpose()+Q;
}

void predictor::update(){
    MatrixXd a=P*H.transpose();
    MatrixXd b=H*P*H.transpose()+R;
    K=a*b.inverse();
    x=x+K*(z-H*x);
    P=(MatrixXd::Identity(4,4)-K*H)*P;
}

void predictor::prediction(Robot robot_last, Robot& robot, bool flag, double t, double BULLED_SPEED){
    this->robot_last=robot_last;
    this->robot=robot;
    time_corrector(t,BULLED_SPEED);
    v_and_a(flag);
    if(!flag)
        reset();
    get_MatrixXd();
    predict();
    update();

    if(!flag)
        reset();
 cout<<"pre_debug: "<<x(1,0)-robot.yaw<<endl;
//    robot.pitch=x(0,0);
    robot.yaw=x(1,0);




    this->pre_robot_last=robot;
    this->pre_robot=robot;
    this->pre_robot.pitch=x(0,0);
    this->pre_robot.yaw=x(1,0);
}

