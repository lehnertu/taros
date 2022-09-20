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
    flag_ping_pending = false;
    flag_msg_pending = false;
    flag_received = false;
    flag_received_completely = false;
    last_time = FC_time_now();
    last_flush = FC_time_now();
    // nothing received yet
    uplink_num_chars = 0;
    message_num_chars = 0;
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

void Modem::setup()
{
    // for debugging we log all downlink transmissions
    sprintf(log_filename, "taros.%05d.downlink.log", SD_file_No);
    myFile = SD.open(log_filename, FILE_WRITE);

    runlevel_ = 0;
    last_time = FC_time_now();
    last_flush = FC_time_now();
    // internal initialization
    while(busy())
    {
        if (FC_elapsed_millis(last_time)>1000)
        {
            status_out.transmit(
                Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATE_CHANGE, "failed to initialize.") );
            runlevel_ = MODULE_RUNLEVEL_ERROR;
            return;
        }
    }
    // open serial port for configuration
    // default SERIAL_8N1  == 0x00
    Serial1.begin(9600);
    Serial1.setTimeout(0);
    // send a message to the system_log
    status_out.transmit(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATE_CHANGE, "initialized.") );
    // init hardware
    // pull M0/M1 high (sleep/config mode)
    pinMode(MODEM_M0_M1, OUTPUT);
    digitalWriteFast(MODEM_M0_M1, HIGH);
    // prepare CTS pin
    pinMode(MODEM_AUX, INPUT);
    // wait 100 ms
    last_time = FC_time_now();
    while (FC_elapsed_millis(last_time) < 100) {};
    // send the configuration command
    Serial1.write(0xc0);
    Serial1.write(0x00);
    Serial1.write(0x05);    // 5 command bytes
    Serial1.write(0x00);
    Serial1.write(0x00);
    Serial1.write(0xE4);    // modem 115200,8N1 air=9600bps
    // Serial1.write(0x64);    // modem 9600,8N1 air=9600bps
    Serial1.write(0x00);
    Serial1.write(0x12);    // freq ch18 = 868.125 MHz
    // check the response from the modem
    last_time = FC_time_now();
    while (Serial1.available() < 3)
    {
        if (FC_elapsed_millis(last_time)>1000)
        {
            status_out.transmit(
                Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATE_CHANGE, "no configuration response.") );
            runlevel_ = MODULE_RUNLEVEL_ERROR;
            return;
        }
    }
    uplink_num_chars = 0;
    last_time = FC_time_now();
    // read the response with 10 ms timeout
    while (FC_elapsed_millis(last_time)<10)
    {
        if (Serial1.available() > 0)
        {
            int incoming = Serial1.read();
            char c = incoming & 0xFF;
            uplink_buffer[uplink_num_chars++] = c;
            last_time = FC_time_now();
        }
    };
    // report the response to system log
    std::string report("configuration response : ");
    for (int i=0; i<uplink_num_chars; i++)
    {
        report += hexbyte(uplink_buffer[i]);
    };
    status_out.transmit(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATUSREPORT, report) );
    // check for correct configuration
    if ((uplink_num_chars==8) and (uplink_buffer[0]==0xC1))
    {
        status_out.transmit(
            Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATE_CHANGE, "configured OK.") );
    }
    else
    {
        status_out.transmit(
            Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATE_CHANGE, "illegal configuration response.") );
        runlevel_ = MODULE_RUNLEVEL_ERROR;
        return;
    }
    // clear the receive buffer
    uplink_num_chars = 0;
    // done with configuration
    digitalWriteFast(MODEM_M0_M1, LOW);
    Serial1.end();
    // wait 100 ms
    last_time = FC_time_now();
    while (FC_elapsed_millis(last_time) < 100) {};
    // re-open using communication mode serial baud rate
    Serial1.begin(115200);
    Serial1.setTimeout(0);
    // wait 100 ms
    last_time = FC_time_now();
    while (FC_elapsed_millis(last_time) < 100) {};
    status_out.transmit(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_MILESTONE, "up and running.") );
    runlevel_ =  MODULE_RUNLEVEL_OPERATIONAL;
    // send out a wakeup message
    std::string buffer("TAROS : "+id+" up and running.\r\n");
    Serial1.write(buffer.c_str(), buffer.size());
    // write to file
    myFile.write(buffer.c_str(), buffer.size());
}

// TODO: handle uplink messages
// these should be command messages (with a unique identifier)
// we should respond with a confirmation containing the UID and RSI
// an empty command could be used as a ping

bool Modem::have_work()
{
    uint32_t elapsed = FC_elapsed_millis(last_time);
    if (FC_elapsed_millis(last_flush) > 2000)
    {
        last_flush = FC_time_now();
        flag_flush_pending = true;
    };
    // we detect the time elapsed as long as there is no activity at the modem
    if(busy())
    {
        elapsed=0;
        last_time = FC_time_now();
    };
    // see if we have received something
    if (Serial1.available() > 0) flag_received = true;
    // when we have received something, but the receeiver is idle for 5ms
    // then we have the complete message
    // this implies the modem is not busy()
    if ((uplink_num_chars>0) and elapsed>5) flag_received_completely = true;
    // if there is something received in one of the input ports
    // we have to handle it unless the modem is busy()
    // we wait 10 ms after busy() giving receiving messages higher priority than sending
    if ((runlevel_>=16) and (downlink.count()>0) and (elapsed>10)) flag_msg_pending = true;
    // if there is no activity for longer than 2s send a ping to the ground station
    if ((runlevel_>=16) and elapsed>2000) flag_ping_pending = true;
    return flag_flush_pending | flag_ping_pending | flag_received | flag_received_completely | flag_msg_pending;
}

void Modem::run()
{

    if (flag_ping_pending)
    {
        // send a ping
        std::string buffer("ping.\r\n");
        Serial1.write(buffer.c_str(), buffer.size());
        // write to file
        myFile.write(buffer.c_str(), buffer.size());
        // reset the flag
        flag_ping_pending = false;
        // record the time
        last_time = FC_time_now();
    }

    if (flag_flush_pending)
    {
        // send a ping
        std::string buffer("flush.\r\n");
        myFile.write(buffer.c_str(), buffer.size());
        myFile.flush();
        // reset the flag
        flag_flush_pending = false;
        // record the time
        last_flush = FC_time_now();
    }

    if (flag_received)
    {
        // something is in the incoming FIFO - read it
        while (Serial1.available() > 0)
        {
            int incoming = Serial1.read();
            char c = incoming & 0xFF;
            uplink_buffer[uplink_num_chars] = c;
            uplink_num_chars++;
        }
        // reset the flag
        flag_received = false;
        // record the time
        last_time = FC_time_now();
    }
    
    if (flag_received_completely)
    {
        // there is something in the uplink buffer but the port is idle for 5ms (complete message)
        // TODO: create an uplink message
        uplink_num_chars = 0;
        flag_received_completely = false;
        // record the time
        last_time = FC_time_now();
    }
    
    // it is not possible, to send out all pending messages at once
    // one message per millisecond is more than the air channel can handle
    // as we reset last_time, the next message will be processed after 10 ms
    if (flag_msg_pending)
    {
        Message msg = downlink.fetch();
        message_num_chars = msg.buffer(message_buffer, 200);
        // write out
        // the write is buffered and returns immediately
        Serial1.write(message_buffer, message_num_chars);
        // write to file
        message_buffer[message_num_chars++] = 10;
        message_buffer[message_num_chars++] = 13;
        myFile.write(message_buffer, message_num_chars);
        // reset the flag
        flag_msg_pending = false;
        // record the time
        last_time = FC_time_now();
    }
    
}

