/*
    All modules communicate by messages.
    The communication can be driven by the sender (streaming)
    or the receiver (message requests). Usually the slower module initiates
    the transmission and determines te data rate.
    
    In all cases the transmission occurs between a sender and a receiver port.
    Here we define the ports a module can have to send or receive messages.
*/

#pragma once

#include <cstdlib>
#include <list>
#include "message.h"

template <typename msg_type>
class ReceiverPort;

/*
 * This port is intended for asynchronous communication.
 * The sender transmits one message and does not care about it anymore.
 * The message gets stored in the input queue of the connected receivers
 * and sits there until it is processed by the receiver module
 */
template <typename msg_type>
class SenderPort {
    public:
        // there can be set several receivers that all will get
        // the messages sent through this port
        void set_receiver(ReceiverPort<msg_type> *receiver);
        void transmit(msg_type message);
    private:
        std::list<ReceiverPort<msg_type>*> list_of_receivers;
};

/*
 * This port is intended for asynchronous communication.
 * Whenever the connected sender decides to send a message it gets stored
 * in the input queue associated with this port.
 * It sits there until it is processed by the module owning this port.
 */
template <typename msg_type>
class ReceiverPort {
    public:
        // When a sender decides to send a message to this port it will 
        // call this method. The receiver port will store the message
        // and do nothing else.
        void receive(msg_type message);
        // The module owning the port must query the number of messages available
        uint16_t count();
        // The module can fetch the message from the queue fror processing.
        msg_type fetch();
    private:
        std::list<msg_type> queue;
};

