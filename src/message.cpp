#include "message.h"
#include <cstdio>
#include <cstdlib> // for C-style memory handling
#include <cstring> // for std::memcpy
// include <iostream> // for std::cout during debugging
#include <Arduino.h> // for USB during debugging

Message::Message(
    std::string sender_module,
    uint16_t    msg_type,
    uint16_t    msg_size,
    void*       msg_data)
{
    // std::cout << "Message standard constructor";
    m_sender_module = sender_module;
    m_type = msg_type;
    m_size = msg_size;
    // std::cout << " size=" << m_size << std::endl;
    if (m_size>0)
    {
        m_data = malloc(m_size);
        std::memcpy(m_data, msg_data, m_size);
    }
    else
        m_data = NULL;
}

Message::Message(const Message& other)
{
    // std::cout << "Message COPY constructor";
    m_sender_module = other.m_sender_module;
    m_type = other.m_type;
    m_size = other.m_size;
    // std::cout << " size=" << m_size << std::endl;
    if (m_size>0)
    {
        m_data = malloc(m_size);
        std::memcpy(m_data, other.m_data, m_size);
    }
    else
        m_data = NULL;
}

Message& Message::operator=(const Message& other)
{
    // protct against invalid self-assignment
    if (this != &other)
    {
        m_sender_module = other.m_sender_module;
        m_type = other.m_type;
        m_size = other.m_size;
        // free the old memory
        if (m_size>0) free(m_data);
        // copy the new data
        if (other.m_size>0)
        {
            m_data = malloc(other.m_size);
            std::memcpy(m_data, other.m_data, other.m_size);
        }
        else
            m_data = NULL;
    }
    return *this;
}

Message::Message(char* buffer)
{
}

Message Message::TextMessage(
    std::string sender_module,
    std::string text)
{
    Message msg = Message(sender_module, MSG_TYPE_TEXT, 0, NULL);
    // std::cout << "MSG_TYPE_TEXT constructor";
    msg.m_size = sizeof(MSG_DATA_TEXT) + text.size();
    msg.m_data = malloc(msg.m_size);
    // std::cout << " size=" << m_size << std::endl;
    // pointer to the allocated memory
    MSG_DATA_TEXT *d = (MSG_DATA_TEXT *)msg.m_data;
    d->text = text.size();
    // point to the rest of the memory block reserved for the string
    // the pointer is advanced by 1x the size of the object
    d++;
    // copy the content of the string
    char *t = (char *)d;
    for (size_t i=0; i<text.size(); i++)
        *t++ = text[i];
    return msg;
}

Message Message::SystemMessage(
    std::string sender_module,
    uint32_t    time,
    uint8_t     severity_level,
    std::string text)
{
    Message msg = Message(sender_module, MSG_TYPE_SYSTEM, 0, NULL);
    // Serial.print("MSG_TYPE_SYSTEM constructor");
    msg.m_size = sizeof(MSG_DATA_SYSTEM) + text.size();
    msg.m_data = malloc(msg.m_size);
    // std::cout << " size=" << m_size << std::endl;
    // Serial.print("  text=");
    // Serial.print(text.size());
    // Serial.print("  m_size=");
    // Serial.println(msg.m_size);
    // pointer to the allocated memory
    MSG_DATA_SYSTEM *d = (MSG_DATA_SYSTEM *)msg.m_data;
    d->severity_level = severity_level;
    d->time = time;
    d->text = text.size();
    // point to the rest of the memory block reserved for the string
    // the pointer is advanced by 1x the size of the object
    d++;
    // copy the content of the string
    char *t = (char *)d;
    for (size_t i=0; i<text.size(); i++)
        *t++ = text[i];
    return msg;
};

Message Message::TelemetryMessage(
    std::string sender_module,
    uint32_t    time,
    std::string variable,
    std::string value)
{
    Message msg = Message(sender_module, MSG_TYPE_TELEMETRY, 0, NULL);
    // std::cout << "MSG_TYPE_TELEMETRY constructor";
    msg.m_size = sizeof(MSG_DATA_SYSTEM) + variable.size() + value.size();
    msg.m_data = malloc(msg.m_size);
    // std::cout << " size=" << m_size << std::endl;
    // pointer to the allocated memory
    MSG_DATA_TELEMETRY *d = (MSG_DATA_TELEMETRY *)msg.m_data;
    d->time = time;
    d->variable = variable.size();
    d->value = value.size();
    // point to the rest of the memory block reserved for the string
    // the pointer is advanced by 1x the size of the object
    d++;
    // copy the content of the string
    char *t = (char *)d;
    for (size_t i=0; i<variable.size(); i++)
        *t++ = variable[i];
    for (size_t i=0; i<value.size(); i++)
        *t++ = value[i];
    return msg;
}

Message::~Message()
{
    // std::cout << "Message destructor ";
    /*
    switch (m_type)
        {
            case MSG_TYPE_ABSTRACT:
                std::cout << "ABSTRACT" << std::endl; break;
            case MSG_TYPE_SYSTEM:
                std::cout << "MSG_TYPE_SYSTEM" << std::endl; break;
            case MSG_TYPE_TEXT:
                std::cout << "MSG_TYPE_TEXT" << std::endl; break;
            case MSG_TYPE_TELEMETRY:
                std::cout << "MSG_TYPE_TELEMETRY" << std::endl; break;
            case MSG_TYPE_GPS_POSITION:
                std::cout << "MSG_TYPE_GPS_POSITION" << std::endl; break;
        };
    */
    if (m_size>0) free(m_data);
}

std::string Message::print_content()
{
    // std::cout << "Message::print_content() size=" << m_size << std::endl;
    // TODO: handle all other message types
    std::string ret("");
    switch (m_type)
        {
            case MSG_TYPE_ABSTRACT:
                {
                    break;
                };
            case MSG_TYPE_SYSTEM:
                {
                    // std::cout << "MSG_TYPE_SYSTEM  header=" << sizeof(MSG_DATA_SYSTEM);
                    MSG_DATA_SYSTEM *ptr = (MSG_DATA_SYSTEM *)m_data;
                    char buffer[12];
                    // time
                    int n = snprintf(buffer, 11, "%10.3f", (double)(ptr->time)*0.001);
                    ret += std::string(buffer,n);
                    // separator
                    ret += std::string(" : ");
                    // severity level
                    n = snprintf(buffer, 5, "%4d", ptr->severity_level);
                    ret += std::string(buffer,n);
                    // separator
                    ret += std::string(" : ");
                    // this is the number of characters in the text
                    int count = ptr->text;
                    // std::cout << " characters=" << count << std::endl;
                    // advance the pointer beyond the data structure (where the text is)
                    ptr++;
                    char *t = (char *)ptr;
                    // message text
                    for (int i=0; i<count; i++)
                        ret += *t++;
                    break;
                };
            case MSG_TYPE_TEXT:
                {
                    // std::cout << "MSG_TYPE_TEXT  header=" << sizeof(MSG_TYPE_TEXT);
                    // this message contains just one string
                    char* ptr = (char *)m_data;
                    // the pointer initially points to the length byte
                    int count = *ptr++;
                    // std::cout << " characters=" << count << std::endl;
                    // now append all characters
                    for (int i=0; i<count; i++)
                        ret += *ptr++;
                    break;
                };
            case MSG_TYPE_TELEMETRY:
                {
                    // std::cout << "MSG_TYPE_TELEMETRY  header=" << sizeof(MSG_DATA_TELEMETRY);
                    MSG_DATA_TELEMETRY *ptr = (MSG_DATA_TELEMETRY *)m_data;
                    char buffer[12];
                    // time
                    int n = snprintf(buffer, 11, "%10.3f", (double)(ptr->time)*0.001);
                    ret += std::string(buffer,n);
                    // separator
                    ret += std::string(" : ");
                    // size of the text fields
                    int var_count = ptr->variable;
                    int val_count = ptr->value;
                    // advance the pointer beyond the data structure (where the text is)
                    ptr++;
                    char *t = (char *)ptr;
                    // variable name
                    for (int i=0; i<var_count; i++)
                        ret += *t++;
                    for (int i=var_count; i<8; i++)
                        ret += " ";
                    // separator
                    ret += std::string(" : ");
                    // value
                    for (int i=0; i<val_count; i++)
                        ret += *t++;
                    break;
                };
            case MSG_TYPE_GPS_POSITION:
                {
                    // std::cout << "MSG_TYPE_GPS_POSITION  header=" << sizeof(MSG_DATA_GPS_POSITION);
                    MSG_DATA_GPS_POSITION *ptr = (MSG_DATA_GPS_POSITION *)m_data;
                    char buffer[16];
                    // latitude
                    int n = snprintf(buffer, 15, "%10.6f", ptr->latitude);
                    ret += "lat=";
                    ret += std::string(buffer,n);
                    // longitude
                    n = snprintf(buffer, 15, "%11.6f", ptr->longitude);
                    ret += ", long=";
                    ret += std::string(buffer,n);
                    // altitude
                    n = snprintf(buffer, 15, "%7.2f", ptr->altitude);
                    ret += ", alti=";
                    ret += std::string(buffer,n);
                    break;
                };
            default:
                {
                    break;
                };
        };
    return ret;
}

std::string Message::printout()
{
    std::string text = m_sender_module;
    // pad with spaces to 8 characters
    size_t len = text.size();
    if (len<8)
    {
        std::string space(8-len, ' ');
        text += space;
    };
    text = text.substr(0, 8);
    // separator
    text += std::string(" : ");
    // message text
    text += print_content();
    return text;
}

Message Message::as_text()
{
    return Message::TextMessage(m_sender_module, print_content());
}

uint8_t Message::buffer(char* buffer, size_t size)
{
    // check for buffer size
    if (size<12) return 0;
    char* ptr = buffer;
    // mesagge type is encoded with two bytes, high byte first
    char *source = (char*) &m_type;
    buffer[1] = *source++;
    buffer[0] = *source++;
    // reserve one byte for the message size (we don't know it yet)
    ptr+=3;
    // sender ID is put as a fixed length of 8 characters
    size_t n=0;
    while ((n<m_sender_module.size()) and (n<8))
    {
        *ptr++ = m_sender_module[n];
        n++;
    };
    while (n<8)
    {
        *ptr++ = 0x20; // fill with spaces
        n++;
    };
    uint8_t n_bytes = 11; // 2+1+8 bytes fixed information
    // the data block is put as compact as possible (depending on the type)
    // number of free bytes remaining in the buffer
    uint8_t remaining = size-11;
    switch (m_type)
    {
        case MSG_TYPE_SYSTEM:
        {
            MSG_DATA_SYSTEM *md = (MSG_DATA_SYSTEM *)m_data;
            int count = md->text;
            // Serial.print("\nMSG_DATA_SYSTEM size=");
            // Serial.println(count);
            // check for buffer size (keep one byte for checksum)
            if (remaining > 5)
            {
	            // if we can at least send a single character we go with a truncated message
            	if (count>remaining-5) count=remaining-5;
                *ptr = md->severity_level;
                std::memcpy(ptr+1, &(md->time), 4);
                n_bytes += 5;
                // the number of characters is not needed in the block, because the total length is known
                // the text content starts at the next character after the m_data struct
                uint8_t *txt = (uint8_t *) m_data;
                txt += sizeof(MSG_DATA_SYSTEM);
                // now append all characters
                std::memcpy(ptr+5, txt, count);
                n_bytes += count;
            };
            // if buffer size is insufficient go ahead with missing data block
            break;
        };
        case MSG_TYPE_TEXT:
        {
            // TODO
            break;
        };
        // TODO: more cases
        default:
        {
        };
    }    
    // the message size is put into the buffer (the type word and size byte are not counted)
    buffer[2] = n_bytes-3;
    // TODO add a CRC checksum
    return n_bytes;
}

