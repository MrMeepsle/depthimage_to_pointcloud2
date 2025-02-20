// Copyright 2017 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <opencv2/imgproc/imgproc.hpp>

#include <depthimage_to_pointcloud2/depth_conversions.hpp>

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/image_encodings.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <sensor_msgs/point_cloud2_iterator.hpp>
#include <message_filters/subscriber.h>

using std::placeholders::_1;
using namespace message_filters;

class Depthimage2Pointcloud2 : public rclcpp::Node
{
public:
    Depthimage2Pointcloud2() : Node("depthimage_to_pointcloud2_node")
    {
        range_max = this->declare_parameter("range_max", 0.0);
        use_quiet_nan = this->declare_parameter("use_quiet_nan", true);

        g_pub_point_cloud = this->create_publisher<sensor_msgs::msg::PointCloud2>("pointcloud2", 10);

        cam_info_sub = this->create_subscription<sensor_msgs::msg::CameraInfo>("depth_camera_info", 10, std::bind(&Depthimage2Pointcloud2::infoCb, this, _1));
        depth_sub = this->create_subscription<sensor_msgs::msg::Image>("depth", 10, std::bind(&Depthimage2Pointcloud2::callback, this, _1));
    }

private:
    void callback(const sensor_msgs::msg::Image::ConstSharedPtr& depth_msg)
    {
        if (nullptr == g_cam_info) {
            RCLCPP_WARN(this->get_logger(), "No camera info, skipping point cloud conversion");
            return;
        }

        sensor_msgs::msg::PointCloud2::SharedPtr cloud_msg = std::make_shared<sensor_msgs::msg::PointCloud2>();
        cloud_msg->header = depth_msg->header;
        cloud_msg->height = depth_msg->height;
        cloud_msg->width = depth_msg->width;
        cloud_msg->is_dense = false;
        cloud_msg->is_bigendian = false;
        sensor_msgs::PointCloud2Modifier pcd_modifier(*cloud_msg);
        pcd_modifier.setPointCloud2FieldsByString(1, "xyz");

        image_geometry::PinholeCameraModel model;
        model.fromCameraInfo(g_cam_info);

        if (depth_msg->encoding == sensor_msgs::image_encodings::TYPE_16UC1) {
            depthimage_to_pointcloud2::convert<uint16_t>(depth_msg, cloud_msg, model, range_max, use_quiet_nan);
        } else if (depth_msg->encoding == sensor_msgs::image_encodings::TYPE_32FC1) {
            depthimage_to_pointcloud2::convert<float>(depth_msg, cloud_msg, model, range_max, use_quiet_nan);
        } else {
            RCLCPP_WARN(this->get_logger(), "Depth image has unsupported encoding [%s]", depth_msg->encoding.c_str());
            return;
        }

        g_pub_point_cloud->publish(*cloud_msg);
    }

    void infoCb(sensor_msgs::msg::CameraInfo::SharedPtr info)
    {
        g_cam_info = info;
    }

    double range_max;
    bool use_quiet_nan;
    bool colorful;

    sensor_msgs::msg::CameraInfo::SharedPtr g_cam_info;
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr g_pub_point_cloud;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr depth_sub;
    rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr cam_info_sub;
};

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Depthimage2Pointcloud2>());
    rclcpp::shutdown();
    return 0;
}
