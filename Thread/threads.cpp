#include "threads.h"
#include"../configer.h"
#include <pthread.h>
#include "../Prediction/pnpsolve.h"
#include "../Detection/detect.h"


void Threads::image(){


#ifdef SAVE_VIDEO
    VideoWriter writer;
    for(int i=0;i<999;i++)
    {
        VideoCapture cap;
        cap.open("Video"+std::to_string(i)+".avi");
        if(!cap.isOpened())
        {
            writer.open("Video"+std::to_string(i)+".avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), frame_rate, Size(640, 480));
            break;
        }
    }
#endif
    dahua.setCameraExposureTime(dahua.cameraSptr,5000,false);
    while (true) {



        while(camera_count - image_count < 1)
        {
        }


//            base.Switch=2;
            if(base.Switch==1){
                base.Switch=0;
            }

            vector<Robot> robots;                  //所有识别到的机器人数据
            Robot robot;                           //当前目标数据!Serial.If_Buff
            robot.yaw=0;
            robot.pitch=0;

            /*
              识别部分:得到robots（RotatedRect、ROI）
            */
            detect detect(base,robots);

            /*
              数字识别部分：补充robots.number,不符合则删除
            */
            robots=bam_svm.output0(robots);

            judge_robotype(robots);


            double t=((double)getTickCount()-t_) / getTickFrequency();           //时间
            t_ =static_cast<double>(getTickCount());
            double FPS=1.0/t;
            cout<<"FPS : "<<FPS<<endl;


            if(robots.size())
            {
                pnpsolve anglesolve(robots,base);
                robot=select_robot(robots,robot_last);
                cout<<"pitch : "<<robot.pitch<<endl;
                cout<<"yaw : "<<robot.yaw<<endl;
                cout<<"distance : "<<robot.distance<<endl;
            }
            else{
                base.flag=0;
                if(robot_last.distance>1)
                    this->count++;
                if(this->count>count_flash){
                    this->count=0;
                    robot_last.distance=0;
                }
            }

            Mat src;
            src=show_final(robot,robot_last,FPS);
    #ifdef SHOW_FINAL
            imshow("FINAL_IMG",src);
    #endif
    #ifdef SAVE_VIDEO
            writer<<src;
    #endif


            cout<<"________________________Serial debug______________________"<<endl;
            printf("get_yaw :%lf\n",Serial.getYaw);
            printf("get_pitch:%lf\n",Serial.getPitch);
            cout<<"get_buff : "<<Serial.If_Buff<<endl;
            cout<<"BULLED_SPEED : "<<Serial.getSpeed<<endl;



            robot.pitch+=Serial.getPitch;       //相对转绝对
            robot.yaw+=Serial.getYaw;
            cout<<"absolute pitch : "<<robot.pitch<<endl;
            cout<<"absolute yaw : "<<robot.yaw<<endl;
            cout<<"absolute distance : "<<robot.distance<<endl;

            if(!base.flag){

                cout<<"no pre_pitch  "<<serial_p<<endl;
                cout<<"no pre_yaw  "<<Serial.getYaw<<endl;

                if(this->count>0&&this->count<this->count_flash&&0){
                    cout<<"no  debug informations____________"<<endl;
                    pre.prediction(pre.pre_robot_last,pre.pre_robot,1,t,Serial.getSpeed);
                    serial_pitch=Serial.getPitch+gravity_pitch;
                    serial_yaw=pre.pre_robot.yaw;
                }
                else{
                    serial_pitch=Serial.getPitch;
                    serial_yaw=Serial.getYaw;
                    final_x=robot.roborect.center.x;
                    final_y=robot.roborect.center.y;
                    fire_mode =NoFind;
                }
                image_count++;
                continue;
            }

            int flag=0;
            if(robot.robonum==robot_last.robonum&&fabs(robot.yaw-robot_last.yaw)<2.0){
                flag=1;
            }

            Robot robot_trans=robot;               //装甲数据转移

            pre.prediction(robot_last,robot,flag,t,Serial.getSpeed);

            cout<<"predict  debug informations____________"<<endl;
            cout<<"pre_pitch  "<<robot.pitch<<endl;
            cout<<"pre_yaw  "<<robot.yaw<<endl;

            if(fire_command(robot,Serial.getYaw)){
                          serial_pitch=robot.pitch;
                          serial_yaw=robot.yaw;
          //                fire_mode=Fire;
                          fire_mode=Fire;
                             cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
                      }
                      else{
                          serial_pitch=robot.pitch;
                          serial_yaw=robot.yaw;
                          fire_mode=NOFire;
                             cout<<"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;
                      }
            serial_pitch=robot.pitch;
            serial_yaw=robot.yaw;

            robot_last=robot_trans;
            image_count++;

        }

}


