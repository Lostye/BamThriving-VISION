#include "number.h"

bool number::get_ROI(RotatedRect rect){

    double tl_x=rect.center.x-rect.size.width*0.3;
    double tl_y=rect.center.y-rect.size.height*0.75;
    Rect ori_rect;
    if(tl_y>0&&tl_y+rect.size.height*1.5<src.rows){
        Rect O_rect(tl_x,tl_y,rect.size.width*0.6,rect.size.height*1.5);
        ori_rect=O_rect;
    }
    if(tl_y<=0){
        Rect O_rect(tl_x,0,rect.size.width*0.6,rect.size.height*1.5);
        ori_rect=O_rect;
    }
    if(tl_y+rect.size.height*1.5>=src.rows){
     //   Rect O_rect(tl_x,tl_y,rect.size.width*0.7,480-tl_y);
        double roi_y=src.rows-tl_y;
//        if(roi_y==0)
//            roi_y=0.1;
        Rect O_rect(tl_x,tl_y,rect.size.width*0.6,roi_y);
        ori_rect=O_rect;
    }
    ROI=Mat(src,ori_rect);
    if(ori_rect.size().width==0||ori_rect.size().height==0){
        return 0;
    }
//    resize(ROI,ROI,Size(50,50));
    cvtColor(ROI,ROI,COLOR_BGR2GRAY);
    minMaxLoc(ROI,NULL,&num_lowst_thresh,NULL,NULL);
    threshold(ROI,ROI,num_lowst_thresh/5,255,THRESH_BINARY);

////    cout<<"DEBUG_)_________LOWSTEST      TRES  "<<num_lowst_thresh<<endl;
//    Mat kernel =getStructuringElement(MORPH_RECT,Size(3,3));

//    morphologyEx(ROI,ROI,MORPH_CLOSE,kernel);
 //   erode(ROI,ROI,kernel);
#ifdef SAVE_ROI
    imwrite("roi.jpg",ROI);
#endif
#ifdef SHOW_ROI_AERA
    Mat roi=ROI;
    imshow("ROI",roi);
#endif
#ifdef DRAW_ROI_RECT

    Mat img;
    src.copyTo(img);
    putText(img, "have roi", Point(10,100), FONT_HERSHEY_PLAIN, 3, Scalar(0, 159, 255), 2, 5);
    rectangle(img,ori_rect,Scalar(0,97,255),2,LINE_AA);
    imshow("roi_rect",img);

#endif

    int nouse = find_ROI_contour();

    return 1;
}

bool number::find_ROI_contour(){

//    Mat kernel =getStructuringElement(MORPH_RECT,Size(3,3));

   // morphologyEx(ROI,ROI,MORPH_OPEN,kernel);
    vector<vector<Point>> contours;
    vector<Vec4i> hiearachy_test;
    resize(ROI,ROI,Size(120,120));
    findContours(ROI, contours, hiearachy_test, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    cout<<"cccccccccccccccccccccccc"<<ROI.size<<endl;
//    if(!contours.size()||contours.size()>5)
//    {
//        cout<<"no the number!"<<endl;
//        return 1;
//    }
    double max=0;
    for(size_t i=0;i<contours.size();i++){
        if(contourArea(contours[i])>max)
            max=contourArea(contours[i]);
//             rect = boundingRect(contours[i]);

    }
    cout<<"debugAAAAAAAAAAAAAAAAAAAAAArex:"<<max<<endl;
    if(max<7000){
        exposure+=2000;
        Switch=1;
    }
    else if (max>8800) {
        exposure-=2000;
        Switch=1;
    }

//    if(max<=250||max>=1500){
//     //   waitKey(120);
//        cout<<"number is wrong!"<<endl;
//        return 0;
//    }


   // ROI=Mat(ROI,rect);
  //  imshow("ROI",ROI);
    return 1;
}



number::number(Base &base, vector<RotatedRect> rect_choose, vector<Robot> &robots)
{
    RotatedRect rect;
    Robot robot;
    this->Switch=base.Switch;
    this->exposure = base.exposure;
    this->src=base.src;
    for(size_t i=0;i<rect_choose.size();i++){
        rect=rect_choose[i];
        if(get_ROI(rect_choose[i])){
            robot.roborect=rect;
            robot.ROI=this->ROI;
            robots.push_back(robot);
        }
        else{
             base.flag = 0;
        }
    }
    base.exposure=this->exposure;
    base.Switch=this->Switch;
}
