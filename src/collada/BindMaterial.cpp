#include <string>
#include "scene/Utils.hpp"
#include "scene/Log.hpp"
#include "scene/Node.hpp"
#include "scene/Bind.hpp"
#include "scene/InstanceGeometry.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;


bool
Importer::parseBindMaterial( InstanceGeometry*  instgeo,
                            xmlNodePtr               bind_material_node )
{
    Logger log = getLogger( "Scene.XML.Importer.parseBindMaterial" );
    if(!assertNode( bind_material_node, "bind_material" ) ) {
        return false;
    }

    xmlNodePtr n = bind_material_node->children;
    skipNodes( bind_material_node, n, "param" );


    if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "technique_common" ) ) {
        xmlNodePtr technique_common_node = n;
        xmlNodePtr m = technique_common_node->children;

        while( m != NULL && xmlStrEqual( m->name, BAD_CAST "instance_material" ) ) {
            xmlNodePtr instance_material_node = m;

            string symbol = attribute( instance_material_node, "symbol" );
            string target = attribute( instance_material_node, "target" );
            if( target.size() > 0 && target[0] == '#' ) {
                target = target.substr( 1 );
            }
            SCENELOG_TRACE( log, "symbol='" << symbol << "', target='" << target << "'" );

            instgeo->addMaterialBinding( symbol, target );


            xmlNodePtr o = m->children;
            while( o!= NULL && xmlStrEqual( o->name, BAD_CAST "bind" ) ) {
                Bind bind;
                if( !parseBind( bind, o ) ) {
                    return false;
                }
                instgeo->addMaterialBindingBind( symbol, bind );

                o = o->next;
            }
            skipNodes( m, o, "bind_vertex_input" );
            ignoreExtraNodes( log, o );



            m = m->next;
        }

        nagAboutRemainingNodes( log, m );
        n = n->next;
    }
    else {
        SCENELOG_ERROR( log, "Required child <technique_common> missing." );
        return false;
    }

    skipNodes( bind_material_node, n, "technique" );
    ignoreExtraNodes( log, n );
    nagAboutRemainingNodes( log, n );

    return true;
}

xmlNodePtr
Exporter::createBindMaterial( Context& context, const InstanceGeometry* instance ) const
{
    if( instance == NULL ) {
        return NULL;
    }
    std::list<std::string> symbols = instance->materialBindingSymbols();
    if( symbols.empty() ) {
        return NULL;
    }
    xmlNodePtr bind_mat_node = newNode( NULL, "bind_material" );
    xmlNodePtr tc_node = newChild( bind_mat_node, NULL, "technique_common" );
    for( auto it=symbols.begin(); it!=symbols.end(); ++it ) {
        xmlNodePtr im_node = newChild( tc_node, NULL, "instance_material" );
        addProperty( im_node, "symbol", *it );
        addProperty( im_node, "target", "#" + instance->materialBindingTargetId( *it ) );
        const std::vector<Bind> binds = instance->materialBindingBind( *it );
        for( size_t i=0; i<binds.size(); i++ ) {
            xmlAddChild( im_node, createBind( context, binds[i] ) );
        }
    }
    return bind_mat_node;
}

    } // of namespace XML
} // of namespace Scene
