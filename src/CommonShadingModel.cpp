#include <scene/Log.hpp>
#include <scene/DataBase.hpp>
#include <scene/CommonShadingModel.hpp>


namespace Scene {

static const std::string package = "Scene.CommonShadingModel";

CommonShadingModel::CommonShadingModel( DataBase& db,
                                        Effect*    effect,
                                        Profile*   profile,
                                        Technique* technique )
    : m_db( db ),
      m_effect( effect ),
      m_profile( profile ),
      m_technique( technique )
{
    for( unsigned int i=0; i<static_cast<unsigned int>( SHADING_COMP_COMP_N); i++ ) {
        m_components[i].m_value = new Value;
        m_components[i].m_state = ShadingModelComponent::NOT_SET;
    }
    touchStructureChanged();
    m_db.touchStructureChanged();
}

CommonShadingModel::~CommonShadingModel()
{
    for( unsigned int i=0; i<static_cast<unsigned int>( SHADING_COMP_COMP_N); i++ ) {
        delete m_components[i].m_value;
    }
}


const Technique*
CommonShadingModel::technique() const
{
    return m_technique;
}


const ShadingModelType
CommonShadingModel::shadingModel() const
{
    return m_model;
}

void
CommonShadingModel::setShadingModel( const ShadingModelType model )
{
    m_model = model;
    touchStructureChanged();
    m_db.touchStructureChanged();
}

void
CommonShadingModel::clearComponentValue( const ShadingModelComponentType comp )
{
    m_components[ comp ].m_state = ShadingModelComponent::NOT_SET;
    touchStructureChanged();
    m_db.touchStructureChanged();
}


void
CommonShadingModel::setComponentValue( const ShadingModelComponentType comp, const Value& value )
{
    Logger log = getLogger( package + ".setComponentValue" );
    switch( comp ) {
    case SHADING_COMP_EMISSION:
    case SHADING_COMP_DIFFUSE:
    case SHADING_COMP_AMBIENT:
    case SHADING_COMP_SPECULAR:
    case SHADING_COMP_REFLECTIVE:
    case SHADING_COMP_TRANSPARENT:
        if( value.type() != VALUE_TYPE_FLOAT4 ) {
            SCENELOG_ERROR( log, "component type " << comp << " requires float4." );
            return;
        }
        break;
    case SHADING_COMP_SHININESS:
    case SHADING_COMP_REFLECTIVITY:
    case SHADING_COMP_TRANSPARENCY:
    case SHADING_COMP_REFINDEX:
        if( value.type() != VALUE_TYPE_FLOAT ) {
            SCENELOG_ERROR( log, "component type " << comp << " requires float." );
            return;
        }
        break;

    case SHADING_COMP_COMP_N:
        SCENELOG_ERROR( log, "Illegal component type." );
        return;
    }

    bool structure_changed =
            (m_components[comp].m_state != ShadingModelComponent::VALUE) ||
            (m_components[comp].m_value->type() != value.type() );
    m_components[ comp ].m_state = ShadingModelComponent::VALUE;
    *(m_components[ comp ].m_value) = value;

    if( structure_changed )
    {
        touchStructureChanged();
        m_db.touchStructureChanged();
    }
    else {
        touchValueChanged();
        m_db.touchValueChanged();
    }
}


void
CommonShadingModel::setComponentParameterReference( const ShadingModelComponentType comp, const std::string& reference )
{
    m_components[ comp ].m_state = ShadingModelComponent::PARAM_REF;
    m_components[ comp ].m_reference = reference;
    touchStructureChanged();
    m_db.touchStructureChanged();
}

void
CommonShadingModel::setComponentImageReference( const ShadingModelComponentType comp, const std::string& reference )
{
    Logger log = getLogger( package + ".setComponentImageReference" );
    switch( comp ) {
    case SHADING_COMP_EMISSION:
    case SHADING_COMP_AMBIENT:
    case SHADING_COMP_SPECULAR:
    case SHADING_COMP_REFLECTIVE:
    case SHADING_COMP_TRANSPARENT:
        break;

    case SHADING_COMP_DIFFUSE:
    case SHADING_COMP_SHININESS:
    case SHADING_COMP_REFLECTIVITY:
    case SHADING_COMP_TRANSPARENCY:
    case SHADING_COMP_REFINDEX:
    case SHADING_COMP_COMP_N:
        SCENELOG_ERROR( log, "image references not allowed for type " << comp );
        return;
        break;
    }

    m_components[ comp ].m_state = ShadingModelComponent::IMAGE_REF;
    m_components[ comp ].m_reference = reference;
    touchStructureChanged();
    m_db.touchStructureChanged();
}


bool
CommonShadingModel::isComponentSet( const ShadingModelComponentType comp ) const
{
    return m_components[ comp ].m_state != ShadingModelComponent::NOT_SET;
}

bool
CommonShadingModel::isComponentValue( const ShadingModelComponentType comp ) const
{
    return m_components[ comp ].m_state == ShadingModelComponent::VALUE;
}

const Value*
CommonShadingModel::componentValue( const ShadingModelComponentType comp ) const
{
    if( m_components[ comp ].m_state == ShadingModelComponent::VALUE ) {
        return m_components[comp].m_value;
    }
    else {
        Logger log = getLogger( package + ".componentValue" );
        SCENELOG_ERROR( log, "No value defined for component " << comp );
        return NULL;
    }
}

bool
CommonShadingModel::isComponentParameterReference( const ShadingModelComponentType comp ) const
{
    return m_components[ comp ].m_state == ShadingModelComponent::PARAM_REF;
}

const std::string&
CommonShadingModel::componentParameterReference( const ShadingModelComponentType comp ) const
{
    if( m_components[ comp ].m_state == ShadingModelComponent::PARAM_REF ) {
        return m_components[comp].m_reference;
    }
    else {
        static const std::string empty = "";
        Logger log = getLogger( package + ".componentParameterReference" );
        SCENELOG_ERROR( log, "No parameter reference defined for component " << comp );
        return empty;
    }
}

bool
CommonShadingModel::isComponentImageReference( const ShadingModelComponentType comp ) const
{
    return m_components[ comp ].m_state == ShadingModelComponent::IMAGE_REF;
}

const std::string&
CommonShadingModel::componentImageReference( const ShadingModelComponentType comp ) const
{
    if( m_components[ comp ].m_state == ShadingModelComponent::IMAGE_REF ) {
        return m_components[comp].m_reference;
    }
    else {
        static const std::string empty = "";
        Logger log = getLogger( package + ".componentImageReference" );
        SCENELOG_ERROR( log, "No image reference defined for component " << comp );
        return empty;
    }
}


} // of namespace Scene
