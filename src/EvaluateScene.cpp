#include "scene/EvaluateScene.hpp"

namespace Scene {
    using std::string;


EvaluateScene::EvaluateScene( VisualScene* visual_scene )
    : m_visual_scene( visual_scene )
{
}

EvaluateScene::~EvaluateScene()
{
    for( auto it=m_render.begin(); it!=m_render.end(); ++it ) {
        delete *it;
    }
    m_render.clear();
}


VisualScene*
EvaluateScene::visualScene()
{
    return m_visual_scene;
}

const VisualScene*
EvaluateScene::visualScene() const
{
    return m_visual_scene;
}

bool
EvaluateScene::enabled() const
{
    return m_enabled;
}

void
EvaluateScene::setEnabled( bool enabled )
{
    m_enabled = enabled;
}

const string&
EvaluateScene::id() const
{
    return m_id;
}

void
EvaluateScene::setId( const string& id )
{
    m_id = id;
}

const string&
EvaluateScene::sid() const
{
    return m_sid;
}

void
EvaluateScene::setSid( const string& sid )
{
    m_sid = sid;
}

size_t
EvaluateScene::renderItems() const
{
    return m_render.size();
}

Render*
EvaluateScene::renderItem( size_t ix )
{
    return m_render[ix];
}


const Render*
EvaluateScene::renderItem( size_t ix ) const
{
    return m_render[ix];
}


Render*
EvaluateScene::addRenderItem()
{
    Render* r = new Render( this );
    m_render.push_back( r );
    return r;
}


} // of namespace Scene

