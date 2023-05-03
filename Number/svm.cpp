#include"svm.h"

void Bam_SVM::get_number(int num)
{
   stringstream ss;
   ss<<"/media/zah/新加卷/学习资料/视觉/ROI/roi/roi/"<<num;
   string filepath=ss.str();
   ss.str("");
   int number=140;
   if(num==4)number=125;
   if(num==5)number=75;
   if(num==6)number=65;
   if(num==7)number=55;
   if(num==8)number=45;
   for(int i=1;i<=number;i++)
   {
   ss<<filepath<<"/"<<i<<".png";
   string files=ss.str();
   ss.str("");
   Mat sampleimg=imread(files);
   if(sampleimg.empty())continue;//cout<<sampleimg.type()<<endl;
   resize(sampleimg,sampleimg,Size(75,90));
   //vector<Mat>channels;
   //split(sampleimg, channels);
   cvtColor(sampleimg,sampleimg,COLOR_BGR2GRAY);//转灰度
   //threshold(sampleimg,sampleimg, 110, 255, THRESH_BINARY);
   //imshow("show",sampleimg);
   //waitKey(50);

   Mat sampleimgs=sampleimg.reshape(1,1);
   sampleimgs.convertTo(sampleimgs,5);
   trainingimg.push_back(sampleimgs);
   traininglabels.push_back(num);
   }
}

void Bam_SVM::get_xml()
{
    Bam_SVM bam_svm;
    for(int i=1;i<=8;i++){
    bam_svm.get_number(i);
    }
    Ptr<SVM>svm=SVM::create();
    //imshow("svm",bam_svm.trainingimg);
    //cout<<bam_svm.traininglabels[15]<<endl;
    svm->setType(SVM::C_SVC);
    svm->setKernel(SVM::LINEAR);
    svm->setTermCriteria(TermCriteria(TermCriteria::EPS,1000,1e-6));
    Mat trainlables(bam_svm.traininglabels);
    //cout<<trainlables.type()<<bam_svm.trainingimg.type()<<endl;
    Ptr<TrainData>trainingdata=TrainData::create(bam_svm.trainingimg,ROW_SAMPLE,trainlables);
    svm->trainAuto(trainingdata);

    svm->save("/home/zah/桌面/项目/Bam-svm/xml/svm_grey.xml");
}

void Bam_SVM::hog(int num)
{

    stringstream ss;
    ss<<"/media/zah/新加卷/学习资料/视觉/ROI/roi/roi/"<<num;
    string filepath=ss.str();
    ss.str("");
    int number=140;
    if(num==4)number=125;
    if(num==5)number=75;
    if(num==6)number=65;
    if(num==7)number=55;
    if(num==8)number=45;
    for(int i=1;i<=number;i++)
    {
    ss<<filepath<<"/"<<i<<".png";
    string files=ss.str();
    ss.str("");
    Mat sampleimg=imread(files);
    if(sampleimg.empty())continue;
    resize(sampleimg,sampleimg,Size(72,88));
    //vector<Mat>channels;
    //split(sampleimg, channels);
    //threshold(channels[1],sampleimg, 100, 255, THRESH_BINARY);
    //imshow("show",sampleimg);
   // waitKey(100);
   // cvtColor(sampleimg,sampleimg,COLOR_BGR2GRAY);//转灰度
    HOGDescriptor hog(Size(72,88),Size(8,8),Size(4,4),Size(4,4),9,1,-1);
   // sampleimg.convertTo(sampleimg,CV_8UC1);
    cvtColor(sampleimg,sampleimg,COLOR_BGRA2GRAY);
    //cout<<sampleimg.type()<<"  "<<endl;
    //imshow("mm",sampleimg);
    //waitKey(100);
    vector<float>cols;
    hog.compute(sampleimg,cols);
    Mat sampleimgs;
    sampleimgs.push_back(cols);
   sampleimgs=sampleimgs.reshape(1,1);
    trainingimg.push_back(sampleimgs);
    traininglabels.push_back(num);
    }
}


void Bam_SVM::get_hog()
{
    Bam_SVM bam_hog;
    for(int i=1;i<=7;i++){
    bam_hog.hog(i);
    }
    Ptr<SVM>hog=SVM::create();
    //imshow("svm",bam_svm.trainingimg);
    //cout<<bam_svm.traininglabels[15]<<endl;
    hog->setType(SVM::C_SVC);
    hog->setKernel(SVM::LINEAR);
    hog->setTermCriteria(TermCriteria(TermCriteria::EPS,1000,1e-6));
    Mat trainlables(bam_hog.traininglabels);
    //cout<<trainlables.type()<<bam_svm.trainingimg.type()<<endl;
    Ptr<TrainData>trainingdata=TrainData::create(bam_hog.trainingimg,ROW_SAMPLE,trainlables);
    hog->trainAuto(trainingdata);

    hog->save("/home/zah/桌面/项目/Bam-svm/xml/hog_grey.xml");


}

void Bam_SVM::output1()
{

    float number=20;
    Ptr<SVM>svm_hog=SVM::load("/home/zah/桌面/项目/Bam-svm/xml/hog1.xml");
    Ptr<SVM>svm=SVM::load("/home/zah/桌面/项目/Bam-svm/xml/svm1.xml");
    //svm = Algorithm::load<SVM>("/home/zah/桌面/项目/Bam-svm/xml/.xml");
    string filepath="/media/zah/新加卷/学习资料/视觉/ROI/roi/roi/";
    for(int i=1;i<=7;i++)
    {
        int n=140;
        if(i==2||i==1)number=20;
        if(i==3)number=7;
        if(i==4)n=125,number=14;
        if(i==5)n=75,number=16;
        if(i==6)n=65,number=20;
        if(i==7)n=55,number=20;
        float result=0;
        stringstream s;
        s.str("");
        s<<filepath<<i<<"/";
        string num=s.str();
        for(int j=0;j<number;j++){
            stringstream ss;
            ss<<num<<j+n<<".png";
            string imgfile=ss.str();
            ss.str("");
            //cout<<imgfile<<endl;
            Mat img=imread(imgfile);
            resize(img,img,Size(75,90));
            Mat p1,p2;

            //imshow("mm",img);
            //waitKey(50);
            p1=img.reshape(1,1);
            p1.convertTo(p1,5);
            int response1=(int)svm->predict(p1);
            resize(img,img,Size(72,88));
            HOGDescriptor hog(Size(72,88),Size(8,8),Size(4,4),Size(4,4),9,1,-1);
            img.convertTo(img,CV_8UC1);
            cvtColor(img,img,COLOR_BGRA2GRAY);
            vector<float>cols;
            hog.compute(img,cols);
            p2.push_back(cols);
            p2=p2.reshape(1,1);
            int response2=(int)svm_hog->predict(p2);
            if(response1==i&&response2==i)result++;
            //if(response1==response2)cout<<response1<<endl;
            //else cout<<"warning"<<endl;

        }
        float b=result/number;
        float c=b*100;
        cout<<i<<"样本数量"<<" "<<number<<"  "<<"识别成功数量"<<"  "<<result<<" "<<"准确率"<<c<<"%"<<endl;
    }

}

void Bam_SVM::output2()
{

    float number=75;
    Ptr<SVM>svm_hog=SVM::load("/home/zah/桌面/项目/Bam-svm/xml/hog1.xml");
    Ptr<SVM>svm=SVM::load("/home/zah/桌面/项目/Bam-svm/xml/svm1.xml");
    //svm = Algorithm::load<SVM>("/home/zah/桌面/项目/Bam-svm/xml/.xml");
    string filepath="/media/zah/新加卷/学习资料/视觉/ROI/roi/roi/8/";
        for(int j=45;j<=number;j++)
        {
//            double dur;
//            clock_t start,end;
//            start=clock();
//            double t_ =static_cast<double>(getTickCount());

            stringstream ss;
            ss.str("");
            ss<<filepath<<j<<".png";
            string imgfile=ss.str();
            ss.str("");
            //cout<<imgfile<<endl;
            Mat img=imread(imgfile);
            resize(img,img,Size(75,90));
            Mat p1,p2;

            //imshow("mm",img);
            //waitKey(50);
            p1=img.reshape(1,1);
            p1.convertTo(p1,5);
            //cout<<p1.cols<<endl;
            int response1=(int)svm->predict(p1);
            resize(img,img,Size(72,88));
            HOGDescriptor hog(Size(72,88),Size(8,8),Size(4,4),Size(4,4),9,1,-1);
            img.convertTo(img,CV_8UC1);
            cvtColor(img,img,COLOR_BGRA2GRAY);
            vector<float>cols;
            hog.compute(img,cols);
            p2.push_back(cols);
            p2=p2.reshape(1,1);
            int response2=(int)svm_hog->predict(p2);
            if(response1==response2)cout<<response1<<endl;
            else cout<<"warning"<<endl;

//            double t=((double)getTickCount()-t_) / getTickFrequency();
//            cout<<1.0/t<<endl;
//            end=clock();
//            dur=(double)(end-start);
//            cout<<"using time"<<1.0/dur*1000<<endl;
        }
}

vector<Robot> Bam_SVM::output0(vector<Robot> robot)
{
    //double t_ =static_cast<double>(getTickCount());

    vector<Robot> robots;
//    Ptr<SVM>svm_hog=SVM::load("/home/zah/桌面/项目/Bam-svm/xml/hog_grey.xml");
//    Ptr<SVM>svm=SVM::load("/home/zah/桌面/项目/Bam-svm/xml/svm_grey.xml");
    //svm = Algorithm::load<SVM>("/home/zah/桌面/项目/Bam-svm/xml/.xml");

       for(size_t i=0;i<robot.size();i++)
       {
            Mat p1,p2;
            Mat ROI =robot[i].ROI;
            resize(ROI,ROI,Size(75,90));
//            waitKey(0);
            p1=ROI.reshape(1,1);
            p1.convertTo(p1,CV_32F,1.0/255);
            //cout<<p1.cols<<endl;
            int response1=(int)svm->predict(p1);
            resize(ROI,ROI,Size(72,88));
            HOGDescriptor hog(Size(72,88),Size(8,8),Size(4,4),Size(4,4),9,1,-1);
//            ROI.convertTo(ROI,CV_8UC1);
            //cout<<ROI.type()<<endl;
//            cvtColor(ROI,ROI,COLOR_BGRA2GRAY);
            ROI.convertTo(ROI,CV_8U);
                        imshow("mm",ROI);


            vector<float>cols;
            hog.compute(ROI,cols);
            cout<<"cols_size"<<cols.size()<<endl;
            p2.push_back(cols);
            p2=p2.reshape(1,1);
            //cout<<p2.type()<<endl;
            int response2=(int)svm_hog->predict(p2);
            if(response1!=response2)continue;
            else {
                Robot R_trans=robot[i];
                R_trans.robonum=response1;
                robots.push_back(R_trans);
                cout<<response1<<endl;
            }
           // double t=((double)getTickCount()-t_) / getTickFrequency();
//            cout<<"using time"<<1.0/t<<endl;
           // cout<<"处理时间： "<<t<<endl;
            //return response1;

       }
       return robots;
}
