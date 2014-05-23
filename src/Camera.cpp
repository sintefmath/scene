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

#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/Library.hpp"
#include "scene/Camera.hpp"

namespace Scene {
    using std::string;

static const string package = "Scene.Camera";



Camera::Camera( Library<Camera>* library_cameras, const std::string& id )
    : m_library_cameras( library_cameras ),
      m_type( CAMERA_N ),
      m_id( id ),
      m_custom_matrix( Value::createFloat4x4() )
{
}

const CameraType
Camera::cameraType() const
{
    return m_type;
}

const std::string&
Camera::id() const
{
    return m_id;
}

const std::string&
Camera::sid() const
{
    return m_sid;
}


float
Camera::near() const
{
    if( (m_type != CAMERA_PERSPECTIVE) && (m_type != CAMERA_ORTHOGONAL) ) {
        Logger log = getLogger( package + ".near" );
        SCENELOG_ERROR( log, "Camera isn't perspective nor orthogonal." );
    }
    return m_near;
}

float
Camera::far() const
{
    if( (m_type != CAMERA_PERSPECTIVE) && (m_type != CAMERA_ORTHOGONAL) ) {
        Logger log = getLogger( package + ".far" );
        SCENELOG_ERROR( log, "Camera isn't perspective nor orthogonal." );
    }
    return m_far;
}

float
Camera::magX() const
{
    if( m_type != CAMERA_ORTHOGONAL ) {
        Logger log = getLogger( package + ".magX" );
        SCENELOG_ERROR( log, "Camera does not have an orthogonal projection." );
    }
    return m_scale_x;
}

float
Camera::magY() const
{
    if( m_type != CAMERA_ORTHOGONAL ) {
        Logger log = getLogger( package + ".magY" );
        SCENELOG_ERROR( log, "Camera does not have an orthogonal projection." );
    }
    return m_scale_y;
}

float
Camera::fovX() const
{
    if( m_type != CAMERA_PERSPECTIVE ) {
        Logger log = getLogger( package + ".fovX" );
        SCENELOG_ERROR( log, "Camera does not have a perspective projection." );
    }
    return m_scale_x;
}

float
Camera::fovY() const
{
    if( m_type != CAMERA_PERSPECTIVE ) {
        Logger log = getLogger( package + ".fovY" );
        SCENELOG_ERROR( log, "Camera does not have a perspective projection." );
    }
    return m_scale_y;
}


void
Camera::setOrthogonal( float mag_x,
                       float mag_y,
                       float near,
                       float far )
{
    m_type = CAMERA_ORTHOGONAL;
    m_scale_x = mag_x;
    m_scale_y = mag_y;
    m_near = near;
    m_far = far;

    // Set a minor update through the chain
    touchValueChanged();
    m_library_cameras->moveForward( *this );
    m_library_cameras->dataBase()->moveForward( *this );
}

void
Camera::setPerspective( float fov_x,
                        float fov_y,
                        float near,
                        float far )
{
    m_type = CAMERA_PERSPECTIVE;
    m_scale_x = fov_x;
    m_scale_y = fov_y;
    m_near = near;
    m_far = far;

    // Set a minor update through the chain
    touchValueChanged();
    m_library_cameras->moveForward( *this );
    m_library_cameras->dataBase()->moveForward( *this );
}


const Value&
Camera::customMatrix() const
{
    return m_custom_matrix;
}

void
Camera::setCustomMatrix( const Value& matrix )
{
    if( matrix.type() == VALUE_TYPE_FLOAT4X4 ) {
        m_type = CAMERA_CUSTOM_MATRIX;
        m_custom_matrix = matrix;
        m_custom_matrix.valueChanged().touch();

        // Set a minor update through the chain
        touchValueChanged();
        m_library_cameras->moveForward( *this );
        m_library_cameras->dataBase()->moveForward( *this );
    }
    else {
        Logger log = getLogger( package + ".setCustomMatrix" );
        SCENELOG_ERROR( log, "Value is not of type float4x4." );
    }
}


} // of namespace Scene
