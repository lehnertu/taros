#pragma once

#include <string>

#include "global.h"
#include "module.h"
#include "message.h"
#include "port.h"

/*

    LoRa modem
    -----------
    EBYTE e220-900t22d

    normal operation M0=M1=0, config mode M0=M1=1
        M0, M1 have an internal pull-up resitor, they are read as high when not connected -> config mode
        (wire with one common GPIO pin, could be RTS) => pin 22
        
    read AUX with GPIO pin (detect start-up sequence)
        low during startup, pulled high when normal operation is reached
        low during init (when leaving config mode), pulled high when normal operation is reached
        low during transmition, pulled high when block is fully sent
        low 2m before received data is sent over TXD, pulled high when block is fully sent
        (could use CTS) => pin 23

*/

/*  
    This is a class encapsulating the transmission channel.
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
    Before starting a transmission one should check, that the transmission buffer
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
    
    // TODO: move initializations here
    virtual void setup() { runlevel_ = MODULE_RUNLEVEL_SETUP_OK; };
    
    // The module is queried by the scheduler every millisecond whether it needs to run.
    // This will return true, when a new dataset from the GPS has been received.
    virtual bool have_work();

    // This is the worker function being executed by the taskmanager.
    // It writes all pending messages to the bus unless a limit of execution time is exceeded.
    virtual void run();

    // destructor
    virtual ~Modem() {};

    // port at which messages are received to be sent
    ReceiverPort downlink;

    // port over which received messages are delivered
    SenderPort uplink;
    
    // port over which status messages are sent
    SenderPort status_out;
    
    // TODO: we should have a reset
    // so the setup can be repeated after an error
    
private:

    // check the AUX pin
    bool        busy();
    
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
