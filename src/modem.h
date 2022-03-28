#pragma once

#include <string>

#include "global.h"
#include "module.h"
#include "message.h"
#include "port.h"

/*  
    This is a module encapsulating a transmission channel.
    It sends all received messages to the ground station.
    It receives command messages from the ground station.
    
    We use the runlevel to encode the state of the setup actions
    1 : hardware serial port has been opened
    2 : modem put into command mode
    3 : modem configuration has been sent
    4 : configuration has been acknowledged
    5 : modem put into transceiver mode
   16 : up and running
   
    At 9600 baud over-the-air rate a single character takes 1ms transmission time.
    Before starting a transmission one should check, that tre transmission buffer
    has been empty for 8ms. A duration of 5ms during which no character
    has bee received is recognized as the end of one message.
*/
class Modem : public Module
{

public:

    // constructor
    Modem(
        std::string name,
        uint32_t baud_rate);
    
    // The module is queried by the scheduler every millisecond whether it needs to run.
    // This will return true, when a new dataset from the GPS has been received.
    virtual bool have_work();

    // This is the worker function being executed by the taskmanager.
    // It writes all pending messages to the bus unless a limit of execution time is exceeded.
    virtual void run();

    // destructor
    virtual ~Modem() {};

    // port at which messages are received to be sent
    MessageReceiverPort downlink;

    // port over which received messages are delivered
    SenderPort<Message> uplink;
    
    // port over which status messages are sent
    SenderPort<Message_System> status_out;
    
    // TODO: we should have a reset
    // so the setup can be repeated after an error
    
private:

    // here are some flags indicating which work is due
    bool        flag_setup_pending;
    bool        flag_msg_pending;
    bool        flag_received;
    bool        flag_received_completely;
    // time of the last setup or channel test action
    uint32_t    last_time;

    // where to store incoming transmissions
    char        uplink_buffer[200];
    uint16_t    uplink_num_chars;
    char        message_buffer[200];
    uint16_t    message_num_chars;
};
