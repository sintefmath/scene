#include "scene/DataBase.hpp"
#include "scene/VisualScene.hpp"

namespace Scene {


VisualScene::VisualScene( Library<VisualScene>* library_visual_scenes, const std::string& id )
    : m_id( id )
{

}

VisualScene::~VisualScene()
{
    for( auto it = m_evaluate_scene.begin(); it!=m_evaluate_scene.end(); ++it ) {
        delete *it;
    }
    m_evaluate_scene.clear();
}


EvaluateScene*
VisualScene::addEvaluateScene()
{
    EvaluateScene* e = new EvaluateScene( this );
    m_evaluate_scene.push_back( e );
    return e;
}



} // of namespace Scene
