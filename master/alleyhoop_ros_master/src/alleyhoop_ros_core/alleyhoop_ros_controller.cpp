#include "alleyhoop_ros_core/alleyhoop_ros_controller.h"
#include "alleyhoop_ros_core/alleyhoop_ros_vehicle.h"
#include "alleyhoop_ros_utils/alleyhoop_ros_feature.h"
#include "alleyhoop_ros_core/alleyhoop_ros_feature_finder.h"

#include <sstream>
#include <iostream>
#include <list>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/PointCloud2.h>


namespace AlleyHoopROSCore
{
    bool Controller::verboseMode = false;

    Controller::Controller(ros::NodeHandle* _nh, AlleyHoopMVC::Vehicle* v)
	: AlleyHoopMVC::Controller(v), nh(*_nh)
    {
        //read params, if fail set a default value
        std::string ultrasonic_sensor_topic_name;
        if(!nh.getParam("AlleyHoop_Sensors/UltrasonicSensor_TopicName",ultrasonic_sensor_topic_name))
            ultrasonic_sensor_topic_name = "/arduino_slave/ultrasonic_sensor";
        
        std::string mono_camera_image_raw_topic_name;
        if(!nh.getParam("AlleyHoop_Sensors/MonoCameraImageRaw_TopicName",mono_camera_image_raw_topic_name))
            mono_camera_image_raw_topic_name = "/raspi_camera/image_raw";

        std::string mono_camera_info_topic_name;
        if(!nh.getParam("AlleyHoop_Sensors/MonoCameraInfo_TopicName",mono_camera_info_topic_name))
            mono_camera_info_topic_name = "/raspi_camera/camera_info";

        std::string lidar_topic_name;
        if(!nh.getParam("AlleyHoop_Sensors/Lidar_TopicName",lidar_topic_name))
            lidar_topic_name = "/scan";

        std::string depth_camera_image_raw_topic_name;
        if(!nh.getParam("AlleyHoop_Sensors/DepthCameraImageRaw_TopicName",depth_camera_image_raw_topic_name))
            depth_camera_image_raw_topic_name = "/depth_camera/image_raw";

        std::string depth_camera_info_topic_name;
        if(!nh.getParam("AlleyHoop_Sensors/DepthCameraInfo_TopicName",depth_camera_info_topic_name))
            depth_camera_info_topic_name = "/depth_camera/camera_info";

        std::string depth_camera_pcl_topic_name;
        if(!nh.getParam("AlleyHoop_Sensors/DepthCameraPcl_TopicName",depth_camera_pcl_topic_name))
            depth_camera_pcl_topic_name = "/depth_camera/points";

        std::string imu_topic_name;
        if(!nh.getParam("AlleyHoop_Sensors/ImuSensor_TopicName",imu_topic_name))
            imu_topic_name = "/arduino_slave/imu_sensor";
        
        //setup sensors, add to controller base class for life line managing and update routine
        ultrasonic_sensor_1 = new AlleyHoopROSSensors::UltrasonicSensor("ultrasonic_sensor", _nh, ultrasonic_sensor_topic_name);
        addSensor(ultrasonic_sensor_1);

        mono_camera_1 = new AlleyHoopROSSensors::MonoCamera("mono_camera_1", _nh, mono_camera_image_raw_topic_name, mono_camera_info_topic_name);
        addSensor(mono_camera_1);

        lidar1 = new AlleyHoopROSSensors::Lidar("lidar1", _nh, lidar_topic_name);
        addSensor(lidar1);

        depth_camera_1 = new AlleyHoopROSSensors::DepthCamera("depth_camera_1", _nh, depth_camera_image_raw_topic_name, depth_camera_pcl_topic_name, depth_camera_info_topic_name);
        addSensor(depth_camera_1);

        imu = new AlleyHoopROSSensors::Imu("imu", _nh, imu_topic_name);
        addSensor(imu);

        //setup feature finders
        featureFinder = new FeatureFinder(_nh);
    }

    Controller::~Controller()
    {
        if(featureFinder != nullptr)
        {
            std::cout << "deleted finder" << std::endl;
            delete featureFinder;
        }
    }

    bool Controller::update()
    {
        if (ros::ok())
        {
            //update routinely
            updateSensors(); //used for conventional implementation // for ros the following line, 'spinOnce()' is called instead 
            ros::spinOnce();

            //get sensors data
            int ultrasonic_sensor_data = ultrasonic_sensor_1->getData();
            cv_bridge::CvImagePtr image_data_1 = mono_camera_1->getData();
            cv_bridge::CvImagePtr image_data_2 = depth_camera_1->getImageData();
            sensor_msgs::PointCloud2 pcl = depth_camera_1->getDepthData();

            //find objects
            std::list<AlleyHoopROSUtils::Feature*> objects;
            featureFinder->findObjectsOnImage(objects, image_data_1);
            featureFinder->processDepthDataOnFeatures(objects, pcl);

            //TODO base the signs on cropped versions of the found objects
            std::list<AlleyHoopROSUtils::Feature*> trafficSigns;
            featureFinder->findTrafficRulesOnImage(trafficSigns, image_data_1);

            //find the road
            std::list<AlleyHoopROSUtils::Feature*> roadFeatures;
            featureFinder->findRoadOnImage(roadFeatures, image_data_1);
            

            //make desicions based on features
            for(std::list<AlleyHoopROSUtils::Feature*>::iterator feature_iter = objects.begin(); feature_iter != objects.end(); feature_iter++)
            {
                if(verboseMode)
                {
                    std::cout << "found an object with type " + std::to_string((*feature_iter)->featureType) << std::endl;
                }
            }

            // control the actuators
            if(AlleyHoopROSCore::Vehicle* ah_vehicle = dynamic_cast<AlleyHoopROSCore::Vehicle*>(vehicle))
            {
                
                if(ultrasonic_sensor_data < 30 && ultrasonic_sensor_data > 0)
                {
                    //turn on led
                    ah_vehicle->led1->setState(true);
                }
                else
                {
                    //turn on led
                    ah_vehicle->led1->setState(false);
                }
            }

            //cleanup
            for(std::list<AlleyHoopROSUtils::Feature*>::iterator feature_iter = objects.begin(); feature_iter != objects.end(); feature_iter++)
            {
                delete (*feature_iter);
            }
            for(std::list<AlleyHoopROSUtils::Feature*>::iterator feature_iter = roadFeatures.begin(); feature_iter != objects.end(); feature_iter++)
            {
                delete (*feature_iter);
            }
            for(std::list<AlleyHoopROSUtils::Feature*>::iterator feature_iter = trafficSigns.begin(); feature_iter != objects.end(); feature_iter++)
            {
                delete (*feature_iter);
            }

            return true;
        }

        std::cout << "program terminated" << std::endl;
        return false;
    }

}
