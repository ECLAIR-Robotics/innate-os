#include <cstdio>
#include <string>
#include <chrono>

#include "rclcpp/rclcpp.hpp" // imports entire rclcpp API, narrow down later
#include "geometry_msgs/msg/twist.hpp" // The Twist message type expresses linear and angular velocity 
#include "geometry_msgs/msg/vector3.hpp"

#include "geometry_msgs/msg/twist_with_covariance.hpp"
#include "geometry_msgs/msg/pose_with_covariance.hpp"
#include "geometry_msgs/msg/pose.hpp"
#include "geometry_msgs/msg/point.hpp"

#include "nav_msgs/msg/odometry.hpp"

using namespace std::chrono_literals;

class CircleMoverNode : public rclcpp::Node{

    public:
        CircleMoverNode() : Node("circle_mover") { // need to actually give the Node constructor the right signature
            publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10); // 10 means limit backup queue to 10 messages, "KeepLast" scheme by detail (this is called QoS) 
            timer_ = this->create_wall_timer(50ms, std::bind(&CircleMoverNode::timer_velocity_callback, this));
            
            subscription_ = this->create_subscription<nav_msgs::msg::Odometry>(
            "/odom", 10, std::bind(&CircleMoverNode::odom_callback, this, std::placeholders::_1)); 

        }

    private:
        geometry_msgs::msg::Twist nextVelocity; // MARS subscribes to /cmd_vel topic to get this.         
        rclcpp::TimerBase::SharedPtr timer_;
        rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;

        nav_msgs::msg::Odometry currentOdometry; // MARS publishes this to /odom topic
        rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr subscription_;

        void timer_velocity_callback(){
            nextVelocity = geometry_msgs::msg::Twist();
            nextVelocity.linear.x = 0.5;
            nextVelocity.angular.z = 0.3;
            publisher_->publish(nextVelocity);
            RCLCPP_INFO(this->get_logger(), "PUBLISHING Twist with linear (%f, 0, 0) and angular (0, 0, %f)", nextVelocity.linear.x, nextVelocity.angular.z);
        }

        void odom_callback(const nav_msgs::msg::Odometry& odom){
            currentOdometry = odom; // hopefully this creates a deep copy
            geometry_msgs::msg::PoseWithCovariance currentPoseWCovariance = currentOdometry.pose; 
            // geometry_msgs::msg::TwistWithCovariance currentTwistWCovariance = currentOdometry.twist;
            
            geometry_msgs::msg::Pose currentPoseEstimate = currentPoseWCovariance.pose; 
            RCLCPP_INFO(this->get_logger(), 
            "Received PoseWithCovariance from /odom : Pose at position (%f, %f, %f)", 
            currentPoseEstimate.position.x, currentPoseEstimate.position.y, currentPoseEstimate.position.z);
        }

};

int main(int argc, char * argv[]){
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CircleMoverNode>());
    rclcpp::shutdown();
    return 0;
}