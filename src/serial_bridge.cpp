/*
 * Serial Bridge for Arduino Due
 * Created by Gioele Giunta
 * Date: 2025-02-25
 *
 * This file provides C-compatible functions to enable serial communication 
 * for C code using Arduino's Serial library.
 */

 #include <Arduino.h>

 extern "C" {
     // Initialize serial communication with the specified baud rate
     void serial_init(unsigned long baudrate) {
         Serial.begin(baudrate);
         while (!Serial) {
             ; 
         }
     }
 
     // Send a single character over the serial port
     void serial_write_char(char c) {
         Serial.write(c);
     }
 
     // Send a string over the serial port
     void serial_write_string(const char* str) {
         Serial.print(str);
     }

     void serial_write_formatted(const char* format, ...) {
        char buffer[128];
        va_list args;
    
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
    
        serial_write_string(buffer);
    }
 }
 