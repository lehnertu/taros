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

    wiring   modem <==> Teensy
        modem pin0 "M0" <== pin 22 "RTS"
        modem pin1 "M1" <== pin 22
        modem pin2 "RXD" <== pin1 "TX1"
        modem pin3 "TXD" ==> pin0 "RX1"
        modem pin4 "AUX" ==> pin 23 "CTS"
        
                
    normal operation M0=M1=0, config mode M0=M1=1
        M0, M1 have an internal pull-up resitor, they are read as high when not connected -> config mode
        (wire with one common GPIO pin, could be RTS)
        
    read AUX with GPIO pin (detect start-up sequence)
        low during startup, pulled high when normal operation is reached
        low during init (when leaving config mode), pulled high when normal operation is reached
        low during transmition, pulled high when block is fully sent
        low 2m before received data is sent over TXD, pulled high when block is fully sent
        (could use CTS)

    
*/

/*  
    This is a class encapsulating the transmission channel.
    It sends all received messages to the ground station.
    It receives command messages from the ground station.
    
    At 9600 baud over-the-air rate a single character takes 1ms transmission time.
    Before starting a transmission one should check, that the transmission buffer
    has been empty for 8ms. A duration of 5ms during which no character
    has been received is recognized as the end of one message.
*/
class Modem : public Module
{

public:

    // constructor
    Modem(
        std::string name,
        uint32_t baud_rate);
    
    // initialization of the modem
    // the modem gets configured for 115200,8N1 serial communication
    // air 9600,8N1   freq ch18 = 868.125 MHz
    // it returns in either MODULE_RUNLEVEL_ERROR or MODULE_RUNLEVEL_OPERATIONAL state
    // TODO: switch to MODULE_RUNLEVEL_LINK_OPEN when a communication to ground station has been established
    virtual void setup();
    
    virtual void interrupt();
    
	// This is one worker function to be executed by te task manager.
	// It is scheduled whenever any characters are received via the uplink.
	// It puts read characters into the uplink_buffer.
	void receive();

	// This is one worker function to be executed by te task manager.
	// When the channel is idle for 5m but has received something,
	// the message is assumed to be complete.
	// Here is it processed.
	void process_message();

	// This is one worker function to be executed by te task manager.
	// It is scheduled when a message waits in the downlink queue and
	// enough time has elapsed from the last transmission.
    // It is not possible, to send out all pending messages at once -
    // one message per millisecond is more than the air channel can handle.
    // The next message will be processed after 10 ms.
	void send_message();

    // destructor
    virtual ~Modem() {};

    // port at which messages are received to be sent
    ReceiverPort downlink;

    // port over which received messages are delivered
    // TODO: we do not create the messages yet
    SenderPort uplink;
    
    // port over which status messages are sent
    SenderPort status_out;
    
    // TODO: we should have a reset
    // so the setup can be repeated after an error
    
private:

    // check the AUX pin
    bool        busy();
    
    // time of the last setup or channel test action
    uint32_t    last_time;

    // where to store incoming transmissions
    char        uplink_buffer[200];
    uint16_t    uplink_num_chars;
    char        message_buffer[200];
    uint16_t    message_num_chars;
    
};
