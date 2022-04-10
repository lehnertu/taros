#include "modem.h"

#include "HardwareSerial.h"

// this is the RTS pin for the modem, used for M0 and M1 wired in parallel
// high means config mode, low is transceiver mode
#define MODEM_M0_M1 22
// this is the CTS pin for the modem, used for AUX
#define MODEM_AUX 23

Modem::Modem(
    std::string name,
    uint32_t baud_rate )
{
    // copy the name
    id = name;
    runlevel_= MODULE_RUNLEVEL_STOP;
    // open the connection
    // Serial.begin(baud_rate);
    // Serial.println("USB_Serial setup done.");
    flag_setup_pending = true;
    flag_msg_pending = false;
    flag_received = false;
    last_time = FC_time_now();
    // nothing received yet
    uplink_num_chars = 0;
    // we cannot send a status message that we are ready to run
    // because the port is not yet wired to any receiver
}

/*
    AUX will go low (busy) during setup times

    AUX low indicates a transmission being received
    expect data 2-3 ms later
    will go high when transfer completed

    AUX will go low while sending data
*/
bool Modem::busy()
{
    return digitalRead(MODEM_AUX) == LOW;
}

bool Modem::have_work()
{
    uint32_t elapsed = FC_elapsed_millis(last_time);
    // see if any setup action is due
    if ((runlevel_)==1 and elapsed>10) flag_setup_pending = true;   // init hardware
    if ((runlevel_)==2 and elapsed>100) flag_setup_pending = true;  // send configuration
    if ((runlevel_)==4 and elapsed>100) flag_setup_pending = true;  // done with configuration
    if ((runlevel_)==5 and elapsed>100) flag_setup_pending = true;  // start communication mode
    if ((runlevel_)==6 and elapsed>100) flag_setup_pending = true;  // ready
    // see if we have received something
    if (Serial1.available() > 0) flag_received = true;
    // when we have received something, but the receeiver is idle for 10ms
    // then we have the complete message
    if ((uplink_num_chars>0) and elapsed>5) flag_received_completely = true;
    // if there is something received in one of the input ports
    // we have to handle it
    if ((runlevel_>=16) and (downlink.count()>0)) flag_msg_pending = true;
    // if there is no activity for longer than 2s send a ping to the ground station
    if ((runlevel_>=16) and elapsed>2000) flag_setup_pending = true;
    return flag_setup_pending | flag_received | flag_received_completely | flag_msg_pending;
}

void Modem::run()
{

    if (flag_setup_pending)
    {
        switch (runlevel_)
        {
            case 0:
                // void begin(uint32_t baud, uint16_t format=0);
                // default SERIAL_8N1  == 0x00
                Serial1.begin(9600);
                Serial1.setTimeout(0);
                // send a message to the system_log
                status_out.transmit(
                    Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATE_CHANGE, "initialized.") );
                runlevel_=1;
                break;
            case 1:
                // init hardware
                // clear the receive buffer
                Serial1.clear();
                // pull M0/M1 high (sleep/config mode)
                pinMode(MODEM_M0_M1, OUTPUT);
                digitalWriteFast(MODEM_M0_M1, HIGH);
                // prepare CTS pin
                pinMode(MODEM_AUX, INPUT);
                runlevel_=2;
                break;
            case 2:
                // send configuration
                // 100ms after going to command mode send the configuration command
                Serial1.write(0xc0);
                Serial1.write(0x00);
                Serial1.write(0x05);    // 5 command bytes
                Serial1.write(0x00);
                Serial1.write(0x00);
                Serial1.write(0xE4);    // modem 115200,8N1 air=9600bps
                // Serial1.write(0x64);    // modem 9600,8N1 air=9600bps
                Serial1.write(0x00);
                Serial1.write(0x12);    // freq ch18 = 868.125 MHz
                status_out.transmit(
                    Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATE_CHANGE, "configured.") );
                runlevel_=3;
                // clear the receive buffer
                uplink_num_chars = 0;
                break;
            // runlevel 3 is handled by receiving a response message
            case 4:
                // done with configuration
                digitalWriteFast(MODEM_M0_M1, LOW);
                // don't know if that's needed
                Serial1.end();
                runlevel_=5;
                break;
            case 5:
                // use communication mode serial baud rate
                Serial1.begin(115200);
                Serial1.setTimeout(0);
                runlevel_=6;
                break;
            case 6:
                // waited long enough, modem should be ready now
                // use communication mode serial baud rate
                {
                    status_out.transmit(
                        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_MILESTONE, "up and running.") );
                    runlevel_=MODULE_RUNLEVEL_OPERATIONAL;
                    // send out a wakeup message
                    std::string buffer("TAROS : "+id+" up and running.\r\n");
                    Serial1.write(buffer.c_str(), buffer.size());
                };
                break;
            case 16:
            case 17:
                // no action for longer than 2s
                {
                    std::string buffer("ping\r\n");
                    Serial1.write(buffer.c_str(), buffer.size());
                };
                break;
            // default:
                // check channel status
        };
        // reset the flag
        flag_setup_pending = false;
        // record the time
        last_time = FC_time_now();
    }

    if (flag_received)
    {
        // something is in the incoming FIFO
        if (runlevel_<3)
        {
            // we are not receiving messages yet, trash it
            while (Serial1.available() > 0) Serial1.read();
        };
        if (runlevel_==3)
        {
            // this should be the response from the configuration command
            while (Serial1.available() > 0)
            {
                int incoming = Serial1.read();
                char c = incoming & 0xFF;
                uplink_buffer[uplink_num_chars] = c;
                uplink_num_chars++;
            }
        };
        // reset the flag
        flag_received = false;
        // record the time
        last_time = FC_time_now();
    }
    
    if (flag_received_completely)
    {
        // there is something in the uplink buffer but the port is idle for 5ms (complete message)
        if (runlevel_==3)
        {
            // during setup: we have received the configuration response
            // report to system log
            std::string report("configuration response : ");
            for (int i=0; i<uplink_num_chars; i++)
            {
                report += hexbyte(uplink_buffer[i]);
            };
            status_out.transmit(
                Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATUSREPORT, report) );
            // check if the response is valid
            if ((uplink_num_chars==8) and (uplink_buffer[0]==0xC1))
            {
                runlevel_=4;
                status_out.transmit(
                    Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATUSREPORT, "configured correctly") );
            } else {
                runlevel_=MODULE_RUNLEVEL_ERROR;
                status_out.transmit(
                    Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_ERROR, "configuration error") );
            }
            // done with this message, clear the buffer
            uplink_num_chars = 0;
        }
        flag_received_completely = false;
        // record the time
        last_time = FC_time_now();
    }
    
    // TODO probably we should check the buffer availability
    // it is not possible, to send out all pending messages at once
    // one message per millisecond is more than the air channel can handle
    if (flag_msg_pending)
    {
        if (busy())
        {
            status_out.transmit(
                Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_WARNING, "busy.") );
        };
        Message msg = downlink.fetch();
        std::string buffer = msg.printout();
        buffer += std::string("\r\n");
        // write out
        // the write is buffered and returns immediately
        Serial1.write(buffer.c_str(), buffer.size());
        // reset the flag
        flag_msg_pending = false;
        // record the time
        last_time = FC_time_now();
    }
    
}

