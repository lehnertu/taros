#include "port.h"
#include "global.h"

template <class msg_type>
void SenderPort<msg_type>::set_receiver(ReceiverPort<msg_type> *receiver)
{
    list_of_receivers.push_back(receiver);
};

template <class msg_type>
void SenderPort<msg_type>::set_receiver(MessageReceiverPort *receiver)
{
    list_of_message_receivers.push_back(receiver);
};

template <class msg_type>
void SenderPort<msg_type>::transmit(msg_type message)
{
    for (auto const& port : list_of_receivers) {
        port->receive(message);
    }
    for (auto const& port : list_of_message_receivers) {
        port->receive(message);
    }
};

// we have to instantiate the class for every possible message type
template class SenderPort<Message>;
template class SenderPort<Message_Text>;
template class SenderPort<Message_System>;
template class SenderPort<Message_GPS_position>;
template class SenderPort<Message_Telemetry>;




template <class msg_type>
void ReceiverPort<msg_type>::receive(msg_type message)
{
    queue.push_back(message);
};

template <class msg_type>
uint16_t ReceiverPort<msg_type>::count()
{
    return queue.size();
};

template <class msg_type>
msg_type ReceiverPort<msg_type>::fetch()
{
    // get the first message
    msg_type msg = queue.front();
    // remove it from the list
    queue.pop_front();
    return msg;
};

// we have to instantiate the class for every possible message type
template class ReceiverPort<Message>;
template class ReceiverPort<Message_Text>;
template class ReceiverPort<Message_System>;
template class ReceiverPort<Message_GPS_position>;
template class ReceiverPort<Message_Telemetry>;


void MessageReceiverPort::receive(Message message)
{
    queue.push_back(message);
};

uint16_t MessageReceiverPort::count()
{
    return queue.size();
};

Message MessageReceiverPort::fetch()
{
    // get the first message
    Message msg = queue.front();
    // remove it from the list
    queue.pop_front();
    return msg;
};

