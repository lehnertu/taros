/*
    Global definitions that are needed throuout the system
*/

#pragma once

#include "logger.h"
#include "file_writer.h"

#ifdef USE_USB_SERIAL
# include "usb_serial.h"
// we declare an alias of the Serial port
// the actual definition is in main.cpp
extern usb_serial_class usb_serial_debug;
#endif

// This is a system-wide available logger for system messages.
// It can be used by all modules to log system information.
// Depending on the setup the messages will be written to a file 
// and/or console or telemetry output when the task loop has started.
// -- actually defined in main.cpp --
extern Logger* system_log;

// This is the file writer, where the system log messages are stored.
// -- actually defined in main.cpp --
extern FileWriter* system_log_file_writer;

// it the SD card has been found and initialized
// -- actually defined in main.cpp --
extern bool SD_card_OK;

// this is the run number used to name log files
// it is assigned when opening the 'taros.0000.system.log'
// and should be used for all other log files of the same run
// -- actually defined in main.cpp --
extern int SD_file_No;

