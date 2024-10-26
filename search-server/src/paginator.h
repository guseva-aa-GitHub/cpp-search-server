#pragma once

#include <vector>

template <typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator b, Iterator e)
    : begin_(b), end_(e), size_(distance(b, e))  { }

    Iterator begin() const  { return begin_; }
    Iterator end() const    { return end_; }
    size_t size() const     { return size_; }

private:
    Iterator begin_, end_;
    size_t size_;
};

template <typename Iterator>
std::ostream& operator<<(std::ostream& out, const IteratorRange<Iterator>& container) {
    for (auto itr = container.begin(); itr != container.end(); itr = std::next(itr, 1)) {
        out<<*itr;
    }
    return out;
}

template <typename Iterator>
class Paginator {
private:
    std::vector<IteratorRange<Iterator>> pages_;
    
public:
    Paginator(Iterator begin, Iterator end, size_t page_s) {
        if (page_s == 0) page_s = 1;
        for (size_t left = distance(begin, end); left > 0;) {
            const size_t current_page_size = std::min(page_s, left);
            const Iterator current_page_end = std::next(begin, current_page_size);

            pages_.push_back({begin, current_page_end});

            left -= current_page_size;
            begin = current_page_end;
        }
    }

    auto begin() const  { return pages_.begin(); }
    auto end() const    { return pages_.end(); }
    size_t size() const { return pages_.size(); }
};

template <typename Container>
auto Paginate(const Container& c, int page_size) {
    return Paginator(begin(c), end(c), page_size);
}
