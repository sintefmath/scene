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

#include <fstream>
#include <cstring>
#include <boost/lexical_cast.hpp>
#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;
        using std::ifstream;
        using std::istreambuf_iterator;


Importer::Importer( Scene::DataBase& database, const std::string base_path )
: m_database( database ),
  m_base_path( base_path )
{
}

const std::string
Importer::attribute( xmlNodePtr node, const std::string& attribute )
{
    std::string ret;
    xmlChar* att = xmlGetProp( node, BAD_CAST attribute.c_str() );
    if( att != NULL ) {
        ret = reinterpret_cast<const char*>( att );
        xmlFree( att );
    }
    return ret;
}

bool
Importer::attribute( unsigned int& result, xmlNodePtr node, const std::string& attribute )
{
    xmlChar* att = xmlGetProp( node, BAD_CAST attribute.c_str() );
    if( att == NULL ) {
        return false;
    }
    bool retval = true;
    try {
        result = boost::lexical_cast<unsigned int>( reinterpret_cast<const char*>( att ) );
    }
    catch( const boost::bad_lexical_cast& e ) {
        retval = false;
    }
    xmlFree( att );
    return retval;
}

xmlNodePtr
Importer::searchBySid( xmlNodePtr start, const std::string& sid )
{
    xmlNodePtr p = start->parent;
    while( p != NULL ) {
        for( xmlNodePtr c=p->children; c!=NULL; c=c->next ) {
            xmlChar* a = xmlGetProp( c, reinterpret_cast<const xmlChar*>( "sid" ) );
            if( a != NULL ) {
                if( xmlStrEqual( a, reinterpret_cast<const xmlChar*>(sid.c_str() ) ) ) {
                    xmlFree( a );
                    return c;
                }
                else {
                    xmlFree( a );
                }
            }
        }
        p = p->parent;
    }
    return NULL;
}


bool
Importer::attribute( float& result, xmlNodePtr node, const std::string& attribute )
{
    xmlChar* att = xmlGetProp( node, BAD_CAST attribute.c_str() );
    if( att == NULL ) {
        return false;
    }
    bool retval = true;
    try {
        result = boost::lexical_cast<float>( reinterpret_cast<const char*>( att ) );
    }
    catch( const boost::bad_lexical_cast& e ) {
        retval = false;
    }
    xmlFree( att );
    return retval;
}


const std::string
Importer::cleanRef( const std::string& ref )
{
    if( ref.size() > 0 && ref[0] == '#' ) {
        return ref.substr( 1 );
    }
    else {
        return ref;
    }
}

int
Importer::strEq( const xmlChar* str1, const std::string& str2)
{
    return xmlStrEqual( str1, reinterpret_cast<const xmlChar*>( str2.c_str() ) );
}


const std::string
Importer::resolvePath( const std::string url )
{
    Logger log = getLogger( "Scene.XML.Importer.resolvePath" );
    std::string protocol;
    std::string resource;
    size_t ix = url.find( "://" );
    if( ix == std::string::npos ) {
        protocol = "file";
        resource = url;
    }
    else {
        protocol = url.substr( 0, ix );
        resource = url.substr( ix+3 );
    }

    if( protocol == "file" ) {
        std::string ret;
        if(!m_base_path.empty()) {
#ifdef _WIN32
            ret = m_base_path + "\\" + resource;
#else
            ret = m_base_path + "//" + resource;
#endif
        }
        else {
            ret = resource;
        }
        SCENELOG_DEBUG( log, "Resolved path to '" << ret << "'" );
        return ret;
    }
    else {
        SCENELOG_FATAL( log, "Unhandled scheme '" << protocol << "' for url " << url );
        return "";
    }
}



bool
Importer::retrieveTextFile( std::string& result, const std::string& url )
{
    Logger log = getLogger( "Scene.XML.Importer.retrieveTextFile" );
    std::string path = resolvePath( url );
    if( path.empty() ) {
        return false;
    }
    SCENELOG_INFO( log, "reading '" << path << "'." );
    ifstream file( path.c_str());
    if(!file) {
        SCENELOG_ERROR( log, "Unable to open file '" << path << "'" );
        return false;
    }
    result = string( std::istreambuf_iterator<char>(file),
                     std::istreambuf_iterator<char>() );
    return true;
}


bool
Importer::retrieveBinaryFile( std::vector<char>& result, const std::string& url )
{
    Logger log = getLogger( "Scene.XML.Importer.retrieveBinaryFile" );
    std::string path = resolvePath( url );
    if( path.empty() ) {
        return false;
    }
    SCENELOG_INFO( log, "reading '" << path << "'." );
    ifstream file( path.c_str() ,std::ios::binary );
    if(!file) {
        SCENELOG_ERROR( log, "Unable to open file '" << path << "'" );
        return false;
    }
    result = vector<char>( std::istreambuf_iterator<char>(file),
                           std::istreambuf_iterator<char>() );
    return true;
}



const string
Importer::getBody( xmlNodePtr node )
{
    xmlChar* p = xmlNodeGetContent( node );
    if( p == NULL ) {
        return "";
    }

    const string retval( reinterpret_cast<const char*>( p ) );
    xmlFree( p );
    return retval;
}


bool
Importer::parseBodyAsFloats( std::vector<float>& result, xmlNodePtr node, size_t expected )
{
    Logger log = getLogger( "Scene.XML.Builder.parseBodyAsFloats" );
    if( expected == 0 ) {
        return true;
    }

    xmlChar* p = xmlNodeGetContent( node );
    if( p == NULL ) {
        SCENELOG_ERROR( log, "No node contents." );
        return false;
    }

    result.resize( expected );
    const char* a = reinterpret_cast<const char*>( p );
    for(size_t i=0; i<expected; i++) {
        char *b = NULL;
#ifdef _WIN32
        //MSVC 10 does not support strtof yet (c++0x feature)
        result[i] = static_cast<float>(strtod( a, &b ));
#else
        result[i] = strtof( a, &b );
#endif
        if( a == b ) {
            SCENELOG_ERROR( log, "Premature end of content." );
            xmlFree( p );
            return false;
        }
        a = b;
    }

    xmlFree( p );
    return true;
}

bool
Importer::parseBodyAsInts( std::vector<int>& result, xmlNodePtr node, size_t expected, size_t offset, size_t stride )
{
    Logger log = getLogger( "Scene.XML.Builder.parseBodyAsInts" );
    if( expected == 0 ) {
        return true;
    }

    xmlChar* p = xmlNodeGetContent( node );
    if( p == NULL ) {
        SCENELOG_ERROR( log, "No node contents." );
        return false;
    }

    result.resize( expected );

    const char* a = reinterpret_cast<const char*>( p );

    // dummy variable to avoid getting warn_unused_result warnings
    int ignored_value;

    // eat offset
    bool success = true;
    for(size_t i=0; i<offset && success; i++) {
        char* b = NULL;
        ignored_value = strtol( a, &b, 0 );
        if( a == b ) {
            success = false;
        }
        a = b;
    }

    for(size_t i=0; i<expected && success; i++) {
        char* b = NULL;
        result[i] = strtol( a, &b, 0 );
        if( a == b ) {
            success = false;
        }
        a = b;

        // eat rest of stride
        for(size_t k=1; k<stride && success; k++) {
            char* b = NULL;
            ignored_value = strtol( a, &b, 0 );
            if( a == b ) {
                success = false;
            }
            a = b;
        }
    }
    if(!success) {
        SCENELOG_ERROR( log, "Premature end of node contents" );
    }
    xmlFree( p );
    return success;
}

bool
Importer::parseBool( const std::string value, bool default_value )
{
    if( value.empty() ) {
        return default_value;
    }
    else if (value == "TRUE" ) {
        return true;
    }
    else if( value == "FALSE" ) {
        return false;
    }
    else {
        Logger log = getLogger( "Scene.XML.Importer.parseBool" );
        SCENELOG_ERROR( log, "Failed to parse bool '" << value << "'." );
        return default_value;
    }
}

bool
Importer::checkNode( xmlNodePtr node, const std::string& name )
{
    if( node == NULL ) {
        return false;
    }
    return xmlStrEqual( node->name, BAD_CAST name.c_str() ) != 0;
}


bool
Importer::assertNode( xmlNodePtr node, const std::string& name )
{
    if( xmlStrEqual( node->name, BAD_CAST name.c_str() ) ) {
        return true;
    }
    else {
        Logger log = getLogger( "Scene.XML.Builder.assertNode" );
        SCENELOG_FATAL( log, "Expected '" << name << "', got '" <<
                       reinterpret_cast<const char*>( node->name) << "'." );
        return false;
    }
}

bool
Importer::assertChild( xmlNodePtr parent, xmlNodePtr child, const std::string& name )
{
    Logger log = getLogger( "Scene.XML.Importer.assertChild" );
    if( child == NULL ) {
        SCENELOG_ERROR( log, "Premature end of <"<<
                        reinterpret_cast<const char*>( parent->name ) <<
                        ">, got NULL, expected <" << name << '>' );
        return false;
    }
    if( !xmlStrEqual( child->name, BAD_CAST name.c_str() ) ) {
        SCENELOG_ERROR( log, "Unexpected node in <"<<
                        reinterpret_cast<const char*>( parent->name ) <<
                        ">, got <" <<
                        reinterpret_cast<const char*>( child->name ) <<
                        ">, expected <" << name << '>' );
        return false;
    }
    return true;
}


void
Importer::skipNode( xmlNodePtr parent, xmlNodePtr& n, const std::string& name )
{
    Logger log = getLogger( "Scene.XML.Importer.skipNodes" );

    if( n!=NULL && xmlStrEqual( n->name, BAD_CAST name.c_str() ) ) {
        SCENELOG_DEBUG( log, "In <" <<
                       reinterpret_cast<const char*>( parent->name ) <<
                       ">, skipping <" <<
                       reinterpret_cast<const char*>( n->name ) <<
                       ">" );
        n = n->next;
    }


}

void
Importer::skipNodes( xmlNodePtr parent, xmlNodePtr& n, const std::string& name )
{
    Logger log = getLogger( "Scene.XML.Importer.skipNodes" );

    while( n!=NULL && xmlStrEqual( n->name, BAD_CAST name.c_str() ) ) {
        SCENELOG_DEBUG( log, "In <" <<
                       reinterpret_cast<const char*>( parent->name ) <<
                       ">, skipping <" <<
                       reinterpret_cast<const char*>( n->name ) <<
                       ">" );
        n = n->next;
    }


}


void
Importer::ignoreExtraNodes( Logger log, xmlNodePtr& n )
{
    while( n!= NULL && xmlStrEqual( n->name, BAD_CAST "extra" ) ) {
        n = n->next;
    }
}


void
Importer::nagAboutParseError( Logger log, xmlNodePtr n, const std::string& why )
{
    xmlNodePtr p = n->parent;
    if( p == NULL ) {
        SCENELOG_ERROR( log, "Failed to parse <" << n->name << ">: " << why );
    }
    else {
        const std::string id = attribute( p, "id" );
        SCENELOG_ERROR( log, "In <" << p->name << " id=\""<<id<<"\">, failed to parse <" <<n->name << ">: " << why );
    }
}


void
Importer::nagAboutRemainingNodes( Logger log, xmlNodePtr& n )
{
    if( n == NULL ) {
        return;
    }
    do {
        const xmlNodePtr p = n->parent;
        if( p == NULL ) {
            SCENELOG_WARN( log, "Encountered unexpected node <" << n->name << ">" );
        }
        else {
            SCENELOG_WARN( log, "In <" << p->name << ">, encountered unexpected node <" << n->name << ">" );
        }
        n = n->next;
    }
    while( n != NULL );
}

void
Importer::clean( xmlNodePtr xml_node )
{
    Logger log = getLogger( "Scene.XML.Importer.clean" );
    if( xml_node == NULL ) {
        SCENELOG_ERROR( log, "xml_node==NULL" );
        return;
    }

    xmlNodePtr n = xml_node->children;
    while( n != NULL ) {
        xmlNodePtr m = n->next;
        if( xmlStrEqual( n->name, BAD_CAST "comment" ) ) {
            const std::string comment = getBody( n );
            std::string cleaned;

            bool space = true;
            for( auto it=comment.begin(); it!=comment.end(); ++it ) {
                if( isspace(*it) ) {
                    if( !space ) {
                        cleaned.push_back( ' ' );
                        space = true;
                    }
                }
                else {
                    cleaned.push_back( *it );
                    space = false;
                }
            }

            SCENELOG_INFO( log, "Comment in <"
                            << reinterpret_cast<const char*>( xml_node->name ) <<
                            ">: " << cleaned );
            xmlUnlinkNode( n );
            xmlFreeNode( n );
        }
        else {
            clean( n );
        }
        n=m;
    }

}

bool
Importer::parseMemory( const char* buffer )
{
    Logger log = getLogger( "Scene.XML.Importer.parseBuffer" );

    xmlDocPtr doc = xmlReadMemory( buffer, strlen( buffer), NULL, NULL, XML_PARSE_NOBLANKS | XML_PARSE_HUGE );
    if( doc == NULL ) {
        SCENELOG_ERROR( log, "libxml failed to parse memory location " << buffer );
        return false;
    }
    else {
        xmlNodePtr root = xmlDocGetRootElement( doc );
        clean( root );
        bool success = parseCollada( root );
        xmlFreeDoc( doc );
        return success;
    }
}


bool
Importer::parse( const std::string &url )
{
    Logger log = getLogger( "Scene.XML.Builder.parse" );

    std::string path = resolvePath( url );
    SCENELOG_INFO( log, "Processing '" << path << "'." );
    xmlDocPtr doc = xmlReadFile( path.c_str(), NULL, XML_PARSE_NOBLANKS | XML_PARSE_HUGE  );
    if( doc == NULL ) {
        SCENELOG_ERROR( log, "libxml failed to parse '" << url << "'." );
        return false;
    }
    else {
#ifdef _WIN32
        const int sep = '\\';
#else
        const int sep = '/';
#endif
        std::string outer = m_base_path;
        size_t ix = path.find_last_of( sep );
        if( ix != std::string::npos ) {
            m_base_path = path.substr( 0, ix );
        }
        xmlNodePtr root = xmlDocGetRootElement( doc );
        clean( root );
        bool success = parseCollada( root );
        xmlFreeDoc( doc );
        m_base_path = outer;
        return success;
    }
}


    } // of namespace Scene
} // of namespace Scene
