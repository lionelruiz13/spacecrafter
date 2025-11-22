#ifndef _DEPORTED_LINEAR_ALLOCATOR_HPP_
#define _DEPORTED_LINEAR_ALLOCATOR_HPP_

template <typename T>
class DeportedLinearAllocator
{
public:
    DeportedLinearAllocator() = delete;
    DeportedLinearAllocator(void *buffer, uint32_t size):
        buffer(reinterpret_cast<T*>(buffer)),
        capacity(size / sizeof(T))
    {
    }
    using value_type = T;
    using size_type = uint32_t;
    using propagate_on_container_copy_assignment = std::true_type; // Copy allocator internals on object copy
    using propagate_on_container_move_assignment = std::true_type; // Move allocator internals on object move
    T *allocate(size_t n)
    {
        if (size + n > capacity)
            throw std::bad_alloc();
        T *res = buffer+size;
        size += n;
        return res;
    }
    void deallocate(T *ptr, size_t n) noexcept
    {
        if (ptr == buffer + (size - n))
            size -= n; // Enable stack-like allocation/deallocation pattern
    }
    void reset() noexcept
    {
        size = 0U;
    }
    bool operator==(const DeportedLinearAllocator &other) const noexcept
    {
        return (buffer == other.buffer) && (capacity == other.capacity) && (size == other.size);
    }
private:
    T *buffer;
    uint32_t capacity;
    uint32_t size = 0U;
};

#endif
