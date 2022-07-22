#include <cstdio>

#include "global.h"
#include "dummy_gps.h"

DummyGPS::DummyGPS(
    std::string name,
    float rate,
    float tm_rate )
{
    // copy the name
    id = name;
    runlevel_= MODULE_RUNLEVEL_INITALIZED;
    gps_rate = rate;
    telemetry_rate = tm_rate;
    startup_time = FC_time_now();
    flag_state_change = true;
    flag_update_pending = false;
    last_update = FC_time_now();
    flag_telemetry_pending = false;
    last_telemetry = FC_time_now();
    status_lock = false;
    // home position
    lat = 51.04943;
    lon = 13.89053;
    alt = 285.0;
    vx = 0.0;
    vy = 0.0;
    vz = 3.0;
    // we cannot send a status message that we are ready to run
    // because the port is not yet wired to any receiver
    runlevel_=MODULE_RUNLEVEL_OPERATIONAL;
}

bool DummyGPS::have_work()
{
    float elapsed = FC_elapsed_millis(last_update);
    flag_update_pending = (elapsed*gps_rate >= 1000.0);
    
    elapsed = FC_elapsed_millis(last_telemetry);
    flag_telemetry_pending = (elapsed*telemetry_rate >= 1000.0);

    return flag_state_change | flag_update_pending | flag_telemetry_pending;
}

#define DEGREE_PER_METER 9e-6
// int32_t random(void);
// delivers positive numbers running 0...2147483647
#define MAX_RANDOM 2147483648.0

void DummyGPS::run()
{
    
    if (flag_update_pending)
    {
        // time in seconds
        float elapsed = 0.001 * FC_elapsed_millis(last_update);
        // velocity damping
        vx -= 0.2 * vx * elapsed;
        vy -= 0.2 * vy * elapsed;
        vz -= 0.5 * vz * elapsed;
        // random velocity change
        vx += 0.5 * elapsed * random()/MAX_RANDOM;
        vy += 0.5 * elapsed * random()/MAX_RANDOM;
        vz += 1.0 * elapsed * random()/MAX_RANDOM;
        // position change
        lat += vy * elapsed * DEGREE_PER_METER;
        lon += vx * elapsed * DEGREE_PER_METER;
        alt += vz * elapsed;

        // TODO: send message
        
        last_update = FC_time_now();
        flag_update_pending = false;
    };

    if (flag_telemetry_pending)
    {
        char buffer[16];
        int n = snprintf(buffer, 15, "%.6f", lat);
        tm_out.transmit(
            Message::TelemetryMessage(id, FC_time_now(), "GPS_LAT", std::string(buffer,n)) );

        n = snprintf(buffer, 15, "%.6f", lon);
        tm_out.transmit(
            Message::TelemetryMessage(id, FC_time_now(), "GPS_LONG", std::string(buffer,n)) );

        n = snprintf(buffer, 15, "%.2f", alt);
        tm_out.transmit(
            Message::TelemetryMessage(id, FC_time_now(), "GPS_ALTI", std::string(buffer,n)) );

        last_telemetry = FC_time_now();
        flag_telemetry_pending = false;
    };
    
    // after 5s the GPS has acquired a lock
    if (!status_lock)
    {
        if (FC_elapsed_millis(startup_time) > 5000)
        {
            status_lock = true;
            flag_state_change = true;
            runlevel_= MODULE_RUNLEVEL_OPERATIONAL;
        };
    };
    
    if (flag_state_change)
    {
        if (status_lock)
        {
            runlevel_=MODULE_RUNLEVEL_LINK_OK;
            system_log->in.receive(
                Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATE_CHANGE, "acquired lock.") );
        } else {
            runlevel_=MODULE_RUNLEVEL_OPERATIONAL;
            system_log->in.receive(
                Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_MILESTONE, "up and running.") );
        };
        flag_state_change = false;
    };
    
}

Message DummyGPS::get_position()
{
    MSG_DATA_GPS_POSITION data {
        .latitude = lat,
        .longitude = lon,
        .altitude = alt };
    Message msg(id, MSG_TYPE_GPS_POSITION, sizeof(MSG_DATA_GPS_POSITION), &data);
    return msg;
}

