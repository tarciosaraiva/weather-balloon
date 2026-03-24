// GPSParser.cpp - Implementation of the GPSParser library
#include "GPSParser.h"

GPSReader::GPSReader(Stream &serial_port) {
  uart = &serial_port;
  message_buffer = "";
  last_data_time = 0;
  has_new_data = false;
  current_data = GPSData();
}

bool GPSReader::update() {
  has_new_data = false;
  unsigned long current_time = millis();
  
  // Check if timeout occurred with data in buffer
  if (current_time - last_data_time > timeout_ms && message_buffer.length() > 0) {
    _process_buffer();
    has_new_data = true;
  }
  
  // Check if new data is available - only read what's currently available
  int bytes_available = uart->available();
  if (bytes_available > 0) {
    // Read ONLY the currently available data - this is the key non-blocking change
    String data = "";
    for (int i = 0; i < bytes_available; i++) {
      char c = uart->read();
      data += c;
    }
    
    // If buffer is empty or we recently received data, append to buffer
    if (message_buffer.length() == 0 || current_time - last_data_time <= timeout_ms) {
      message_buffer += data;
    } else {
      // If timeout occurred, process the old buffer first
      _process_buffer();
      // Then start a new buffer
      message_buffer = data;
      has_new_data = true;
    }
    
    // Update last data time
    last_data_time = current_time;
  }
  
  return has_new_data;
}

GPSData GPSReader::get_data() {
  update();
  return current_data;
}

void GPSReader::_process_buffer() {
  if (message_buffer.length() == 0) {
    return;
  }
  
  current_data = _process_nmea_data(message_buffer);
  message_buffer = "";
}

// Convenience methods for direct access to GPS data
float GPSReader::latitude() {
  update();
  return current_data.latitude;
}

float GPSReader::longitude() {
  update();
  return current_data.longitude;
}

float GPSReader::altitude() {
  update();
  return current_data.altitude;
}

bool GPSReader::hasFix() {
  update();
  return current_data.has_fix;
}

int GPSReader::satellites() {
  update();
  return current_data.satellites;
}

float GPSReader::speed() {
  update();
  return current_data.speed_knots;
}

String GPSReader::time() {
  update();
  return current_data.time;
}

String GPSReader::date() {
  update();
  return current_data.date;
}

// For backward compatibility
GPSData parse_gps_data(String nmea_chunk) {
  GPSReader tmp(Serial);
  return tmp._process_nmea_data(nmea_chunk);
}

GPSData GPSReader::_process_nmea_data(String nmea_data) {
  // Initialize data class
  GPSData gps_data;
  
  // Split the chunk into individual NMEA sentences
  int start = 0;
  int end = nmea_data.indexOf('$', start + 1);
  
  while (start < nmea_data.length()) {
    if (end == -1) {
      end = nmea_data.length();
    }
    
    String sentence = nmea_data.substring(start, end);
    sentence = sentence.trim();
    
    // Parse different sentence types
    if (sentence.startsWith("$GPRMC")) {
      _parse_rmc(sentence, gps_data);
    } else if (sentence.startsWith("$GPGGA")) {
      _parse_gga(sentence, gps_data);
    } else if (sentence.startsWith("$GPGSA")) {
      _parse_gsa(sentence, gps_data);
    }
    
    // Update for next iteration
    start = end;
    end = nmea_data.indexOf('$', start + 1);
  }
  
  return gps_data;
}

void GPSReader::_parse_rmc(String sentence, GPSData &gps_data) {
  // Split the sentence into parts
  int comma_indices[13];
  int comma_count = 0;
  
  // Find positions of all commas
  for (int i = 0; i < sentence.length() && comma_count < 13; i++) {
    if (sentence.charAt(i) == ',') {
      comma_indices[comma_count++] = i;
    }
  }
  
  if (comma_count < 11) {
    return;
  }
  
  // Check if we have a fix
  String status = sentence.substring(comma_indices[1] + 1, comma_indices[2]);
  if (status == "A") {
    gps_data.has_fix = true;
  } else {
    gps_data.has_fix = false;
    // Don't return here, continue to extract time and date
  }
  
  // Extract time (format: HHMMSS.SS) with error handling
  String time_str = sentence.substring(comma_indices[0] + 1, comma_indices[1]);
  if (time_str.length() >= 6) {
    String hour = time_str.substring(0, 2);
    String minute = time_str.substring(2, 4);
    String second = time_str.substring(4);
    gps_data.time = hour + ":" + minute + ":" + second;
  }
  
  // Extract date (format: DDMMYY) with error handling
  String date_str = sentence.substring(comma_indices[8] + 1, comma_indices[9]);
  if (date_str.length() >= 6) {
    String day = date_str.substring(0, 2);
    String month = date_str.substring(2, 4);
    String year = "20" + date_str.substring(4, 6);  // Assuming we're in the 2000s
    gps_data.date = day + "/" + month + "/" + year;
  }
  
  // Only extract position and speed if we have a valid fix
  if (gps_data.has_fix) {
    // Extract latitude and longitude with sign based on direction
    String lat_str = sentence.substring(comma_indices[2] + 1, comma_indices[3]);
    String lat_dir = sentence.substring(comma_indices[3] + 1, comma_indices[4]);
    String lon_str = sentence.substring(comma_indices[4] + 1, comma_indices[5]);
    String lon_dir = sentence.substring(comma_indices[5] + 1, comma_indices[6]);
    
    if (lat_str.length() > 0 && lon_str.length() > 0) {
      // Latitude
      float lat_deg = lat_str.substring(0, 2).toFloat();
      float lat_min = lat_str.substring(2).toFloat();
      float lat_decimal = lat_deg + (lat_min / 60);
      
      // Apply sign based on direction (N is positive, S is negative)
      if (lat_dir == "S") {
        lat_decimal = -lat_decimal;
      }
      gps_data.latitude = lat_decimal;
      
      // Longitude
      float lon_deg = lon_str.substring(0, 3).toFloat();
      float lon_min = lon_str.substring(3).toFloat();
      float lon_decimal = lon_deg + (lon_min / 60);
      
      // Apply sign based on direction (E is positive, W is negative)
      if (lon_dir == "W") {
        lon_decimal = -lon_decimal;
      }
      gps_data.longitude = lon_decimal;
    }
    
    // Extract speed in knots
    String speed_str = sentence.substring(comma_indices[6] + 1, comma_indices[7]);
    if (speed_str.length() > 0) {
      gps_data.speed_knots = speed_str.toFloat();
    }
  }
}

void GPSReader::_parse_gga(String sentence, GPSData &gps_data) {
  // Split the sentence into parts
  int comma_indices[16];
  int comma_count = 0;
  
  // Find positions of all commas
  for (int i = 0; i < sentence.length() && comma_count < 16; i++) {
    if (sentence.charAt(i) == ',') {
      comma_indices[comma_count++] = i;
    }
  }
  
  if (comma_count < 14) {
    return;
  }
  
  // Extract number of satellites
  String sats_str = sentence.substring(comma_indices[6] + 1, comma_indices[7]);
  if (sats_str.length() > 0) {
    gps_data.satellites = sats_str.toInt();
  }
  
  // Extract HDOP (Horizontal Dilution of Precision)
  String hdop_str = sentence.substring(comma_indices[7] + 1, comma_indices[8]);
  if (hdop_str.length() > 0) {
    gps_data.hdop = hdop_str.toFloat();
  }
  
  // Extract altitude
  String alt_str = sentence.substring(comma_indices[8] + 1, comma_indices[9]);
  String alt_unit = sentence.substring(comma_indices[9] + 1, comma_indices[10]);
  if (alt_str.length() > 0 && alt_unit == "M") {
    gps_data.altitude = alt_str.toFloat();
  }
}

void GPSReader::_parse_gsa(String sentence, GPSData &gps_data) {
  // Split the sentence into parts
  int comma_indices[19];
  int comma_count = 0;
  
  // Find positions of all commas
  for (int i = 0; i < sentence.length() && comma_count < 19; i++) {
    if (sentence.charAt(i) == ',') {
      comma_indices[comma_count++] = i;
    }
  }
  
  if (comma_count < 17) {
    return;
  }
  
  // Extract PDOP (Position Dilution of Precision)
  String pdop_str = sentence.substring(comma_indices[14] + 1, comma_indices[15]);
  if (pdop_str.length() > 0) {
    gps_data.pdop = pdop_str.toFloat();
  }
  
  // Extract HDOP (Horizontal Dilution of Precision)
  String hdop_str = sentence.substring(comma_indices[15] + 1, comma_indices[16]);
  if (hdop_str.length() > 0) {
    gps_data.hdop = hdop_str.toFloat();
  }
  
  // Extract VDOP (Vertical Dilution of Precision)
  String vdop_part = sentence.substring(comma_indices[16] + 1);
  // Remove checksum part
  int asterisk_pos = vdop_part.indexOf('*');
  String vdop_str = asterisk_pos > 0 ? vdop_part.substring(0, asterisk_pos) : vdop_part;
  
  if (vdop_str.length() > 0) {
    gps_data.vdop = vdop_str.toFloat();
  }
}
