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

#include <tinia/renderlist/DataBase.hpp>
#include <scene/DataBase.hpp>
#include <scene/SeqPos.hpp>
#include <scene/runtime/Resolver.hpp>
#include <scene/runtime/RenderList.hpp>
#include <scene/runtime/TransformCache.hpp>

namespace Scene {
    namespace Tinia {

/** Holds cached items for exporter render lists. */
class Bridge
{
public:
    Bridge( const DataBase&     database,
                ProfileType         profile,
                const std::string&  platform = "" );

    ~Bridge();

    Runtime::Resolver&
    resolver();

    bool
    build( const std::string&  visual_scene );

    const tinia::renderlist::DataBase&
    renderListDataBase() const;

protected:
    const DataBase&             m_database;
    Runtime::Resolver                    m_resolver;
    tinia::renderlist::DataBase m_renderlist_db;
    Runtime::RenderList                  m_renderlist;
    Runtime::TransformCache              m_transform_cache;
    SeqPos                      m_last_update;

    void
    push();

};



    } // of namespace Runtime
} // of namespace Scene
