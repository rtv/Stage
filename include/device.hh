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
//  $Revision: 1.3 $
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
#define MSG4(m, a, b, c, d) printf("Msg   : %s : "m"\n", __PRETTY_FUNCTION__, a, b, c, d)

#if ENABLE_TRACE
    #define TRACE0(m)    printf("Debug : %s : "m"\n", __PRETTY_FUNCTION__)
    #define TRACE1(m, a) printf("Debug : %s : "m"\n", __PRETTY_FUNCTION__, a)
    #define TRACE2(m, a, b) printf("Debug : %s : "m"\n", __PRETTY_FUNCTION__, a, b)
    #define TRACE3(m, a, b, c) printf("Debug : %s : "m"\n", __PRETTY_FUNCTION__, a, b, c)
    #define TRACE4(m, a, b, c, d) printf("Debug : %s : "m"\n", __PRETTY_FUNCTION__, a, b, c, d)
#else
    #define TRACE0(m)
    #define TRACE1(m, a)
    #define TRACE2(m, a, b)
    #define TRACE3(m, a, b, c)
    #define TRACE4(m, a, b, c, d)
#endif


////////////////////////////////////////////////////////////////////////////////
// Define length-specific data types
//
#define BYTE unsigned char
#define UINT16 unsigned short

#define LOBYTE(w) ((BYTE) (w & 0xFF))
#define HIBYTE(w) ((BYTE) ((w >> 8) & 0xFF))
#define MAKEUINT16(lo, hi) ((((UINT16) (hi)) << 8) | ((UINT16) (lo)))


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
    // buffer points to a single buffer containing the data, command and configuration buffers.
    //
    protected: CDevice(void *buffer, size_t data_len, size_t command_len, size_t config_len);
        
    // Initialise the device
    //
    public: virtual bool Startup(CRobot *robot, CWorld *world);

    // Close the device
    //
    public: virtual bool Shutdown();

    // Update the device
    // This is pure virtual and *must* be overloaded
    //
    public: virtual bool Update() = 0;

    // See if the device is subscribed
    //
    public: bool IsSubscribed();
    
    // Write to the data buffer
    // Returns the number of bytes copied
    //
    protected: size_t PutData(void *data, size_t len);

    // Read from the command buffer
    // Returns the number of bytes copied
    //
    protected: size_t GetCommand(void *data, size_t len);

    // Read from the configuration buffer
    // Returns the number of bytes copied
    //
    protected: size_t GetConfig(void *data, size_t len);

    // Pointer to the robot and world objects
    //
    protected: CRobot *m_robot;
    protected: CWorld *m_world;

    // Pointer to shared info buffers
    //
    private: BYTE *m_info_buffer;
    private: size_t m_info_len;

    // Pointer to shared data buffers
    //
    private: void *m_data_buffer;
    private: size_t m_data_len;

    // Pointer to shared command buffers
    //
    private: void *m_command_buffer;
    private: size_t m_command_len;

    // Pointer to shared config buffers
    //
    private: void *m_config_buffer;
    private: size_t m_config_len;
};


#endif
