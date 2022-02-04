#include "port.h"

template <typename msg_type>
void SenderPort<msg_type>::set_receiver(ReceiverPort<msg_type> *receiver)
{
    list_of_receivers.push_back(receiver);
};

template <typename msg_type>
void SenderPort<msg_type>::transmit(msg_type message)
{
    // Serial.println("SenderPort::transmit()");
    // std::list<ReceiverPort<msg_type>*> list_of_receivers;
    for (auto const& port : list_of_receivers) {
        port->receive(message);
    }
};

// we have to instantiate the class for every possible message type
template class SenderPort<MESSAGE_TEXT>;
template class SenderPort<MESSAGE_GPS_POSITION>;
template class SenderPort<MESSAGE_TELEMETRY>;




template <typename msg_type>
void ReceiverPort<msg_type>::receive(msg_type message)
{
    // Serial.println("ReceiverPort::receive()");
    queue.push_back(message);
};

template <typename msg_type>
uint16_t ReceiverPort<msg_type>::count()
{
    return queue.size();
};

template <typename msg_type>
msg_type ReceiverPort<msg_type>::fetch()
{
    // Serial.println("ReceiverPort::fetch()");
    // get the first message
    msg_type msg = queue.front();
    // remove it from the list
    queue.pop_front();
    return msg;
};

// we have to instantiate the class for every possible message type
template class ReceiverPort<MESSAGE_TEXT>;
template class ReceiverPort<MESSAGE_GPS_POSITION>;
template class ReceiverPort<MESSAGE_TELEMETRY>;
