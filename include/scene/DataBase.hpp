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

#include <GL/glew.h>
#include <string>
#include <vector>
#include <unordered_map>

#include "scene/Asset.hpp"
#include "scene/Scene.hpp"
#include "scene/Value.hpp"
#include "scene/Effect.hpp"
#include "scene/Primitives.hpp"
#include "scene/Library.hpp"
#include <scene/SeqPos.hpp>

namespace Scene {

/** Asset database
  *
  * This class holds the set of assets required for rendering. It can be
  * layered on top on another database, such that if one asset isn't found
  * in this database, the fallback is searched and so on.
  *
  * The assets are organized into a set of libraries according to type.
  *
  * No checking of references between assets are done (e.g. checking that
  * the source buffers required by a geometry exists). This is handled by the
  * runtime, see Scene::Runtime::Resolver.
  *
  * Rendering of contents is handled by the runtime, in particular, RenderList
  * is an abstract renderlist and GLSLRenderList provides a specialization for
  * OpenGL.
  *
  * Import and export to files are handled by the Importer and Exporter classes
  * in the XML namespace.
  *
  * The contents of the database is organized in a set of libraries, each
  * containing a particular set of assets:
  * - Source buffers are named buffers containing either vertex attributes
  *   like positions and normals, or indices.
  * - Geometries describe geometric shapes like triangle meshes.
  * - Effects describe a set of rendering passes, including shader programs
  *   and render state. Effects exports a set of parameters that can be
  *   manipulated.
  * - Materials are instances of effect. They can set values to the parameters
  *   exported by the effects.
  * - Images can be used as textures or as render targets. They are referenced
  *   through effect parameters.
  * - Nodes are used to encode transform hierarchies. Node hierarchies exist
  *   either in the node library or directly inside visual scenes.
  * - Visual scenes describe a rendering entry point, which set of passes to
  *   render, which camera to use etc.
  *
  *
  */
class DataBase : public StructureValueSequences
{
public:

    DataBase( DataBase* fallback = NULL );

    ~DataBase();

    const DataBase*
    fallback() const { return m_fallback; }



    /** Delete all contents in the database.
      *
      * \note Remember to clear everything that has references/pointers into
      * the database, like Resolver, RenderList, etc.
      */
    void
    clear();

    const Asset&
    asset() const { return m_asset; }

    Asset&
    asset() { return m_asset; }

    void
    setAsset( const Asset& asset ) { m_asset = asset; }



    /** Get hold of a library for a particular type of assets.
      *
      * This function retrieves a reference to a library to a particular type
      * of assets, depending on the template type. Currently supported template
      * types are Geometry, Image, Camera, Effect, Material, SourceBuffer, and
      * VisualScene.
      */
    template<class T>
    Library<T>&
    library();

    /** Get hold of a library for a particular type of assets.
      *
      * This function retrieves a reference to a library to a particular type
      * of assets, depending on the template type. Currently supported template
      * types are Geometry, Image, Camera, Effect, Material, SourceBuffer, and
      * VisualScene.
      */
    template<class T>
    const Library<T>&
    library() const;


protected:
    const DataBase*                          m_fallback;
    Asset                                    m_asset;
    Library<Geometry>                        m_library_geometries;
    Library<Image>                           m_library_images;
    Library<Camera>                          m_library_cameras;
    Library<Light>                           m_library_lights;
    Library<Effect>                          m_library_effects;
    Library<Material>                        m_library_materials;
    Library<Node>                            m_library_nodes;
    Library<SourceBuffer>                    m_library_source_buffers;
    Library<VisualScene>                     m_library_visual_scenes;
};

}  // of namespace Scene
