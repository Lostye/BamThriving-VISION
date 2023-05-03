#include "pnpsolve.h"



void pnpsolve::setRobot_ArmorSize_3D(){
    switch (type)
    {
    case SMALL:
        SMALL_POINTS_3D.clear();
                                                                  //以中心为原点，顺时针输入
          SMALL_POINTS_3D.push_back(Point3f(-65, 37,0.0));
          SMALL_POINTS_3D.push_back(Point3f( 65, 37,0.0));
          SMALL_POINTS_3D.push_back(Point3f( 65,-37,0.0));
          SMALL_POINTS_3D.push_back(Point3f(-65,-37,0.0));

//            SMALL_POINTS_3D.push_back(Point3f(-82, 61,0.0));
//            SMALL_POINTS_3D.push_back(Point3f( -82, -61,0.0));
//            SMALL_POINTS_3D.push_back(Point3f( 82,-61,0.0));
//            SMALL_POINTS_3D.push_back(Point3f(82,61,0.0));

          break;

    case BIG:
        BIG_POINTS_3D.clear();
          BIG_POINTS_3D.push_back(Point3f(-112.5, 37, 0.0));
          BIG_POINTS_3D.push_back(Point3f( 112.5, 37, 0.0));
          BIG_POINTS_3D.push_back(Point3f( 112.5,-37, 0.0));
          BIG_POINTS_3D.push_back(Point3f(-112.5,-37, 0.0));

        break;

    case BUFF:
          BUFF_POINTS_3D.clear();
          BUFF_POINTS_3D.push_back(Point3f(-80, 80, 0.0));
          BUFF_POINTS_3D.push_back(Point3f( 80, 80, 0.0));
          BUFF_POINTS_3D.push_back(Point3f( 80,-80, 0.0));
          BUFF_POINTS_3D.push_back(Point3f(-80,-80, 0.0));

        break;

    default:
        break;

    }
}

void pnpsolve::setTarget_Contour_2D(RotatedRect rect){
    targetContour_2D.clear();
    Point2f vertices[4];
    float lx = (rect.center.x - rect.size.width/2);
    float ly = (rect.center.y - rect.size.height/2);
    float rx = (rect.center.x + rect.size.width/2);
    float ry = (rect.center.y + rect.size.height/2);
    vertices[0] = Point2f(lx,ly);                     //与3D点输入循序一致
    vertices[1] = Point2f(rx,ly);
    vertices[2] = Point2f(rx,ry);
    vertices[3] = Point2f(lx,ry);
    targetContour_2D.push_back(vertices[0]);
    targetContour_2D.push_back(vertices[1]);
    targetContour_2D.push_back(vertices[2]);
    targetContour_2D.push_back(vertices[3]);

}


void pnpsolve::PinHole_solver()
{
    double fx = CAMERA_MATRIX.at<double>(0, 0);
    double fy = CAMERA_MATRIX.at<double>(1, 1);
    double cx = CAMERA_MATRIX.at<double>(0, 2);
    double cy = CAMERA_MATRIX.at<double>(1, 2);
    Point2f pnt;
    vector<cv::Point2f> in;
    vector<cv::Point2f> out;
    in.push_back(targetCenter);

    undistortPoints(in, out, CAMERA_MATRIX, DISTORTION_COEFF, noArray(), CAMERA_MATRIX);
    pnt = out.front();

    double rxNew = (pnt.x - cx) / fx;
    double ryNew = (pnt.y - cy) / fy;

    yaw = atan(rxNew) / CV_PI * 180 ;
    pitch = -atan(ryNew) / CV_PI * 180 ;
}

void pnpsolve::compensation()
{
    if(type==BUFF){

    }


    else{
        if(distance<1500){
            pitch = pitch -6.5;
            yaw =yaw +2;
        }
        else if(distance<2500){
            pitch = pitch -6;
            yaw =yaw +3.2;
        }
        else if(distance<3500){
            pitch = pitch -5.8;
            yaw =yaw +3.58;
        }
        else if(distance<4500){
            pitch = pitch -5.2;
            yaw =yaw +3.78;
        }
        else if(distance<5000){
            pitch = pitch -5.05;
            yaw =yaw +3.78;
        }
        else {
            pitch = pitch -4.5;
            yaw =yaw +3.9;
        }
    }

}

void pnpsolve::solvepnp(vector<double>& x_y_z)
{
    Mat _rvec;
    switch (type)
    {
                                                 //PnP解算3轴矩阵
    case SMALL:

        solvePnP(SMALL_POINTS_3D, targetContour_2D, CAMERA_MATRIX, DISTORTION_COEFF, _rvec, tVec,false,SOLVEPNP_AP3P);
        break;

    case BIG:
        solvePnP(BIG_POINTS_3D, targetContour_2D, CAMERA_MATRIX, DISTORTION_COEFF, _rvec, tVec,false,SOLVEPNP_AP3P);
        break;

    case BUFF:
        solvePnP(BUFF_POINTS_3D, targetContour_2D, CAMERA_MATRIX, DISTORTION_COEFF, _rvec, tVec,false,SOLVEPNP_AP3P);
        break;

    default:
        break;
    }
/*
 *      平移矩阵转换
 * */
    tVec.at<double>(1, 0) += GUN_CAM_DISTANCE_Y;
    double x_pos = tVec.at<double>(0, 0);
    double y_pos = tVec.at<double>(1, 0);
    double z_pos = tVec.at<double>(2, 0);
    x_y_z[0] = x_pos;
    x_y_z[1] = y_pos;
    x_y_z[2] = z_pos;


    distance = sqrt(x_pos * x_pos + y_pos * y_pos + z_pos * z_pos);

    if (distance <= 10000)
    {
        double theta_p =atan2(y_pos,sqrt(x_pos * x_pos + z_pos * z_pos));
        double theta_y = atan2(x_pos,sqrt(y_pos * y_pos + z_pos * z_pos));

        pitch = -theta_p*180/CV_PI*1.1;
        yaw = theta_y*180/CV_PI*1.1;
    }
    else
    {
      //  PinHole_solver();
    }

    compensation();


}

pnpsolve::pnpsolve(vector<Robot> &robots,Base base)
{
    this->CAMERA_MATRIX=base.CAMERA_MATRIX;
    this->DISTORTION_COEFF=base.DISTORTION_COEFF;
    for(size_t i=0;i<robots.size();i++){

        this->targetCenter=robots[i].roborect.center;
        this->type=robots[i].robot_type;

        setRobot_ArmorSize_3D();
        setTarget_Contour_2D(robots[i].roborect);

        vector<double> x_y_z(3);
        solvepnp(x_y_z);
        robots[i].pitch=this->pitch;
        robots[i].yaw=0-this->yaw;
        robots[i].distance=this->distance;
        robots[i].value_x=x_y_z[0];
        robots[i].value_y=x_y_z[1];
        robots[i].value_z=x_y_z[2];


//        cout<<"pitch : "<<pitch<<endl;
//        cout<<"yaw : "<<yaw<<endl;
//        cout<<"distance : "<<distance<<endl;
    }
}







