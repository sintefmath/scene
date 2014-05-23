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

#include <scene/Scene.hpp>
#include <scene/runtime/Resolver.hpp>
#include <scene/runtime/RenderList.hpp>
#include <scene/runtime/TransformCache.hpp>
namespace Scene {
    namespace Tools {

/** Update the bounding box of a given geometry.
 *
 * If geometry or any of the buffers used by the geometry has changed since the
 * bounding box was last updated, this function runs through all the vertex
 * positions in the geometry and determines the axis-aligned bounding box.
 *
 * \param geometry  The geometry to update.
 * \returns True for a non-empty bounding box.
 */
bool
updateBoundingBox( Geometry* geometry );

/** Update the bounding boxes of all geometries in a database.
 *
 * Convenience function, see updateBoundingBox for details.
 */
void
updateBoundingBoxes( DataBase& database );


/** Determine the axis-aligned bounding box for a visual scene.
 *
 * Function transforms the bounding boxes of all geometries in a render list
 * into the coordinate system of the root of the visual scene used to create
 * the render list (aka world space), and finds a new bounding box that
 * covers all geometry.
 *
 * Note that this function assumes that all bounding boxes are up-to-date.
 *
 */
bool
visualSceneExtents( Scene::Value& bbmin,
                    Scene::Value& bbmax,
                    const Runtime::RenderList& renderlist );



    } // of namespace Tools
} // of namespace Scene
