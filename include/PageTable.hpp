#ifndef PAGETABLE_H
#define PAGETABLE_H

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <vector>
#include <chrono>

struct PageMetadata {
    int page_address;
    int page_layer;
    std::chrono::steady_clock::time_point last_access_time;
    size_t access_count = 0;

    PageMetadata(int addr = 0, int layer = 0);
};

class PageTable {
public:
    PageTable(size_t size);

    // Read-only operations
    PageMetadata getPage(size_t index) const;
    size_t size() const;

    // Write operations  
    void updatePage(size_t index, const PageMetadata& metadata);
    void updateAccess(size_t index);
    void updatePageLayer(size_t index, int new_layer);
    PageMetadata scanNext();
    void resetAccessCount();

private:
    std::vector<PageMetadata> table_;
    mutable boost::shared_mutex mutex_;
    size_t current_index_;
};

#endif // PAGETABLE_H