#include "stream.h"
#include "global.h"
#include "message.h" // for the data types

template <typename datatype>
void StreamSender<datatype>::set_receiver(StreamReceiver<datatype> *receiver)
{
    list_of_receivers.push_back(receiver);
};

template <typename datatype>
void StreamSender<datatype>::transmit(datatype data)
{
    for (auto const& port : list_of_receivers) {
        port->receive(data);
    }
};



template <typename datatype>
void StreamReceiver<datatype>::receive(datatype data)
{
    queue.push_back(data);
};

template <typename datatype>
uint16_t StreamReceiver<datatype>::count()
{
    return queue.size();
};

template <typename datatype>
datatype StreamReceiver<datatype>::fetch()
{
    // get the first message
    datatype data = queue.front();
    // remove it from the list
    queue.pop_front();
    return data;
};


// we have to instantiate the classes for every possible data type
template class StreamSender<DATA_IMU_AHRS>;
template class StreamReceiver<DATA_IMU_AHRS>;
template class StreamSender<DATA_IMU_GYRO>;
template class StreamReceiver<DATA_IMU_GYRO>;
