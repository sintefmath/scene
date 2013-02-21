#pragma once

#include <vector>
#include <unordered_map>
#include <scene/runtime/CacheKey.hpp>

namespace Scene {
    namespace Runtime {

#if 1

template<size_t N>
class CacheLUT
{
public:
    typedef CacheKey<N> Key;

    void
    clear()
    { m_map.clear(); }

    static const size_t
    none()
    { return static_cast<size_t>( ~0ul ); }

    void
    insert( const Key& key, const size_t value )
    { m_map[ key ] = value; }

    const size_t
    find( const Key& key )
    {
        auto it = m_map.find( key );
        if( it == m_map.end() ) {
            return none();
        }
        else {
            return it->second;
        }
    }


protected:
    std::unordered_map<CacheKey<N>,size_t>  m_map;

};



#else
template<size_t N>
class CacheLUT
{
public:
    typedef CacheKey<N> Key;

    void
    clear()
    { m_items.clear(); }

    static const size_t
    none()
    { return static_cast<size_t>( ~0ul ); }

    size_t
    find( const CacheKey<N>& key )
    {
        for( size_t i=0; i<m_items.size(); i++ ) {
            if( m_items[i].m_key == key ) {
                return m_items[i].m_value;
            }
        }
        return none();
    }

    void
    insert( const CacheKey<N>& key, const size_t value )
    {
        m_items.resize( m_items.size()+1);
        m_items.back().m_key   = key;
        m_items.back().m_value = value;
    }

protected:
    struct Item {
        CacheKey<N>     m_key;
        size_t          m_value;
    };
    std::vector<Item>   m_items;
};

#endif


    }
}
