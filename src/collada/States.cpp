#include <sstream>
#include <boost/lexical_cast.hpp>
#include "scene/Effect.hpp"
#include "scene/Profile.hpp"
#include "scene/Technique.hpp"
#include "scene/Pass.hpp"
#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/Utils.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;

bool
Importer::parseStates( Pass* pass,
                       xmlNodePtr  states_node )
{
    Logger log = getLogger( "Scene.XML.Importer.parseStates" );
    if(!assertNode( states_node, "states" ) ) {
        return false;
    }

    xmlNodePtr n = states_node->children;
    for( ; n!=NULL; n=n->next) {

        const string value = attribute( n, "value" );

        StateType type = STATE_N;
        size_t expected = 0;

        if( xmlStrEqual( n->name, BAD_CAST "point_size" ) ) {
            type = STATE_POINT_SIZE;
            expected = 1;
        }
        else if( xmlStrEqual( n->name, BAD_CAST "blend_func" ) ) {
            type = STATE_BLEND_FUNC;
            expected = 2;
        }
        else if( xmlStrEqual( n->name, BAD_CAST "polygon_offset" ) ) {
            type = STATE_POLYGON_OFFSET;
            expected = 2;
        }
        else if( xmlStrEqual( n->name, BAD_CAST "polygon_offset_fill_enable" ) ) {
            type = STATE_POLYGON_OFFSET_FILL_ENABLE;
            expected = 1;
        }
        else if( xmlStrEqual( n->name, BAD_CAST "blend_enable" ) ) {
            type = STATE_BLEND_ENABLE;
            expected = 1;
        }
        else if( xmlStrEqual( n->name, BAD_CAST "depth_mask" ) ) {
            type = STATE_DEPTH_MASK;
            expected = 1;
        }
        else if( xmlStrEqual( n->name, BAD_CAST "depth_test_enable" ) ) {
            type = STATE_DEPTH_TEST_ENABLE;
            expected = 1;
        }
        else if( xmlStrEqual( n->name, BAD_CAST "cull_face_enable" ) ) {
            type = STATE_CULL_FACE_ENABLE;
            expected = 1;
        }
        else if( xmlStrEqual( n->name, BAD_CAST "cull_face" ) ) {
            type = STATE_CULL_FACE;
            expected = 1;
        }

        else if( xmlStrEqual( n->name, BAD_CAST "polygon_mode" ) ) {
            type = STATE_POLYGON_MODE;
            expected = 2;
        }

        if( type != STATE_N ) {

            // tokenize value
            vector<string> tokens;
            size_t a = 0;
            size_t l = value.length();
            while( a < l ) {
                // beginning of token
                for( ; a<l && isspace( value[a] ); a++ ) ;
                // end of token
                size_t b=a;
                for( ; b<l && !isspace( value[b] ); b++ ) ;

                if( a < b ) {
                    tokens.push_back( value.substr( a, b-a ) );
                }
                else {
                    break;
                }
                a = b;
            }

            if( tokens.size() != expected ) {
                SCENELOG_ERROR( log, "<" << reinterpret_cast<const char*>( n->name ) << "> expected " << expected <<
                                " tokens but got " << tokens.size() );
                return false;
            }
            else {
                switch( type ) {
                case STATE_POINT_SIZE:
                    pass->addState( STATE_POINT_SIZE,
                                    Value::createFloat( boost::lexical_cast<float>( tokens[0] ) ) );
                    break;
                case STATE_BLEND_FUNC:
                    pass->addState( STATE_BLEND_FUNC,
                                    Value::createEnum2( GL::blendFuncValue( tokens[0] ),
                                                        GL::blendFuncValue( tokens[1] ) ) );
                    break;
                case STATE_POLYGON_OFFSET:
                    pass->addState( STATE_POLYGON_OFFSET,
                                    Value::createFloat2( boost::lexical_cast<float>( tokens[0] ),
                                                         boost::lexical_cast<float>( tokens[1] ) ) );
                    break;
                case STATE_POLYGON_OFFSET_FILL_ENABLE:
                    pass->addState( STATE_POLYGON_OFFSET_FILL_ENABLE,
                                    Value::createBool( GL::booleanValue( tokens[0] ) ) );
                    break;
                case STATE_BLEND_ENABLE:
                    pass->addState( STATE_BLEND_ENABLE,
                                    Value::createBool( GL::booleanValue( tokens[0] ) ) );
                    break;
                case STATE_DEPTH_MASK:
                    pass->addState( STATE_DEPTH_MASK,
                                    Value::createBool( GL::booleanValue( tokens[0] ) ) );
                    break;
                case STATE_DEPTH_TEST_ENABLE:
                    pass->addState( STATE_DEPTH_TEST_ENABLE,
                                    Value::createBool( GL::booleanValue( tokens[0] ) ) );
                    break;
                case STATE_CULL_FACE_ENABLE:
                    pass->addState( STATE_CULL_FACE_ENABLE,
                                    Value::createBool( GL::booleanValue( tokens[0] ) ) );
                    break;
                case STATE_CULL_FACE:
                    pass->addState( STATE_CULL_FACE,
                                    Value::createEnum( GL::faceValue( tokens[0] ) ) );
                    break;
                case STATE_POLYGON_MODE:
                    pass->addState( STATE_POLYGON_MODE,
                                    Value::createEnum2( GL::faceValue( tokens[0] ),
                                                        GL::polygonModeValue( tokens[1] ) ) );
                    break;
                case STATE_N:
                    break;
                }
            }

        }
        else {
            break;
        }

    }
    nagAboutRemainingNodes( log, n );
    return true;
}

xmlNodePtr
Exporter::createStates( Context& context, const Pass* pass ) const
{
    if( (pass == NULL) || (pass->states() == 0) ) {
        return NULL;
    }
    xmlNodePtr states_node = newNode( NULL, "states" );
    for( size_t i=0; i<pass->states(); i++ ) {
        std::string key;
        std::stringstream value;
        std::string ref;
        std::stringstream index;


        switch( pass->stateType( i) ) {
        case STATE_POINT_SIZE:
            key = "point_size";
            if( pass->stateIsParameterReference(i) ) {
                ref = pass->stateParameterReference(i);
            }
            else if( pass->stateValue(i)->type() == VALUE_TYPE_FLOAT ) {
                value << pass->stateValue(i)->floatData()[0];
            }
            break;
        case STATE_POLYGON_OFFSET:
            key = "polygon_offset";
            if( pass->stateIsParameterReference(i) ) {
                ref = pass->stateParameterReference(i);
            }
            else if( pass->stateValue(i)->type() == VALUE_TYPE_FLOAT2 ) {
                value << pass->stateValue(i)->floatData()[0]
                      << " "
                      << pass->stateValue(i)->floatData()[1];
            }
            break;
        case STATE_POLYGON_OFFSET_FILL_ENABLE:
            key = "polygon_offset_fill_enable";
            if( pass->stateIsParameterReference(i) ) {
                ref = pass->stateParameterReference(i);
            }
            else if( pass->stateValue(i)->type() == VALUE_TYPE_BOOL ) {
                value << GL::booleanString( pass->stateValue(i)->boolData()[0] );
            }
            break;
        case STATE_BLEND_ENABLE:
            key = "blend_enable";
            if( pass->stateIsParameterReference(i) ) {
                ref = pass->stateParameterReference(i);
            }
            else if( pass->stateValue(i)->type() == VALUE_TYPE_BOOL ) {
                value << GL::booleanString( pass->stateValue(i)->boolData()[0] );
            }
            break;
        case STATE_BLEND_FUNC:
            key = "blend_func";
            if( pass->stateIsParameterReference(i) ) {
                ref = pass->stateParameterReference(i);
            }
            else if( pass->stateValue(i)->type() == VALUE_TYPE_ENUM2 ) {
                value << GL::blendFuncString( pass->stateValue(i)->enumData()[0] )
                      << " "
                      << GL::blendFuncString( pass->stateValue(i)->enumData()[1] );
            }
            break;
        case STATE_CULL_FACE_ENABLE:
            key = "cull_face_enable";
            if( pass->stateIsParameterReference(i) ) {
                ref = pass->stateParameterReference(i);
            }
            else if( pass->stateValue(i)->type() == VALUE_TYPE_BOOL ) {
                value << GL::booleanString( pass->stateValue(i)->boolData()[0] );
            }
            break;
        case STATE_CULL_FACE:
            key = "cull_face";
            if( pass->stateIsParameterReference(i) ) {
                ref = pass->stateParameterReference(i);
            }
            else if( pass->stateValue(i)->type() == VALUE_TYPE_ENUM ) {
                value << GL::faceString( pass->stateValue(i)->enumData()[0] );
            }
            break;
        case STATE_DEPTH_TEST_ENABLE:
            key = "depth_test_enable";
            if( pass->stateIsParameterReference(i) ) {
                ref = pass->stateParameterReference(i);
            }
            else if( pass->stateValue(i)->type() == VALUE_TYPE_BOOL ) {
                value << GL::booleanString( pass->stateValue(i)->boolData()[0] );
            }
            break;
        case STATE_DEPTH_MASK:
            key = "depth_mask";
            if( pass->stateIsParameterReference(i) ) {
                ref = pass->stateParameterReference(i);
            }
            else if( pass->stateValue(i)->type() == VALUE_TYPE_BOOL ) {
                value << GL::booleanString( pass->stateValue(i)->boolData()[0] );
            }
            break;
        case STATE_POLYGON_MODE:
            key = "polygon_mode";
            if( pass->stateIsParameterReference(i) ) {
                ref = pass->stateParameterReference(i);
            }
            else if( pass->stateValue(i)->type() == VALUE_TYPE_ENUM2 ) {
                value << GL::faceString( pass->stateValue(i)->enumData()[0] )
                      << " "
                      << GL::polygonModeString( pass->stateValue(i)->enumData()[1] );
            }
            break;
        case STATE_N:
            break;
        }
        if( !key.empty() ) {
            xmlNodePtr state_node = newChild( states_node, NULL, key );
            if( !ref.empty() ) {
                addProperty( state_node, "param", ref );
            }
            else {
                addProperty( state_node, "value", value.str() );
            }
            if( index.str().size() != 0 ) {
                addProperty( state_node, "index", index.str() );
            }
        }
    }


    return states_node;
}




    } // of namespace XML
} // of namespace Scene
