# BamThriving-VISION
robomaster欣竹战队视觉

### 环境
  ubuntu18/20  
  
  矩阵库（eigen3） 
  
  opencv4.5 
  
  大华相机驱动
 
### 准备
安装矩阵库

    sudo apt-get install libeigen3-dev
    sudo cp -r /usr/include/eigen3/Eigen /usr/local/include/
    sudo cp -r /usr/include/eigen3 /usr/local/include/
    
安装相机驱动    
下载链接：https://pan.baidu.com/s/1OplS2ST2jfeDzYpOI8dbYg
密码：1024 
解压后执行命令安装
    
    sudo chmod +x MVviewer_Ver2.2.5_Linux_x86_Build20200910.run
    sudo ./MVviewer_Ver2.2.5_Linux_x86_Build20200910.run

### 构建运行
接入相机后运行

    mkdir build
    cd build
    cmake ..
    make -j8
    ./BamThriving-VISION
    
### 一些说明
次项目算法实现比较简单，全为传统视觉实现，体量较小，能够实现数图案分类和卡尔曼运动预测。项目一边迭代一边测试，很多代码没有仔细斟酌。实际效果可查看上面提到的网盘或查阅比赛视频。
在此特别感谢战队所有视觉组成员！！

##### 欢迎交流
WX：-Yangxiaolang
