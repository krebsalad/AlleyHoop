#include "alleyhoop_master/alleyhoop_controller.h"
#include <sstream>
#include <iostream>

namespace AutonomousDriving
{

    AlleyHoopController::AlleyHoopController(ros::NodeHandle* _nh, Vehicle* v)
	: Controller(v), nh(*_nh)
    {
    }

    void AlleyHoopController::update()
    {
        if (ros::ok())
        {
            // print text
            std::stringstream ss;
            ss << "controller running..";
            ROS_INFO("%s", ss.str().c_str());
            ros::spinOnce();
        }
        else
        {
            std::cout << "ros was not running!" << std::endl;
        }
    }

}
