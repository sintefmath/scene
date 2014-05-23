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
