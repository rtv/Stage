///////////////////////////////////////////////////////////////////////////
//
// File: device.hh
// Author: Andrew Howard
// Date: 28 Nov 2000
// Desc: Base class for all devices
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/device.hh,v $
//  $Author: ahoward $
//  $Revision: 1.1 $
//
// Usage:
//  (empty)
//
// Theory of operation:
//  (empty)
//
// Known bugs:
//  (empty)
//
// Possible enhancements:
//  (empty)
//
///////////////////////////////////////////////////////////////////////////

#ifndef DEVICE_HH
#define DEVICE_HH

// For size_t
//
#include <stddef.h>


////////////////////////////////////////////////////////////////////////////////
// Misc useful macros

#define ARRAYSIZE(m) sizeof(m) / sizeof(m[0])


////////////////////////////////////////////////////////////////////////////////
// Error, msg, trace macros

#define ENABLE_TRACE 1

#include <assert.h>

#define ASSERT(m) assert(m)
#define VERIFY(m) assert(m)

#define ERROR(m)  printf("Error : %s : %s\n", __PRETTY_FUNCTION__, m)
#define MSG(m)       printf("Msg   : %s : "m"\n", __PRETTY_FUNCTION__)
#define MSG1(m, a)   printf("Msg   : %s : "m"\n", __PRETTY_FUNCTION__, a)
#define MSG2(m, a, b) printf("Msg   : %s : "m"\n", __PRETTY_FUNCTION__, a, b)
#define MSG3(m, a, b, c) printf("Msg   : %s : "m"\n", __PRETTY_FUNCTION__, a, b, c)

#if ENABLE_TRACE
    #define TRACE0(m)    printf("Debug : %s : "m"\n", __PRETTY_FUNCTION__)
    #define TRACE1(m, a) printf("Debug : %s : "m"\n", __PRETTY_FUNCTION__, a)
    #define TRACE2(m, a, b) printf("Debug : %s : "m"\n", __PRETTY_FUNCTION__, a, b)
    #define TRACE3(m, a, b, c) printf("Debug : %s : "m"\n", __PRETTY_FUNCTION__, a, b, c)
#else
    #define TRACE0(m)
    #define TRACE1(m, a)
    #define TRACE2(m, a, b)
    #define TRACE3(m, a, b, c)
#endif


////////////////////////////////////////////////////////////////////////////////
// Define length-specific data types
//
#define BYTE unsigned short
#define WORD16 unsigned short


////////////////////////////////////////////////////////////////////////////////
// Define some usefull math stuff
//
#ifndef M_PI
	#define M_PI        3.14159265358979323846
#endif

// Convert radians to degrees
//
#define RTOD(r) ((r) * 180 / M_PI)

// Convert degrees to radians
//
#define DTOR(d) ((d) * M_PI / 180)


////////////////////////////////////////////////////////////////////////////////
// Forward declare the world and robot classes
//
class CWorld;
class CRobot;


////////////////////////////////////////////////////////////////////////////////
// Base class for all simulated devices
//
class CDevice
{
    // Default constructor
    // This is an abstract class and *cannot* be instantiated directly
    //
    protected: CDevice(void *data_buffer, size_t data_len);
        
    // Initialise the device
    //
    public: virtual bool Open(CRobot *robot, CWorld *world);

    // Close the device
    //
    public: virtual bool Close();

    // Update the device
    // This is pure virtual and *must* be overloaded
    //
    public: virtual bool Update() = 0;

    // Write to the data buffer
    //
    protected: void PutData(void *data, size_t len);

    // Read from the command buffer
    //
    protected: void GetCommand(void *data, size_t len);

    // Read from the configuration buffer
    //
    protected: void GetConfig(void *data, size_t len);

    // Pointer to the robot and world objects
    //
    protected: CRobot *m_robot;
    protected: CWorld *m_world;

    // Pointer to shared mem buffers
    //
    private: void *m_data_buffer;
    private: size_t m_data_len;
};


#endif
