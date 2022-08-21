/*
    In addition to sending/receiving messages, modules can communicate
    with streams. These are intended for small fixed-type data blocks
    that need to be exchanged at high rate. The sender just broadcasts
    data blocks of known type to all registered receivers without any metadata.
*/

#pragma once

#include <cstdint>
#include <cstdlib>
#include <list>

#include "types.h"

template <typename datatype>
class StreamReceiver;

/*
 * This port is intended for asynchronous communication.
 * The sender transmits a data block and does not care about it anymore.
 * The message gets stored in the input queue of the connected receivers
 * and sits there until it is processed by the receiver module
 */
template <typename datatype>
class StreamSender {
    public:
        // there can be set several receivers that all will get
        // the messages sent through this port
        void set_receiver(StreamReceiver<datatype> *receiver);
        void transmit(datatype data);
    protected:
        std::list<StreamReceiver<datatype>*> list_of_receivers;
};

/*
 * This port is intended for asynchronous communication.
 * Whenever the connected sender decides to send a data block it gets stored
 * in the input queue associated with this port.
 * It sits there until it is processed by the module owning this port.
 */
template <typename datatype>
class StreamReceiver {
    public:
        // When a sender decides to send a message to this port it will 
        // call this method. The receiver port will store the message
        // and do nothing else.
        void receive(datatype data);
        // The module owning the port must query the number of messages available
        uint16_t count();
        // The module can fetch the message from the queue for processing.
        datatype fetch();
    protected:
        std::list<datatype> queue;
};

