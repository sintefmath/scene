#include <ctime>
#include <cstring>
#include "scene/Asset.hpp"
#include "scene/Log.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;

bool
Importer::parseAsset( Importer::Context& context,
                      Asset&      asset,
                      xmlNodePtr  asset_node )
{
        Logger log = getLogger( "Scene.XML.Builder.parseAsset" );

        xmlNodePtr n = asset_node->children;
        if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "contributor" ) ) {
            // ignore
            n=n->next;
        }
        if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "coverage" ) ) {
            // ignore
            n=n->next;
        }
        if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "created" ) ) {
            asset.setCreated( getBody(n) );
            n=n->next;
        }
        else {
            SCENELOG_WARN( log, "In <asset>, required child <created> missing." );
        }

        if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "keywords" ) ) {
            // ignore
            n=n->next;
        }
        if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "modified" ) ) {
            asset.setModified( getBody(n) );
            n=n->next;
        }
        else {
            SCENELOG_WARN( log, "In <asset>, required child <modified> missing." );
        }

        if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "revision" ) ) {
            // ignore
            n=n->next;
        }
        if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "subject" ) ) {
            asset.setSubject( getBody(n) );
            n=n->next;
        }
        if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "title" ) ) {
            // ignore
            n=n->next;
        }
        if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "unit" ) ) {
            float unit;
            if( attribute( unit, n, "meter" ) ) {
                context.m_unit = unit;
            }
            else {
                SCENELOG_WARN( log, "In <unit>, failure to parse 'meter' attribute" );
            }
            n=n->next;
        }
        if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "up_axis" ) ) {
            const std::string val = getBody( n );
            if( val == "X_UP" ) {
                context.m_up_axis = Context::X_UP;
            }
            else if( val == "Y_UP" ) {
                context.m_up_axis = Context::Y_UP;
            }
            else if( val == "Z_UP" ) {
                context.m_up_axis = Context::Z_UP;
            }
            else {
                SCENELOG_WARN( log, "In <up_axis>, unknown value '" << val << "'" );
            }
            // ignore
            n=n->next;
        }
        while( n != NULL ) {
            if( xmlStrEqual( n->name, BAD_CAST "extra" ) ) {
                // ignore
            }
            else {
                SCENELOG_WARN( log, "In <asset>, unexpected node <" <<
                               reinterpret_cast<const char*>( n->name ) << ">." );
                n = n->next;
            }
        }

        return true;
    }

bool
Importer::parseAsset( Asset&      asset,
                      xmlNodePtr  asset_node,
                      float*      scope_unit )
{
    Logger log = getLogger( "Scene.XML.Builder.parseAsset" );

    xmlNodePtr n = asset_node->children;
    if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "contributor" ) ) {
        // ignore
        n=n->next;
    }
    if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "coverage" ) ) {
        // ignore
        n=n->next;
    }
    if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "created" ) ) {
        asset.setCreated( getBody(n) );
        n=n->next;
    }
    else {
        SCENELOG_WARN( log, "In <asset>, required child <created> missing." );
    }

    if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "keywords" ) ) {
        // ignore
        n=n->next;
    }
    if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "modified" ) ) {
        asset.setModified( getBody(n) );
        n=n->next;
    }
    else {
        SCENELOG_WARN( log, "In <asset>, required child <modified> missing." );
    }

    if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "revision" ) ) {
        // ignore
        n=n->next;
    }
    if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "subject" ) ) {
        asset.setSubject( getBody(n) );
        n=n->next;
    }
    if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "title" ) ) {
        // ignore
        n=n->next;
    }
    if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "unit" ) ) {
        if( scope_unit != NULL && !attribute( *scope_unit, n, "meter" ) ) {
            SCENELOG_INFO( log, "<unit> without meter attribute" );
        }
        n=n->next;
    }
    if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "up_axis" ) ) {
        // ignore
        n=n->next;
    }
    while( n != NULL ) {
        if( xmlStrEqual( n->name, BAD_CAST "extra" ) ) {
            // ignore
        }
        else {
            SCENELOG_WARN( log, "In <asset>, unexpected node <" <<
                           reinterpret_cast<const char*>( n->name ) << ">." );
            n = n->next;
        }
    }

    return true;
}


xmlNodePtr
Exporter::createAsset( Context& context, const Asset& parent, const Asset& asset ) const
{
    return NULL;
}


xmlNodePtr
Exporter::createAsset( const Asset& asset ) const
{
    xmlNodePtr asset_node = xmlNewNode( NULL, BAD_CAST "asset" );
    if( !asset.created().empty() ) {
        newChild( asset_node, NULL, "created", asset.created() );
    }
    if( !asset.modified().empty() ) {
        newChild( asset_node, NULL, "modified", asset.modified() );
    }
    if( !asset.subject().empty() ) {
        newChild( asset_node, NULL, "subject", asset.subject() );
    }

    if( asset_node->children == NULL ) {
        xmlFreeNode( asset_node );
        return NULL;
    }
    else {
        return asset_node;
    }
}



    } // of namespace Scene
} // of namespace Scene
