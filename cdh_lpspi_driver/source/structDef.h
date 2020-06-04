#ifndef SENSOR_STRUCT_DEF
#define SENSOR_STRUCT_DEF

// Define some sensor ID
typedef enum{
	magnetometer1,
	magnetometer2,
	magnetometer3,
	gyroscope1,
	gyroscope2,
	gyroscope3
}SensorID;

// Define a type of message to send to queue
typedef struct {
	SensorID id;
	// TODO find a better way to define the dataset instead of
	// strictly saying there's own 3 variables. Don't use dynamic allocation after initialization
	float x;
	float y;
	float z;
	bool flag;  // If the sensor is faulty, then set flag to 0
} Data_t;

#endif
