#pragma once

#include <random>

namespace mdlr
{
    inline float random(float min, float max)
    {
        return double(rand()) / double(RAND_MAX) * (max - min) + min;
    }

    constexpr float clamp(const float& in, const float& lo, const float& hi)
    {
        return (in < lo ? lo : (in > hi ? hi : in));
    }

    struct RisingEdgeDetector
    {
        float last = 0.f;
        bool armed = true;

        bool process(float in)
        {
            bool result = false;
            if (in > last && armed)
            {
                result = true;
                armed = false;
            }
            else if (in < last)
                armed = true;

            last = in;
            return result;
        }
    };

    template <typename T, int ChunkSize = 32>
    struct stable_vector
    {
        using chunk = std::array<T, ChunkSize>;
        std::vector<chunk> chunks;
        size_t items = 0;

        stable_vector() = default;
        stable_vector(size_t size, const T& value = {})
        {
            resize(size);
            for (size_t i = 0; i < size; i++)
                at(i) = value;
        }
        stable_vector(size_t size, T&& value)
        {
            resize(size);
            for (size_t i = 0; i < size; i++)
                at(i) = value;
        }
        stable_vector(std::initializer_list<T> list)
        {
            for (const auto& v: list)
                push_back(v);
        }

        size_t size() const { return items; }
        bool empty() const { return size() == 0; }

        void resize(size_t s, const T& value = {})
        {
            reserve(s);
            for (size_t i = items; i < s; i++)
                at(i) = value;
            items = s;
        }
        void reserve(size_t s)
        {
            size_t chunk_index = s / ChunkSize;
            while (chunk_index >= chunks.size())
                chunks.push_back({});
        }

        void push_back(const T& value)
        {
            size_t chunk_index = items / ChunkSize;
            if (chunk_index >= chunks.size())
                chunks.push_back({});
            chunks[chunk_index][items%ChunkSize] = value;
            items++;
        }
        void push_back(T&& value)
        {
            size_t chunk_index = items / ChunkSize;
            if (chunk_index >= chunks.size())
                chunks.push_back({});
            chunks[chunk_index][items%ChunkSize] = std::move(value);
            items++;
        }
        T& emplace_back(T&& value)
        {
            size_t chunk_index = items / ChunkSize;
            if (chunk_index >= chunks.size())
                chunks.push_back({});
            chunks[chunk_index][items%ChunkSize] = std::move(value);
            auto& result = chunks[chunk_index][items%ChunkSize];
            items++;
            return result;
        }

        void pop_back() { items--; }

        T& front() { return at(0); }
        const T& front() const { return at(0); }
        T& back() { return at(size() - 1); }
        const T& back() const { return at(size() - 1); }

        T& operator[](size_t index) { return chunks[index / ChunkSize][index % ChunkSize]; }
        const T& operator[](size_t index) const { return chunks[index / ChunkSize][index % ChunkSize]; }
        T& at(size_t index) { return chunks.at(index / ChunkSize).at(index % ChunkSize); }
        const T& at(size_t index) const { return chunks.at(index / ChunkSize).at(index % ChunkSize); }

        template <bool Const>
        struct base_iterator
        {
            using parent_type = std::conditional_t<Const, const stable_vector*, stable_vector*>;
            parent_type parent = nullptr;
            size_t index = 0;

            T& operator*() requires(!Const) { return parent->at(index); }
            const T& operator*() const { return parent->at(index); }
            base_iterator& operator++() { index++; return *this; }
            base_iterator operator++(int) { auto res = *this; (*this)++; return res; }
            constexpr bool operator==(const base_iterator& other) const { return parent == other.parent && index == other.index; }
            constexpr bool operator!=(const base_iterator& other) const { return !(*this == other); }
        };
        using iterator = base_iterator<false>;
        using const_iterator = base_iterator<true>;
        
        iterator begin() { return iterator { this, 0 }; }
        iterator end() { return iterator { this, size() }; }
        const_iterator begin() const { return const_iterator { this, 0 }; }
        const_iterator end() const { return const_iterator { this, size() }; }
    };
}