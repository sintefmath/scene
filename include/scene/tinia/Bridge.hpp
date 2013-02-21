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
