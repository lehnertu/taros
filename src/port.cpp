#include "port.h"
#include "global.h"

void SenderPort::set_receiver(ReceiverPort *receiver)
{
    list_of_receivers.push_back(receiver);
};

void SenderPort::transmit(Message message)
{
    for (auto const& port : list_of_receivers) {
        port->receive(message);
    }
};




void ReceiverPort::receive(Message message)
{
    queue.push_back(message);
};

uint16_t ReceiverPort::count()
{
    return queue.size();
};

Message ReceiverPort::fetch()
{
    // get the first message
    Message msg = queue.front();
    // remove it from the list
    queue.pop_front();
    return msg;
};

