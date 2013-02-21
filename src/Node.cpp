#include <sstream>
#include "scene/Log.hpp"
#include "scene/Scene.hpp"
#include "scene/DataBase.hpp"
#include "scene/Node.hpp"
#include "scene/InstanceGeometry.hpp"

namespace Scene {
    using std::string;
    using std::stringstream;

static const string package = "Scene.Node";


Node::Node( Library<Node>* library_nodes, const std::string& id )
    : m_library_nodes( library_nodes ),
      m_id( id ),
      m_parent( NULL )
{
    includeInAllProfiles();
//    Logger log = getLogger( package + ".Node" );
}

void
Node::setSid(  const std::string& sid )
{
    m_sid = sid;
}

Node::~Node()
{

    for( auto it = m_instance_geometry.begin(); it!= m_instance_geometry.end(); ++it ) {
        delete *it;
    }

    // Remove child-parent links
    if( m_parent != NULL ) {
        for( auto it=m_parent->m_children.begin(); it!=m_parent->m_children.end(); ++it ) {
            if( *it == this ) {
                m_parent->m_children.erase( it );
                break;
            }
        }
        m_parent = NULL;
    }

    for( auto it = m_children.begin(); it != m_children.end(); ++it ) {
        (*it)->m_parent = NULL;
    }
}

const Node*
Node::parent() const
{
    return m_parent;
}

bool
Node::setParent( Node* parent )
{
    // un-set parent
    if( m_parent != NULL ) {
        for( auto it=m_parent->m_children.begin(); it!=m_parent->m_children.end(); ++it ) {
            if( *it == this ) {
                m_parent->m_children.erase( it );
                break;
            }
        }
        m_parent = NULL;
    }

    // TODO: Check if sid is legal within parent node.
    // TODO: Check if this creates a cycle
    if( parent != NULL ) {
        m_parent = parent;
        m_parent->m_children.push_back( this );
    }
    return true;
}

size_t
Node::children() const
{
    return m_children.size();
}

const Node*
Node::child( const size_t index ) const
{
    return m_children[ index ];
}



const string
Node::debugString() const
{
    stringstream o;
    o << "Node(";
    if( !m_id.empty() ) {
        o << "id=" << m_id;
    }
    else if( !m_sid.empty() ) {
        o << "sid=" << m_sid;
    }
    else {
        o << this;
    }
    o << ")";
    return o.str();
}

void
Node::addInstanceCamera( const std::string& sid,
                         const std::string& url )
{
    m_instance_camera.push_back( url );
}


size_t
Node::transformIndexBySid( const std::string& sid ) const
{
    if( !sid.empty() ) {
        for( size_t i=0; i<m_transforms_.size(); i++ ) {
            if( m_transforms_[i].m_sid == sid ) {
                return i;
            }
        }
    }
    return ~0u;
}

size_t
Node::transformAdd( const std::string& sid )
{
    if( !sid.empty() ) {
        size_t ix = transformIndexBySid(sid);
        if( ix != ~0u ) {
            Logger log = getLogger( package + ".transformAdd" );
            SCENELOG_WARN( log, "SID '" << sid << "' is already present in node, returing existing." );
            return ix;
        }
    }
    m_transforms_.resize( m_transforms_.size()+1 );
    m_transforms_.back().m_type = TRANSFORM_N;
    m_transforms_.back().m_sid = sid;

    touchStructureChanged();
    m_library_nodes->moveForward( *this );
    m_library_nodes->dataBase()->moveForward( *this );

    return m_transforms_.size()-1u;
}


void
Node::transformSetTranslate( size_t ix, float x, float y, float z )
{
    if( ix < m_transforms_.size() ) {
        m_transforms_[ix].m_type = TRANSFORM_TRANSLATE;
        m_transforms_[ix].m_value = Value::createFloat3( x, y, z );

        // Propogate minor update through the hierarchy
        touchValueChanged();
        m_library_nodes->moveForward( *this );
        m_library_nodes->dataBase()->moveForward( *this );
    }
    else {
        Logger log = getLogger( package + ".transformSetTranslate" );
        SCENELOG_ERROR( log, "Illegal transform index " << ix );
    }
}

void
Node::transformSetRotate( size_t ix,
                          float axis_x, float axis_y, float axis_z, float angle_rad )
{
    if( ix < m_transforms_.size() ) {
        m_transforms_[ix].m_type = TRANSFORM_ROTATE;
        m_transforms_[ix].m_value = Value::createFloat4( axis_x, axis_y, axis_z, angle_rad );

        // Propogate minor update through the hierarchy
        touchValueChanged();
        m_library_nodes->moveForward( *this );
        m_library_nodes->dataBase()->moveForward( *this );
    }
    else {
        Logger log = getLogger( package + ".transformSetRotate" );
        SCENELOG_ERROR( log, "Illegal transform index " << ix );
    }
}

void
Node::transformSetMatrix( size_t ix, const Value& matrix )
{
    Logger log = getLogger( package + ".transformSetRotate" );
    if( matrix.type() != VALUE_TYPE_FLOAT4X4 ) {
        SCENELOG_ERROR( log, "argument is not of type float4x4." );
    }
    else if( ix < m_transforms_.size() ) {
        m_transforms_[ix].m_type = TRANSFORM_MATRIX;
        m_transforms_[ix].m_value = matrix;
        m_transforms_[ix].m_value.valueChanged().touch();

        // Propogate minor update through the hierarchy
        touchValueChanged( );
        m_library_nodes->moveForward( *this );
        m_library_nodes->dataBase()->moveForward( *this );
    }
    else {
        SCENELOG_ERROR( log, "Illegal transform index " << ix );
    }
}

void
Node::transformSetScale( size_t ix, float x, float y, float z )
{
    if( ix < m_transforms_.size() ) {
        m_transforms_[ix].m_type = TRANSFORM_SCALE;
        m_transforms_[ix].m_value = Value::createFloat3( x, y, z );

        // Propogate minor update through the hierarchy
        touchValueChanged();
        m_library_nodes->moveForward( *this );
        m_library_nodes->dataBase()->moveForward( *this );
    }
    else {
        Logger log = getLogger( package + ".transformSetScale" );
        SCENELOG_ERROR( log, "Illegal transform index " << ix );
    }
}

void
Node::transformSetLookAt( size_t ix,
                          float eye_x, float eye_y, float eye_z,
                          float coi_x, float coi_y, float coi_z,
                          float up_x, float up_y, float up_z )
{
    if( ix < m_transforms_.size() ) {
        m_transforms_[ix].m_type = TRANSFORM_LOOKAT;
        m_transforms_[ix].m_value = Value::createFloat3x3( eye_x, eye_y, eye_z,
                                                           coi_x, coi_y, coi_z,
                                                           up_x, up_y, up_z );

        // Propogate minor update through the hierarchy
        touchValueChanged();
        m_library_nodes->moveForward( *this );
        m_library_nodes->dataBase()->moveForward( *this );
    }
    else {
        Logger log = getLogger( package + ".transformSetLookAt" );
        SCENELOG_ERROR( log, "Illegal transform index " << ix );
    }
}




void
Node::addInstanceNode( const std::string& sid,
                       const std::string& name,
                       const std::string& url,
                       const std::string& proxy )
{
    m_instance_node.push_back( url );
    touchStructureChanged();
    m_library_nodes->moveForward( *this );
    m_library_nodes->dataBase()->moveForward( *this );
}

size_t
Node::instanceNodes() const
{
    return m_instance_node.size();
}

const std::string&
Node::instanceNode( size_t ix ) const
{
    return m_instance_node[ix];
}


void
Node::add( InstanceGeometry* instance_geometry )
{
    m_instance_geometry.push_back( instance_geometry );
    touchStructureChanged();
    m_library_nodes->moveForward( *this );
    m_library_nodes->dataBase()->moveForward( *this );
}


const size_t
Node::geometryInstances() const
{
    return m_instance_geometry.size();
}

const InstanceGeometry*
Node::geometryInstance( const size_t index ) const
{
    return m_instance_geometry[index];
}

void
Node::addToLayer( const std::string& layer )
{
    for( auto it=m_layers.begin(); it!=m_layers.end(); ++it ) {
        if( *it == layer ) {
            return;
        }
    }
    m_layers.push_back( layer );
    touchStructureChanged();
    m_library_nodes->moveForward( *this );
    m_library_nodes->dataBase()->moveForward( *this );
}

void
Node::excludeFromLayer( const std::string& layer )
{
    for( auto it=m_layers.begin(); it!=m_layers.end(); ++it ) {
        if( *it == layer ) {
            m_layers.erase( it );
            touchStructureChanged();
            m_library_nodes->moveForward( *this );
            m_library_nodes->dataBase()->moveForward( *this );
            return;
        }
    }
}



const unsigned int
Node::profileMask() const
{
    return m_profile_mask;
}

void
Node::includeInNoProfiles()
{
    m_profile_mask = 0u;
}

void
Node::includeInAllProfiles()
{
    m_profile_mask =
            PROFILE_BRIDGE |
            PROFILE_CG |
            PROFILE_COMMON |
            PROFILE_GLES |
            PROFILE_GLES2 |
            PROFILE_GLSL;
}

void
Node::includeInProfile( ProfileType profile )
{
    m_profile_mask = m_profile_mask | profile;
}

void
Node::excludeFromProfile( ProfileType profile )
{
    m_profile_mask = m_profile_mask & (~profile);
}



size_t
Node::transforms() const
{
    return m_transforms_.size();
}

size_t
Node::instanceLights() const
{
    return m_light_instances.size();
}

const std::string&
Node::instanceLightSid( size_t index ) const
{
    return m_light_instances[ index ].m_sid;
}


void
Node::addInstanceLight( const std::string& ref, const std::string& sid )
{
    m_light_instances.resize( m_light_instances.size() + 1 );
    m_light_instances.back().m_sid = sid;
    m_light_instances.back().m_ref = ref;
    touchStructureChanged();
    m_library_nodes->moveForward( *this );
    m_library_nodes->dataBase()->moveForward( *this );
}

size_t
Node::instanceLightIndexBySid( const std::string& sid ) const
{
    for( size_t i=0; i<m_light_instances.size(); i++ ) {
        if( m_light_instances[i].m_sid == sid ) {
            return i;
        }
    }
    return ~0u;
}

const std::string&
Node::instanceLightRef( size_t index ) const
{
    return m_light_instances[ index ].m_ref;
}



} // of namespace Scene

