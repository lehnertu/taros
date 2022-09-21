/*
    Global definitions that are needed throuout the system
*/

#pragma once

#include <string>

#include "logger.h"
#include "file_writer.h"

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
extern bool SD_card_OK;

// this is the run number used to name log files
// it is assigned when opening the 'taros.0000.system.log'
// and should be used for all other log files of the same run
extern int SD_file_No;

// miliseconds since program start (about 50 days capacity)
uint32_t FC_time_now();

// report the time elapsed since the timestamp
// if current FC_systick_millis_count is small than timestamp wrap-around
uint32_t FC_elapsed_millis(uint32_t timestamp);

std::string hexbyte(char c);
