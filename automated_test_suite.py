import serial
import time
import unittest

"""
Automated Hardware-in-the-Loop (HIL) Test Suite
Target: STM32 Firmware Controller
Author: Abdelrhman Mabrouk
"""

# Serial Configuration
SERIAL_PORT = 'COM3'
BAUD_RATE = 115200

class FirmwareIntegrationTest(unittest.TestCase):
    
    @classmethod
    def setUpClass(cls):
        """Initialize Serial Connection before tests start"""
        try:
            cls.ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
            time.sleep(2) # Wait for MCU reset
            print(f"Connected to {SERIAL_PORT}")
        except serial.SerialException:
            print("Error: Could not open serial port.")
            raise

    @classmethod
    def tearDownClass(cls):
        """Close connection after tests"""
        if cls.ser.is_open:
            cls.ser.close()

    def send_command(self, cmd):
        """Helper to send bytes to MCU"""
        self.ser.write(cmd.encode())
        time.sleep(0.1) # Allow processing time

    def test_01_idle_to_init_transition(self):
        """Test Case 01: Verify system wakes up on START command"""
        print("\nRunning Test 01: Wake Up Sequence...")
        
        # Flush buffer
        self.ser.reset_input_buffer()
        
        # Send Start Command (Defined in firmware as 0x01 or 'S')
        self.send_command('S')
        
        # Read Response
        response = self.ser.readline().decode().strip()
        
        # Assert that the firmware acknowledged the state change
        self.assertIn("IDLE -> INIT", response, "Firmware did not transition to INIT state")
        print(" -> Test 01 PASSED")

    def test_02_safety_trigger(self):
        """Test Case 02: Verify system enters ERROR state on fault simulation"""
        print("\nRunning Test 02: Safety Shutdown...")
        
        # Simulate High Temp command
        self.send_command('F') # 'F' for Fault Simulation
        
        response = self.ser.readline().decode().strip()
        self.assertIn("ALERT", response, "System failed to detect fault condition")
        print(" -> Test 02 PASSED")

if __name__ == '__main__':
    unittest.main()
