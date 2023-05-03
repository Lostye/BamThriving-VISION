#ifndef _STREAMRETRIEVE_H_
#define _STREAMRETRIEVE_H_
#ifdef __unix__
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
#include "GenICam/Frame.h"
#include "Infra/Thread.h"
#include "GenICam/StreamSource.h"
#include <opencv2/opencv.hpp>


class StreamRetrieve : public Dahua::Infra::CThread {
public:
    StreamRetrieve(Dahua::GenICam::IStreamSourcePtr &streamSptr);

    bool start();

    void stop();

    cv::Mat getMatImage() const {
        return matImage_;
    }

    bool getCalReady() const {
        return calMatReady_;
    }

    double getFrameFps() const {
        return frameFps_;
    }

    void setCalReady(bool iReady) {
        calMatReady_ = iReady;
    }

    int getLostCameraCNT() const {
        return lostCameraCNT_;
    }

private:
    bool calMatReady_;
    bool showMatReady_;
    bool m_isLoop;//shark：是否循环
    int lostCameraCNT_ = 0;
    double frameFps_ = 0;
    cv::Mat originImage_;
    cv::Mat matImage_;
    Dahua::GenICam::IStreamSourcePtr m_streamSptr;

    void threadProc();

    void calFrameFps();

    static void imageConvert(Dahua::GenICam::CFrame &input, cv::Mat &originPut, cv::Mat &output);
};

typedef Dahua::Memory::TSharedPtr<StreamRetrieve> StreamRetrievePtr;


#endif
