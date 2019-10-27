/*
 * Project lab3-wifi-tempsensor
 * Description:
 * Author:
 * Date:
 */

SYSTEM_THREAD(ENABLED);

SYSTEM_MODE(SEMI_AUTOMATIC);

#include "include/sensor.h"
#include "include/display-oled.h"
#include "include/serial-monitor.h"
#include "include/net-wifi.h"

#define BOARD_SYSTEM_LED                D7

#define SYSTEM_HEARTBEAT_MS             (250)

#define SYSTEM_DISPLAY_UPDATE_MS        (5000)

#define WIFI_AP_NAME    "Ramirez"
#define WIFI_AP_PASSW   "rusty2009"

typedef enum {
    SYS_STATE_INIT,
    SYS_STATE_NET_CONNECT,
    SYS_STATE_NET_PENDING,
    SYS_STATE_NET_READY
}SystemState_t;

static SystemState_t _systemState = SYS_STATE_INIT;

// Temperature sensor Init and timer objects
Timer timerSystemUIupdate(SYSTEM_DISPLAY_UPDATE_MS, SystemUIrefresh);

// Display Average Temperature
void SystemUIrefresh(void)
{
    // Get the average Temperature from the sensor task
    // Update OLED display
    DisplayOLEDupdateTemp(SensorTempGetAverage());
}


// System heartbeat loop()
// Flash Board LED to indicate loop() execution
void SystemHeartbeat(void)
{
    static bool firstTime = true;
    static bool ledState = true;

    if (firstTime)
    {
        pinMode(BOARD_SYSTEM_LED, OUTPUT);
        firstTime = false;
        digitalWrite(BOARD_SYSTEM_LED, HIGH);
    }
    else
    {
        if (ledState)
        {
            digitalWrite(BOARD_SYSTEM_LED, LOW);
            ledState = false;
        }
        else
        {
            digitalWrite(BOARD_SYSTEM_LED, HIGH);
            ledState = true;
        }
    }

    delay(SYSTEM_HEARTBEAT_MS);
}

void SystemStateProc(void)
{
    switch(_systemState)
    {
        case SYS_STATE_INIT:
            _systemState = SYS_STATE_NET_CONNECT;
            break;

        case SYS_STATE_NET_CONNECT:
            // Start WiFi connection
            NetWifiStart(WIFI_AP_NAME, WIFI_AP_PASSW);
            _systemState = SYS_STATE_NET_PENDING;
            break;

        case SYS_STATE_NET_PENDING:
            // Check if we are connected to WiFi AP
            if(NetWifiReady())
            {
                _systemState = SYS_STATE_NET_READY;
            }
            break;

        case SYS_STATE_NET_READY:
            // Connected to WiFi AP
            break;
    }

    SystemHeartbeat();

    // Run the Serial Monitor processor
    // Check for incoming commands from serial port
    SerialMonitorInputProc();
}

// setup() runs once, when the device is first turned on.
void setup()
{
    // Put initialization like pinMode and begin functions here.

    // Start the Serial Monitor 
    SerialMonitorInit(NULL);
    
    // Initialize OLED display
    DisplayOLEDinit();
    
    // Start Sensor task
    SensorTempInit();
    
    // Start UI refresh timer
    // Update the display (OLED) every SYSTEM_DISPLAY_UPDATE_MS
    timerSystemUIupdate.start();
}

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
    // The core of your code will likely live here.
    SystemStateProc();
}