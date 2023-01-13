#pragma once

#include "defs.h"

#include <vector>
#include <cstdint>

FR_NAMESPACE_BEGIN

template <typename Tag>
class Handle {
public:
    Handle() = default;
    explicit Handle(uint32_t id, uint32_t gen) : m_id(id), m_gen(gen) {}
    Handle(const Handle &other) = default;

    Handle &operator=(const Handle &other) = default;

    static inline Handle<Tag> null() { return {}; }

    bool is_null() const { return m_gen == 0; }

    operator bool() const { return !is_null(); }

    bool operator==(const Handle<Tag> &other) const { return m_id == other.m_id && m_gen == other.m_gen; }
    bool operator!=(const Handle<Tag> &other) const { return m_id != other.m_id || m_gen != other.m_gen; }

private:
    uint32_t m_id{0};
    uint32_t m_gen{0};

    template <typename, typename>
    friend class Pool;
};

template <typename T, typename HandleType>
class Pool {
public:
    HandleType alloc()
    {
        FR_ASSERT(m_objects.size() == m_gens.size());

        if (!m_free_list.empty()) {
            uint32_t id = m_free_list.back();
            m_free_list.pop_back();
            uint32_t gen = m_gens[id];
            return HandleType(id, gen);
        }

        m_objects.emplace_back();
        m_gens.emplace_back(1);
        uint32_t id = (uint32_t)(m_objects.size() - 1);
        uint32_t gen = m_gens[id];
        return HandleType(id, gen);
    }

    bool free(HandleType handle)
    {
        if (!is_valid(handle))
            return false;

        m_free_list.push_back(handle.m_id);
        m_gens[handle.m_id]++;

        return true;
    }

    bool is_valid(HandleType handle) const
    {
        if (handle.m_id >= m_objects.size())
            return false;
        if (handle.m_gen != m_gens[handle.m_id])
            return false;
        return true;
    }

    T *get(HandleType handle)
    {
        if (!is_valid(handle))
            return nullptr;

        return &m_objects[handle.m_id];
    }

    const T *get(HandleType handle) const
    {
        if (!is_valid(handle))
            return nullptr;

        return &m_objects[handle.m_id];
    }

    T *operator[](HandleType handle) { return get(handle); }
    const T *operator[](HandleType handle) const { return get(handle); }

private:
    std::vector<T> m_objects;
    std::vector<uint32_t> m_gens;
    std::vector<uint32_t> m_free_list;
};

FR_NAMESPACE_END
