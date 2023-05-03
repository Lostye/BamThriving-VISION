#include "threads.h"
#include <pthread.h>
#include "../serial/serial.h"


void Threads::T_serial(){
    while (1) {
        Serial.port_init(ss);

        tcflush(ss,TCIFLUSH);
        waitKey(1);
        Serial.port_RECV(ss);                              //串口接收

        while (serial_flag) {
        }

        cout<<"serial send  debug informations____________"<<endl;
        cout<<"send_pitch  "<<serial_pitch<<endl;
        cout<<"send_yaw  "<<serial_yaw<<endl;
        Serial.port_SEND(ss,serial_pitch,serial_yaw,fire_mode);

        double t=((double)getTickCount()-t__) / getTickFrequency();           //时间
        t__ =static_cast<double>(getTickCount());
        double FPS=1.0/t;
        cout<<" - - - - - - - - - - - - FPS_serial : "<<FPS<<endl;

    }

}
