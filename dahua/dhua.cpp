#include "dhua.h"

//****** 本Demo 简单演示SDK 发现相机，连接相机，取图，断开相机的使用********//
//****** This demo simply demonstrates the use of SDK to discover cameras, connect cameras, get frame, and disconnect cameras ********//

/*这是用初始化列表初始化CSystem
DHUA::DHUA(Dahua::GenICam::CSystem&a):systemObj(a)
{

};
*/
DHUA::DHUA(){
    discovery_device();
    set_option();
    creat_stream();
};

bool DHUA::set_option(){
    //*手动设置参数
    setGrabMode(cameraSptr, true);    // 设置相机为连续取流模式
    setExposureMODE(cameraSptr,false);//设置为非自动曝光模式
    setCameraExposureTime(cameraSptr,CameraExposureTime,false);//设置曝光时间
    setBalanceRatio(cameraSptr,dRED,dGREEN,dBLUE);//设置白平衡值
    setGainRawValue(cameraSptr,GainRawValue,false);//设置总体增益值（不改曝光的前提下提高可见度）
    SetGamma(cameraSptr,Gamma,false);//设置gamma值
    int64_t nwidth,nheight;
    getResolution(cameraSptr,nwidth,nheight);//shark：获取分辨率
    printf("Resolution:%ld*%ld\n",nwidth,nheight);
    int64_t nX,nY;
    getROI(cameraSptr,nX,nY,nwidth,nheight);
    printf("befor set ROI is :(%ld,%ld),W:(%ld),H(%ld)\n",nX,nY,nwidth,nheight);
//    setROI(cameraSptr ,640, 300, 640, 480);//shark：xy确定矩形左上角坐标（建议值为640，300，640，480这样刚好框在中心）；  1

//    setROI(cameraSptr ,320, 272, 640, 480);          //2

    setROI(cameraSptr ,96, 152, 1088, 720);          //3


    printf("after set ROI is :(%ld,%ld),W:(%ld),H(%ld)\n",nX,nY,nwidth,nheight);
    double dFrameRate;
    getAcquisitionFrameRate(cameraSptr,dFrameRate);//shark:获取帧率
    setAcquisitionFrameRate(cameraSptr,500);//shark：设定帧率为500
}



bool DHUA::creat_stream(){
    // 创建流对象
    // create acquisitioncontrol object
    streamPtr = systemObj.createStreamSource(cameraSptr);
    if (NULL == streamPtr)
    {
        printf("create stream obj  fail.\r\n");
        return 0;
    }

    // 开始取图
    // start grabbing
    bool isStartGrabbingSuccess = streamPtr->startGrabbing();//shark：startGrabbing的参数，第一个是最大抓图数，为0则为连续抓图；第二个默认值为按顺序在缓存里抓图.抓图是指将数据从缓存抓取到内存
    if (!isStartGrabbingSuccess)
    {
        printf("StartGrabbing  fail.\n");
    }

    // 建取流线程
    // create get frame thread
    Dahua::Memory::TSharedPtr<StreamRetrieve> streamThreadSptr(new StreamRetrieve (streamPtr));
    DHUA::streamThreadSptr=streamThreadSptr;//shark：StreamRetrieve流检索(这段不是太懂，只知道大概是创建线程，那图片是取到哪里了？)
    if (NULL == streamThreadSptr)
    {
        printf("create thread obj failed.\n");
        return 0;
    }
    // 线程开始取图
    // start get frame thread
    streamThreadSptr->start();

    // 取图2秒
    // get frame 2 seconds
    Dahua::Infra::CThread::sleep(100);//shark：这里的sleep可千万不能删除，删除了线程崩溃
    camera_src=streamThreadSptr->getMatImage();

}


bool DHUA::discovery_device()
{
#if 0
    PrintOptions printOptions = {0};
    printOptions.color = 1;
    setPrintOptions(printOptions);

    LogPrinterProc procFun = LogPrinterFunc;
    setLogPrinter(procFun);
#endif
    // ①发现设备
    // discovery device
    bool isDiscoverySuccess = systemObj.discovery(vCameraPtrList);//shark：将discovery发现的设备存到VCameraPtrList这个列表中。并用isDiscoverySuccess判断是否找到设备
    if (!isDiscoverySuccess)
    {
        printf("discovery device fail.\n");
        return 0;
    }

    if (vCameraPtrList.size() == 0)
    {
        printf("no devices.\n");
        return 0;
    }
    // ②打印相机基本信息（序号,类型,制造商信息,型号,序列号,用户自定义ID,IP地址）
    // print camera info (index,Type,vendor name, model,serial number,DeviceUserID,IP Address)
    displayDeviceInfo(vCameraPtrList);//shark:显示相机列表内相应的数据
    //int cameraIndex = selectDevice(vCameraPtrList.size());//shark：selectDevice是让用户选择相机引索（ID），然后将其值返回到cameraIndex中
    cameraSptr = vCameraPtrList[0];//shark:把列表中客户输入ID对应的相机数据赋给cameraSptr


    // ③GigE相机时，连接前设置相机Ip与网卡处于同一网段上
    // When a GigE camera is connected, set the camera IP to be on the same network segment as the network card
    if( Dahua::GenICam::ICamera::typeGige == cameraSptr->getType())
    {
        if(autoSetCameraIP(cameraSptr) != 0)
        {
            printf("set camera Ip failed.\n");
            return 0;
        }
    }

    //  ④连接相机
    // connect camera
    if (!cameraSptr->connect())
    {
        printf("connect cameral failed.\n");
        return 0;
    }

}



DHUA::~DHUA(){
    //停止拉流线程
    // Stop streaming thread
    streamThreadSptr->stop();


    // 停止相机拉流
    // stop camera grabbing
    streamPtr->stopGrabbing();

    // 断开设备
    // disconnect device
    if (!cameraSptr->disConnect())
    {
        printf("disConnect camera failed\n");
    }
    printf("disConnect successfully thread ID :%d\n", Dahua::Infra::CThread::getCurrentThreadID());

}










/*shark:**************************************下面不用看，主要功能都在上面************************************************************/


// 设置相机采图模式（连续采图、触发采图）
// Set camera acquisition mode (continuous acquisition, triggered acquisition)
int32_t DHUA::setGrabMode(Dahua::GenICam::ICameraPtr cameraSptr, bool bContious)
{
    int32_t bRet;
    Dahua::GenICam::IAcquisitionControlPtr sptrAcquisitionControl = Dahua::GenICam::CSystem::getInstance().createAcquisitionControl(cameraSptr);//shark:Dahua::GenICam::IAcquisitionControlPtr（In Acquisition control pointer输入采集控制指针）
    if (NULL == sptrAcquisitionControl)
    {
        return -1;
    }

    Dahua::GenICam::CEnumNode enumNode = sptrAcquisitionControl->triggerSelector();//shark：triggerSelector触发器选择器
    bRet = enumNode.setValueBySymbol("FrameStart");//设置为FrameStart帧开始类型（连续采图）
    if (false == bRet)//shark:设置失败
    {
        printf("set TriggerSelector fail.\n");
        return -1;
    }

    if (true == bContious)//shark:设置为连续时执行
    {
        enumNode = sptrAcquisitionControl->triggerMode();//shark:获取触发器模式
        bRet = enumNode.setValueBySymbol("Off");//
        if (false == bRet)
        {
            printf("set triggerMode fail.\n");
            return -1;
        }
    }
    else
    {
        enumNode = sptrAcquisitionControl->triggerMode();
        bRet = enumNode.setValueBySymbol("On");
        if (false == bRet)
        {
            printf("set triggerMode fail.\n");
            return -1;
        }

        // 设置触发源为软触发（硬触发为Line1）
        // Set trigger source as soft trigger (hard trigger as Line1)
        enumNode = sptrAcquisitionControl->triggerSource();
        bRet = enumNode.setValueBySymbol("Software");
        if (false == bRet)
        {
            printf("set triggerSource fail.\n");
            return -1;
        }
    }
    return 0;
}

// 获取相机采图模式
// Get camera acquisition mode
int32_t getGrabMode(Dahua::GenICam:: ICameraPtr& cameraSptr, bool &bContious)
{
    int32_t bRet;
    Dahua::GenICam::IAcquisitionControlPtr sptrAcquisitionControl = Dahua::GenICam::CSystem::getInstance().createAcquisitionControl(cameraSptr);
    if (NULL == sptrAcquisitionControl)
    {
        return -1;
    }

    Dahua::GenICam::CEnumNode enumNode = sptrAcquisitionControl->triggerSelector();
    bRet = enumNode.setValueBySymbol("FrameStart");
    if (false == bRet)
    {
        printf("set TriggerSelector fail.\n");
        return -1;
    }

    Dahua::Infra:: CString strValue;
    enumNode = sptrAcquisitionControl->triggerMode();
    bRet = enumNode.getValueSymbol(strValue);
    if (false == bRet)
    {
        printf("get triggerMode fail.\n");
        return -1;
    }

    if (strValue == "Off")
    {
        bContious = true;
    }
    else if (strValue == "On")
    {
        bContious = false;
    }
    else
    {
        printf("get triggerMode fail.\n");
        return -1;
    }
    return 0;
}

// 软件触发
// software trigger
int32_t triggerSoftware(Dahua::GenICam:: ICameraPtr& cameraSptr)
{
    int32_t bRet;
    Dahua::GenICam::IAcquisitionControlPtr sptrAcquisitionControl = Dahua::GenICam::CSystem::getInstance().createAcquisitionControl(cameraSptr);
    if (NULL == sptrAcquisitionControl)
    {
        printf("AcquisitionControl fail.\n");
        return -1;
    }

    Dahua::GenICam:: CCmdNode  cmdNode = sptrAcquisitionControl->triggerSoftware();
    bRet = cmdNode.execute();
    if (false == bRet)
    {
        printf("triggerSoftware execute fail.\n");
        return -1;
    }
    return 0;
}

// 设置传感器采样率（采集分辨率）
// Set sensor sampling rate (acquisition resolution)
int32_t setResolution(Dahua::GenICam:: ICameraPtr& cameraSptr, int nWidth, int nHeight)
{
    int32_t bRet;
    Dahua::GenICam::IImageFormatControlPtr sptrImageFormatControl = Dahua::GenICam::CSystem::getInstance().createImageFormatControl(cameraSptr);
    if (NULL == sptrImageFormatControl)
    {
        return -1;
    }

    Dahua::GenICam::CIntNode intNode = sptrImageFormatControl->width();
    bRet = intNode.setValue(nWidth);
    if (false == bRet)
    {
        printf("set width fail.\n");
        return -1;
    }

    intNode = sptrImageFormatControl->height();
    bRet = intNode.setValue(nHeight);
    if (false == bRet)
    {
        printf("set height fail.\n");
        return -1;
    }
    return 0;
}

// 获取传感器采样率
// Get sensor sample rate
int32_t DHUA::getResolution(Dahua::GenICam:: ICameraPtr& cameraSptr, int64_t &nWidth, int64_t &nHeight)
{
    int32_t bRet;
    Dahua::GenICam:: IImageFormatControlPtr sptrImageFormatControl = Dahua::GenICam::CSystem::getInstance().createImageFormatControl(cameraSptr);
    if (NULL == sptrImageFormatControl)
    {
        return -1;
    }

    Dahua::GenICam::CIntNode intNode = sptrImageFormatControl->width();
    bRet = intNode.getValue(nWidth);
    if (false == bRet)
    {
        printf("get width fail.\n");
        return -1;
    }

    intNode = sptrImageFormatControl->height();
    bRet = intNode.getValue(nHeight);
    if (false == bRet)
    {
        printf("get height fail.\n");
        return -1;
    }
    return 0;
}

// 设置binning (Off X Y XY)
// set binning (Off X Y XY)
int32_t setBinning(Dahua::GenICam:: ICameraPtr& cameraSptr)
{
    Dahua::GenICam::CEnumNodePtr ptrParam(new Dahua::GenICam::CEnumNode(cameraSptr, "Binning"));
    if (ptrParam)
    {
        if (false == ptrParam->isReadable())
        {
            printf("binning not support.\n");
            return -1;
        }

        if (false == ptrParam->setValueBySymbol("XY"))
        {
            printf("set Binning XY fail.\n");
            return -1;
        }

        if (false == ptrParam->setValueBySymbol("Off"))
        {
            printf("set Binning Off fail.\n");
            return -1;
        }
    }
    return 0;
}

// 获取传感器最大分辩率
// get the maximum resolution of the sensor
int32_t getMaxResolution(Dahua::GenICam:: ICameraPtr& cameraSptr, int64_t &nWidthMax, int64_t &nHeightMax)
{
    Dahua::GenICam::CIntNodePtr ptrParamSensorWidth(new Dahua::GenICam::CIntNode(cameraSptr, "SensorWidth"));
    if (ptrParamSensorWidth)
    {
        if (false == ptrParamSensorWidth->getValue(nWidthMax))
        {
            printf("get WidthMax fail.\n");
            return -1;
        }
    }

    Dahua::GenICam::CIntNodePtr ptrParamSensorHeight(new Dahua::GenICam::CIntNode(cameraSptr, "SensorHeight"));
    if (ptrParamSensorHeight)
    {
        if (false == ptrParamSensorHeight->getValue(nWidthMax))
        {
            printf("get WidthMax fail.\n");
            return -1;
        }
    }

    return 0;
}

// 设置图像ROI
// set image ROI
int32_t DHUA::setROI(Dahua::GenICam:: ICameraPtr& cameraSptr, int64_t nX, int64_t nY, int64_t nWidth, int64_t nHeight)
{
    bool bRet;
    Dahua::GenICam:: IImageFormatControlPtr sptrImageFormatControl = Dahua::GenICam::CSystem::getInstance().createImageFormatControl(cameraSptr);
    if (NULL == sptrImageFormatControl)
    {
        return -1;
    }

    // 设置宽
    // set width
    Dahua::GenICam::CIntNode intNode = sptrImageFormatControl->width();
    bRet = intNode.setValue(nWidth);
    if (!bRet)
    {
        printf("set width fail.\n");
        return -1;
    }

    // 设置长
    // set height
    intNode = sptrImageFormatControl->height();
    bRet = intNode.setValue(nHeight);
    if (!bRet)
    {
        printf("set height fail.\n");
        return -1;
    }

    // 设置X偏移
    // set OffsetX
    intNode = sptrImageFormatControl->offsetX();
    bRet = intNode.setValue(nX);
    if (!bRet)
    {
        printf("set offsetX fail.\n");
        return -1;
    }

    // 设置Y偏移
    // set OffsetY
    intNode = sptrImageFormatControl->offsetY();
    bRet = intNode.setValue(nY);
    if (!bRet)
    {
        printf("set offsetY fail.\n");
        return -1;
    }

    return 0;
}

// 获取图像ROI
// get image ROI
int32_t DHUA::getROI(Dahua::GenICam:: ICameraPtr& cameraSptr, int64_t &nX, int64_t &nY, int64_t &nWidth, int64_t &nHeight)
{
    bool bRet;
    Dahua::GenICam:: IImageFormatControlPtr sptrImageFormatControl = Dahua::GenICam::CSystem::getInstance().createImageFormatControl(cameraSptr);
    if (NULL == sptrImageFormatControl)
    {
        return -1;
    }

    // 设置宽
    // set width
    Dahua::GenICam::CIntNode intNode = sptrImageFormatControl->width();
    bRet = intNode.getValue(nWidth);
    if (!bRet)
    {
        printf("get width fail.\n");
    }

    // 设置长
    // set height
    intNode = sptrImageFormatControl->height();
    bRet = intNode.getValue(nHeight);
    if (!bRet)
    {
        printf("get height fail.\n");
    }

    // 设置X偏移
    // set OffsetX
    intNode = sptrImageFormatControl->offsetX();
    bRet = intNode.getValue(nX);
    if (!bRet)
    {
        printf("get offsetX fail.\n");
    }

    // 设置Y偏移
    // set OffsetY
    intNode = sptrImageFormatControl->offsetY();
    bRet = intNode.getValue(nY);
    if (!bRet)
    {
        printf("get offsetY fail.\n");
    }
    return 0;
}

// 获取采图图像宽度
// Get the width of the image
int32_t getWidth(Dahua::GenICam:: ICameraPtr& cameraSptr, int64_t &nWidth)
{
    bool bRet;
    Dahua::GenICam:: IImageFormatControlPtr sptrImageFormatControl = Dahua::GenICam::CSystem::getInstance().createImageFormatControl(cameraSptr);
    if (NULL == sptrImageFormatControl)
    {
        return -1;
    }

    Dahua::GenICam::CIntNode intNode = sptrImageFormatControl->width();
    bRet = intNode.getValue(nWidth);
    if (!bRet)
    {
        printf("get width fail.\n");
    }
    return 0;
}

// 获取采图图像高度
// Get the height of the image
int32_t DHUA::getHeight(Dahua::GenICam:: ICameraPtr& cameraSptr, int64_t &nHeight)
{
    bool bRet;
    Dahua::GenICam:: IImageFormatControlPtr sptrImageFormatControl = Dahua::GenICam::CSystem::getInstance().createImageFormatControl(cameraSptr);
    if (NULL == sptrImageFormatControl)
    {
        return -1;
    }

    Dahua::GenICam::CIntNode intNode = sptrImageFormatControl->height();
    bRet = intNode.getValue(nHeight);
    if (!bRet)
    {
        printf("get height fail.\n");
        return -1;
    }
    return 0;
}

// 设置曝光模式(曝光、自动曝光/手动曝光)shark:这只有连续曝光和手动曝光，没有一次曝光
// Set exposure value (exposure, auto exposure / manual exposure)
int32_t DHUA::setExposureMODE(Dahua::GenICam:: ICameraPtr& cameraSptr, bool bAutoExposure = false)
{
    bool bRet;
    Dahua::GenICam::IAcquisitionControlPtr sptrAcquisitionControl = Dahua::GenICam::CSystem::getInstance().createAcquisitionControl(cameraSptr);
    if (NULL == sptrAcquisitionControl)
    {
        return -1;
    }

    if (bAutoExposure)
    {
        Dahua::GenICam::CEnumNode enumNode = sptrAcquisitionControl->exposureAuto();//shark：获取曝光属性
        bRet = enumNode.setValueBySymbol("Continuous");
        if (false == bRet)
        {
            printf("set exposureAuto fail.\n");
            return -1;
        }
    }
    else
    {
        Dahua::GenICam::CEnumNode enumNode = sptrAcquisitionControl->exposureAuto();
        bRet = enumNode.setValueBySymbol("Off");
        if (false == bRet)
        {
            printf("set exposureAuto fail.\n");
            return -1;
        }
    }
    return 0;
}
//shark:自己写的设置曝光值(addFlag：true添加型)
void DHUA::setCameraExposureTime(Dahua::GenICam:: ICameraPtr &cameraSptr, double exposureTimeSet, bool addFlag) {
    Dahua::GenICam::IAcquisitionControlPtr sptrAcquisitionControl =Dahua::GenICam::CSystem::getInstance().createAcquisitionControl(cameraSptr);
    if (nullptr == sptrAcquisitionControl) {
        return;
    }
    double exposureTimeValue = 0.0;
    Dahua::GenICam::CDoubleNode exposureTime = sptrAcquisitionControl->exposureTime();
    if(exposureTime.getValue(exposureTimeValue)){
        if (addFlag) {
            exposureTimeValue += exposureTimeSet;
            if (exposureTimeValue < 1.0)
                exposureTimeValue = 1.0;
            else if (exposureTimeValue > 10000.0)
                exposureTimeValue = 10000.0;
            exposureTime.setValue(exposureTimeValue);
        } else {
            if (exposureTimeSet < 1.0)
                exposureTimeSet = 1.0;
            else if (exposureTimeSet > 100000.0)
                exposureTimeSet = 100000.0;
            exposureTime.setValue(exposureTimeSet);
        }
        exposureTime.getValue(exposureTimeValue);
        printf("after change ,exposureTime is %lf\n",(exposureTimeValue));
    }
    else if(!exposureTime.getValue(exposureTimeValue)){
        printf("get exposureTime fail.\n");
    }
}

/*（官方）
            // 获取曝光时间
            // get exposureTime
              int32_t getExposureTime(Dahua::GenICam:: ICameraPtr& cameraSptr, double &dExposureTime)
            {
                bool bRet;
                Dahua::GenICam::IAcquisitionControlPtr sptrAcquisitionControl = Dahua::GenICam::CSystem::getInstance().createAcquisitionControl(cameraSptr);
                if (NULL == sptrAcquisitionControl)
                {
                    return -1;
                }

                Dahua::GenICam::CDoubleNode doubleNode = sptrAcquisitionControl->exposureTime();
                bRet = doubleNode.getValue(dExposureTime);
                if (false == bRet)
                {
                    printf("get exposureTime fail.\n");
                    return -1;
                }
                return 0;
            }

            // 获取曝光范围
            // Get exposure range
              int32_t getExposureTimeMinMaxValue(Dahua::GenICam:: ICameraPtr& cameraSptr, double &dMinValue, double &dMaxValue)
            {
                bool bRet;
                Dahua::GenICam::IAcquisitionControlPtr sptrAcquisitionControl = Dahua::GenICam::CSystem::getInstance().createAcquisitionControl(cameraSptr);
                if (NULL == sptrAcquisitionControl)
                {
                    return -1;
                }

                Dahua::GenICam::CDoubleNode doubleNode = sptrAcquisitionControl->exposureTime();
                bRet = doubleNode.getMinVal(dMinValue);
                if (false == bRet)
                {
                    printf("get exposureTime minValue fail.\n");
                    return -1;
                }

                bRet = doubleNode.getMaxVal(dMaxValue);
                if (false == bRet)
                {
                    printf("get exposureTime maxValue fail.\n");
                    return -1;
                }
                return 0;
            }
            */
/*（官方）
            // 设置增益值
            // set gain
              int32_t setGainRaw(Dahua::GenICam:: ICameraPtr& cameraSptr, double dGainRaw)
            {
                bool bRet;
                Dahua::GenICam::IAnalogControlPtr sptrAnalogControl = Dahua::GenICam::CSystem::getInstance().createAnalogControl(cameraSptr);
                if (NULL == sptrAnalogControl)
                {
                    return -1;
                }

                Dahua::GenICam::CDoubleNode doubleNode = sptrAnalogControl->gainRaw();
                bRet = doubleNode.setValue(dGainRaw);
                if (false == bRet)
                {
                    printf("set gainRaw fail.\n");
                    return -1;
                }
                return 0;
            }

            // 获取增益值
            // get gain value
              int32_t getGainRaw(Dahua::GenICam:: ICameraPtr& cameraSptr, double &dGainRaw)
            {
                bool bRet;
                Dahua::GenICam::IAnalogControlPtr sptrAnalogControl = Dahua::GenICam::CSystem::getInstance().createAnalogControl(cameraSptr);
                if (NULL == sptrAnalogControl)
                {
                    return -1;
                }

                Dahua::GenICam::CDoubleNode doubleNode = sptrAnalogControl->gainRaw();
                bRet = doubleNode.getValue(dGainRaw);
                if (false == bRet)
                {
                    printf("get gainRaw fail.\n");
                    return -1;
                }
                return 0;
            }

            // 获取增益值范围
            // Get gain range
              int32_t getGainRawMinMaxValue(Dahua::GenICam:: ICameraPtr& cameraSptr, double &dMinValue, double &dMaxValue)
            {
                bool bRet;
                Dahua::GenICam::IAnalogControlPtr sptrAnalogControl = Dahua::GenICam::CSystem::getInstance().createAnalogControl(cameraSptr);
                if (NULL == sptrAnalogControl)
                {
                    return -1;
                }

                Dahua::GenICam::CDoubleNode doubleNode = sptrAnalogControl->gainRaw();
                bRet = doubleNode.getMinVal(dMinValue);//shark:将获取的最小值存到dMinValue中
                if (false == bRet)
                {
                    printf("get gainRaw minValue fail.\n");
                    return -1;
                }

                bRet = doubleNode.getMaxVal(dMaxValue);
                if (false == bRet)
                {
                    printf("get gainRaw maxValue fail.\n");
                    return -1;
                }
                return 0;
            }
            */

//shark：自己写的设置相机增益
void DHUA::setGainRawValue(Dahua::GenICam:: ICameraPtr &cameraSptr,double gainRawSet ,bool addFlag)
{Dahua::GenICam::IAnalogControlPtr sptrAnalogControl = Dahua::GenICam::CSystem::getInstance().createAnalogControl(cameraSptr);
    if (NULL == sptrAnalogControl) {
        return ;
    }
    double gainRawValue = 0.0;//shark：原属性值
    double dMaxValue,dMinValue;
    Dahua::GenICam::CDoubleNode gainRaw = sptrAnalogControl->gainRaw();
    gainRaw.getValue(gainRawValue);
    gainRaw.getMaxVal(dMaxValue);
    gainRaw.getMinVal(dMinValue);
    if (addFlag) {//shark：如果是添加模式就将原值加设值
        gainRawValue += gainRawSet;
        if (gainRawValue < dMinValue)//shark：保证最小值和最大值不超过限度
            gainRawValue = dMinValue;
        else if (gainRawValue > dMaxValue)
            gainRawValue = dMaxValue;
        gainRaw.setValue(gainRawValue);
    } else {//如果是设定模式就直接设置
        if (gainRawSet < dMinValue)
            gainRawSet = dMinValue;
        else if (gainRawSet > dMaxValue)
            gainRawSet =dMaxValue;
        gainRaw.setValue(gainRawSet);
    }
    gainRaw.getValue(gainRawValue);
    printf("after change ,gainraw is:%lf\n",gainRawValue);
}

/*(官方)
            // 设置伽马值
            // Set gamma
              int32_t setGamma(Dahua::GenICam:: ICameraPtr& cameraSptr, double dGamma)
            {
                bool bRet;
                Dahua::GenICam::IAnalogControlPtr sptrAnalogControl = Dahua::GenICam::CSystem::getInstance().createAnalogControl(cameraSptr);
                if (NULL == sptrAnalogControl)
                {
                    return -1;
                }

                Dahua::GenICam::CDoubleNode doubleNode = sptrAnalogControl->gamma();
                bRet = doubleNode.setValue(dGamma);
                if (false == bRet)
                {
                    printf("set gamma fail.\n");
                    return -1;
                }
                return 0;
            }

            // 获取伽马值
            // Get gamma
              int32_t getGamma(Dahua::GenICam:: ICameraPtr& cameraSptr, double &dGamma)
            {
                bool bRet;
                Dahua::GenICam::IAnalogControlPtr sptrAnalogControl = Dahua::GenICam::CSystem::getInstance().createAnalogControl(cameraSptr);
                if (NULL == sptrAnalogControl)
                {
                    return -1;
                }

                Dahua::GenICam::CDoubleNode doubleNode = sptrAnalogControl->gamma();
                bRet = doubleNode.getValue(dGamma);
                if (false == bRet)
                {
                    printf("get gamma fail.\n");
                    return -1;
                }
                return 0;
            }

            // 获取伽马值范围
            // Get gamma range
              int32_t getGammaMinMaxValue(Dahua::GenICam:: ICameraPtr& cameraSptr, double &dMinValue, double &dMaxValue)
            {
                bool bRet;
                Dahua::GenICam::IAnalogControlPtr sptrAnalogControl = Dahua::GenICam::CSystem::getInstance().createAnalogControl(cameraSptr);
                if (NULL == sptrAnalogControl)
                {
                    return -1;
                }

                Dahua::GenICam::CDoubleNode doubleNode = sptrAnalogControl->gamma();
                bRet = doubleNode.getMinVal(dMinValue);
                if (false == bRet)
                {
                    printf("get gamma minValue fail.\n");
                    return -1;
                }

                bRet = doubleNode.getMaxVal(dMaxValue);
                if (false == bRet)
                {
                    printf("get gamma maxValue fail.\n");
                    return -1;
                }
                return 0;
            }
            */

//shark：自己写的gamma设置
void DHUA::SetGamma(Dahua::GenICam:: ICameraPtr &cameraSptr, double gammaSet, bool addFlag) {
    Dahua::GenICam::IAnalogControlPtr sptrAnalogControl =Dahua::GenICam::CSystem::getInstance().createAnalogControl(cameraSptr);
    if (nullptr == sptrAnalogControl) {
        return;
    }
    double gammaValue = 0.0;
    Dahua::GenICam::CDoubleNode gamma = sptrAnalogControl->gamma();
    gamma.getValue(gammaValue);
    if (addFlag) {
        gammaValue += gammaSet;
        if (gammaValue < 0.1)
            gammaValue = 0.1;
        else if (gammaValue > 4.0)
            gammaValue = 4.0;
        gamma.setValue(gammaValue);
    } else {
        if (gammaSet < 0.1)
            gammaSet = 0.1;
        else if (gammaSet > 4.0)
            gammaSet = 4.0;
        gamma.setValue(gammaSet);
    }
    gamma.getValue(gammaValue);
    printf("after change ,gamma is %lf\n",gammaValue);
}



// 设置白平衡值（有三个白平衡值）
// Set the white balance value ( three white balance values)
int32_t DHUA::setBalanceRatio(Dahua::GenICam:: ICameraPtr& cameraSptr, double dRedBalanceRatio, double dGreenBalanceRatio, double dBlueBalanceRatio)
{
    bool bRet;
    Dahua::GenICam::IAnalogControlPtr sptrAnalogControl = Dahua::GenICam::CSystem::getInstance().createAnalogControl(cameraSptr);
    if (NULL == sptrAnalogControl)
    {
        return -1;
    }

    // 关闭自动白平衡
    // Turn off auto white balance
    Dahua::GenICam::CEnumNode enumNode = sptrAnalogControl->balanceWhiteAuto();
    if (false == enumNode.isReadable())
    {
        printf("balanceRatio not support.\n");
        return -1;
    }

    bRet = enumNode.setValueBySymbol("Off");
    if (false == bRet)
    {
        printf("set balanceWhiteAuto Off fail.\n");
        return -1;
    }

    enumNode = sptrAnalogControl->balanceRatioSelector();
    bRet = enumNode.setValueBySymbol("Red");
    if (false == bRet)
    {
        printf("set red balanceRatioSelector fail.\n");
        return -1;
    }

    Dahua::GenICam::CDoubleNode doubleNode = sptrAnalogControl->balanceRatio();
    bRet = doubleNode.setValue(dRedBalanceRatio);
    if (false == bRet)
    {
        printf("set red balanceRatio fail.\n");
        return -1;
    }

    enumNode = sptrAnalogControl->balanceRatioSelector();
    bRet = enumNode.setValueBySymbol("Green");
    if (false == bRet)
    {
        printf("set green balanceRatioSelector fail.\n");
        return -1;
    }

    doubleNode = sptrAnalogControl->balanceRatio();
    bRet = doubleNode.setValue(dGreenBalanceRatio);
    if (false == bRet)
    {
        printf("set green balanceRatio fail.\n");
        return -1;
    }

    enumNode = sptrAnalogControl->balanceRatioSelector();
    bRet = enumNode.setValueBySymbol("Blue");
    if (false == bRet)
    {
        printf("set blue balanceRatioSelector fail.\n");
        return -1;
    }

    doubleNode = sptrAnalogControl->balanceRatio();
    bRet = doubleNode.setValue(dBlueBalanceRatio);
    if (false == bRet)
    {
        printf("set blue balanceRatio fail.\n");
        return -1;
    }
    return 0;
}

// 获取白平衡值（有三个白平衡值)
// Get white balance value (three white balance values)
int32_t DHUA::getBalanceRatio(Dahua::GenICam:: ICameraPtr& cameraSptr, double &dRedBalanceRatio, double &dGreenBalanceRatio, double &dBlueBalanceRatio)
{
    bool bRet;
    Dahua::GenICam::IAnalogControlPtr sptrAnalogControl = Dahua::GenICam::CSystem::getInstance().createAnalogControl(cameraSptr);
    if (NULL == sptrAnalogControl)
    {
        return -1;
    }

    Dahua::GenICam::CEnumNode enumNode = sptrAnalogControl->balanceRatioSelector();
    if (false == enumNode.isReadable())
    {
        printf("balanceRatio not support.\n");
        return -1;
    }

    bRet = enumNode.setValueBySymbol("Red");
    if (false == bRet)
    {
        printf("set red balanceRatioSelector fail.\n");
        return -1;
    }

    Dahua::GenICam::CDoubleNode doubleNode = sptrAnalogControl->balanceRatio();
    bRet = doubleNode.getValue(dRedBalanceRatio);
    if (false == bRet)
    {
        printf("get red balanceRatio fail.\n");
        return -1;
    }

    enumNode = sptrAnalogControl->balanceRatioSelector();
    bRet = enumNode.setValueBySymbol("Green");
    if (false == bRet)
    {
        printf("set green balanceRatioSelector fail.\n");
        return -1;
    }

    doubleNode = sptrAnalogControl->balanceRatio();
    bRet = doubleNode.getValue(dGreenBalanceRatio);
    if (false == bRet)
    {
        printf("get green balanceRatio fail.\n");
        return -1;
    }

    enumNode = sptrAnalogControl->balanceRatioSelector();
    bRet = enumNode.setValueBySymbol("Blue");
    if (false == bRet)
    {
        printf("set blue balanceRatioSelector fail.\n");
        return -1;
    }

    doubleNode = sptrAnalogControl->balanceRatio();
    bRet = doubleNode.getValue(dBlueBalanceRatio);
    if (false == bRet)
    {
        printf("get blue balanceRatio fail.\n");
        return -1;
    }
    return 0;
}

// 获取白平衡值范围
// Get white balance value range
int32_t DHUA::getBalanceRatioMinMaxValue(Dahua::GenICam:: ICameraPtr& cameraSptr, double &dMinValue, double &dMaxValue)
{
    bool bRet;
    Dahua::GenICam::IAnalogControlPtr sptrAnalogControl = Dahua::GenICam::CSystem::getInstance().createAnalogControl(cameraSptr);
    //shark:根据相机客户端找到相应属性的位置然后创建相应的对象指针（sptrAnalogControl），然后通过相机实例赋值
    if (NULL == sptrAnalogControl)//若对象为空
    {
        return -1;
    }

    Dahua::GenICam::CDoubleNode doubleNode = sptrAnalogControl->balanceRatio();//获取浮点数属性
    if (false == doubleNode.isReadable())//判断该属性是否可读
    {
        printf("balanceRatio not support.\n");
        return -1;
    }

    bRet = doubleNode.getMinVal(dMinValue);
    if (false == bRet)
    {
        printf("get balanceRatio min value fail.\n");
        return -1;
    }

    bRet = doubleNode.getMaxVal(dMaxValue);
    if (false == bRet)
    {
        printf("get balanceRatio max value fail.\n");
        return -1;
    }

    return 0;
}

// 设置采图速度（秒帧数）
// Set the acquisition speed (seconds\frames)
int32_t DHUA::setAcquisitionFrameRate(Dahua::GenICam:: ICameraPtr& cameraSptr, double dFrameRate)
{
    bool bRet;
    Dahua::GenICam::IAcquisitionControlPtr sptAcquisitionControl = Dahua::GenICam::CSystem::getInstance().createAcquisitionControl(cameraSptr);
    if (NULL == sptAcquisitionControl)
    {
        return -1;
    }

    Dahua::GenICam::CBoolNode booleanNode = sptAcquisitionControl->acquisitionFrameRateEnable();
    bRet = booleanNode.setValue(true);
    if (false == bRet)
    {
        printf("set acquisitionFrameRateEnable fail.\n");
        return -1;
    }

    Dahua::GenICam::CDoubleNode doubleNode = sptAcquisitionControl->acquisitionFrameRate();
    bRet = doubleNode.setValue(dFrameRate);
    if (false == bRet)
    {
        printf("set acquisitionFrameRate fail.\n");
        return -1;
    }
    printf("after set, acquisitonFrameRate is:%lf\n",dFrameRate);
    return 0;
}

// 获取采图速度（秒帧数）
// Get the acquisition speed (seconds and frames)
int32_t DHUA::getAcquisitionFrameRate(Dahua::GenICam:: ICameraPtr& cameraSptr, double &dFrameRate)
{
    bool bRet;
    Dahua::GenICam::IAcquisitionControlPtr sptAcquisitionControl = Dahua::GenICam::CSystem::getInstance().createAcquisitionControl(cameraSptr);
    if (NULL == sptAcquisitionControl)
    {
        return -1;
    }

    Dahua::GenICam::CDoubleNode doubleNode = sptAcquisitionControl->acquisitionFrameRate();
    bRet = doubleNode.getValue(dFrameRate);
    if (false == bRet)
    {
        printf("get acquisitionFrameRate fail.\n");
        return -1;
    }
    printf("get acquistionFrameRate is :%lf\n",dFrameRate);
    return 0;
}

// 保存参数
// Save parameters
int32_t DHUA::userSetSave(Dahua::GenICam:: ICameraPtr& cameraSptr)
{
    bool bRet;
    Dahua::GenICam::IUserSetControlPtr sptUserSetControl = Dahua::GenICam::CSystem::getInstance().createUserSetControl(cameraSptr);
    if (NULL == sptUserSetControl)
    {
        return -1;
    }

    bRet = sptUserSetControl->saveUserSet(Dahua::GenICam::IUserSetControl::userSet1);
    if (false == bRet)
    {
        printf("saveUserSet fail.\n");
        return -1;
    }

    return 0;
}

// 加载参数
// Load parameters
int32_t DHUA::loadUserSet(Dahua::GenICam:: ICameraPtr& cameraSptr)
{
    bool bRet;
    Dahua::GenICam::IUserSetControlPtr sptUserSetControl = Dahua::GenICam::CSystem::getInstance().createUserSetControl(cameraSptr);
    if (NULL == sptUserSetControl)
    {
        return -1;
    }

    bRet = sptUserSetControl->setCurrentUserSet(Dahua::GenICam::IUserSetControl::userSet1);
    if (false == bRet)
    {
        printf("saveUserSet fail.\n");
        return -1;
    }

    return 0;
}
/*shark：不需要触发
            // 设置外触发延时时间
            // set external trigger delay time
              int32_t setTriggerDelay(Dahua::GenICam:: ICameraPtr& cameraSptr, double dDelayTime)
            {
                bool bRet;
                Dahua::GenICam::IAcquisitionControlPtr sptAcquisitionControl = Dahua::GenICam::CSystem::getInstance().createAcquisitionControl(cameraSptr);
                if (NULL == sptAcquisitionControl)
                {
                    return -1;
                }

                Dahua::GenICam::CDoubleNode doubleNode = sptAcquisitionControl->triggerDelay();
                bRet = doubleNode.setValue(dDelayTime);
                if (false == bRet)
                {
                    printf("set triggerDelay fail.\n");
                    return -1;
                }

                return 0;
            }

            // 获取外触发延时时间
            // get external trigger delay time
              int32_t getTriggerDelay(Dahua::GenICam:: ICameraPtr& cameraSptr, double &dDelayTime)
            {
                bool bRet;
                Dahua::GenICam::IAcquisitionControlPtr sptAcquisitionControl = Dahua::GenICam::CSystem::getInstance().createAcquisitionControl(cameraSptr);
                if (NULL == sptAcquisitionControl)
                {
                    return -1;
                }

                Dahua::GenICam::CDoubleNode doubleNode = sptAcquisitionControl->triggerDelay();
                bRet = doubleNode.getValue(dDelayTime);
                if (false == bRet)
                {
                    printf("set triggerDelay fail.\n");
                    return -1;
                }

                return 0;
            }

            // 设置外触发模式（上升沿触发、下降沿触发）
            // Set external trigger mode (rising edge trigger, falling edge trigger)
              int32_t setLineTriggerMode(Dahua::GenICam:: ICameraPtr& cameraSptr, bool bRisingEdge)
            {
                bool bRet;
                Dahua::GenICam::IAcquisitionControlPtr sptAcquisitionControl = Dahua::GenICam::CSystem::getInstance().createAcquisitionControl(cameraSptr);
                if (NULL == sptAcquisitionControl)
                {
                    return -1;
                }

                Dahua::GenICam::CEnumNode enumNode = sptAcquisitionControl->triggerSelector();
                if (false == enumNode.setValueBySymbol("FrameStart"))
                {
                    printf("set triggerSelector fail.\n");
                    return -1;
                }

                enumNode = sptAcquisitionControl->triggerMode();
                if (false == enumNode.setValueBySymbol("On"))
                {
                    printf("set triggerMode fail.\n");
                    return -1;
                }

                enumNode = sptAcquisitionControl->triggerSource();
                if (false == enumNode.setValueBySymbol("Line1"))
                {
                    printf("set triggerSource fail.\n");
                    return -1;
                }

                enumNode = sptAcquisitionControl->triggerActivation();
                if (true == bRisingEdge)
                {
                    bRet = enumNode.setValueBySymbol("RisingEdge");
                }
                else
                {
                    bRet = enumNode.setValueBySymbol("FallingEdge");
                }

                return 0;
            }

            // 获取外触发模式（上升沿触发、下降沿触发）
            // Get external trigger mode (rising edge trigger, falling edge trigger)
              int32_t getLineTriggerMode(Dahua::GenICam:: ICameraPtr& cameraSptr, bool &bRisingEdge)
            {
                bool bRet;
                Dahua::GenICam::IAcquisitionControlPtr sptAcquisitionControl = Dahua::GenICam::CSystem::getInstance().createAcquisitionControl(cameraSptr);
                if (NULL == sptAcquisitionControl)
                {
                    return -1;
                }

                Dahua::GenICam::CEnumNode enumNode = sptAcquisitionControl->triggerSelector();
                if (false == enumNode.setValueBySymbol("FrameStart"))
                {
                    printf("set triggerSelector fail.\n");
                    return -1;
                }

                Dahua::Infra:: CString strValue;
                enumNode = sptAcquisitionControl->triggerActivation();
                if (true == bRisingEdge)
                {
                    bRet = enumNode.getValueSymbol(strValue);
                }
                else
                {
                    bRet = enumNode.getValueSymbol(strValue);
                }

                if (false == bRet)
                {
                    printf("get triggerActivation fail.\n");
                    return -1;
                }

                if (strValue == "RisingEdge")
                {
                    bRisingEdge = true;
                }
                else if (strValue == "FallingEdge")
                {
                    bRisingEdge = false;
                }
                else
                {
                    printf("get triggerActivation fail.\n");
                    return -1;
                }

                return 0;
            }

            // 设置外触发信号滤波时间
            // Set filtering time of external trigger signal
              int32_t setLineDebouncerTimeAbs(Dahua::GenICam:: ICameraPtr& cameraSptr, double dLineDebouncerTimeAbs)
            {
                IDigitalIOControlPtr sptDigitalIOControl = Dahua::GenICam::CSystem::getInstance().createDigitalIOControl(cameraSptr);
                if (NULL == sptDigitalIOControl)
                {
                    return -1;
                }

                Dahua::GenICam::CEnumNode enumNode = sptDigitalIOControl->lineSelector();
                if (false == enumNode.setValueBySymbol("Line1"))
                {
                    printf("set lineSelector fail.\n");
                    return -1;
                }

                Dahua::GenICam::CDoubleNode doubleNode = sptDigitalIOControl->lineDebouncerTimeAbs();
                if (false == doubleNode.setValue(dLineDebouncerTimeAbs))
                {
                    printf("set lineDebouncerTimeAbs fail.\n");
                    return -1;
                }

                return 0;
            }

            // 获取外触发信号滤波时间
            // Acquisition of filtering time of external trigger signal
              int32_t getLineDebouncerTimeAbs(Dahua::GenICam:: ICameraPtr& cameraSptr, double &dLineDebouncerTimeAbs)
            {
                IDigitalIOControlPtr sptDigitalIOControl = Dahua::GenICam::CSystem::getInstance().createDigitalIOControl(cameraSptr);
                if (NULL == sptDigitalIOControl)
                {
                    return -1;
                }

                Dahua::GenICam::CEnumNode enumNode = sptDigitalIOControl->lineSelector();
                if (false == enumNode.setValueBySymbol("Line1"))
                {
                    printf("set lineSelector fail.\n");
                    return -1;
                }

                Dahua::GenICam::CDoubleNode doubleNode = sptDigitalIOControl->lineDebouncerTimeAbs();
                if (false == doubleNode.getValue(dLineDebouncerTimeAbs))
                {
                    printf("get lineDebouncerTimeAbs fail.\n");
                    return -1;
                }

                return 0;
            }

            // 设置外触发脉冲宽度（不支持）  | Set external trigger width (not supported)
            // 获取外触发脉冲宽度（不支持）  | Get external trigger width (not supported)
            // 设置输出信号线（控制光源用）（面阵相机是Line0） | Set the output signal line (for controlling the light source) (the area array camera is line0)
            // 获取输出信号线（面阵相机是Line0） | get the output signal line (the area array camera is line0)
            // 设置外部光源曝光时间（设置输出值为TRUE的时间） | Set the exposure time of the external light source (set the time when the output value is true)
              int32_t setOutputTime(Dahua::GenICam:: ICameraPtr& cameraSptr, int nTimeMS)
            {
                IDigitalIOControlPtr sptDigitalIOControl = Dahua::GenICam::CSystem::getInstance().createDigitalIOControl(cameraSptr);
                if (NULL == sptDigitalIOControl)
                {
                    return -1;
                }

                Dahua::GenICam::CEnumNode paramLineSource(cameraSptr, "LineSource");
                if (false == paramLineSource.setValueBySymbol("UserOutput1"))
                {
                    printf("set LineSource fail.");
                    return -1;
                }

                // 将输出信号拉高然后拉低
                // Pull the output signal up and down
                Dahua::GenICam::CBoolNode booleanNode = sptDigitalIOControl->userOutputValue();
                if (false == booleanNode.setValue(true))
                {
                    printf("set userOutputValue fail.\n");
                    return -1;
                }

                CThread::sleep(nTimeMS);

                if (false == booleanNode.setValue(false))
                {
                    printf("set userOutputValue fail.\n");
                    return -1;
                }

                return 0;
            }
            */
/*shark：不需要翻转
            //  获取外部光源曝光时间（输出信号的时间由软件侧控制） | get the exposure time of external light source (the time of output signal is controlled by the software side)
            //  设置X轴翻转  | Set X-axis flip
              int32_t setReverseX(Dahua::GenICam:: ICameraPtr& cameraSptr, bool flag)
            {
                Dahua::GenICam:: IImageFormatControlPtr sptrImageFormatControl = Dahua::GenICam::CSystem::getInstance().createImageFormatControl(cameraSptr);

                Dahua::GenICam::CBoolNode boolNodeReverseX = sptrImageFormatControl->reverseX();
                if(!boolNodeReverseX.setValue(flag))
                {
                    printf("set reverseX fail.\n");
                    return -1;
                }

                return 0;
            }

            // 设置Y轴翻转
            // Set X-axis flip
              int32_t setReverseY(Dahua::GenICam:: ICameraPtr& cameraSptr, bool flag)
            {
                Dahua::GenICam:: IImageFormatControlPtr sptrImageFormatControl = Dahua::GenICam::CSystem::getInstance().createImageFormatControl(cameraSptr);

                Dahua::GenICam::CBoolNode boolNodeReverseY = sptrImageFormatControl->reverseY();
                if(!boolNodeReverseY.setValue(flag))
                {
                    printf("set reverseY fail.\n");
                    return -1;
                }

                return 0;
            }
            */

// 当相机与网卡处于不同网段时，自动设置相机IP与网卡处于同一网段 （与相机连接之前调用）
// When the camera and the network card are in different network segments, automatically set the camera IP and the network card in the same network segment (before calling the camera).
int32_t DHUA::autoSetCameraIP(Dahua::GenICam:: ICameraPtr& cameraSptr)
{
    Dahua::GenICam::IGigECameraPtr gigeCameraPtr = Dahua::GenICam::IGigECamera::getInstance(cameraSptr);
    if(NULL == gigeCameraPtr)
    {
        return -1;
    }

    // 获取Gige相机相关信息
    // Get GigE camera information
    Dahua::Infra:: CString ip = gigeCameraPtr->getIpAddress();
    Dahua::Infra:: CString subnetMask = gigeCameraPtr->getSubnetMask();
    Dahua::Infra:: CString gateway = gigeCameraPtr->getGateway();
    Dahua::Infra:: CString macAddress = gigeCameraPtr->getMacAddress();
    printf("ip address is %s.\r\n", ip.c_str());
    printf("subnetMask is %s.\r\n", subnetMask.c_str());
    printf("gateway is %s.\r\n", gateway.c_str());
    printf("macAddress is %s.\r\n", macAddress.c_str());
    printf("\n");

    unsigned long devIpValue = ntohl(inet_addr(gigeCameraPtr->getIpAddress().c_str()));
    unsigned long devSubMaskValue = ntohl(inet_addr(gigeCameraPtr->getSubnetMask().c_str()));

    // 获取对应接口的网卡信息
    // Get the network card information of the corresponding interface
    Dahua::GenICam::IGigEInterfacePtr gigeInterfaceSPtr = Dahua::GenICam::IGigEInterface::getInstance(cameraSptr);
    if(NULL == gigeInterfaceSPtr)
    {
        return -1;
    }

    Dahua::Infra:: CString interfaceIp = gigeInterfaceSPtr->getIpAddress();
    Dahua::Infra:: CString interfaceSubnetMask = gigeInterfaceSPtr->getSubnetMask();
    Dahua::Infra:: CString interfaceGateway = gigeInterfaceSPtr->getGateway();
    Dahua::Infra:: CString interfaceMacAddress = gigeInterfaceSPtr->getMacAddress();
    printf("ip address of interface  is %s.\r\n", interfaceIp.c_str());
    printf("subnetMask of interface is %s.\r\n", interfaceSubnetMask.c_str());
    printf("gateway of interface is %s.\r\n", interfaceGateway.c_str());
    printf("macAddress of interface is %s.\r\n", interfaceMacAddress.c_str());
    printf("\n");

    unsigned long InterfaceIpValue = ntohl(inet_addr(gigeInterfaceSPtr->getIpAddress().c_str()));
    unsigned long InterfaceSubMaskValue = ntohl(inet_addr(gigeInterfaceSPtr->getSubnetMask().c_str()));

    if( (devIpValue & devSubMaskValue) != (InterfaceIpValue & InterfaceSubMaskValue) )
    {
        // 设备与网卡不在同一网段，强制设置设备与网卡在同一网段
        // The device and network card are not in the same network segment. It is mandatory to set the device and network card in the same network segment
        unsigned char newIPStr[20] = {0};

        while(1)
        {
            unsigned long newIpValue = rand() % 254 + 1; //1~254
            if(newIpValue != (InterfaceIpValue & 0xff))
            {
                newIpValue = (InterfaceIpValue & 0xffffff00) + newIpValue;
                struct in_addr   stInAddr;
                stInAddr.s_addr	= htonl(newIpValue);
                memcpy(newIPStr, inet_ntoa(stInAddr), strlen(inet_ntoa(stInAddr)));
                break;
            }
        }

        if(!gigeCameraPtr->forceIpAddress((const char*)newIPStr, gigeInterfaceSPtr->getSubnetMask().c_str(), gigeInterfaceSPtr->getGateway().c_str()))
        {
            printf("Set device ip failed.\n");
            return -1;
        }
    }

    return 0;
}

// 设置相机IP （与相机连接之前调用）
// Set up camera IP (before calling with camera)
int32_t DHUA::setCameraIp(Dahua::GenICam:: ICameraPtr& cameraSptr, char* ipAddress, char* subnetMask, char* gateway)
{
    Dahua::GenICam::IGigECameraPtr gigeCameraPtr = Dahua::GenICam::IGigECamera::getInstance(cameraSptr);
    if(NULL == gigeCameraPtr)
    {
        return -1;
    }

    if(!gigeCameraPtr->forceIpAddress(ipAddress, subnetMask, gateway))
    {
        printf("Set device ip failed.\n");
        return -1;
    }

    return 0;
}

// 设置相机静态IP （与相机连接之后调用）
// Set camera   IP (after calling with camera)
int32_t DHUA::setCameraPersistentIP(Dahua::GenICam:: ICameraPtr& cameraSptr)
{
    Dahua::GenICam::IGigECameraPtr gigeCameraPtr = Dahua::GenICam::IGigECamera::getInstance(cameraSptr);
    if(NULL == gigeCameraPtr)
    {
        printf("gigeCameraPtr is null.\n");
        return -1;
    }

    Dahua::GenICam::ITransportLayerControlPtr transportLayerControlPtr= Dahua::GenICam::CSystem::getInstance().createTransportLayerControl(cameraSptr);

    if(NULL == transportLayerControlPtr)
    {
        printf("transportLayerControlPtr is null.\n");
        return -1;
    }

    transportLayerControlPtr->gevCurrentIPConfigurationPersistentIP().setValue(true);
    transportLayerControlPtr->gevPersistentDefaultGateway().setValue(gigeCameraPtr->getGateway().c_str());
    transportLayerControlPtr->gevPersistentIPAddress().setValue(gigeCameraPtr->getIpAddress().c_str());
    transportLayerControlPtr->gevPersistentSubnetMask().setValue(gigeCameraPtr->getSubnetMask().c_str());

    return 0;
}

/* shark:官方这句没什么用，就每次都会曝光加2罢了，似乎。
             * 修改曝光时间 （与相机连接之后调用）
            // Modify exposure time (after calling connect camera)
              void modifyCamralExposureTime(Dahua::GenICam::CSystem &systemObj, Dahua::GenICam:: ICameraPtr& cameraSptr)
            {
                Dahua::GenICam::IAcquisitionControlPtr sptrAcquisitionControl = systemObj.createAcquisitionControl(cameraSptr);
                if (NULL == sptrAcquisitionControl)
                {
                    return;
                }

                double exposureTimeValue = 0.0;
                Dahua::GenICam::CDoubleNode exposureTime = sptrAcquisitionControl->exposureTime();

                exposureTime.getValue(exposureTimeValue);
                printf("before change ,exposureTime is %f. thread ID :%d\n", exposureTimeValue, CThread::getCurrentThreadID());
                exposureTime.setValue(exposureTimeValue + 2);
                exposureTime.getValue(exposureTimeValue);
                printf("after change ,exposureTime is %f. thread ID :%d\n", exposureTimeValue, CThread::getCurrentThreadID());
            }
            */
void DHUA::LogPrinterFunc(const char* log)
{
    return;
}

// ********************** 这部分处理与SDK操作相机无关，用于显示设备列表 begin*****************************
// ***********BEGIN: These functions are not related to API call and used to display device info***********
void DHUA::displayDeviceInfo(Dahua::Infra::TVector<Dahua::GenICam:: ICameraPtr>& vCameraPtrList)
{
    Dahua::GenICam:: ICameraPtr cameraSptr;
    // 打印Title行
    // Print title lineusing namespace Dahua::GenICam;
    using namespace Dahua::Infra;

    printf("\nIdx Type Vendor     Model      S/N             DeviceUserID    IP Address    \n");
    printf("------------------------------------------------------------------------------\n");
    for (int cameraIndex = 0; cameraIndex < vCameraPtrList.size(); cameraIndex++)
    {
        cameraSptr = vCameraPtrList[cameraIndex];
        // Idx 设备列表的相机索引 最大表示字数：3
        // Camera index in device list, display in 3 characters
        printf("%-3d", cameraIndex + 1);

        // Type 相机的设备类型（GigE，U3V，CL，PCIe
        // Camera type (eg:GigE，U3V，CL，PCIe)
        switch (cameraSptr->getType())
        {
        case Dahua::GenICam::ICamera::typeGige:
            printf(" GigE");
            break;
        case Dahua::GenICam::ICamera::typeU3v:
            printf(" U3V ");
            break;
        case Dahua::GenICam::ICamera::typeCL:
            printf(" CL  ");
            break;
        case Dahua::GenICam::ICamera::typePCIe:
            printf(" PCIe");
            break;
        default:
            printf("     ");
            break;
        }

        // VendorName 制造商信息 最大表示字数：10
        // Camera vendor name, display in 10 characters
        const char* vendorName = cameraSptr->getVendorName();
        char vendorNameCat[11];
        if (strlen(vendorName) > 10)
        {
            strncpy(vendorNameCat, vendorName, 7);
            vendorNameCat[7] = '\0';
            strcat(vendorNameCat, "...");
            printf(" %-10.10s", vendorNameCat);
        }
        else
        {
            printf(" %-10.10s", vendorName);
        }

        // ModeName 相机的型号信息 最大表示字数：10
        // Camera model name, display in 10 characters
        printf(" %-10.10s", cameraSptr->getModelName());

        // Serial Number 相机的序列号 最大表示字数：15
        // Camera serial number, display in 15 characters
        printf(" %-15.15s", cameraSptr->getSerialNumber());

        // deviceUserID 自定义用户ID 最大表示字数：15
        // Camera user id, display in 15 characters
        const char* deviceUserID = cameraSptr->getName();
        char deviceUserIDCat[16] = {0};
        if (strlen(deviceUserID) > 15)
        {
            strncpy(deviceUserIDCat, deviceUserID, 12);
            deviceUserIDCat[12] = '\0';
            strcat(deviceUserIDCat, "...");
            printf(" %-15.15s", deviceUserIDCat);
        }
        else
        {
            // 防止console显示乱码,UTF8转换成ANSI进行显示
            // Prevent console from displaying garbled code and convert utf8 to ANSI for display
            memcpy(deviceUserIDCat, deviceUserID, sizeof(deviceUserIDCat));
            printf(" %-15.15s", deviceUserIDCat);
        }

        // IPAddress GigE相机时获取IP地址
        // IP address of GigE camera
        Dahua::GenICam::IGigECameraPtr gigeCameraPtr = Dahua::GenICam::IGigECamera::getInstance(cameraSptr);
        if (NULL != gigeCameraPtr.get())
        {
            Dahua::Infra:: CString ip = gigeCameraPtr->getIpAddress();
            printf(" %s", ip.c_str());
        }
        printf("\n");

    }
}

/*shark：默认只有一个相机，默认选1，也就不需要选择了。以后若是上多相机这段记得复原。
              char* trim(char* pStr)
            {
                char* pDst = pStr;
                char* pTemStr = NULL;
                int ret = -1;

                if (pDst != NULL)
                {
                    pTemStr = pDst + strlen(pStr) - 1;
                    // 除去字符串首部空格
                    // remove the first space of the string
                    while ( *pDst ==' ')
                    {
                        pDst++;
                    }
                    // 除去字符串尾部空格
                    // remove trailing space from string
                    while ((pTemStr > pDst) &&(*pTemStr == ' '))
                    {
                        *pTemStr-- = '\0';
                    }
                }
                return pDst;
            }
            // 函数功能：判断pInpuStr字符串每个字符是否都为数字。
            // function: judge whether each character of pinpustr string is a number.
            // 该函数与SDK接口操作相机无关
            // this function is independent of the SDK interface operation camera
              int isInputValid(char* pInpuStr)
            {
                char numChar;
                char* pStr = pInpuStr;
                while (*pStr != '\0')
                {
                    numChar = *pStr;
                    if ((numChar > '9') || (numChar < '0'))
                    {
                        return -1;
                    }
                    pStr++;
                }
                return 0;
            }

            // 函数功能：从displayDeviceInfo显示的相机列表选择需要操作的相机对象。
            // function: select the camera object to be operated from the list of cameras displayed in displaydeviceinfo
            // 该函数与SDK接口操作相机无关
            // this function is independent of the SDK interface operation camera
              int selectDevice(int cameraCnt)
            {
                char inputStr[256] = {0};
                char* pTrimStr;
                char* find = NULL;
                int inputIndex = -1;
                int ret = -1;
                // 提示用户选择
                // inform user to select
                printf("\nPlease input the camera index: ");
                while (1)
                {
                    // 获取输入内容字符串
                    // get input string/
                    memset(inputStr, 0, sizeof(inputStr));
                    fgets(inputStr, sizeof(inputStr), stdin);

                    // 清空输入缓存
                    // clear flush
                    fflush(stdin);

                    // fgets比gets多吃一个换行符号，取出换行符号
                    // fgets eats one more line feed symbol than gets, and takes out the line feed symbol
                    find = strchr(inputStr, '\n');
                    if (find) { *find = '\0'; }

                    // 除去字符串首尾空格
                    // remove starting and trailing spaces from string
                    pTrimStr = trim(inputStr);
                    // 判断输入字符串是否为数字
                    // judge whether the input string is a number
                    ret = isInputValid(pTrimStr);
                    if (ret == 0)
                    {
                        // 输入的字符串转换成为数字
                        // the input string is converted to a number
                        inputIndex = atoi(pTrimStr);
                        // 判断用户选择合法性 /
                        // judge the validity of user selection
                        inputIndex -= 1;//显示的Index是从1开始 english: input index starts from 1
                        if ((inputIndex >= 0) && (inputIndex < cameraCnt))
                        {
                            break;
                        }
                    }

                    printf("Input invalid! Please input the camera index: ");
                }
                return inputIndex;
            }
            */

// ********************** 这部分处理与SDK操作相机无关，用于显示设备列表 end*****************************
// *** This part of the processing is independent of the SDK operation camera and is used to display the device list**

//shark：官方的初始化步骤。把0改为1即可编译。比#ifdef方便一些。预编译知识，学到了
#if 0
setGrabMode(cameraSptr, true);
setGrabMode(cameraSptr, false);
bool bContious = false;
getGrabMode(cameraSptr, bContious);
triggerSoftware(cameraSptr);

int64_t nWidth, nHeight;
setResolution(cameraSptr, 640, 480);
getResolution(cameraSptr, nWidth, nHeight);

setBinning(cameraSptr);

getMaxResolution(cameraSptr, nWidth, nHeight);

int64_t nX, nY, nROIWidth, nROIHeight;
setROI(cameraSptr, 120, 120, 640, 480);
getROI(cameraSptr, nX, nY, nROIWidth, nROIHeight);

getWidth(cameraSptr, nWidth);
getHeight(cameraSptr, nHeight);

double dExposureTime = 0;
setExposureTime(cameraSptr, 100, true);
setExposureTime(cameraSptr, 10000, false);
getExposureTime(cameraSptr, dExposureTime);

double dMinExposure, dMaxExposure;
getExposureTimeMinMaxValue(cameraSptr, dMinExposure, dMaxExposure);

double dGainRaw = 0;
double dGainRawMin = 0;
double dGainRawMax = 0;
setGainRaw(cameraSptr, 1.2);
getGainRaw(cameraSptr, dGainRaw);
getGainRawMinMaxValue(cameraSptr, dGainRawMin, dGainRawMax);

double dGamma = 0;
double dGammaMin = 0;
double dGammaMax = 0;
setGamma(cameraSptr, 0.8);
getGamma(cameraSptr, dGamma);
getGammaMinMaxValue(cameraSptr, dGammaMin, dGammaMax);

double dRedBalanceRatio = 0;
double dGreenBalanceRatio = 0;
double dBlueBalanceRatio = 0;
double dMinBalanceRatio = 0;
double dMaxBalanceRatio = 0;
setBalanceRatio(cameraSptr, 1.5, 1.5, 1.5);
getBalanceRatio(cameraSptr, dRedBalanceRatio, dGreenBalanceRatio, dBlueBalanceRatio);
getBalanceRatioMinMaxValue(cameraSptr, dMinBalanceRatio, dMaxBalanceRatio);

double dFrameRate = 0;
setAcquisitionFrameRate(cameraSptr, 20);
getAcquisitionFrameRate(cameraSptr, dFrameRate);

userSetSave(cameraSptr);
loadUserSet(cameraSptr);

double dDelayTime = 0;
setTriggerDelay(cameraSptr, 20);
getTriggerDelay(cameraSptr, dDelayTime);

bool bRisingEdge = true;
setLineTriggerMode(cameraSptr, bRisingEdge);
getLineTriggerMode(cameraSptr, bRisingEdge);

double dLineDebouncerTimeAbs = 0;
setLineDebouncerTimeAbs(cameraSptr, 20);
getLineDebouncerTimeAbs(cameraSptr, dLineDebouncerTimeAbs);

setOutputTime(cameraSptr, 1000);

setReverseX(cameraSptr, false);
setReverseY(cameraSptr, false);
#endif





