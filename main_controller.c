/*
 * Project: Industrial Firmware Controller
 * Author: Abdelrhman Mabrouk
 * Description: Main control loop implementing a Finite State Machine (FSM)
 *              for robust system operation and error handling.
 */

#include "system_config.h"
#include "uart_driver.h"
#include "sensors.h"

// Define System States
typedef enum {
    STATE_IDLE,
    STATE_INITIALIZING,
    STATE_RUNNING,
    STATE_ERROR,
    STATE_SHUTDOWN
} SystemState_t;

// Global State Variable
SystemState_t currentState = STATE_IDLE;

void System_Init(void) {
    UART_Init(115200);
    Sensors_Init();
    Log_Message("System Booted Successfully.");
}

int main(void) {
    System_Init();
    uint8_t command = 0;

    while (1) {
        // Non-blocking State Machine
        switch (currentState) {
            
            case STATE_IDLE:
                // Wait for Start Command from Host/User
                if (UART_Receive(&command) == UART_OK) {
                    if (command == CMD_START) {
                        currentState = STATE_INITIALIZING;
                        Log_Message("Transition: IDLE -> INIT");
                    }
                }
                break;

            case STATE_INITIALIZING:
                // Calibrate Sensors
                if (Sensors_Calibrate() == SENSOR_OK) {
                    currentState = STATE_RUNNING;
                } else {
                    Log_Message("Error: Calibration Failed");
                    currentState = STATE_ERROR;
                }
                break;

            case STATE_RUNNING:
                // Main Control Loop
                float sensorData = Sensors_Read();
                
                // Safety Check: If temp exceeds limit, trigger safe shutdown
                if (sensorData > MAX_TEMP_LIMIT) {
                    currentState = STATE_ERROR;
                } else {
                    Actuator_Control(sensorData);
                }
                break;

            case STATE_ERROR:
                // Safe State: Stop all motors
                Actuator_Stop();
                UART_Transmit("ALERT: System Fault Detected\n");
                
                // Wait for manual reset
                if (UART_Receive(&command) == UART_OK && command == CMD_RESET) {
                    currentState = STATE_IDLE;
                }
                break;
        }
    }
}
