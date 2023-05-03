#include "kalman.h"
using namespace cv;
using namespace std;
//#define Sin
//加速度的倍率决定预测步长
#define alpha 30

kalman::kalman()
{
kalman_init();
}

void kalman::kalman_init()
{
    KF=KalmanFilter(2, 1, 0);
    measurement = Mat::zeros(1, 1, CV_32F);
    KF.transitionMatrix = (Mat_<float>(2, 2) <<//转移矩阵A
                           1, 1,
                           0, 1);
    setIdentity(KF.measurementMatrix,Scalar::all(1));//观测矩阵H
    setIdentity(KF.processNoiseCov, Scalar::all(1e-5));//过程噪声Q，
    setIdentity(KF.measurementNoiseCov, Scalar::all(1e-2));//测量噪声R，形容测量的准确性
    setIdentity(KF.errorCovPost, Scalar::all(1));//后验错误估计协方差矩阵P
    randn(KF.statePost, Scalar::all(0), Scalar::all(0.1));
}

cv::Point2f kalman::kalmanRun(Mat src, Point2f measurePt, Point2f center, double R,bool color)
{
    Mat img=src.clone();
    Mat prediction;
    Point predictPt;

    this->color=color;

    //updates statePost & errorCovPost//更新

    double MeasureAngle=Point2angle(measurePt,center);
    #ifdef Sin
    measurement.at<float>(0) =sin(MeasureAngle/2);
    #endif
    #ifndef Sin
    measurement.at<float>(0) =MeasureAngle;
    #endif
    //measurement = (Mat_<float>(1,1) <<CV_PI);
    KF.correct(measurement);
    prediction = KF.predict();

    cout<< KF.predict()<<endl;

    float predictAngle[2];
    predictAngle[0]= MeasureAngle;
    predictAngle[1]=prediction.at<float>(1);//预测的角加速度
    printf("\n测量的角度：%f\n",  MeasureAngle);
    printf("预测的角加速度：%f\n", predictAngle[1]);



    predictPt = PreAngle(center, R, predictAngle);//预测点  

    Mat test = Mat(KF.transitionMatrix*KF.statePost);


//    float pre_test_angle[2];
//    pre_test_angle[0]=0;
//    pre_test_angle[1]=0;
//    drawMarker(img,predictPt, Scalar(0, 0, 255), cv::MARKER_SQUARE, 20, 2);//红色框是预测点
//    drawMarker(img,measurePt,Scalar(255, 255, 255), cv::MARKER_STAR, 10, 2);//测量点
//    drawMarker(img, PreAngle(center,R,pre_test_angle),Scalar(255, 0, 255), cv::MARKER_STAR, 20, 2);//测量点

//    namedWindow("Kalman",0);
//    resizeWindow("Kalman",Size(640,480));
//    imshow( "Kalman", img );


    return predictPt;
}

inline Point2f kalman:: PreAngle(Point2f center, double R, float* angles)
{
    float angle;
#ifdef Sin
    float theta_angle=asin(angles[1])*2;
#endif
#ifndef Sin
    float theta_angle=angles[1];
    if(angles[1]>limit_a) theta_angle=limit_a;
    else if (angles[1]<-limit_a)theta_angle=-limit_a;
//    if(angles[1]<0&&angles[1]>-0.0002)waitKey(0);
#endif
//    if(theta_angle>0)theta_angle*=-1;

//    if(!color)
        angle=angles[0]+theta_angle*alpha;
//    else
//        angle=angles[0]-theta_angle*alpha;//sunshizhen

    cout<<"要还原的角度："<<angles[0]<<endl;
    cout<<"还原后的角度："<<angle<<endl;

    return Point2f(center.x+cos(angle)*R, center.y-sin(angle)*R);
  }




/*
    因为存在求解出的角度在0—360之间存在突变，所以这里改成不直接对角度进行迭代了，而是对cos（angle）进行滤波
    sin（）在0～360度之间连续可导，且在0～180不存在突变。最后再用arcsin（）求出角度
*/
inline float kalman::Point2angle(Point2f point ,Point2f center)
{   //fastAtan2()第一个参数是角的对边，第二个是角的邻边,返回值是角度0～360
    float angle=fastAtan2(center.y-point.y,point.x-center.x)/180*CV_PI;
    diff=angle-last_angle;
    if(diff>CV_PI*1.5)diff-=2*CV_PI;
    else if(diff<-CV_PI*1.5)diff+=2*CV_PI;
    cout<<"                                     diff:"<<diff<<endl;
    real_angle+=diff;
    last_angle=angle;
    return real_angle;
}

