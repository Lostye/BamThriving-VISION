#ifndef DHUA_H
#define DHUA_H
#ifndef __unix__
#include <Winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include "GenICam/System.h"
#include "GenICam/Camera.h"
#include "GenICam/StreamSource.h"
#include "GenICam/GigE/GigECamera.h"
#include "GenICam/GigE/GigEInterface.h"
#include "Infra/PrintLog.h"
#include "StreamRetrieve.h"
#include "Memory/SharedPtr.h"
#include "Infra/Thread.h"

#include<iostream>
#include<opencv4/opencv2/opencv.hpp>

#define CameraExposureTime 4000//曝光值
#define GainRawValue 1//增益值
#define Gamma 1//伽马值
/*****shark:打什么颜色的队伍就把相应的增益调高，把己方颜色增益降低(正常显示颜色的参数：R（1.84），G（1），B（2.42）)*/
#define dRED 1.52//红平衡值
#define dGREEN 1.1//绿平衡值
#define dBLUE 1.52//蓝平衡值

class DHUA
{
public:
    cv::Mat camera_src;
    Dahua::GenICam::CSystem &systemObj=Dahua::GenICam::CSystem::getInstance();/*shark:这是直接初始化systemjob引用变量*/
    //shark:引用变量必须初始化，跟const变量一样不支持赋值操作。不然就得在构造函数那里用初始化列表。但是这个比玩意初始化的值又是一个返回值。我猜
    Dahua::GenICam:: ICameraPtr cameraSptr;//相机指针
    Dahua::Infra::TVector<Dahua::GenICam:: ICameraPtr> vCameraPtrList;//相机列表
    Dahua::GenICam::IStreamSourcePtr streamPtr;//信号流指针
    Dahua::Memory::TSharedPtr<StreamRetrieve> streamThreadSptr;//线程指针

    bool creat_stream();
    bool discovery_device();
    bool set_option();

    DHUA();//构造函数（创建对象时自动）
    ~DHUA();

//    int CameraExposureTime =20000;
    void setCameraExposureTime(Dahua::GenICam:: ICameraPtr &cameraSptr, double exposureTimeSet, bool addFlag);
    //shark：自己写的设置相机增益
private:
    static int32_t setGrabMode(Dahua::GenICam::ICameraPtr cameraSptr, bool bContious);
    // 获取相机采图模式
    // Get camera acquisition mode
    static int32_t getGrabMode(Dahua::GenICam:: ICameraPtr& cameraSptr, bool &bContious);
    // 软件触发
    // software trigger
    static int32_t triggerSoftware(Dahua::GenICam:: ICameraPtr& cameraSptr);
    // 设置传感器采样率（采集分辨率）
    // Set sensor sampling rate (acquisition resolution)
    static int32_t setResolution(Dahua::GenICam:: ICameraPtr& cameraSptr, int nWidth, int nHeight);
    // 获取传感器采样率
    // Get sensor sample rate
    static int32_t getResolution(Dahua::GenICam:: ICameraPtr& cameraSptr, int64_t &nWidth, int64_t &nHeight);
    // 设置binning (Off X Y XY)
    // set binning (Off X Y XY)
    static int32_t setBinning(Dahua::GenICam:: ICameraPtr& cameraSptr);
    // 获取传感器最大分辩率
    // get the maximum resolution of the sensor
    static int32_t getMaxResolution(Dahua::GenICam:: ICameraPtr& cameraSptr, int64_t &nWidthMax, int64_t &nHeightMax);
    // 设置图像ROI
    // set image ROI
    static int32_t setROI(Dahua::GenICam:: ICameraPtr& cameraSptr, int64_t nX, int64_t nY, int64_t nWidth, int64_t nHeight);
    // 获取图像ROI
    // get image ROI
    static int32_t getROI(Dahua::GenICam:: ICameraPtr& cameraSptr, int64_t &nX, int64_t &nY, int64_t &nWidth, int64_t &nHeight);
    // 获取采图图像宽度
    // Get the width of the image
    static int32_t getWidth(Dahua::GenICam:: ICameraPtr& cameraSptr, int64_t &nWidth);
    // 获取采图图像高度
    // Get the height of the image
    static int32_t getHeight(Dahua::GenICam:: ICameraPtr& cameraSptr, int64_t &nHeight);
    // 改动的设置曝光值(曝光、自动曝光/手动曝光)shark:这只有连续曝光和手动曝光，没有一次曝光
    // Set exposure value (exposure, auto exposure / manual exposure)
    static int32_t setExposureMODE(Dahua::GenICam:: ICameraPtr& cameraSptr, bool bAutoExposure );
    //shark:自己写的设置曝光值(addFlag：true添加模式)

    static void setGainRawValue(Dahua::GenICam:: ICameraPtr &cameraSptr,double gainRawSet ,bool addFlag);
    //shark：自己写的gamma设置
    void SetGamma(Dahua::GenICam:: ICameraPtr &cameraSptr, double gammaSet, bool addFlag);
    // 设置白平衡值（有三个白平衡值）
    // Set the white balance value ( three white balance values)
    static int32_t setBalanceRatio(Dahua::GenICam:: ICameraPtr& cameraSptr, double dRedBalanceRatio, double dGreenBalanceRatio, double dBlueBalanceRatio);
    // 获取白平衡值（有三个白平衡值)
    // Get white balance value (three white balance values)
    static int32_t getBalanceRatio(Dahua::GenICam:: ICameraPtr& cameraSptr, double &dRedBalanceRatio, double &dGreenBalanceRatio, double &dBlueBalanceRatio);
    // 获取白平衡值范围
    // Get white balance value range
    static int32_t getBalanceRatioMinMaxValue(Dahua::GenICam:: ICameraPtr& cameraSptr, double &dMinValue, double &dMaxValue);
    // 设置采图速度（秒帧数）
    // Set the acquisition speed (seconds\frames)
    static int32_t setAcquisitionFrameRate(Dahua::GenICam:: ICameraPtr& cameraSptr, double dFrameRate);
    // 获取采图速度（秒帧数）
    // Get the acquisition speed (seconds and frames)
    static int32_t getAcquisitionFrameRate(Dahua::GenICam:: ICameraPtr& cameraSptr, double &dFrameRate);
    // 保存参数
    // Save parameters
    static int32_t userSetSave(Dahua::GenICam:: ICameraPtr& cameraSptr);
    // 加载参数
    // Load parameters
    static int32_t loadUserSet(Dahua::GenICam:: ICameraPtr& cameraSptr);
    // 当相机与网卡处于不同网段时，自动设置相机IP与网卡处于同一网段 （与相机连接之前调用）
    // When the camera and the network card are in different network segments, automatically set the camera IP and the network card in the same network segment (before calling the camera).
    static int32_t autoSetCameraIP(Dahua::GenICam:: ICameraPtr& cameraSptr);
    // 设置相机IP （与相机连接之前调用）
    // Set up camera IP (before calling with camera)
    static int32_t setCameraIp(Dahua::GenICam:: ICameraPtr& cameraSptr, char* ipAddress, char* subnetMask, char* gateway);
    // 设置相机静态IP （与相机连接之后调用）
    // Set camera static IP (after calling with camera)
    static int32_t setCameraPersistentIP(Dahua::GenICam:: ICameraPtr& cameraSptr);
    //shark:似乎是打印日志功能，但官方似乎只是给了个模板，让用户自己编写日志功能，这部分可以参考桂林理工中的日志部分（RMLOG.h）。后辈们们有空的话尽量完善，毕竟对维护代码，排除问题很有帮助
    void LogPrinterFunc(const char* log);
    // ********************** 这部分处理与SDK操作相机无关，用于显示设备列表 begin*****************************
    // ***********BEGIN: These functions are not related to API call and used to display device info***********
    static void displayDeviceInfo(Dahua::Infra::TVector<Dahua::GenICam:: ICameraPtr>& vCameraPtrList);

};

#endif // DHUA_H
