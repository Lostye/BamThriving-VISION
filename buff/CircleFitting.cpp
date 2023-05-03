#include"CircleFitting.h"


#define Plus
///
/// \brief CircleFitting    根据轨迹拟合圆
/// \param pts              轨迹点集
/// \param center           输出的中心
/// \param radius           输出的半径
/// \return
///
unsigned char CircleFitting(
    std::vector<cv::Point> &pts,		// 2d点坐标存储向量
//    cv::Point2f& center,				//圆心坐标
    float& radius)						//圆的半径


{cv::Point2f last_center;double last_radius;
    if (pts.size() <3){ //判断输入点的个数若小于三个则不能拟合
        printf("拟合异常，点集小于3\n");
        radius=0;
//        center=cv::Point2d(0,0);
        return -1;//拟合少于3个点的情况
    }
    printf("%zu\n",pts.size());
    if(pts.size()>500){//点集超过1000个点，结束拟合
        printf("拟合完成，终止拟合\n");
        return 0;//拟合完成
    }
    //center = cv::Point2d(0, 0);			//初始化圆心为(0,0)
    radius = 0.0;						//半径为0

    //定义计算中间变量
    double sumX1 = 0.0; //代表Xi的和(从1~n) ，X1代表X的1次方
    double sumY1 = 0.0;
    double sumX2 = 0.0;	//代表(Xi)^2的和(i从1~n)，X2代表X的二次方
    double sumY2 = 0.0;
    double sumX3 = 0.0;
    double sumY3 = 0.0;
    double sumX1Y1 = 0.0;
    double sumX1Y2 = 0.0;
    double sumX2Y1 = 0.0;
    const double N = (double)pts.size();//获得输入点的个数
    for (size_t i = 0; i <pts.size(); ++i)
    {
        double x = pts.at(i).x;			//获得第i个点的x坐标
        double y = pts.at(i).y;			//获得第i个点的y坐标
        double x2 = x * x;				//计算x^2
        double y2 = y * y;				//计算y^2
        double x3 = x2 * x;				//计算x^3
        double y3 = y2 * y;				//计算y^3
        double xy = x * y;				//计算xy
        double x1y2 = x * y2;			//计算x*y^2
        double x2y1 = x2 * y;			//计算x^2*y

        sumX1 += x;						//sumX=sumX+x;计算x坐标的和
        sumY1 += y;						//sumY=sumY+y;计算y坐标的和
        sumX2 += x2;					//计算x^2的和
        sumY2 += y2;					//计算各个点的y^2的和
        sumX3 += x3;					//计算x^3的和
        sumY3 += y3;
        sumX1Y1 += xy;
        sumX1Y2 += x1y2;
        sumX2Y1 += x2y1;
    }
    double C = N * sumX2 - sumX1 * sumX1;
    double D = N * sumX1Y1 - sumX1 * sumY1;
    double E = N * sumX3 + N * sumX1Y2 - (sumX2 + sumY2) * sumX1;
    double G = N * sumY2 - sumY1 * sumY1;
    double H = N * sumX2Y1 + N * sumY3 - (sumX2 + sumY2) * sumY1;

    double denominator = C * G - D * D;
    if (std::abs(denominator) < DBL_EPSILON) return false;//判断分母的绝对值是否接近于（等于）0，使用此行判断须将函数改为bool类型，才能有返回值
    double a = (H * D - E * G) / (denominator);
    if (std::abs(denominator) < DBL_EPSILON) return false;
    double b = (H * C - E * D) / (-denominator);
    double c = -(a * sumX1 + b * sumY1 + sumX2 + sumY2) / N;


    float x = a / (-2);
    float y = b / (-2);

//    center.x = a / (-2);
//    center.y = b / (-2);
    radius = std::sqrt(a * a + b * b - 4 * c) / 2;
//    last_center=center;
    last_radius=radius;





#ifdef Plus
    int pt_len=pts.size();
    const int lr = 2;				// 学习率, 一般拟合的点都比较多, 所以我设置的比较大, 可以根据你的情况来设置
    const int nIters = pt_len;		// 迭代次数, 我设置成了有多少个点就迭代多少次, 也可以根据实际情况设置

    std::vector<float> losses(pt_len);	// 每次迭代后的 loss 值
    std::vector<float> min_loss(pt_len);	// 每次迭代后的最小 loss
    std::vector<float> root_val(pt_len);	// 每次迭代中的开平方值, 方便以后使用

    for (int i = 0; i < nIters; i++)
    {
        float loop_loss = 0;

        for (int j = 0; j < pt_len; j++)
        {
            // 这里第一次迭代的 x, y, r 是最小二乘的结果, 第二次迭代开始就是修正后的结果
            root_val[j] = sqrt((pts[j].x - x) * (pts[j].x - x) + (pts[j].y - y) * (pts[j].y - y));

            const float loss = root_val[j] - radius;

            losses[j] = loss;
            loop_loss += fabs(loss);
        }

        min_loss[i] = loop_loss;

        // 如果 loss 值不再减小, 就提前结束
        if (i > 0 && min_loss[i] > min_loss[i - 1])
        {
            break;
        }

        // 下面三个是梯度值
        float gx = 0;
        float gy = 0;
        float gr = 0;

        for (int j = 0; j < pt_len; j++)
        {
            // 在计算梯度时要先计算偏导数, 再将 x 代数公式得到
            float gxi = (x - pts[j].x) / root_val[j];

            if (losses[j] < 0)
            {
                gxi *= (-1);
            }

            float gyi = (y - pts[j].y) / root_val[j];

            if (losses[j] < 0)
            {
                gyi *= (-1);
            }

            float gri = -1;

            if (losses[j] < 0)
            {
                gri = 1;
            }

            gx += gxi;
            gy += gyi;
            gr += gri;
        }

        gx /= pt_len;
        gy /= pt_len;
        gr /= pt_len;

        x -= (lr * gx);
        y -= (lr * gy);
        radius -= (lr * gr);
//        printf("x:%f\n",x);
//        printf("y:%f\n",y);

    }

#endif
    return 1;//将函数改为bool类型需要加返回值
}


