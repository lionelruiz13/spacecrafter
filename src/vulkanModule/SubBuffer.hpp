#ifndef SUB_BUFFER_HPP
#define SUB_BUFFER_HPP

struct SubBuffer {
    VkBuffer buffer;
    int offset;
    int size;
};

#endif /* end of include guard: SUB_BUFFER_HPP */
