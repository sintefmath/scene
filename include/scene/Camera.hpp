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

#include <string>

#include <scene/Scene.hpp>
#include <scene/Asset.hpp>
#include <scene/Value.hpp>
#include <scene/SeqPos.hpp>

#ifdef _WIN32
/**
  * WinDef.h defines near and far macros... ugh
  */
#undef near
#undef far
#endif

namespace Scene {

class Camera : public StructureValueSequences
{
    friend class Library<Camera>;
public:

    const std::string&
    id() const;

    const std::string&
    sid() const;

    const CameraType
    cameraType() const;


    /** Get distance to near-plane. */
    float
    near() const;

    /** Get distance to far-plane. */
    float
    far() const;

    /** Magnification in camera's local X-direction, only sensible for orthogonal projections. */
    float
    magX() const;

    /** Magnification in camera's local Y-direction, only sensible for orthogonal projections. */
    float
    magY() const;

    /** Field of view in camera's local X-direction, only sensible for perspective projections. */
    float
    fovX() const;

    /** Field of view in camera's local Y-direction, only sensible for perspective projections. */
    float
    fovY() const;

    const Value&
    customMatrix() const;

    void
    setCustomMatrix( const Value& matrix );

    void
    setOrthogonal( float mag_x,
                   float mag_y,
                   float near,
                   float far );

    void
    setPerspective( float fov_x,
                    float fov_y,
                    float near,
                    float far );


    const Asset&
    asset() const { return m_asset; }

    Asset&
    asset() { return m_asset; }


protected:
    Library<Camera>*   m_library_cameras;
    Asset              m_asset;
    CameraType         m_type;
    std::string        m_id;
    std::string        m_sid;
    Value              m_custom_matrix;
    float              m_scale_x; // mag for ortho, fov for persp
    float              m_scale_y; // mag for ortho, fov for persp
    float              m_near;
    float              m_far;


    Camera( Library<Camera>* library_cameras, const std::string& id );

};



}
