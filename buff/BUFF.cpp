#include "BUFF.h"

using namespace cv;
using namespace std;
buff::buff()
{

}

bool buff::find_R(cv::Mat src,bool color,Point2f & R_center,Ptr<KNearest> model_R){
    bool flag=0;
    vector<Mat> channels;
    split(src,channels);
    Mat gray;
    if(color==0)//红色时处理
    {
        Mat kernel =getStructuringElement(MORPH_RECT, Size(3,3));
        threshold(channels[2]*1.2-channels[0],gray,100,255,THRESH_BINARY);//根据最大的值来动态二值化
        morphologyEx(gray,gray,MORPH_OPEN,kernel,Point(-1,-1));//闭操作去除较小的黑点，使图像连贯
        imshow("t",gray);
        kernel =getStructuringElement(MORPH_RECT, Size(5,5));
        dilate(gray,gray,kernel);

    }
    else{//蓝色部分处理
        Mat kernel =getStructuringElement(MORPH_RECT, Size(3,3));
        threshold(channels[0]-channels[2],gray,100,255,THRESH_BINARY);//根据最大的值来动态二值化
        morphologyEx(gray,gray,MORPH_OPEN,kernel,Point(-1,-1));//闭操作去除较小的黑点，使图像连贯
        kernel =getStructuringElement(MORPH_RECT, Size(5,5));
        dilate(gray,gray,kernel);
    }



#ifdef SHOW_ALL
    imshow("R",gray);
#endif
    vector<vector<Point>> contours;
    vector<Vec4i> hiearachy_test;
    findContours(gray, contours, hiearachy_test, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    if(contours.size()>0){
        for(size_t i=0;i<contours.size();i++){
            cout<<"conconcocnocnocnocnon: "<<contourArea(contours[i])<<endl;
            if(contourArea(contours[i])>=100&&contourArea(contours[i])<=650){
                cout<<"conconco: "<<contourArea(contours[i])<<endl;
                Rect rect = boundingRect(contours[i]);
                Mat ROI=Mat(gray,rect);
                resize(ROI,ROI,Size(50,50));

                vector<vector<Point>> contours_;
                vector<Vec4i> hiearachy_test_;
                findContours(ROI, contours_, hiearachy_test_, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
//                double max=0;
//                for(size_t j=0;i<contours_.size();j++){
//                    if(contourArea(contours_[j])>max)
//                          max=contourArea(contours_[j]);
//                }

                if(contours_.size()<2){
#ifdef SHOW_ALL
                    imshow("ROI",ROI);
#endif
                    Mat ROI_img_test;
                    Mat tmp3;
                    Mat predict_mat;
                    ROI.convertTo(ROI_img_test, CV_32F);
                    ROI_img_test.copyTo(tmp3);
                    predict_mat=tmp3.reshape(0, 1);

                    Mat predict_simple = predict_mat;
                    float r = model_R->predict(predict_simple);

//                    cout<<"RRRRRRRRRRRRRRRRRRRRRRRRRRRRRR: "<<r<<endl;

                    if(r){
                        R_center=Point2f(rect.x+rect.width/2,rect.y+rect.height/2);
                        flag=1;
                    }
                    else {
                        flag=1;
                    }
                }

            }
        }
    }

    return flag;
}


void buff::findSquares(const cv::Mat& image,bool color,cv::Point2f circle_center)//红0蓝1,因为风车颜色和敌方装甲板相反
{
    buff_flag=0;
//    vector<Point> aim;//疑似装甲板和疑似风车中心点
//    aim.clear();

    Mat src,dst,process,_thresh;
    vector<Mat> channels;//分离后的通道

    src = image.clone();
    dst = image.clone();
    split(src,channels);

//    CircleFitting(track,R);
//    cout<<"RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR:  "<<R<<endl;
    circle(dst,circle_center,R,Scalar(255,2,123),2,8);


    if(color==0)//红色时处理
    {
        Mat kernel =getStructuringElement(MORPH_RECT, Size(5,5));
        threshold(channels[2]*2-channels[0],_thresh,100,255,THRESH_BINARY);//根据最大的值来动态二值化
        morphologyEx(_thresh,_thresh,MORPH_CLOSE,kernel,Point(-1,-1));//闭操作去除较小的黑点，使图像连贯
        dilate(_thresh, _thresh, Mat(3,3,CV_8UC1), Point(-1, -1),2);//膨胀使轮廓明显
        process=_thresh.clone();
        floodFill(_thresh,Point(0,0),255);
        floodFill(_thresh,Point(0,_thresh.cols/3),255);
        erode(_thresh, _thresh,Mat(10,10,CV_8UC1),Point(-1, -1),2);


    }
    else{//蓝色部分处理

        Mat kernel =getStructuringElement(MORPH_RECT, Size(5,5));
        threshold(channels[0]-channels[2],_thresh,100,255,THRESH_BINARY);//根据最大的值来动态二值化
        morphologyEx(_thresh,_thresh,MORPH_CLOSE,kernel,Point(-1,-1));//闭操作去除较小的黑点，使图像连贯
        dilate(_thresh, _thresh, Mat(3,3,CV_8UC1), Point(-1, -1),2);//膨胀使轮廓明显
        process=_thresh.clone();
        floodFill(_thresh,Point(0,0),255);
        floodFill(_thresh,Point(0,_thresh.cols/3),255);
        erode(_thresh, _thresh,Mat(10,10,CV_8UC1),Point(-1, -1),2);


    }
//    namedWindow("二值化结果",0);
//    resizeWindow("二值化结果",Size(640,480));
//    namedWindow("二值化结果",0);
//    resizeWindow("二值化结果",Size(640,480));
#ifdef SHOW_ALL
    imshow("二值化结果",_thresh);
#endif

    vector<vector<Point> > contours1,contours2;
    vector<Vec4i> hierarchy1,hierarchy2;
    findContours(_thresh, contours1, hierarchy1, RETR_TREE,CHAIN_APPROX_NONE);
    findContours(_thresh, contours1, hierarchy1, RETR_TREE,CHAIN_APPROX_NONE);

    for(size_t i=0;i<contours1.size();i++)
    {
        RotatedRect minRect;
        if(contours1[i].size()>=6)
            minRect=fitEllipse(contours1[i]);
        else continue;
        if(minRect.size.area()<6000&&minRect.size.area()>100)
        {
            printf("第%zu个面积大小%f\n",i,minRect.size.area());
            cout<<"angle="<<minRect.angle<<endl;
            drawMarker(dst,minRect.center,Scalar(255,255,255),MARKER_STAR,20,5);
            track.push_back(minRect.center);
            point_find=minRect.center;
            Last_point_find=point_find;
            buff_flag=1;
        }
    }

    drawMarker(dst,point_find,Scalar(255, 255, 255), cv::MARKER_STAR, 10, 2);//测量点

    if(R>0)
    point_find=a.kalmanRun(dst,point_find,circle_center,R,color);



    drawMarker(dst,point_find, Scalar(0, 0, 255), cv::MARKER_SQUARE, 20, 2);//红色框是预测点

#ifdef SHOW_ALL
    imshow("查找结果",dst);
#endif
//    buff_flag=1;

    //掉帧保护部分
    if(0){
        track.clear();
        cout<<"no find armor!"<<endl;
        point_find=Last_point_find;
        //cout<<"上一帧目标点"<<point_find<<endl;
        //waitKey(0);
//        return ;
        buff_flag=0;
    }


}






//画出图像
void buff::draw(Mat dst,vector<vector<Point>> &aim){
    for (size_t i = 0; i < aim.size(); i++)
    {
        polylines(dst, aim[i],true, Scalar(255,255,255), 3, LINE_4);
    }
}

void buff::draw(Mat dst,RotatedRect aim){
    Point2f vertex[4];//储存矩形四个点
    aim.points(vertex);
    for(size_t i = 0;i<4;i++){
        line(dst,vertex[i],vertex[(i+2)%4],Scalar(255,31,66),3,LINE_4);
        line(dst,vertex[i],vertex[(i+3)%4],Scalar(25,31,66),3,LINE_4);
    }

}



