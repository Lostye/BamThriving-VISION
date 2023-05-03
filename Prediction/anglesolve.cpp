#include "anglesolve.h"


void anglesolve::setCameraParam(){
    this->CAMERA_MATRIX = (Mat_<double>(3, 3) <<1.272839738190869e+03 , 0 , 3.459885430731165e+02 , 0 , 1.272339252547457e+03 , 2.519683753050819e+02 , 0 , 0 , 1);
    this->DISTORTION_COEFF = (Mat_<double>(4, 1) <<-0.103600289802457 , 0.222426187179441 , 0 , 0);

}

void anglesolve::setRobot_ArmorSize(){
    switch (type)
    {
    case SMALL:
        SMALL_POINTS_3D.clear();
                                                                 //以中心为原点，顺时针输入
          SMALL_POINTS_3D.push_back(Point3f(-65, 37,0.0));
          SMALL_POINTS_3D.push_back(Point3f( 65, 37,0.0));
          SMALL_POINTS_3D.push_back(Point3f( 65,-37,0.0));
          SMALL_POINTS_3D.push_back(Point3f(-65,-37,0.0));

          break;

    case BIG:
        BIG_POINTS_3D.clear();
          BIG_POINTS_3D.push_back(Point3f(-112.5, 37, 0.0));
          BIG_POINTS_3D.push_back(Point3f( 112.5, 37, 0.0));
          BIG_POINTS_3D.push_back(Point3f( 112.5,-37, 0.0));
          BIG_POINTS_3D.push_back(Point3f(-112.5,-37, 0.0));

        break;

    default:
        break;

    }
}

void anglesolve::setTarget_Contour(RotatedRect rect){
    targetContour.clear();
    Point2f vertices[4];
    float lx = (rect.center.x - rect.size.width/2);
    float ly = (rect.center.y - rect.size.height/2);
    float rx = (rect.center.x + rect.size.width/2);
    float ry = (rect.center.y + rect.size.height/2);
    vertices[0] = Point2f(lx,ly);                     //点序从左上顺时针到左下
    vertices[1] = Point2f(rx,ly);
    vertices[2] = Point2f(rx,ry);
    vertices[3] = Point2f(lx,ry);
    targetContour.push_back(vertices[0]);
    targetContour.push_back(vertices[1]);
    targetContour.push_back(vertices[2]);
    targetContour.push_back(vertices[3]);

}

void anglesolve::P4P_solver()
{
    double x_pos = tVec.at<double>(0, 0);
    double y_pos = tVec.at<double>(1, 0);
    double z_pos = tVec.at<double>(2, 0);

    cout<<x_pos<<endl;
    cout<<y_pos<<endl;
    cout<<z_pos<<endl;

    double tan_pitch =atan2(y_pos,sqrt(x_pos * x_pos + z_pos * z_pos));
    double tan_yaw = atan2(x_pos,sqrt(y_pos * y_pos + z_pos * z_pos));

    pitch = -tan_pitch*180/CV_PI;
    yaw = tan_yaw*180/CV_PI;
}

void anglesolve::PinHole_solver()
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

    yaw = atan(rxNew) / CV_PI * 180;
    pitch = -atan(ryNew) / CV_PI * 180;
}

void anglesolve::solveAngles()
{
    Mat _rvec;
    switch (type)
    {
                                                 //PnP解算3轴矩阵
    case SMALL:

        solvePnP(SMALL_POINTS_3D, targetContour, CAMERA_MATRIX, DISTORTION_COEFF, _rvec, tVec,false,SOLVEPNP_AP3P);

     break;
    case BIG:
        solvePnP(BIG_POINTS_3D, targetContour, CAMERA_MATRIX, DISTORTION_COEFF, _rvec, tVec,false,SOLVEPNP_AP3P);

    default:
        break;
    }
/*
 *      平移矩阵转换
 * */
   // tVec.at<double>(1, 0) += GUN_CAM_DISTANCE_Y;
    double x_pos = tVec.at<double>(0, 0);
    double y_pos = tVec.at<double>(1, 0);
    double z_pos = tVec.at<double>(2, 0);

    distance = sqrt(x_pos * x_pos + y_pos * y_pos + z_pos * z_pos);



/*
 *     旋转矩阵转换
 * */
//    Mat rotMat;
//    Rodrigues(_rvec, rotMat);  //由于solvePnP返回的是旋转向量，故用罗德里格斯变换变成旋转矩阵

//    //Mat到Eigen格式转换
//    Eigen::Matrix3f R_n;
//    Eigen::Vector3f T_n;
//    cv2eigen(rotMat, R_n);
//    cv2eigen(tVec, T_n);
//    Eigen::Vector3f P_oc;


//    P_oc = -R_n.inverse()*T_n;
//    cout << "世界坐标" << P_oc << std::endl;
////cout<<P_oc[0]<<endl;


//double x_pos = P_oc(0,0);
//double y_pos = P_oc(1,0);
//double z_pos = P_oc(2,0);
//    distance = sqrt(x_pos * x_pos + y_pos * y_pos + z_pos * z_pos);

////  double tan_pitch = y_pos / sqrt(x_pos * x_pos + z_pos * z_pos);
//double tan_pitch =atan2(y_pos,sqrt(x_pos * x_pos + z_pos * z_pos));
//// double tan_yaw = x_pos / z_pos;
//double tan_yaw = atan2(x_pos,sqrt(y_pos * y_pos + z_pos * z_pos));
////  pitch = -atan(tan_pitch)*180/CV_PI;
//// yaw = atan(tan_yaw)*180/CV_PI;
//pitch = -tan_pitch*180/CV_PI;
//yaw = tan_yaw*180/CV_PI;



//    double r11 = rotM.ptr<double>(0)[0];
//    double r12 = rotM.ptr<double>(0)[1];
//    double r13 = rotM.ptr<double>(0)[2];
//    double r21 = rotM.ptr<double>(1)[0];
//    double r22 = rotM.ptr<double>(1)[1];
//    double r23 = rotM.ptr<double>(1)[2];
//    double r31 = rotM.ptr<double>(2)[0];
//    double r32 = rotM.ptr<double>(2)[1];
//    double r33 = rotM.ptr<double>(2)[2];

//    double thetaz = atan2(r21, r11) / CV_PI * 180;
//    double thetay = atan2(-1 * r31, sqrt(r32*r32 + r33*r33)) / CV_PI * 180;
//    double thetax = atan2(r32, r33) / CV_PI * 180;

//   pitch=thetay;
//   yaw=thetax;

    if (distance <= 8000)
    {
        P4P_solver();
    }

//    else
//    {
//        PinHole_solver();
//    }
}

anglesolve::anglesolve(vector<Robot> &robots,Base base)
{
//    setCameraParam();
    this->CAMERA_MATRIX=base.CAMERA_MATRIX;
    this->DISTORTION_COEFF=base.DISTORTION_COEFF;
    for(size_t i=0;i<robots.size();i++){
        this->targetCenter=robots[i].roborect.center;
        this->type=robots[i].robot_type;
        setRobot_ArmorSize();
        setTarget_Contour(robots[i].roborect);
        solveAngles();
        robots[i].pitch=this->pitch;
        robots[i].yaw=this->yaw;
        robots[i].distance=this->distance;
        cout<<"pitch : "<<pitch<<endl;
        cout<<"yaw : "<<yaw<<endl;
        cout<<"distance : "<<distance<<endl;
    }
}







