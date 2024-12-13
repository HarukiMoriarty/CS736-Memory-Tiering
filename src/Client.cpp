#include "Client.hpp"

#include <iostream>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp> 

Client::Client(RingBuffer<Message>& buffer, int client_id, int message_cnt, int memory_space, AccessPattern pattern)
    : buffer_(buffer),
    client_id_(client_id),
    message_cnt_(message_cnt),
    generator_(pattern, memory_space) {
}

void Client::run() {
    for (int i = 0; i < message_cnt_; ++i) {
        int offset = generator_.generateOffset();
        OperationType op = (i % 2 == 0) ? OperationType::READ : OperationType::WRITE;

        Message msg(client_id_, offset, op);
        while (!buffer_.push(msg)) {
            boost::this_thread::sleep_for(boost::chrono::nanoseconds(100));
        }

        std::cout << "Client sent: " << msg.toString() << std::endl;
    }
}