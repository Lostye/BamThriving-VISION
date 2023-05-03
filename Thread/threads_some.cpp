#include "threads.h"
#include <pthread.h>

Threads::Threads()
{

}
void Threads::camera(){

//    VideoCapture cap("/home/shayu/桌面/欣竹视觉_步兵B/BamThriving-VISION/build/Video60.avi");
    while (true) {

        while(camera_count - image_count >= 1)
        {
        }

        Mat src=dahua.camera_src.clone();
        cout<<"frame sise:  "<< src.size<<endl;
        waitKey(1);
        base.src=src.clone();
//       cap>>base.src;


        camera_count++;
        if(camera_count >= 99900)
        {
            camera_count = 1;
            image_count = 0;
        }
        cout<<"s";
        imshow("camera",base.src);

    }
}


void Threads::judge_robotype(vector<Robot> &robots){

    for(size_t i=0;i<robots.size();i++){
        RotatedRect rect=robots[i].roborect;
        if(rect.size.width/rect.size.height>3.2){
            robots[i].robot_type=BIG;
        }
        else {
            robots[i].robot_type=SMALL;
        }
        cout<<"debug_type:  "<<robots[i].robot_type<<endl;
    }
}


Robot Threads::select_robot(vector<Robot> robots,Robot robot_last){

    double score=0;
//    double weight_x=0.1;
//    double weight_y=0.1;
//    double weight_z=0.0001;
    size_t j=0;
    int somsom=0;                             //同数字多装甲版标志

    for(size_t i=0;i<robots.size();i++){
        if(robots[i].robonum==robot_last.robonum&&robots[i].distance<=5500){

            if(somsom){
                if(robots[j].roborect.center.x<robots[i].roborect.center.x){

                }
                else{
                    j=i;
                }
                break;
            }

            somsom=1;
            j=i;
        }

        else if(robots[i].distance>score&&somsom==0)
          {
            score =robots[i].distance;
            j=i;
          }
    }
    base.flag=1;
    if(robot_last.robonum&&robots[j].robonum!=robot_last.robonum){
        count++;
        if(count>count_flash){
            base.flag=1;
        //    this->t_pre=0;
        }
        else{
            base.flag=0;
       //     if(count>0)
       //        this->t_pre+=t;
        }

    }
    if(count>count_flash||base.flag){
        count=0;
    }
    return robots[j];
}

Mat Threads::show_final(Robot robot,Robot robot_last,double fps){

    Mat img1;
//    Point2f vertex[4];
    base.src.copyTo(img1);

    line(img1,Point(img1.cols/2-350,img1.rows/2),Point(img1.cols/2+350,img1.rows/2),Scalar(64,224,208),1,LINE_8);
    line(img1,Point(img1.cols/2,img1.rows/2-200),Point(img1.cols/2,img1.rows/2+200),Scalar(64,224,208),1,LINE_8);

    if(robot.robonum==robot_last.robonum){
//        robot.roborect.points(vertex);
//        for (size_t i = 0; i < 4; i++)
//        {
//            line(img1, vertex[i], vertex[(i + 1) % 4], Scalar(18, 97, 255), 1, LINE_AA);
//        }

        circle(img1,robot.roborect.center,10,Scalar(255,30,255),2);
#ifdef Visual_Kalman
        circle(img1,Point2f(final_x,final_y),10,Scalar(0,255,0),2);
#endif


        stringstream stream;
        stream << robot.robonum;
        string str;
        stream >> str;
        Point2f poi(robot.roborect.center.x-robot.roborect.size.width/2,robot.roborect.center.y-robot.roborect.size.height/2);
        putText(img1, str, poi, FONT_HERSHEY_PLAIN, 3, Scalar(0, 159, 255), 2, 5);
     }
//     robot_last.roborect.points(vertex);
//     for (size_t i = 0; i < 4; i++)
//     {
//        line(img1, vertex[i], vertex[(i + 1) % 4], Scalar(183, 43, 226), 1, LINE_AA);
//     }

     putText(img1,"FPS:"+std::to_string(fps),Point(10,20),1,1,Scalar(0,255,0),2,LINE_8);


    //imshow("robots",img1);
    return img1;
}

bool Threads::fire_command(Robot robot_pre, double robot_nopre){
    if(fabs(robot_pre.yaw-robot_nopre)<7)
        return 1;
    return 0;
}

void Threads::pitch_compensation(double SPEED,double getPITCH,Robot &robot){

    //一些计算参数
    double m=3.1/1000;
    double g=0.98;
    double air_coef=0.01;
    double real_height=0;
    double temp_height=robot.value_y;

    double distance_horizontal=sqrt(pow(robot.value_x,2)+pow(robot.value_z,2));
    double angle=tan(robot.value_y/distance_horizontal);//不补偿时仰角
    double delta_h=0;
    double iter_count=0;
    while(1)//当临时目标点与计算值相近，停止迭代
    {
        double equation1=m*m*g/pow(air_coef,2);//m^2*g/k^2
        double ln_temp=1-air_coef*distance_horizontal/(m*SPEED*cos(angle));//1-k(xz)/mvcos
        double ln=log(ln_temp)/log(2.71828);
        double equation2=SPEED*sin(angle)+m*g/air_coef;//vsin+mg/k
        double equation3=distance_horizontal/(SPEED*cos(angle));//xz/vcos

        real_height=equation1*ln+equation2*equation3;//用仰角计算理论实际高度
        delta_h=fabs(temp_height-real_height);//由于重力偏离高度,若敌方机器人在我方机器人下方，real_height<0,temp_height<0,且real_height<temp_height,(temp_height-real_height),但补偿高度>0故取绝对值
        temp_height+=delta_h;//加上偏离高度作为下一次枪管对准高度
        angle=tan(temp_height/distance_horizontal);
        cout<<"第"<<iter_count<<"次迭代"<<"angle:"<<angle<<"temp y:"<<temp_height<<"delta_h:"<<delta_h<<endl;
        iter_count++;

        if(delta_h<1e-6)
            break;
    }

    robot.pitch=angle;
}
