#include "Scanner.hpp"

#include <iostream>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp> 

Scanner::Scanner(PageTable& page_table)
    : page_table_(page_table), running_(false) {
}

// Check if a page is hot based on both access count and time threshold
bool Scanner::classifyHotPage(const PageMetadata& page, size_t min_access_count) const {
    return page.access_count >= min_access_count;
}

// Check if a page is cold based on both access count and time threshold
bool Scanner::classifyColdPage(const PageMetadata& page, std::chrono::seconds time_threshold) const {
    auto current_time = std::chrono::steady_clock::now();
    auto time_since_last_access = std::chrono::duration_cast<std::chrono::seconds>(
        current_time - page.last_access_time);
    return time_since_last_access >= time_threshold;
}

// Continuously classify pages using scanNext()
void Scanner::runClassifier(RingBuffer<MemMoveReq>& move_page_buffer, size_t min_access_count, std::chrono::seconds time_threshold) {
    running_ = true;
    while (running_) {
        PageMetadata page = page_table_.scanNext();

        switch (page.page_layer) {
        case PageLayer::NUMA_LOCAL: {
            // Only detect cold pages for local NUMA
            if (classifyColdPage(page, time_threshold)) {
                std::cout << "Cold page detected in NUMA_LOCAL: " << page.page_address << std::endl;
                MemMoveReq msg(page.page_id, PageLayer::NUMA_REMOTE);
                while (!move_page_buffer.push(msg)) {
                    boost::this_thread::sleep_for(boost::chrono::nanoseconds(100));
                }
            }
            break;
        }

        case PageLayer::NUMA_REMOTE: {
            // Check cold first, then hot if not cold
            if (classifyColdPage(page, time_threshold)) {
                std::cout << "Cold page detected in NUMA_REMOTE: " << page.page_address << std::endl;
                MemMoveReq msg(page.page_id, PageLayer::PMEM);
                while (!move_page_buffer.push(msg)) {
                    boost::this_thread::sleep_for(boost::chrono::nanoseconds(100));
                }
            }
            else if (classifyHotPage(page, min_access_count)) {
                std::cout << "Hot page detected in NUMA_REMOTE: " << page.page_address << std::endl;
                MemMoveReq msg(page.page_id, PageLayer::NUMA_LOCAL);
                while (!move_page_buffer.push(msg)) {
                    boost::this_thread::sleep_for(boost::chrono::nanoseconds(100));
                }
            }
            break;
        }

        case PageLayer::PMEM: {
            // Only detect hot pages for PMEM
            if (classifyHotPage(page, min_access_count)) {
                std::cout << "Hot page detected in PMEM: " << page.page_address << std::endl;
                MemMoveReq msg(page.page_id, PageLayer::NUMA_REMOTE);
                while (!move_page_buffer.push(msg)) {
                    boost::this_thread::sleep_for(boost::chrono::nanoseconds(100));
                }
            }
            break;
        }
        }

        // Sleep for a short duration to prevent tight loop
        boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
    }
}


void Scanner::stopClassifier() {
    running_ = false;
}