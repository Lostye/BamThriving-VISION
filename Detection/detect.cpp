#include "detect.h"


bool detect::w_h_Ratio(RotatedRect rect){
    if (rect.size.height / rect.size.width >1.1 && rect.size.height / rect.size.width < 20.0  &&
            rect.size.width > 0.3 &&  rect.size.width <40 && rect.size.height > 2.0  && rect.size.height < 150)
        return 1;
    else
        return 0;
}

bool detect::angle(RotatedRect rect){
    if (rect.angle <= 180 && rect.angle >= 150)
        return 1;
    else if(rect.angle <= 30 && rect.angle >= 0)
        return 1;
    else
        return 0;
}

bool cmp (RotatedRect x ,RotatedRect y)                      //灯条点排序
{
    if(x.size.height < 20)
    {
        if(abs(x.center.y - y.center.y) < 20 && abs(x.center.x - y.center.x) > 10 )
            return x.center.x < y.center.x;
        else
            return x.center.y < y.center.y;
    }
    else
    {
        if(abs(x.center.y - y.center.y) < 40 && abs(x.center.x - y.center.x) > 10 )
            return x.center.x < y.center.x;
        else
            return x.center.y < y.center.y;
    }
}

bool cmp1(RotatedRect x ,RotatedRect y)                        //灯条点排序
{
    if(abs(x.center.y - y.center.y) > 10)
    {
        return x.center.x < y.center.x;
    }
    return 0;
}

void detect::adaptthresh(){
    single_light_lowest_threshold=0;
    for (size_t i = 0; i < LED_SINGLE_CHOOSE.size(); i++)
    {
        for (size_t j = i+1; j < LED_SINGLE_CHOOSE.size(); j++)
        {
            if(abs(LED_SINGLE_CHOOSE[j].center.x - LED_SINGLE_CHOOSE[i].center.x) < 10)
            {
                if(abs(LED_SINGLE_CHOOSE[j].center.y - LED_SINGLE_CHOOSE[i].center.y) < 50)
                {
                    single_light_lowest_threshold++;
                }
            }
        }
    }
  //  cout << "single_light_threshould  :  " << single_light_lowest_threshold << endl;
}
bool detect::singlelight()
{
    LED_SINGLE_CHOOSE.clear();
    vector<Mat> channels;
    split(src, channels);
    Mat enemy_img;
    RotatedRect rect;
    cvtColor(src, gray, COLOR_BGR2GRAY);
    if (color == 1)		//提取不同颜色下的灯条图像
    {
        subtract(channels[2] , channels[0] , enemy_img);
//        subtract(enemy_img , channels[1] , enemy_img);
    }
    else
    {
        subtract(channels[0] , channels[2] , enemy_img);
   //     subtract(enemy_img , channels[1] , enemy_img);
    }
#ifdef SHOW_GRAY
    imshow("gray",enemy_img);
#endif
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    threshold(enemy_img, enemy_img, single_light_lowest_threshold, 255, THRESH_BINARY);
    Mat kernel = getStructuringElement(MORPH_RECT, Size(5 ,5));
    dilate(enemy_img, enemy_img, kernel);
    kernel = getStructuringElement(MORPH_RECT, Size(3 ,3));
    erode(enemy_img,enemy_img,kernel);
#ifdef SHOW_SINGLE_THRESH
    imshow("single thresh",enemy_img);
#endif
    findContours(enemy_img, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    if(!contours.size()){
        cout<<"can not find the lights!"<<endl;
        return 0;
    }
//    return 0;
    for (size_t i = 0; i < contours.size(); i++)
    {
        if(contours[i].size() < 2)
        {
            continue;
        }
        rect = fitEllipse(contours[i]);
        if (!angle(rect))
        {
            continue;
        }
        if (!w_h_Ratio(rect))
        {
            continue;
        }
        LED_SINGLE_CHOOSE.push_back(rect);
//        cout<<"debug------------"<<endl;
//        cout<<"angle :"<<rect.angle<<endl;
//        cout<<"width"<<rect.size.width<<endl;
//        cout<<"higth"<<rect.size.height;
//        putText(src,std::to_string(rect.angle),Point(150,20),1,1,Scalar(255,255,255),2);

    }
    if (LED_SINGLE_CHOOSE.size() < 2)
    {
        cout<<"no two lights!"<<endl;
        return 0;
    }
    sort(LED_SINGLE_CHOOSE.begin(),LED_SINGLE_CHOOSE.end(),cmp);
    adaptthresh();

#ifdef DRAW_LIGHTS
    Mat img;
        src.copyTo(img);
        Point2f vertex[4];
        for (size_t j = 0; j < LED_SINGLE_CHOOSE.size(); j++)
        {
            LED_SINGLE_CHOOSE[j].points(vertex);
            for (size_t i = 0; i < 4; i++)
            {
                line(img, vertex[i], vertex[(i + 1) % 4], Scalar(18, 97, 255), 1, LINE_AA);
            }
            string ON=std::to_string(j);
         //   putText(img, ON, Point(LED_SINGLE_CHOOSE[j].center.x - 10, LED_SINGLE_CHOOSE[j].center.y - 50), cv::FONT_HERSHEY_PLAIN, 2, Scalar(0, 0, 255), 2, 1);

#ifdef OUTPUT_SINGLE_information
            printf("x,y:%f,%f , width,height:%f,%f , angle:%f\n", LED_SINGLE_CHOOSE[j].center.x, LED_SINGLE_CHOOSE[j].center.y, LED_SINGLE_CHOOSE[j].size.width, LED_SINGLE_CHOOSE[j].size.height, LED_SINGLE_CHOOSE[j].angle);
#endif
        }
        imshow("single", img);
#ifdef LARGE_SRC
        resize(img, img, Point(img.rows * 0.7, img.cols * 0.5));
#endif
#ifdef OUTPUT_SINGLE_information

        cout << "LED_SINGLE_CHOOS: "  << LED_SINGLE_CHOOSE.size() << endl;
        cout << "LED_SINGLE_CHOOS: "  << LED_SINGLE_CHOOSE.size() << endl;
#endif
#endif

    return 1;
}

detect::detect(Base &base, vector<Robot> &robots)
{

    color=base.enemy_team;
    this->src=base.src;

    if(singlelight()){
        if(form_armor())
        {
            number num(base,rect_choose,robots);
        }
        else{
#ifdef SHOW_ROI_AERA
            Mat trans=base.src;
            putText(trans, "no roi", Point(10,100), FONT_HERSHEY_PLAIN, 3, Scalar(0, 159, 255), 2, 5);
            imshow("roi_rect",trans);
#endif
            base.flag = 0;
        }
    }
    else
        base.flag = 0;
    cout<<"-------------------- BEGIN -------------------"<<endl;
#ifdef SHOW_ROBOTS_INFORMATIONS
    if (robots.size()>0) {
        Mat img1;
       // RNG rng(time(0));
        Point2f vertex[4];
        src.copyTo(img1);
       // cout<<"debug  :"<<robots.size()<<endl;
        for (size_t i = 0; i < robots.size(); i++)
        {
            robots[i].roborect.points(vertex);
            for (size_t i = 0; i < 4; i++)
            {
                line(img1, vertex[i], vertex[(i + 1) % 4], Scalar(18, 97, 255), 1, LINE_AA);
            }
            stringstream stream;
            stream << robots[i].robonum;
            string str;
            stream >> str;
            Point2f poi(robots[i].roborect.center.x-robots[i].roborect.size.width/2,robots[i].roborect.center.y-robots[i].roborect.size.height/2);
            putText(img1, str, poi, FONT_HERSHEY_PLAIN, 3, Scalar(0, 159, 255), 2, 5);
        }
        imshow("robots",img1);
    }
#endif

}
