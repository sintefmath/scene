/* Copyright STIFTELSEN SINTEF 2014
 * 
 * This file is part of Scene.
 * 
 * Scene is free software: you can redistribute it and/or modifyit under the
 * terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 * 
 * Scene is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more
 * details.
 *  
 * You should have received a copy of the GNU Affero General Public License
 * along with the Scene.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "scene/Scene.hpp"

namespace Scene {
    namespace Runtime {


/** A key consisting of an n-tuple of pointers to be used as a unordered_map key.
  *
  *
  */
template<size_t N>
class CacheKey
{
public:
    CacheKey()
    {
        static_assert( N>0, "N must at least be one" );
    }

    CacheKey( const void* p0 )
    {
        static_assert( N==1, "Constructor only allowed when N==1" );
        m_pointers[0] = p0;
    }

    CacheKey( const void* p0, const void* p1 )
    {
        static_assert( N==2, "Constructor only allowed when N==2" );
        m_pointers[0] = p0;
        m_pointers[1] = p1;
    }

    CacheKey( const void* p0, const void* p1, const void* p2 )
    {
        static_assert( N==3, "Constructor only allowed when N==3" );
        m_pointers[0] = p0;
        m_pointers[1] = p1;
        m_pointers[2] = p2;
    }

    CacheKey( const void* p0, const void* p1, const void* p2, const void* p3 )
    {
        static_assert( N==4, "Constructor only allowed when N==4" );
        m_pointers[0] = p0;
        m_pointers[1] = p1;
        m_pointers[2] = p2;
        m_pointers[3] = p3;
    }

    const void*&
    operator[]( size_t ix )
    {
        return m_pointers[ ix ];
    }

    const void*&
    operator[]( size_t ix ) const
    {
        return m_pointers[ ix ];
    }

    bool
    operator==( const CacheKey<N>& other ) const
    {
        for(size_t i=0; i<N; i++) {
            if( m_pointers[i] != other.m_pointers[i] ) {
                return false;
            }
        }
        return true;
    }


    // Hash function
    size_t
    hash( /*const CacheKey<N>& key*/ ) const
    {
        // Hash-function-hack
        size_t hash = reinterpret_cast<size_t>( m_pointers[0] );
        for( size_t i=1; i<N; i++) {
            // multiply by 13?
            // Note: key values can be zero on upper -> giving bad hash'es
            hash ^= (hash <<13) ^ reinterpret_cast<size_t>( m_pointers[i] );
        }
        return hash;
    }

    size_t
    operator()( const CacheKey<N>& key ) const
    {
        return key.hash();
    }



protected:
    const void*  m_pointers[N];

};

    } // of namespace Runtime
} // of namespace Scene

namespace std {

    template<size_t N>
    struct hash< Scene::Runtime::CacheKey<N> >
    {
        size_t
        operator ()( const Scene::Runtime::CacheKey<N>& key ) const
        {
            return key.hash();
        }


    };


}
