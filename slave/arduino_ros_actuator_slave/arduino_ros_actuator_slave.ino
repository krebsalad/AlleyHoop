//ros includes
#include <ros.h>
#include <std_msgs/Bool.h>
#include <std_msgs/Int16.h>

#include "arduino_motors.h"

//ros node (only one possible on arduino)
ros::NodeHandle nodeHandle;

//led1 data
const int led13_pin = 13;
bool led13_state = false;
const long cmd_led13_timer_rate = 500; 
long cmd_led13_timer = millis() + cmd_led13_timer_rate;  //timer to check if led data was updated
void ledMessageCb( const std_msgs::Bool& msg)
{
  led13_state = msg.data;
  cmd_led13_timer = millis() + cmd_led13_timer_rate; //reset the timer since new data has arrived
}
ros::Subscriber<std_msgs::Bool> led13_sub("/arduino_actuator_slave/led13", &ledMessageCb );


//steering data
int side = 0;
const long cmd_side_timer_rate = 150;
long cmd_side_timer = millis() + cmd_side_timer_rate; //timer to check if motor data was updated
void sideMessageCb( const std_msgs::Int16& msg)
{
  cmd_side_timer = millis() + cmd_side_timer_rate; //reset the timer since new data has arrived
  side = msg.data;
}
ros::Subscriber<std_msgs::Int16> side_sub("/arduino_actuator_slave/side", &sideMessageCb );

//velocity data
int direction = 0;
const long cmd_direction_timer_rate = 150;
long cmd_direction_timer = millis() + cmd_direction_timer_rate; //timer to check if motor data was updated
void directionMessageCb( const std_msgs::Int16& msg)
{
  cmd_direction_timer = millis() + cmd_direction_timer_rate; //reset the timer since new data has arrived
  direction = msg.data;
}
ros::Subscriber<std_msgs::Int16> direction_sub("/arduino_actuator_slave/direction", &directionMessageCb );


//led 1 routine function
void update_actuators()
{
  //update led state
  if(led13_state && millis() < cmd_led13_timer)
  {
    digitalWrite(led13_pin, HIGH);
  }
  else
  {
    digitalWrite(led13_pin, LOW);
  }

  //update motors if received messages
  if(millis() < cmd_direction_timer && millis() < cmd_side_timer)
  {
    update_motors(direction, side);
  }
  else
  {
    //stop motors
    update_motors(0, 0);
  }
  
}

//arduino setup
void setup()
{
  //node init
  nodeHandle.initNode();

  //setup led
  pinMode(led13_pin, OUTPUT);
  nodeHandle.subscribe(led13_sub);  

  //setup motors
  setup_motors();
  nodeHandle.subscribe(direction_sub);
  nodeHandle.subscribe(side_sub);
}

//routine
void loop()
{
  //update actuators based subscribed data
  update_actuators();

  //callback and make sure received important messages
  nodeHandle.spinOnce();
}