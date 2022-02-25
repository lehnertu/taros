#include "port.h"
#include "global.h"

template <class msg_type>
void SenderPort<msg_type>::set_receiver(ReceiverPort<msg_type> *receiver)
{
    list_of_receivers.push_back(receiver);
};

template <class msg_type>
void SenderPort<msg_type>::transmit(msg_type message)
{
    for (auto const& port : list_of_receivers) {
        port->receive(message);
    }
};

// we have to instantiate the class for every possible message type
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
template class ReceiverPort<Message_Text>;
template class ReceiverPort<Message_System>;
template class ReceiverPort<Message_GPS_position>;
template class ReceiverPort<Message_Telemetry>;


template <typename msg_type>
void TimedReceiverPort<msg_type>::receive(msg_type message)
{
    // the this pointer is necessary to get the superclass member into scope
    this->queue.push_back(message);
    uint32_t t = FC_systick_millis_count;
    time.push_back(t);
};

template <typename msg_type>
msg_type TimedReceiverPort<msg_type>::fetch()
{
    // get the first message
    msg_type msg = this->queue.front();
    last_time = time.front();
    // remove it from the list
    this->queue.pop_front();
    time.pop_front();
    return msg;
};

template <typename msg_type>
uint32_t TimedReceiverPort<msg_type>::fetch_time()
{
    return last_time;
};

// we have to instantiate the class for every possible message type
template class TimedReceiverPort<Message_Text>;
template class TimedReceiverPort<Message_System>;

