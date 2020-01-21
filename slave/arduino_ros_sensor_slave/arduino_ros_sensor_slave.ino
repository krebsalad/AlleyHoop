//ros includes
#include <ros.h>
#include <ros/time.h>
#include <std_msgs/Bool.h>
#include <std_msgs/UInt8.h>
#include <sensor_msgs/Imu.h>

#include "arduino_imu.h"
#include "arduino_ultrasonic_sensor.h"

//ros node (only one possible on arduino)
ros::NodeHandle nodeHandle;

//ultrasonic data
long ultrasonic_sensor_data = 0;
long ultrasonic_pub_timer = millis() + 1000;
std_msgs::UInt8 ultrasonic_msg;
ros::Publisher ultrasonic_pub("/arduino_sensor_slave/ultrasonic_sensor", &ultrasonic_msg);

//led1 data
int led13_pin = 13;
bool led13_state = false;
void messageCb( const std_msgs::Bool& msg)
{
  led13_state = msg.data;
}
ros::Subscriber<std_msgs::Bool> led13_sub("/arduino_sensor_slave/led13", &messageCb );

//imu data
sensor_msgs::Imu imu_msg;
long imu_pub_timer = millis() + 1000;
ros::Publisher imu_pub("/arduino_sensor_slave/imu_sensor", &imu_msg);

//ultrasonic routine function
void update_sensors()
{
  //publish ultrasonic data
  if(millis() > ultrasonic_pub_timer)
  {
    //update data
    update_ultrasonic_sensor( ultrasonic_sensor_data );

    //publish data
    ultrasonic_msg.data = ultrasonic_sensor_data;
    millis();
    ultrasonic_pub.publish( &ultrasonic_msg );
    ultrasonic_pub_timer = millis() + 1000;
  }

  //update imu data
  if(millis() > imu_pub_timer)
  {
    //update data
    update_imu();

    //publish data
    imu_msg.header.frame_id = 0;
    imu_msg.header.stamp = nodeHandle.now();
    
    imu_msg.orientation_covariance[0] = -1;
    imu_msg.orientation.w = 1;
    imu_msg.orientation.x = Mxyz[0];
    imu_msg.orientation.y = Mxyz[1];
    imu_msg.orientation.z = Mxyz[2];

    imu_msg.linear_acceleration_covariance[0] = -1;
    imu_msg.linear_acceleration.x = Axyz[0];
    imu_msg.linear_acceleration.y = Axyz[1];
    imu_msg.linear_acceleration.z = Axyz[2];

    imu_msg.angular_velocity_covariance[0] = -1;
    imu_msg.angular_velocity.x = Gxyz[0];
    imu_msg.angular_velocity.y = Gxyz[1];
    imu_msg.angular_velocity.z = Gxyz[2];

    //publish data
    millis();
    imu_pub.publish( &imu_msg );
    imu_pub_timer = millis() + 1000;
  }
}

//led 1 routine function
void update_actuators()
{
  //update led state
  if(led13_state)
    digitalWrite(led13_pin, HIGH);
  else
    digitalWrite(led13_pin, LOW);
}

//arduino setup
void setup()
{
  //node init
  nodeHandle.initNode();

  //setup ultrasonic sensor
  setup_ultrasonic_sensor();
  nodeHandle.advertise(ultrasonic_pub);

  //setup imu
  setup_imu();
  nodeHandle.advertise(imu_pub);

  //setup led
  pinMode(led13_pin, OUTPUT);
  nodeHandle.subscribe(led13_sub);  

}

//routine
void loop()
{
  //update actuators based subscribed data
  update_actuators();

  //update sensors and publish data
  update_sensors();

  //callback and send data
  nodeHandle.spinOnce();

}