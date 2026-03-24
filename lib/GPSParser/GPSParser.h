// GPSParser.h - A simple library for parsing NMEA GPS data
// For use with UART GPS modules on Arduino
// Converted from MicroPython implementation

#ifndef GPSParser_h
#define GPSParser_h

#include <Arduino.h>
#include <SoftwareSerial.h>

class GPSData {
  public:
    GPSData() {
      has_fix = false;
      latitude = 0.0;
      longitude = 0.0;
      speed_knots = 0.0;
      time = "";
      date = "";
      satellites = 0;
      altitude = 0.0;
      hdop = 0.0;
      pdop = 0.0;
      vdop = 0.0;
    }
    
    bool has_fix;
    float latitude;
    float longitude;
    float speed_knots;
    String time;
    String date;
    int satellites;
    float altitude;
    float hdop;  // Horizontal Dilution of Precision
    float pdop;  // Position Dilution of Precision
    float vdop;  // Vertical Dilution of Precision
};

class GPSReader {
  private:
    Stream* uart;
    String message_buffer;
    unsigned long last_data_time;
    const unsigned long timeout_ms = 500;  // 500ms timeout between message parts
    GPSData current_data;
    bool has_new_data;
    
    void _process_buffer();
    GPSData _process_nmea_data(String nmea_data);
    void _parse_rmc(String sentence, GPSData &gps_data);
    void _parse_gga(String sentence, GPSData &gps_data);
    void _parse_gsa(String sentence, GPSData &gps_data);
    
  public:
    GPSReader(Stream &serial_port);
    bool update();
    GPSData get_data();
    
    // Convenience methods for direct access to GPS data
    float latitude();
    float longitude();
    float altitude();
    bool hasFix();
    int satellites();
    float speed();
    String time();
    String date();
};

// For backward compatibility
GPSData parse_gps_data(String nmea_chunk);

#endif
