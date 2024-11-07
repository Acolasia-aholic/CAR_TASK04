// 发布导航目标点，小车实现导航，实现导航结束后的下一次导航
#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <tf2/LinearMath/Quaternion.h>
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>
#include <tesseract/baseapi.h>
#include <string.h>
#include <iostream>

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

std::string full_result;

void imageCallback(const sensor_msgs::ImageConstPtr &msg)
{
    try
    {
        // 将ROS图像消息转换为OpenCV图像格式
        cv::Mat image = cv_bridge::toCvShare(msg, "bgr8")->image;
        // 转换为灰度图像
        cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
        // 二值化
        cv::threshold(image, image, 128, 255, cv::THRESH_BINARY);
        // 在这里添加你的字母识别逻辑
        // 初始化Tesseract OCR
        tesseract::TessBaseAPI tess;
        tess.Init(NULL, "eng");                                           // 使用英语语言模型
        tess.SetImage(image.data, image.cols, image.rows, 1, image.cols); // cols

        // 进行OCR文本识别
        tess.Recognize(0);

        // 获取识别结果
        const char *result = tess.GetUTF8Text();

        // 去除最后一个字符
        full_result = result;
        if (!full_result.empty())
        {
            full_result = full_result.substr(0, full_result.size() - 1);
        }
        std::cout << full_result << std::endl;

        // 释放识别结果内存
        delete[] result;

        // 显示图像
        cv::imshow("Camera Image", image);
        cv::waitKey(1);
    }
    catch (cv_bridge::Exception &e)
    {
        ROS_ERROR("cv_bridge exception: %s", e.what());
    }
}

int main(int argc, char **argv)
{
    // 设置编码
    setlocale(LC_ALL, "");

    // 初始化ROS节点
    ros::init(argc, argv, "nav_goal_publisher_subscriber");

    // 创建两个NodeHandle
    ros::NodeHandle nh, nh1;

    // 创建一个Publisher，将导航目标点消息发布到"/move_base_simple/goal"话题上
    ros::Publisher pub = nh.advertise<geometry_msgs::PoseStamped>("/move_base_simple/goal", 10);

    // 创建一个图像订阅者，订阅相机图像话题
    ros::Subscriber sub = nh1.subscribe("/camera/color/image_raw", 1, imageCallback);

    // 创建一个导航行为客户端
    MoveBaseClient ac("move_base", true);

    // 等待move_base服务器启动
    ROS_INFO("等待 move_base 服务器启动");
    ac.waitForServer();

        return 0;
}
