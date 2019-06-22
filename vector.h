//
// Created by nikita on 19.06.19.
//

#ifndef EXAM1_VECTOR_H
#define EXAM1_VECTOR_H

#include <glob.h>
#include <algorithm>
#include <memory>

// TODO: check ALL exception guarantee
// TODO: insert (&), push_back(&)

namespace {
    template<typename T>
    struct data {
        size_t size;
        size_t capacity;
        size_t nRefs;
        T data[];
    };

    template<typename T>
    data<T>* allocate(size_t n) {
        data<T>* tmp = static_cast<data<T>*>(operator new(sizeof(data<T>) + sizeof(T) * n));
        tmp->nRefs = 1;
        tmp->capacity = n;
        tmp->size = 0;
        return tmp;
    }

    template <typename T>
    void deleteLink(data<T>* d) {
        if (!d) return;
        --d->nRefs;
        if (d->nRefs == 0) {
            for (size_t i = 0; i < d->size; ++i) {
               d->data[i].~T();
            }
            operator delete(d);
        }
    }

    template <typename T>
    void addLink(data<T>* d) {
        ++d->nRefs;
    }

    template <typename T>
    bool isOnlyUser(data<T>* d) {
        return d->nRefs == 1;
    }
}

template <typename T>
struct vector {
    typedef T value_type;
    typedef T* iterator;
    typedef T const* const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    vector() : isSmall_(true), hasElements_(false), container_() {}

    vector(vector const& v) {
        isSmall_ = v.isSmall();
        hasElements_ = v.hasElements_;
        if (v.empty())  {
            container_.data_ = nullptr;
            return;
        }
        if (v.isSmall()) {
            new (&container_.small) T(v.container_.small);
           return;
        }
        container_.data_ = v.container_.data_;
        addLink(container_.data_);
    }


    // TODO: reserve correct if I haven't buffer?
    template <typename InputIterator>
    vector(InputIterator first, InputIterator last) {
        size_t length = last - first;
        isSmall_ = length <= 1;
        hasElements_ = length != 0;
        if (empty())  {
            container_.data_ = nullptr;
            return;
        }
        if (isSmall()) {
            new (&container_.small) T(*first);
            return;
        }
        container_.data_ = allocate<T>(length);
        try {
            std::uninitialized_copy(first, last, begin());
        } catch (...) {
            deleteLink(container_.data_);
            hasElements_ = false;
            throw;
        }
        container_.data_->size = length;
    }

    ~vector() {
        if (isSmall() && hasElements_) {
            container_.small.~T();
        } else if (!isSmall()) {
            deleteLink(container_.data_);
        }
    }

    vector& operator=(vector const& v) {
        if (this == &v) return *this;
        if (v.empty()) {
            clear();
            return *this;
        }
        if (isSmall() && v.isSmall()) {
            if (empty()) {
                new (&container_.small) T(v.container_.small);
            } else {
                container_.small = v.container_.small;
            }
        } else if (!isSmall() && !v.isSmall()) {
            if (container_.data_ == v.container_.data_)
                return *this;
            deleteLink(this->container_.data_);
            container_.data_ = v.container_.data_;
            addLink(container_.data_);
        } else if (isSmall()) {
            // small <- big
            clear();
            container_.data_ = v.container_.data_;
            addLink(container_.data_);
            this->isSmall_ = false;
        } else {
            // big <- small
            clear();
            push_back(v.container_.small);
        }
        this->hasElements_ = true;
        return *this;
    }

    template <typename InputIterator>
    void assign(InputIterator first, InputIterator last) {
        size_t length = last - first;
        if (length == 0) {
            clear();
            return;
        }
        if (isSmall() && length == 1) {
            if (empty()) {
                new (&container_.small) T(*first);
            } else {
                container_.small = *first;
            }
        } else {
            if (isSmall() && !empty())
                clear();
            auto tmp = allocate<T>(length);
            try {
               std::uninitialized_copy(first, last, tmp->data);
            } catch (...) {
                if (!isSmall())
                    deleteLink(tmp);
                throw;
            }
            if (!isSmall())
                deleteLink(container_.data_);
            tmp->size = length;
            container_.data_ = tmp;
            isSmall_ = false;
            container_.data_->size = length;
        }
        this->hasElements_ = true;
    }

    T& operator[](size_t i) {
        getNewBuffer();
        return *(begin() + i);
    }

    T const& operator[](size_t i) const {
        return *(begin() + i);
    }

    T& front() {
        return *begin();
    }

    T& back() {
        return *(end() - 1);
    }

    T const& front() const{
        return *begin();
    }

    T const& back() const {
        return *(end() - 1);
    }

    void push_back(T val) {
//        std::cout << "pb" << std::endl;
        getNewBuffer();
        if (empty()) {
            if (isSmall()) {
                new(&container_.small) T(val);
            } else {
                new(&container_.data_->data[0]) T(val);
                ++container_.data_->size;
            }
            hasElements_ = true;
            return;
        }
        if (size() == capacity()) {
            reserve(size() * 2);
        }
        auto place = end();
        new(place) T(val);
        ++container_.data_->size;
    }

    void pop_back() {
        if (empty()) return;
        if (isSmall()) {
            container_.small.~T();
            hasElements_ = false;
            return;
        }
        --container_.data_->size;
        end()->~T();
        if (size() == 0) hasElements_ = false;

    }
    T const* data() const {
        return begin();
    };

    iterator begin() {
        if(empty()) return nullptr;
        if (isSmall()) return &container_.small;
        return container_.data_->data;
    }

    iterator end() {
        if (empty()) return nullptr;
        if (isSmall()) return &container_.small + 1;
        return container_.data_->data + container_.data_->size;
    }

    const_iterator begin() const {
        if(empty()) return nullptr;
        if (isSmall()) return &container_.small;
        return container_.data_->data;
    }

    const_iterator end() const {
        if (empty()) return nullptr;
        if (isSmall()) return &container_.small + 1;
        return container_.data_->data + container_.data_->size;
    }

    reverse_iterator rbegin() {
        return std::reverse_iterator<iterator>(end());
    }
    reverse_iterator rend() {
        return std::reverse_iterator<iterator>(begin());
    };

    const_reverse_iterator rbegin() const {
        return std::reverse_iterator<const_iterator>(end());
    }
    const_reverse_iterator rend() const {
        return std::reverse_iterator<const_iterator>(begin());
    };

    bool empty() const {
        return !hasElements_;
    }

    size_t size() const {
        if (empty()) return 0;
        if (isSmall()) return 1;
        return container_.data_->size;
    };

    size_t capacity() const {
        if (empty()) return 1;
        if (isSmall()) return 1;
        return container_.data_->capacity;
    };

    // capacity always >= 1
    // strong
    void reserve(size_t new_size) {
        if (capacity() >= new_size && isOnlyUser(container_.data_)) {
            this->container_.data_->size = new_size;
            return;
        }

        size_t tmp_size = size();
        auto tmp = allocate<T>(new_size);
        try {
            std::uninitialized_copy(begin(), end(), tmp->data);
        } catch (...) {
            deleteLink(tmp);
            throw;
        }
        tmp->size = tmp_size;
//        auto tmp = getNewBuffer(new_size);
// TODO: what if empty, but data_ != nullptr?
        if (!empty()) {
            if (!isSmall())
                deleteLink(container_.data_);
            else
                container_.small.~T();
        }
        container_.data_ = tmp;
        isSmall_ = false;
    }

    // TODO: check it
    void shrink_to_fit() {
        if (empty() || isSmall())
            return;
        size_t new_size = size();
        auto tmp = allocate<T>(new_size);
        std::uninitialized_copy(begin(), end(), tmp->data);
        tmp->size = new_size;
        container_.data_ = tmp;
        isSmall_ = false;
    }

    void resize(size_t new_size, T const& val) {
        if (size() == new_size) return;
        if (new_size == 0) {
            clear();
            return;
        }
        // new_size <= old_size (=> big, not small)
        if (size() >= new_size) {
            for (auto it = begin() + new_size; it != end() ; ++it) {
                it->~T();
            }
            container_.data_->size = new_size;
            return;
        }
        // new_size > old_size (big or small->big)
        reserve(new_size);
        std::uninitialized_fill(end(), begin() + new_size, val);
        container_.data_->size = new_size;
    }

    void resize(size_t size) {
        resize(size, T());
    }

    void clear() {
        if (empty()) return;
        getNewBuffer();
        for (const auto &item : *this) {
           item.~T();
        }
        if (!isSmall() && !empty())
            container_.data_->size = 0;
        hasElements_ = false;
    };

    iterator insert(const_iterator pos, T val) {
//        std::cout << "insert " << val.data << std::endl;
        if (pos == end() || empty()) {
            push_back(val);
            return end() - 1;
        }
        // big or small->big
        getNewBuffer();
        auto p = pos - begin();
        if (size() == capacity()) {
            reserve(size() * 2);
        }
        auto it = end();
        new (it) T(*(it - 1));
        ++container_.data_->size;
        --it;
        for ( ; it != begin() + p; --it) {
//            std::cout << "shift" << std::endl;
           *it = *(it - 1);
        }
        auto place = begin() + p;
        *place = val;
        return place;
    };

    iterator erase(const_iterator pos) {
        if (pos == end()) {
            pop_back();
            return end();
        }
        return erase(pos, pos + 1);
    };

    iterator erase(const_iterator first, const_iterator last) {
        if (last - first == 1 && last == end()) {
            pop_back();
            return end();
        }
        // big
        getNewBuffer();
        auto distance = last - first;
        auto it = const_cast<iterator>(first);
        for ( ; it + distance != end(); ++it) {
           *it = *(it + distance);
//            (it + distance)->~T();
        }
        for (auto it2 = it; it2 != end(); ++it2) {
            it2->~T();
        }
        container_.data_->size -= distance;
        if (container_.data_->size == 0) hasElements_ = false;
        return it;
    };


 private:
    bool isSmall_, hasElements_;
    union container {
        container(size_t n, T const& v) : data_(allocate<T>(n)) {
            std::uninitialized_fill(data_->data, data_->data + n, v);
        }
        explicit container(T const& v) : small(v) {}

        container() : data_(nullptr) {};

        ~container() {}

        ::data<T>* data_;
        T small;
    } container_;

    bool isSmall() const {
        return isSmall_;
    }

    void getNewBuffer() {
        if (isSmall())
            return;
        if (isOnlyUser(container_.data_))
            return;
        auto tmp = allocate<T>(capacity());
        try {
            std::uninitialized_copy(begin(), end(), tmp->data);
        } catch (...) {
            deleteLink(tmp);
            throw;
        }
        tmp->size = size();
        deleteLink(container_.data_);
        container_.data_ = tmp;
    }

    template <typename U>
    friend void swap(vector<U>& lhs, vector<U>& rhs);
};

template <typename T>
void swap(vector<T>& lhs, vector<T>& rhs) {
    if (&lhs == &rhs) return;
    if (lhs.empty()) {
        lhs = rhs;
        rhs.clear();
        return;
    }
    if (rhs.empty()) {
        rhs = lhs;
        lhs.clear();
        return;
    }
    if (lhs.isSmall() && rhs.isSmall()) {
        std::swap(lhs.container_.small, rhs.container_.small);
        return;
    }
    if (!lhs.isSmall() && !rhs.isSmall()) {
        std::swap(lhs.container_.data_, rhs.container_.data_);
        return;
    }
    if (lhs.isSmall()) {
        auto data = rhs.container_.data_;
//        rhs.container_.small = lhs.container_.small;
        new (&rhs.container_.small) T(lhs.container_.small);
        lhs.container_.small.~T();
        lhs.container_.data_ = data;
    } else {
        auto data = lhs.container_.data_;
//        lhs.container_.small = lhs.container_.small;
        new (&lhs.container_.small) T(rhs.container_.small);
        rhs.container_.small.~T();
        rhs.container_.data_ = data;
    }
    std::swap(lhs.isSmall_, rhs.isSmall_);
}

// sign(return) == sign(a - b)
template <typename T>
int cmp(vector<T> const& a, vector<T> const& b) {
    auto it1 = a.begin(), it2 = b.begin();
    for ( ; it1 != a.end() && it2 != b.end(); ++it1, ++it2) {
        if (*it1 != *it2) {
            return *it1 < *it2 ? -1 : 1;
        }
    }
    if (it1 != a.end())
        return 1;
    if (it2 != b.end())
        return -1;
    return 0;
}


template <typename T>
bool operator==(vector<T> const& a, vector<T> const& b) {
    return cmp(a, b) == 0;
}

template <typename T>
bool operator!=(vector<T> const& a, vector<T> const& b) {
    return !(a == b);
}

template <typename T>
bool operator<=(vector<T> const& a, vector<T> const& b) {
    return cmp(a, b) <= 0;
}

template <typename T>
bool operator>=(vector<T> const& a, vector<T> const& b) {
    return cmp(a, b) >= 0;
}

template <typename T>
bool operator>(vector<T> const& a, vector<T> const& b) {
    return cmp(a, b) > 0;
}

template <typename T>
bool operator<(vector<T> const& a, vector<T> const& b) {
    return cmp(a, b) < 0;
}



#endif //EXAM1_VECTOR_H
