#include "detect.h"

bool detect::double_x(RotatedRect lightL, RotatedRect lightR){
    float width = max(lightR.size.width, lightL.size.width);
   // size_t threshold = (size_t)(width);
    if(abs(lightR.size.width-lightL.size.width)< 5*width)
    {
//        cout<<"debug____________________________________________"<<endl;
        if (fabs((lightR.center.x) - (lightL.center.x)) >= 0.5* width && fabs((lightR.center.x) - (lightL.center.x)) < 30 * width)
            return 1;
    }

    return 0;
}

bool detect::double_y(RotatedRect lightL, RotatedRect lightR){
    float height = min(lightL.size.height,lightR.size.height);
    if (abs(lightR.size.height - lightL.size.height) < 0.8 * height)
    {
        if(fabs((lightR.center.y) - (lightL.center.y)) >= 0.0 && fabs((lightR.center.y) - (lightL.center.y)) < 1.0 * height)
        return 1;
    }

    return 0;
}

bool detect::double_angle(RotatedRect lightL, RotatedRect lightR){
    double angle_double_threshould=10;
    float lL = lightL.angle, lR = lightR.angle;
    if(lL < 200 && lL > 165 && lR < 35 && lR > -1)
    {
        if (180 - lL + lR < angle_double_threshould)
        {
            return 1;
        }
        else
            return 0;
    }
    if (lR < 200 && lR > 165 && lL < 35 && lL > -1)
    {
        if (180 - lR + lL < angle_double_threshould)
            return 1;
        else
            return 0;
    }
    if (fabs(lL - lR) < angle_double_threshould)
    {
        return 1;
    }
    else
        return 0;
}

bool detect::form_armor(){
    rect_choose.clear();
    RotatedRect lightL, lightR;
    RotatedRect rect;
    for (size_t i = 0; i < LED_SINGLE_CHOOSE.size() - 1; i++)
    {
        for (size_t j = i + 1; j<i+2 && j < LED_SINGLE_CHOOSE.size(); j++)
        {
            if (LED_SINGLE_CHOOSE[i].center.x < LED_SINGLE_CHOOSE[j].center.x)
            {
                lightL = LED_SINGLE_CHOOSE[i];
                lightR = LED_SINGLE_CHOOSE[j];
//                cout<<"debug   ----------"<<endl;
//                cout<<"L_width:  "<<lightL.size.width<<endl;
//                cout<<"L_heigtht:  "<<lightL.size.height<<endl;
//                cout<<"R_width:  "<<lightR.size.width<<endl;
//                cout<<"R_heigtht:  "<<lightR.size.height<<endl;
            }
            if (!double_x(lightL, lightR)){
                cout<<"false double x distance!"<<endl;
             //   waitKey(100);
                continue;
            }
            if (!double_y(lightL, lightR)){
                cout<<"false double y distance!"<<endl;
                continue;
            }
            if (!double_angle(lightL,lightR)){
                cout<<"false double angle!"<<endl;
                continue;
            }

            double width=fabs(lightL.center.x-lightR.center.x);
//            if (abs(lightL.angle - lightR.angle) > 140 ) {
                rect = RotatedRect(Point2f((lightL.center.x + lightR.center.x)/2, (lightL.center.y + lightR.center.y)/2),Size2f(width, (lightL.size.height+ lightR.size.height)/2), 0);
//            }
//            else {
//                rect = RotatedRect(Point2f((lightL.center.x + lightR.center.x)/2, (lightL.center.y + lightR.center.y)/2),Size2f(width, (lightL.size.height+lightR.size.height)/2), (lightL.angle +lightR.angle)/2);
//            }
            if(fabs(lightL.size.height-lightR.size.height)<2.0){
                Vec4i LineA;
                Vec4i LineB;
                LineA[0]=lightL.center.x;
                LineA[1]=lightR.center.y+lightR.size.height/2;
                LineA[2]=lightR.center.x;
                LineA[3]=lightL.center.y-lightL.size.height/2;
                LineB[0]=lightL.center.x;
                LineB[1]=lightR.center.y-lightR.size.height/2;
                LineB[2]=lightR.center.x;;
                LineB[3]=lightL.center.y+lightL.size.height/2;
                double ka, kb;
                ka = (double)(LineA[3] - LineA[1]) / (double)(LineA[2] - LineA[0]); //求出LineA斜率
                kb = (double)(LineB[3] - LineB[1]) / (double)(LineB[2] - LineB[0]); //求出LineB斜率

                Point2f crossPoint;
                crossPoint.x = (ka*LineA[0] - LineA[1] - kb*LineB[0] + LineB[1]) / (ka - kb);
                crossPoint.y = (ka*kb*(LineA[0] - LineB[0]) + ka*LineB[1] - kb*LineA[1]) / (ka - kb);

                rect.center=crossPoint;
             }
            rect_choose.push_back(rect);

        }
    }
    if(!rect_choose.size()){
         cout<<"no two lights form to armor!"<<endl;
         return 0;
    }
#ifdef SHOW_DOUBLE_LIGHTS
            Mat img1;
            RNG rng(time(0));
            Point2f vertex[4];
            src.copyTo(img1);
            for (size_t i = 0; i < rect_choose.size(); i++)
            {
                rect_choose[i].points(vertex);
              //  cout << "rect_choosexy:" << rect_choose[0].center.x << " " << rect_choose[0].center.y << endl;
             //   cout << "rect_choosehw:" << rect_choose[0].size.height << " " << rect_choose[0].size.width << endl;
                for (size_t i = 0; i < 4; i++)
                {
                    line(img1, vertex[i], vertex[(i + 1) % 4], Scalar(18, 97, 255), 1, LINE_AA);
               //     cout << "point:"<< vertex[i].x << " " << vertex[i].y << "\n" << endl;
                }
               /* float lx = rect_choose[i].center.x - rect_choose[i].size.width/2;
                float ly = rect_choose[i].center.y - rect_choose[i].size.height/2;
                float rx = rect_choose[i].center.x + rect_choose[i].size.width/2;
                float ry = rect_choose[i].center.y + rect_choose[i].size.height/2;
                rectangle(img1, Point(lx,ly), Point(rx,ry), Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)), 3, LINE_8, 0);*/
            }
         //   cout << "rect_choose: " << rect_choose.size() << endl;
           /* putText(img1, "0", Point(pointlist1[0].x , pointlist1[0].y - 50), cv::FONT_HERSHEY_PLAIN, 2, Scalar(0, 0, 255), 2, 1);
            putText(img1, "1", Point(pointlist1[1].x , pointlist1[1].y - 50), cv::FONT_HERSHEY_PLAIN, 2, Scalar(0, 0, 255), 2, 1);
            putText(img1, "2", Point(pointlist1[2].x , pointlist1[2].y + 50), cv::FONT_HERSHEY_PLAIN, 2, Scalar(0, 0, 255), 2, 1);
            putText(img1, "3", Point(pointlist1[3].x , pointlist1[3].y + 50), cv::FONT_HERSHEY_PLAIN, 2, Scalar(0, 0, 255), 2, 1);*/
#ifdef LARGE_SRC
            resize(img1, img1, Point(img1.rows * 0.7, img1.cols * 0.5));
#endif
            imshow("doublelight", img1);
            waitKey(1);

#endif
     return 1;
}
