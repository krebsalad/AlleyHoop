#ifndef VEHICLE_H_
#define VEHICLE_H_

namespace AutonomousDriving
{

    class Vehicle
    {
        public:
            virtual ~Vehicle() = default;
            virtual void update() = 0;
	    protected:
	        Vehicle() {};
    };

}

#endif //! VEHICLE_H_
