#include <iostream>
#include <thread>
#include "Thread/threads.h"
#include "configer.h"

using namespace std;
using namespace cv;

int main()
{

   Threads thread;

#ifdef SERIAL_OPEN
   std::thread serial_(&Threads::T_serial,&thread);
#endif
   std::thread camera_(&Threads::camera,&thread);

   std::thread image_(&Threads::image,&thread);

#ifdef SERIAL_OPEN
   serial_.join();
#endif
   waitKey(1);

   camera_.join();
//   waitKey(1);
   image_.join();

   return 0;
}
