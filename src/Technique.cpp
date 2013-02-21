#include <string>
#include <stdexcept>
#include "scene/Technique.hpp"
#include "scene/Profile.hpp"
#include "scene/Utils.hpp"
#include "scene/Log.hpp"

namespace Scene {
    using std::string;
    using std::runtime_error;
    using std::unordered_map;

const std::string package = "Scene.Technique";

Technique::Technique( DataBase&           db,
                      Effect*             effect,
                      Profile*            profile,
                      const std::string&  sid )
    : m_db(db),
      m_effect( effect ),
      m_profile( profile ),
      m_sid( sid ),
      m_common_shading_model( NULL )
{
}

Technique::~Technique()
{
    for( auto it=m_passes.begin(); it!=m_passes.end(); ++it) {
        delete *it;
    }
    m_passes.clear();
}

CommonShadingModel*
Technique::createCommonShadingModel(const ShadingModelType model)
{
    Logger log = getLogger( package + ".createCommonShadingModel" );
    if( m_common_shading_model != NULL ) {
        SCENELOG_ERROR( log, "Technique already has a shading model" );
        return NULL;
    }
    if( m_profile->type() != PROFILE_COMMON ) {
        SCENELOG_ERROR( log, "Only profile_COMMON supports a common shading model" );
        return NULL;
    }
    m_common_shading_model = new CommonShadingModel( m_db,
                                                     m_effect,
                                                     m_profile,
                                                     this );
    m_common_shading_model->setShadingModel( model );
    return m_common_shading_model;
}

Pass*
Technique::createPass( const std::string& sid )
{
    Logger log = getLogger( "Scene.Technique.createPass" );

    if( !sid.empty() ) {
        for(size_t i=0; i<m_passes.size(); i++) {
            if( m_passes[i]->sid() == sid ) {
                SCENELOG_ERROR( log, "Pass with sid='" << sid << "' already exists." );
                return NULL;
            }
        }
    }

    Pass* pass = new Pass( m_db,
                           m_effect,
                           m_profile,
                           this,
                           sid );
    m_passes.push_back( pass );
    return pass;
}


const std::string
Technique::key() const
{
    if( m_id.empty() ) {
        return m_profile->key() + "/" + m_sid;
    }
    else {
        return m_id;
    }
}

const Pass*
Technique::pass( const std::string& sid ) const
{
    Logger log = getLogger( "Scene.Technique.pass" );
    if( sid.empty() ) {
        SCENELOG_FATAL( log, "Cannot find a particular pass based on an empty sid." );
        return NULL;
    }
    for(auto it=m_passes.begin(); it!=m_passes.end(); ++it) {
        if( (*it)->sid() == sid ) {
            return *it;
        }
    }
    return NULL;
}




}
