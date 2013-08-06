#include <list>
#include <scene/Log.hpp>
#include <scene/Effect.hpp>
#include <scene/Profile.hpp>
#include <scene/CommonShadingModel.hpp>
#include <scene/tools/ShaderGen.hpp>

namespace Scene {
    namespace Tools {

    static const std::string package = "Scene.Tools.ShaderGen";

    
bool
ShaderGenColor( Pass*         dst_pas,
                std::string&  fs_head,
                std::string&  fs_body,
                int&          texcoord_comps,
                const Profile* profile_common,
                const CommonShadingModel* sm,
                const ShadingModelComponentType comp, const std::string comp_str,
                const float R, const float G, const float B, const float A )
{
    Logger log = getLogger( package + ".Color" );

    if( comp == SHADING_COMP_DIFFUSE && sm->isComponentImageReference( comp ) ) {
        std::string ref = sm->componentImageReference( comp );
        const Parameter* param = profile_common->parameter( ref );
        if( param == NULL ) {
            SCENELOG_ERROR( log, "Unable to find a parameter reference '" << ref << "'." ); 
            return false;
        }
        else {
            const Value* value = param->value();
            if( value == NULL ) {
                SCENELOG_FATAL( log, "Got NULL pointer!" );
                return false;
            }
            else if( value->type() == VALUE_TYPE_SAMPLER2D ) {
                
                texcoord_comps = std::max( texcoord_comps, 2 );
                
                fs_head += "uniform sampler2D texture_" + comp_str + ";\n";
                fs_body += "vec4 material_"+comp_str+" = texture2D( texture_"+comp_str+", fs_texcoord.xy );\n";
                
                dst_pas->setUniform( "material_"+comp_str, *value );
            }
            else {
                fs_body += "vec4 material_"+comp_str+" = vec4( 1.f, 0.f, 0.f, 1.f );\n";
            }
        }
    }
    else {
        fs_head += "uniform vec4 material_"+comp_str+";\n";
        if( sm->isComponentParameterReference( comp ) ) {
            dst_pas->setUniform( "material_"+comp_str, sm->componentParameterReference( comp ) );
        }
        else if( sm->isComponentValue( comp ) ) {
            dst_pas->setUniform( "material_"+comp_str, *(sm->componentValue( comp )) );
        }
        else {
            dst_pas->setUniform( "material_"+comp_str, Value::createFloat4( R, G, B, A ) );
        }
    }
    return true;
}


bool
generateShaderFromCommon( Effect*             effect,
                          const int   profile_mask,
                          const GenerateMode  generate_mode )
{
    Logger log = getLogger( package + ".generate" );

    // sanity checks
    if( effect == NULL ) {
        SCENELOG_ERROR( log, "effect == NULL" );
        return false;
    }
    // fetch the profile_COMMON
    const Profile* profile_common = effect->profile( PROFILE_COMMON );
    if( profile_common == NULL ) {
        SCENELOG_WARN( log, "No common profile." );
        return false;
    }
    // and make sure that we have at least one technique (we use the first).
    if( profile_common->techniques() == 0 ) {
        SCENELOG_WARN( log, "Common profile has no techniques." );
        return false;
    }
    const Technique* common_technique = profile_common->technique(0);

    // and get the shading model data from 
    const CommonShadingModel* sm = common_technique->commonShadingModel();
    if( sm == NULL ) {
        SCENELOG_ERROR( log, "Technique in common profile has no shading model" );
        return false;
    }


    // copy all parameters from common profile to effect, if not existing
    for( size_t i=0; i<profile_common->parameters(); i++ ) {
        effect->addParameter( *(profile_common->parameter(i)) );
    }

    std::list<ProfileType> supported_profile_types = { PROFILE_GLES2, PROFILE_GLSL };
    //std::list<Profile*> target_profiles;
    for(auto it=supported_profile_types.begin(); it!=supported_profile_types.end(); ++it ) {
        if( (*it & profile_mask) != 0 ) {
            // Create requested profile
            // -----------------------------------------------------------------            
            Profile* dst_profile = effect->profile( *it );
            if( dst_profile == NULL ) {
                dst_profile = effect->createProfile( *it );
            }
            dst_profile->deleteTechnique( "auto_generated" );
            Technique* dst_tec = dst_profile->createTechnique( "auto_generated" );
            Pass* dst_pas = dst_tec->createPass( "default" );
            
            
            bool need_normal = false;
            int texcoord_comps = 0;
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
            dst_profile->addParameter( par );
            if( need_normal ) {
            }
        
        
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
                dst_profile->addParameter( par );
        
                fs_body += "vec3 n = normalize( es_normal );\n";
        
            }
            if( need_emission &&
                    ShaderGenColor( dst_pas, fs_head, fs_body, texcoord_comps,
                                    profile_common, sm, SHADING_COMP_EMISSION, "emission",
                                    0.1f, 0.1f, 0.1f, 1.0) )
            {
                fs_body += "color += 0.1*material_emission;\n";
            }

            if( need_ambient &&
                    ShaderGenColor( dst_pas, fs_head, fs_body, texcoord_comps,
                                    profile_common, sm, SHADING_COMP_AMBIENT, "ambient",
                                    0.3f, 0.3f, 0.3f, 1.0) )
            {
                fs_body += "color += 0.1*material_ambient;\n";
            }

            if( need_diffuse &&
                    ShaderGenColor( dst_pas, fs_head, fs_body, texcoord_comps,
                                    profile_common, sm, SHADING_COMP_DIFFUSE, "diffuse",
                                    0.8f, 0.8f, 1.f, 1.0 ) )
            {
                fs_body += "color += material_diffuse*max(0.0, dot( n, light_z ) );\n";
            }

            if( need_specular &&
                    ShaderGenColor( dst_pas, fs_head, fs_body, texcoord_comps,
                                    profile_common, sm, SHADING_COMP_SPECULAR, "specular",
                                    0.8f, 0.8f, 1.f, 1.0 ) ) 
            {

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
        
                if( need_half_vec ) {
        
                }
                else {
        
                }
                fs_body += "vec3 v = vec3( 0.0, 0.0, 1.0 );\n";
                fs_body += "vec3 h = normalize( v + light_z );\n";
                fs_body += "float s = pow( max( dot(h,n), 0.0 ), material_shininess );\n";
                fs_body += "color += material_specular*s;\n";
            }
        
            switch( texcoord_comps ) {
            case 1: fs_head += "varying float fs_texcoord;\n"; break;
            case 2: fs_head += "varying vec2 fs_texcoord;\n"; break;
            case 3: fs_head += "varying vec3 fs_texcoord;\n"; break;
            }
            
            dst_pas->setShaderSource( STAGE_FRAGMENT,
                                      fs_head +
                                      "void main() {\n" +
                                      "vec4 color = vec4(0.0);\n" +
                                      fs_body +
                                      "gl_FragColor = color;\n" +
                                      "}\n" );
        
        
        
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
                dst_profile->addParameter( par );
                dst_pas->setUniform( "NM", "auto_NM" );
        
                vs_head += "attribute vec3 normal;\n";
        
                vs_head += "varying vec3 es_normal;\n";
                vs_body += "es_normal = NM * normal;\n";
                dst_pas->setAttribute( VERTEX_NORMAL, "normal" );
            }
            if( (texcoord_comps > 0) && (texcoord_comps < 4) ) {
                std::string t;
                switch( texcoord_comps ) {
                case 1: t = "float"; break;
                case 2: t = "vec2"; break;
                case 3: t = "vec3"; break;
                }
                vs_head += "attribute " + t + " texcoord;\n";
                vs_head += "varying " + t + " fs_texcoord;\n";
                vs_body += "fs_texcoord = texcoord;\n";
                dst_pas->setAttribute( VERTEX_TEXCOORD, "texcoord" );
            }
            
            dst_pas->setShaderSource( STAGE_VERTEX,
                                      vs_head +
                                      "void main() {" +
                                      vs_body +
                                      "}\n" );
            
            
            dst_pas->addState( STATE_DEPTH_TEST_ENABLE, Value::createBool( true ) );
        
        }
    }
    
    return true;
}

void
generateShadersFromCommon( DataBase&           db,
                           const int   profile_mask,
                           const GenerateMode  generate_mode )
{
    for( size_t i=0; i<db.library<Effect>().size(); i++ ) {
        generateShaderFromCommon( db.library<Effect>().get( i ),
                                  profile_mask,
                                  generate_mode );
    }
}


    } // of namespace Tools
} // of namespace Scene
