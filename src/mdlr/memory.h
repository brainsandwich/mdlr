#pragma once

#include <cstdlib>
#include <cstring>
#include <utility>
#include <algorithm>

namespace mdlr
{
    struct Memory
    {
        void* data = nullptr;
        bool owning : 1 = false;
        size_t size : 63 = 0;

        constexpr Memory(void* data, bool owning, size_t size)
            : data(data)
            , owning(owning)
            , size(size)
        {
            // fmt::println(">> New memory block of size {} ({})"
            //     , (size_t) size
            //     , owning ? "owning" : "reference");
        }

        constexpr ~Memory()
        {
            if (owning && data)
            {
                // fmt::println(">> Freeing memory block of size {}", (size_t) size);
                free(data);
            }
        }

        constexpr Memory() = default;
        constexpr Memory(const Memory&) = delete;
        constexpr Memory& operator=(const Memory&) = delete;

        constexpr Memory(Memory&& other)
        {
            std::swap(data, other.data);
            bool o = owning;
            owning = other.owning;
            other.owning = o;

            size_t s = size;
            size = other.size;
            other.size = s;
        }

        constexpr Memory& operator=(Memory&& other)
        {
            std::swap(data, other.data);
            bool o = owning;
            owning = other.owning;
            other.owning = o;

            size_t s = size;
            size = other.size;
            other.size = s;
            return *this;
        }

        static constexpr Memory ref(void* data, size_t size) { return Memory(data, false, size); }
        static constexpr Memory slice(Memory& memory, size_t offset, size_t size)
        {
            return Memory((char*) memory.data + offset, false, size);
        }

        static Memory allocate(size_t size) { return Memory(malloc(size), true, size); }
        static Memory copy(void* data, size_t size)
        {
            Memory mem = allocate(size);
            memcpy(mem.data, data, size);
            return mem;
        }
        static Memory copy(const Memory& other) { return copy(other.data, other.size); }
        static void copy(Memory& dst, const Memory& src) { memcpy(dst.data, src.data, std::min(src.size, dst.size)); }
    };
}