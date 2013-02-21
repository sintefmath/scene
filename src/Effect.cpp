#include <stdexcept>
#include <vector>
#include "scene/DataBase.hpp"
#include "scene/Primitives.hpp"
#include "scene/Utils.hpp"
#include "scene/Profile.hpp"
#include "scene/Log.hpp"

namespace Scene {
    using std::vector;
    using std::string;
    using std::runtime_error;
    using std::unordered_map;

static const std::string package = "Scene.Effect";

Effect::Effect( DataBase&  db, const string& id )
: m_db( db ),
  m_id( id ),
  m_profile_common( NULL ),
  m_profile_glsl( NULL ),
  m_profile_gles( NULL ),
  m_profile_gles2( NULL )
{
}

Effect::Effect( Library<Effect>* library_effects, const std::string& id )
    : m_db( *library_effects->dataBase() ),
      m_id( id ),
      m_profile_common( NULL ),
      m_profile_glsl( NULL ),
      m_profile_gles( NULL ),
      m_profile_gles2( NULL )
{
}




Effect::~Effect( )
{
    if( m_profile_common != NULL) {
        delete m_profile_common;
    }
    if( m_profile_glsl != NULL ) {
        delete m_profile_glsl;
    }
    if( m_profile_gles != NULL ) {
        delete m_profile_gles;
    }
    if( m_profile_gles2 != NULL ) {
        delete m_profile_gles2;
    }
    for(auto it=m_parameters.begin(); it!=m_parameters.end(); ++it ) {
        delete *it;
    }
}

void
Effect::generate( ProfileType profile )
{
    Logger log = getLogger( package + ".generate" );
    if( m_profile_common == NULL ) {
        SCENELOG_WARN( log, "No COMMON profile, unable to generate shaders" );
        return;
    }
    if( m_profile_common->techniques() == 0 ) {
        SCENELOG_ERROR( log, "Common profile has no techniques" );
        return;
    }
    const Technique* src_tec = m_profile_common->technique();

    Profile* dst_pro;
    if( profile == PROFILE_GLSL ) {
        if( m_profile_glsl == NULL ) {
            createProfile( PROFILE_GLSL );
            SCENELOG_DEBUG( log, "Creating GLSL profile" );
        }
        dst_pro = m_profile_glsl;
    }
    else if( profile == PROFILE_GLES2 ) {
        if( m_profile_gles2 == NULL ) {
            createProfile( PROFILE_GLES2 );
            SCENELOG_DEBUG( log, "Creating GLES2 profile" );
        }
        dst_pro = m_profile_gles2;
    }
    else {
        SCENELOG_ERROR( log, "Can only generate shaders for profiles GLSL and GLES2" );
        return;
    }
    const CommonShadingModel* sm = src_tec->commonShadingModel();
    if( sm == NULL ) {
        SCENELOG_ERROR( log, "Technique in common profile has no shading model" );
        return;
    }

    dst_pro->deleteTechnique( "auto_generated" );
    Technique* dst_tec = dst_pro->createTechnique( "auto_generated" );
    Pass* dst_pas = dst_tec->createPass( "default" );



    // copy all parameters from common profile to effect, if not existing
    for( size_t i=0; i<m_profile_common->parameters(); i++ ) {
        addParameter( *(m_profile_common->parameter(i)) );
    }

    bool need_normal = false;
    bool need_emission = false;
    bool need_ambient = false;
    bool need_diffuse = false;
    bool need_specular = false;
    bool need_ref_vec = false;
    bool need_half_vec = false;

    switch( sm->shadingModel() ) {
    case SHADING_BLINN:
        // color = emission + ambient*al + diffuse*max(N*L,0) + specular*max(H*N,0)^shininess
        need_normal = true;
        need_emission = true;
        need_ambient = true;
        need_diffuse = true;
        need_specular = true;
        need_ref_vec = false;
        need_half_vec = true;
        break;

    case SHADING_LAMBERT:
        // color = emission + ambient*al + diffuse*max(N*L,0)
        need_normal = true;
        need_emission = true;
        need_ambient = true;
        need_diffuse = true;
        need_specular = false;
        need_ref_vec = false;
        need_half_vec = false;
        break;

    case SHADING_PHONG:
        // color = emission + ambient*al + diffuse*max(N*L,0) + specular*max(R*I,0)^shininess
        need_normal = true;
        need_emission = true;
        need_ambient = true;
        need_diffuse = true;
        need_specular = true;
        need_ref_vec = true;
        need_half_vec = false;
        break;

    case SHADING_CONSTANT:
        // color = emission + ambient * sum_ambient color
        need_normal = false;
        need_emission = true;
        need_ambient = true;
        need_diffuse = false;
        need_specular = false;
        need_ref_vec = false;
        need_half_vec = false;
        break;
    }


    Parameter par;
    par.set( "auto_MVP", RUNTIME_MODELVIEW_PROJECTION_MATRIX, Value::createFloat4x4() );
    dst_pro->addParameter( par );
    if( need_normal ) {
    }

    // --- vertex shader
    std::string vs_head;
    std::string vs_body;

    vs_head += "uniform mat4 MVP;\n";
    vs_head += "attribute vec3 position;\n";
    vs_body += "gl_Position = MVP * vec4( position, 1.0 );\n";
    dst_pas->setUniform( "MVP", "auto_MVP" );
    dst_pas->setAttribute( VERTEX_POSITION, "position" );
    if( need_normal ) {
        vs_head += "uniform mat3 NM;\n";
        par.set( "auto_NM", RUNTIME_NORMAL_MATRIX, Value::createFloat3x3() );
        dst_pro->addParameter( par );
        dst_pas->setUniform( "NM", "auto_NM" );

        vs_head += "attribute vec3 normal;\n";

        vs_head += "varying vec3 es_normal;\n";
        vs_body += "es_normal = NM * normal;\n";
        dst_pas->setAttribute( VERTEX_NORMAL, "normal" );
    }
    dst_pas->setShaderSource( STAGE_VERTEX,
                              vs_head +
                              "void main() {" +
                              vs_body +
                              "}\n" );

    // --- fragment shader
    std::string fs_head;
    std::string fs_body;
    fs_head += "#ifdef GL_ES\n";
    fs_head += "precision highp float;\n";
    fs_head += "#endif\n";
    if( need_normal ) {
        fs_head += "varying vec3 es_normal;\n";
        fs_head += "uniform vec3 light_z;\n";
        dst_pas->setUniform( "light_z", "auto_light0_z" );
        par.set( "auto_light0_z", RUNTIME_LIGHT0_Z_WORLD, Value::createFloat3( 0.f, 0.f, 1.f ) );
        dst_pro->addParameter( par );

        fs_body += "vec3 n = normalize( es_normal );\n";

    }
    if( need_emission ) {
        fs_head += "uniform vec4 material_emission;\n";
        if( sm->isComponentParameterReference( SHADING_COMP_EMISSION ) ) {
            dst_pas->setUniform( "material_emission", sm->componentParameterReference( SHADING_COMP_EMISSION ) );
        }
        else if( sm->isComponentValue( SHADING_COMP_EMISSION ) ) {
            dst_pas->setUniform( "material_emission", *(sm->componentValue( SHADING_COMP_EMISSION )) );
        }
        else {
            dst_pas->setUniform( "material_emission", Value::createFloat4( 0.8f, 0.8f, 1.f, 1.0 ) );
        }
        fs_body += "color += 0.1*material_emission;\n";
    }
    if( need_ambient ) {
        fs_head += "uniform vec4 material_ambient;\n";
        if( sm->isComponentParameterReference( SHADING_COMP_AMBIENT ) ) {
            dst_pas->setUniform( "material_ambient", sm->componentParameterReference( SHADING_COMP_AMBIENT ) );
        }
        else if( sm->isComponentValue( SHADING_COMP_AMBIENT ) ) {
            dst_pas->setUniform( "material_ambient", *(sm->componentValue( SHADING_COMP_AMBIENT )) );
        }
        else {
            dst_pas->setUniform( "material_ambient", Value::createFloat4( 0.8f, 0.8f, 1.f, 1.0 ) );
        }
        fs_body += "color += 0.3*material_ambient;\n";
    }

    if( need_diffuse ) {
        fs_head += "uniform vec4 material_diffuse;\n";
        if( sm->isComponentParameterReference( SHADING_COMP_DIFFUSE ) ) {
            dst_pas->setUniform( "material_diffuse", sm->componentParameterReference( SHADING_COMP_DIFFUSE ) );
        }
        else if( sm->isComponentValue( SHADING_COMP_DIFFUSE ) ) {
            dst_pas->setUniform( "material_diffuse", *(sm->componentValue( SHADING_COMP_DIFFUSE )) );
        }
        else {
            dst_pas->setUniform( "material_diffuse", Value::createFloat4( 0.8f, 0.8f, 1.f, 1.0 ) );
        }
        fs_body += "color += material_diffuse*max(0.0, dot( n, light_z ) );\n";
    }
    if( need_specular ) {
        fs_head += "uniform float material_shininess;\n";
        if( sm->isComponentParameterReference( SHADING_COMP_SHININESS ) ) {
            dst_pas->setUniform( "material_shininess", sm->componentParameterReference( SHADING_COMP_SHININESS ) );
        }
        else if( sm->isComponentValue( SHADING_COMP_SHININESS ) ) {
            dst_pas->setUniform( "material_shininess", *(sm->componentValue( SHADING_COMP_SHININESS )) );
        }
        else {
            dst_pas->setUniform( "material_shininess", Value::createFloat( 10.f ) );
        }

        fs_head += "uniform vec4 material_specular;\n";
        if( sm->isComponentParameterReference( SHADING_COMP_SPECULAR ) ) {
            dst_pas->setUniform( "material_specular", sm->componentParameterReference( SHADING_COMP_SPECULAR ) );
        }
        else if( sm->isComponentValue( SHADING_COMP_SPECULAR ) ) {
            dst_pas->setUniform( "material_specular", *(sm->componentValue( SHADING_COMP_SPECULAR )) );
        }
        else {
            dst_pas->setUniform( "material_specular", Value::createFloat4( 0.8f, 0.8f, 1.f, 1.0 ) );
        }

        if( need_half_vec ) {

        }
        else {

        }
        fs_body += "vec3 v = vec3( 0.0, 0.0, 1.0 );\n";
        fs_body += "vec3 h = normalize( v + light_z );\n";
        fs_body += "float s = pow( max( dot(h,n), 0.0 ), material_shininess );\n";
        fs_body += "color += material_specular*s;\n";
    }

    dst_pas->setShaderSource( STAGE_FRAGMENT,
                              fs_head +
                              "void main() {\n" +
                              "vec4 color = vec4(0.0);\n" +
                              fs_body +
                              "gl_FragColor = color;\n" +
                              "}\n" );


    dst_pas->addState( STATE_DEPTH_TEST_ENABLE, Value::createBool( true ) );

}



const Parameter*
Effect::parameter( const size_t index ) const
{
    Logger log = getLogger( "Scene.Effect.parameter" );

    SCENELOG_TRACE( log, m_id << ": " << m_parameters[index]->value()->debugString() );


    return m_parameters[index];
}


void
Effect::addParameter( const Parameter& p )
{
    Logger log = getLogger( "Scene.Effect.addParameter" );
    if( !p.sid().empty() ) {
        for( auto it=m_parameters.begin(); it!=m_parameters.end(); ++it ) {
            if( (*it)->sid() == p.sid() ) {
                if( ((*it)->semantic() == p.semantic()) && (((*it)->value()->type() == p.value()->type() )) ) {
                    // do nothing
                    return;
                }
                else {
                    SCENELOG_WARN( log, "Parameter sid='"<<p.sid() <<"' with slightly different definition already exists in effect" );
                    return;
                }
            }
        }
    }
    m_parameters.push_back( new Parameter(p) );
    SCENELOG_DEBUG( log, "Adding parameter " <<
                    "sid='" << m_parameters.back()->sid() <<
                    "', semantic=" << std::hex << m_parameters.back()->semantic() << std::dec <<
                    ", value=" << m_parameters.back()->value()->debugString() );


}

const std::string&
Effect::key() const
{
    return m_id;
}

Profile*
Effect::createProfile( ProfileType type )
{
    Logger log = getLogger( "Scene.Effect.createProfile" );
    if( profile( type ) != NULL ) {
        SCENELOG_ERROR( log, "Profile "
                        << std::hex << type << std::dec <<
                        " is already defined." );
        return NULL;
    }
    if( type == PROFILE_COMMON ) {
        m_profile_common = new Profile( m_db, this, PROFILE_COMMON );
        return m_profile_common;
    }
    if( type == PROFILE_GLSL ) {
        m_profile_glsl = new Profile( m_db, this, PROFILE_GLSL );
        return m_profile_glsl;
    }
    else if( type == PROFILE_GLES ) {
        m_profile_gles = new Profile( m_db, this, PROFILE_GLES );
        return m_profile_gles;
    }
    else if( type == PROFILE_GLES2 ) {
        m_profile_gles2 = new Profile( m_db, this, PROFILE_GLES2 );
        return m_profile_gles2;
    }
    SCENELOG_ERROR( log, "Profile " << std::hex << type << std::dec << " not supported yet." );
    return NULL;
}

const Profile*
Effect::profile( ProfileType type ) const
{
    switch( type ) {
    case PROFILE_COMMON:
        return m_profile_common;
    case PROFILE_GLSL:
        return m_profile_glsl;
    case PROFILE_GLES:
        return m_profile_gles;
    case PROFILE_GLES2:
        return m_profile_gles2;
    default:
        return NULL;
    }
}


} // of namespace Scene
