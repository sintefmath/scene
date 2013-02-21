#include <png.h>

#include <fstream>
#include <cstring>
#include "boost/random/mersenne_twister.hpp"
#include "boost/random/uniform_int.hpp"
#include "boost/random/uniform_real.hpp"
#include "boost/random/variate_generator.hpp"
#include "scene/Log.hpp"
#include "scene/Image.hpp"
#include "scene/DataBase.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;


/** Helper class to let libPNG read directly from a char array. */

class LibPNGUserReadWrapper
{
public:
    LibPNGUserReadWrapper( const vector<char>& content, png_structp png_ptr )
        : m_content( content ),
          m_offset( 0 )
    {
        png_set_read_fn( png_ptr, this, user_read_data );
    }

protected:
    static void
    user_read_data( png_structp png_ptr, png_bytep data, png_size_t length )
    {
        LibPNGUserReadWrapper* me = reinterpret_cast<LibPNGUserReadWrapper*>( png_get_io_ptr( png_ptr ) );

        if( me->m_offset + length >= me->m_content.size() ) {
            Logger log = getLogger( "Scene.XML.Import.LibPNGUserReadWrapper" );
            SCENELOG_WARN( log, "Reading outside file contents" <<
                            ", offset=" << me->m_offset <<
                            ", length=" << length <<
                            ", content_length=" << me->m_content.size() );
            length = me->m_content.size()-me->m_offset;
        }

        memcpy( data, &me->m_content[ me->m_offset ], length );
        me->m_offset += length;
    }
    const vector<char>& m_content;
    size_t              m_offset;
};




static
bool
readPNG( GLenum& iformat,
         GLenum& format,
         GLenum& type,
         size_t&  width,
         size_t&  height,
         vector<unsigned char>& data,
         const std::vector<char>& content )
{
    Logger log = getLogger( "Scene.XML.Import.readPNG" );

    png_structp png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
    if( png_ptr == NULL ) {
        SCENELOG_ERROR( log, "Failed to create png_read_struct" );
        return false;
    }
    LibPNGUserReadWrapper( content, png_ptr );

    png_infop info_ptr = png_create_info_struct( png_ptr );
    if( info_ptr == NULL ) {
        SCENELOG_ERROR( log, "Failed to create png_info_struct" );
        png_destroy_read_struct( &png_ptr, &info_ptr, NULL );
        return false;
    }


    if( setjmp( png_jmpbuf(png_ptr) ) ) {
        SCENELOG_ERROR( log, "Failed to setjmp." );
        png_destroy_read_struct( &png_ptr, &info_ptr, NULL );
        return false;
    }

    png_read_png( png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, NULL );

    int color_type = png_get_color_type( png_ptr, info_ptr );
    int bit_depth = png_get_bit_depth( png_ptr, info_ptr );

    if( color_type == PNG_COLOR_TYPE_RGB && bit_depth == 8 ) {
        width = png_get_image_width( png_ptr, info_ptr );
        height = png_get_image_height( png_ptr, info_ptr );
        iformat = GL_RGB;
        format = GL_RGB;
        type = GL_UNSIGNED_BYTE;

        png_bytepp rows = png_get_rows( png_ptr, info_ptr );

        data.resize( width*height*3 );
        for(size_t j=0; j<height; j++) {
            for(size_t i=0; i<width; i++) {
                data[ 3*(j*width+i) + 0 ] = rows[j][ 3*i + 0 ];
                data[ 3*(j*width+i) + 1 ] = rows[j][ 3*i + 1 ];
                data[ 3*(j*width+i) + 2 ] = rows[j][ 3*i + 2 ];
            }
        }
    }
    else if( color_type == PNG_COLOR_TYPE_RGB_ALPHA && bit_depth == 8 ) {
        width = png_get_image_width( png_ptr, info_ptr );
        height = png_get_image_height( png_ptr, info_ptr );
        iformat = GL_RGBA;
        format = GL_RGBA;
        type = GL_UNSIGNED_BYTE;

        png_bytepp rows = png_get_rows( png_ptr, info_ptr );

        data.resize( width*height*4 );
        for(size_t j=0; j<height; j++) {
            for(size_t i=0; i<width; i++) {
                data[ 4*(j*width+i) + 0 ] = rows[j][ 4*i + 0 ];
                data[ 4*(j*width+i) + 1 ] = rows[j][ 4*i + 1 ];
                data[ 4*(j*width+i) + 2 ] = rows[j][ 4*i + 2 ];
                data[ 4*(j*width+i) + 3 ] = rows[j][ 4*i + 3 ];
            }
        }
    }

    else {
        SCENELOG_ERROR( log, "Unsupported color type, color_type=" << color_type << ", bit_depth=" << bit_depth );
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return false;
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return true;
}

enum ChannelHint
{
    CHANNEL_HINT_RGB,
    CHANNEL_HINT_RGBA,
    CHANNEL_HINT_RGBE,
    CHANNEL_HINT_L,
    CHANNEL_HINT_LA,
    CHANNEL_HINT_D,
    CHANNEL_HINT_END
};

enum RangeHint
{
    RANGE_HINT_SNORM,
    RANGE_HINT_UNORM,
    RANGE_HINT_SINT,
    RANGE_HINT_UINT,
    RANGE_HINT_FLOAT
};

enum PrecisionHint
{
    PRECISION_HINT_DEFAULT,
    PRECISION_HINT_LOW,
    PRECISION_HINT_MID,
    PRECISION_HINT_HIGH,
    PRECISION_HINT_MAX
};

enum SpaceHint
{
    SPACE_HINT_NONE
};

struct
{
    ChannelHint    channel;
    RangeHint      range;
    PrecisionHint  precision;
    SpaceHint      space;
    GLenum         iformat;
    GLenum         format;
    GLenum         type;
    string         debug;
}
resolve_hints[] =
{
{ CHANNEL_HINT_RGB, RANGE_HINT_UNORM, PRECISION_HINT_DEFAULT, SPACE_HINT_NONE, GL_RGB,       GL_RGB,       GL_UNSIGNED_BYTE, "R8G8B8" },
{ CHANNEL_HINT_L,   RANGE_HINT_UNORM, PRECISION_HINT_DEFAULT, SPACE_HINT_NONE, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, "L8" },
{ CHANNEL_HINT_END }
};


bool
Importer::parseFormat( GLenum&  iformat,
                       GLenum&  format,
                       GLenum&  type,
                       xmlNodePtr format_node )
{
    Logger log = getLogger( "Scene.XML.Importer.parseFormat" );

    xmlNodePtr n = format_node->children;

    if( n == NULL ) {
        SCENELOG_ERROR( log, "Empty <format> node" );
        return false;
    }
    else if( xmlStrEqual( n->name, BAD_CAST "hint" ) ) {
        ChannelHint channel_hint;
        const string channels_str = attribute( n, "channels" );
        if( channels_str == "RGB" ) {
            channel_hint = CHANNEL_HINT_RGB;
        }
        else if( channels_str == "RGBA" ) {
            channel_hint = CHANNEL_HINT_RGBA;
        }
        else if( channels_str == "RGBE" ) {
            channel_hint = CHANNEL_HINT_RGBE;
        }
        else if( channels_str == "L" ) {
            channel_hint = CHANNEL_HINT_L;
        }
        else if( channels_str == "LA" ) {
            channel_hint = CHANNEL_HINT_LA;
        }
        else if( channels_str == "D" ) {
            channel_hint = CHANNEL_HINT_D;
        }
        else {
            SCENELOG_ERROR( log, "Unrecognzied channels hint '" << channels_str << '\'' );
            return false;
        }

        RangeHint range_hint;
        const string range_str = attribute( n, "range" );
        if( range_str == "SNORM" ) {
            range_hint = RANGE_HINT_SNORM;
        }
        else if( range_str == "UNORM" ) {
            range_hint = RANGE_HINT_UNORM;
        }
        else if( range_str == "SINT" ) {
            range_hint = RANGE_HINT_SINT;
        }
        else if( range_str == "UINT" ) {
            range_hint = RANGE_HINT_UINT;
        }
        else if( range_str == "FLOAT" ) {
            range_hint = RANGE_HINT_FLOAT;
        }
        else {
            SCENELOG_ERROR( log, "Unrecognized range hint '" << range_str << '\'');
            return false;
        }

        PrecisionHint precision_hint;
        const string precision_str = attribute( n, "precision" );
        if( precision_str.empty() ) {
            precision_hint = PRECISION_HINT_DEFAULT;
        }
        else if( precision_str == "DEFAULT") {
            precision_hint = PRECISION_HINT_DEFAULT;
        }
        else if( precision_str == "LOW") {
            precision_hint = PRECISION_HINT_LOW;
        }
        else if( precision_str == "MID") {
            precision_hint = PRECISION_HINT_MID;
        }
        else if( precision_str == "HIGH") {
            precision_hint = PRECISION_HINT_HIGH;
        }
        else if( precision_str == "MAX") {
            precision_hint = PRECISION_HINT_MAX;
        }
        else {
            SCENELOG_ERROR( log, "Unrecognized precison hint '" << precision_str << '\'' );
            return false;
        }

        SpaceHint space_hint;
        const string space_str = attribute( n, "space" );
        if( space_str.empty() ) {
            space_hint = SPACE_HINT_NONE;
        }
        else {
            SCENELOG_ERROR( log, "Unrecognized space hint '" << space_str << '\'' );
            return false;
        }


        iformat = GL_NONE;
        for(size_t i=0; resolve_hints[i].channel != CHANNEL_HINT_END; i++ ) {
            if( (resolve_hints[i].channel == channel_hint ) &&
                (resolve_hints[i].range == range_hint ) &&
                (resolve_hints[i].precision == precision_hint ) &&
                (resolve_hints[i].space == space_hint ) )
            {
                iformat = resolve_hints[i].iformat;
                format = resolve_hints[i].format;
                type = resolve_hints[i].type;


                SCENELOG_DEBUG( log, "choosing format " << resolve_hints[i].debug << std::hex <<
                                ", iformat=0x" << iformat <<
                                ", format=0x" << format <<
                                ", type=0x" << type << std::dec );
                break;
            }
        }
        if( iformat == GL_NONE ) {
            SCENELOG_ERROR( log, "unable to choose an appropriate format." );
            return false;
        }
    }
    else {
        SCENELOG_ERROR( log, "Required child <hint> missing." );
        return false;
    }

    // todo: parse exact


    return true;
}

bool
Importer::parseSize( size_t* width, size_t* height, size_t* depth, xmlNodePtr n )
{
    Logger log = getLogger( "Scene.XML.Importer.parseImage.parseSize" );

    if( width != NULL ) {
        int t = atoi( attribute( n, "width" ).c_str() );
        if( t < 1 ) {
            SCENELOG_ERROR( log, "image width must be at least one." );
            return false;
        }
        *width = t;
    }

    if( height != NULL ) {
        int t = atoi( attribute( n, "height" ).c_str() );
        if( t < 1 ) {
            SCENELOG_ERROR( log, "image height must be at least one." );
            return false;
        }
        *height = t;
    }

    if( depth != NULL ) {
        int t = atoi( attribute( n, "depth" ).c_str() );
        if( t < 1 ) {
            SCENELOG_ERROR( log, "image depth must be at least one." );
            return false;
        }
        *depth = t;
    }
    return true;
}

bool
Importer::parseArray( size_t& array_length, xmlNodePtr array_node )
{
    Logger log = getLogger( "Scene.XML.Importer.parseImage.parseArray" );

    const string length_str = attribute( array_node, "length" );
    if( length_str.empty() ) {
         SCENELOG_ERROR( log, "Required <array> attribute 'length' empty." );
         return false;
    }
    int length = atoi( length_str.c_str() );
    if( length < 1 ) {
        SCENELOG_ERROR( log, "Array length must at least be one." );
        return false;
    }
    array_length = length;
    return true;
}


bool
Importer::parseMips( size_t& mips, bool& auto_generate, xmlNodePtr mips_node )
{
    Logger log = getLogger( "Scene.XML.Importer.parseMips" );

    const string mips_str = attribute( mips_node, "levels" );
    if( mips_str.empty() ) {
        SCENELOG_ERROR( log, "Required <mips> attribute 'levels' empty." );
        return false;
    }
    int t = atoi( mips_str.c_str() );
    if( t < 0 ) {
        SCENELOG_ERROR( log, "Number of mipmap levels must be a non-negative number." );
        return false;
    }
    mips = t;

    const string auto_gen_str = attribute( mips_node, "auto_generate" );
    if( auto_gen_str.empty() ) {
        SCENELOG_ERROR( log, "Required <mips> attribute 'auto_generate' empty." );
        return false;
    }
    auto_generate = parseBool( auto_gen_str, mips != 1 );

    return true;
}

bool
Importer::parseInitFrom( Image* image, xmlNodePtr init_from_node )
{
    Logger log = getLogger( "Scene.XML.Importer.parseInitFrom" );

    size_t array_index;
    const string array_index_str = attribute( init_from_node, "array_index" );
    if( array_index_str.empty() ) {
        array_index = 0;
    }
    else {
        int t = atoi( array_index_str.c_str() );
        if( t < 0 ) {
            SCENELOG_ERROR( log, "Illegal negative array index." << t );
            return false;
        }
        array_index = t;
    }

    size_t mip_index;
    const string mip_index_str = attribute( init_from_node, "mip_index" );
    if(mip_index_str.empty() ) {
        SCENELOG_ERROR( log, "Required <init_from> attribute 'mip_index' empty." );
        return false;
    }
    else {
        int t = atoi( mip_index_str.c_str() );
        if( t < 0 ) {
            SCENELOG_ERROR( log, "Illegal negative mip index " << t );
            return false;
        }
        mip_index = t;
    }

    size_t depth = 0;
    const string depth_str = attribute( init_from_node, "depth" );
    if( image->type() == IMAGE_3D ) {
        if( depth_str.empty() ) {
            SCENELOG_ERROR( log, "<init_from> attribute 'depth' required for 3D textures" );
            return false;
        }
        int t = atoi( depth_str.c_str() );
        if( t < 0 ) {
            SCENELOG_ERROR( log, "Illegal negative depth " << t );
            return false;
        }
        depth = t;
    }
    else {
        if(!depth_str.empty()) {
            SCENELOG_WARN( log, "<init_from> attribute 'depth' only allowed for 3D textures." );
        }
    }

    size_t face;
    const string face_str = attribute( init_from_node, "face" );
    if( image->type() == IMAGE_CUBE ) {
        if( face_str.empty() ) {
            SCENELOG_ERROR( log, "<init_from> attribute 'face' required for CUBE textures." );
            return false;
        }
        if( face_str == "POSITIVE_X" ) {
            face = 0;
        }
        else if( face_str == "NEGATIVE_X" ) {
            face = 1;
        }
        else if( face_str == "POSITIVE_Y" ) {
            face = 2;
        }
        else if( face_str == "NEGATIVE_Y" ) {
            face = 3;
        }
        else if( face_str == "POSITIVE_Z" ) {
            face = 4;
        }
        else if( face_str == "NEGAWTIVE_Z" ) {
            face = 5;
        }
        else {
            SCENELOG_ERROR( log, "Unrecognized CUBE map face specifier '" << face_str << '\'' );
            return false;
        }
    }
    else {
        if( !face_str.empty() ) {
            SCENELOG_WARN( log, "<init_from> attribute 'face' only allowed for CUBE textures." );
        }
    }

    xmlNodePtr m = init_from_node->children;
    if( checkNode( m, "generate" ) ) {
        SCENELOG_WARN( log, "<generate> is not standard COLLADA." );

        string mode = attribute( m, "mode" );
        if( mode == "RANDOM" ) {
            string seed_str = attribute( m, "seed" );
            if( seed_str.empty() ) {
                SCENELOG_ERROR( log, "Required <generate MODE='RANDOM'> attribute 'seed' empty." );
                return false;
            }
            unsigned int seed = atoi( seed_str.c_str() );
            boost::mt19937 rng(seed);

            size_t count = image->texelsInSlice();
            if( image->elementType() == GL_UNSIGNED_BYTE ) {
                boost::uniform_int<> r1(0,255);
                boost::variate_generator<boost::mt19937&, boost::uniform_int<> > die(rng, r1);
                vector<GLubyte> data( count );
                for(size_t i=0; i<data.size(); i++) {
                    data[i] = die();
                }
                image->set( 0, depth, &data[0] );
            }
            else if( image->elementType() == GL_FLOAT ) {
                boost::uniform_real<float> r2(0.0f,1.0f);
                boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > die(rng, r2);
                vector<GLfloat> data( count );
                for(size_t i=0; i<data.size(); i++) {
                    data[i] = die();
                }
                image->set( 0, 0, &data[0] );
            }
            else {
                SCENELOG_ERROR( log, "Image has unsupported element type." );
                return false;
            }



        }
        else {
            SCENELOG_ERROR( log, "Unknown mode '" << mode << '\'' );
            return false;
        }

    }
    else if( checkNode( m, "ref" ) ) {
        const string URL = getBody(m);
        vector<char> contents;
        if( !retrieveBinaryFile( contents, URL ) ) {
            SCENELOG_ERROR( log, "Failed to get '" << URL << '\'' );
            return false;
        }

    }
    else if( checkNode( m, "hex" ) ) {
        SCENELOG_FATAL( log, "Unimplemented code pathh @ " << __LINE__ );
        return false;
    }
    else {
        SCENELOG_ERROR( log, "Premature end of <init_from> node, expected <ref> or <hex>" );
        return false;
    }


    return true;
}



bool
Importer::parseImage( const Asset& asset_parent,
                      xmlNodePtr image_node )
{
    Logger log = getLogger( "Scene.XML.Importer.parseImage" );
    if( !assertNode( image_node, "image" ) ) {
        return false;
    }

    // COLLADA doesn't require an ID attribute, but we do.
    std::string id = attribute( image_node, "id" );
    if( id.empty() ) {
        SCENELOG_ERROR( log, "<image> with empty id attribute." );
        return false;
    }

    Image* image = m_database.library<Image>().add( id );
    if( image == NULL ) {
        SCENELOG_ERROR( log, "Error creating image '" << id << "'." );
        return false;
    }


    xmlNodePtr n = image_node->children;

    Asset asset;
    if( n!=NULL && xmlStrEqual( n->name, BAD_CAST "asset" ) ) {
        if( !parseAsset( asset, n ) )  {
            SCENELOG_ERROR( log, "Failed to parse <asset>" );
            return false;
        }
    }
    else {
        asset = asset_parent;
    }

    if( n!=NULL && xmlStrEqual( n->name, BAD_CAST "renderable" ) ) {
        //bool share = parseBool( attribute( n, "share"), false );
    }

    if( n==NULL ) {

    }
    else if( xmlStrEqual( n->name, BAD_CAST "init_from" ) ) {
        xmlNodePtr m = n->children;

        bool auto_generate = parseBool( attribute( m, "mips_generate"), true );

        std::string URL;
        if( checkNode( m->children, "ref" ) ) {
            URL = getBody( m->children );
        }
        else if( checkNode( m->children, "hex" ) ) {
            SCENELOG_ERROR( log, "init_from/hex not implemented");
            return false;
        }
        else {
            std::string body = getBody( m );
            if( !body.empty() ) {
                SCENELOG_WARN( log, "COLLADA 1.4-ism: no init_from/(ref|hex) child, assuming body is ref" );
                URL = body;
            }
        }

        // read file
        vector<char> contents;
        if( !retrieveBinaryFile( contents, URL ) ) {
            SCENELOG_ERROR( log, "Failed to get '" << URL << '\'' );
            return false;
        }

        if( URL.length() > 4 && URL.substr(URL.length()-4) == ".png" ) {
            GLenum iformat, format, type;
            size_t width, height;

            std::vector<unsigned char> data;
            if(readPNG( iformat, format, type, width, height, data, contents )) {
                image->init2D( iformat,
                               format,
                               type,
                               width,
                               height,
                               1,
                               auto_generate ? 0 : 1,
                               auto_generate );
                image->set( 0, 0, &data[0] );
            }
            else {
                SCENELOG_ERROR( log, "Failed to parse PNG  '" << URL << '\'' );
            }
        }
        else {
            SCENELOG_ERROR(log, "Unknown image suffix '" << URL << "'." );
            return false;
        }
        n=n->next;
    }
    else if( xmlStrEqual( n->name, BAD_CAST "create_2d" ) ) {

        GLenum iformat = GL_RGBA;
        GLenum type = GL_UNSIGNED_BYTE;
        GLenum format = GL_RGBA;
        size_t width, height, mips, array_length=1;
        bool auto_gen_mips;
        bool use_ratio;
        float ratio_width = 0.f;
        float ratio_height = 0.f;

        xmlNodePtr m = n->children;
        if( checkNode( m, "size_exact" ) ) {
            use_ratio = false;
            if(!parseSize( &width, &height, NULL, m ) ) {
                SCENELOG_ERROR( log, "Failed to parse <size_exact>" );
                return false;
            }
            m = m->next;
        }
        else if( checkNode( m, "size_ratio" ) ) {
            use_ratio = true;
            SCENELOG_FATAL( log, "Unimplemented code path @ " << __LINE__ );
            return false;
        }
        else {
            SCENELOG_ERROR( log, "Expected <size_exact> or <size_ratio>" );
            return false;
        }

        if(!assertChild( n, m, "mips" ) ) {
            return false;
        }
        if(!parseMips( mips, auto_gen_mips, m )) {
            SCENELOG_ERROR( log, "Failed to parse <mips>" );
            return false;
        }
        m = m->next;

        if( checkNode( m, "unnormalized" ) ) {
            SCENELOG_WARN( log, "<unnormalized> currently ignored." );
            m = m->next;
        }

        if( checkNode( m, "array" ) ) {
            if(!parseArray( array_length, m ) ) {
                SCENELOG_ERROR( log, "Failed to parse <array>" );
                return false;
            }
            m = m->next;
        }
        if( checkNode( m, "format" ) ) {
            if(!parseFormat( iformat, format, type, m ) ) {
                SCENELOG_ERROR( log, "Failed to parse <format>" );
                return false;
            }
            m = m->next;
        }


        if( use_ratio ) {
            image->init2D( iformat,
                           format,
                           type,
                           ratio_width,
                           ratio_height,
                           array_length,
                           mips,
                           auto_gen_mips );
        }
        else {
            image->init2D( iformat,
                           format,
                           type,
                           width,
                           height,
                           array_length,
                           mips,
                           auto_gen_mips );
        }
        while( checkNode( m, "init_from" ) ) {
            parseInitFrom( image, m );
            m = m->next;
        }
        nagAboutRemainingNodes( log, m );

        n=n->next;
    }


    else if( xmlStrEqual( n->name, BAD_CAST "create_cube" ) ) {
        GLenum iformat = GL_RGBA;
        GLenum type = GL_UNSIGNED_BYTE;
        GLenum format = GL_RGBA;
        size_t width, mips, array_length = 1;
        bool auto_gen_mips;

        xmlNodePtr m = n->children;
        if(!assertChild( n, m, "size" ) ) {
            return false;
        }
        if(!parseSize( &width, NULL, NULL, m ) ) {
            SCENELOG_ERROR( log, "failed to parse <size>" );
            return false;
        }
        m = m->next;

        if(!assertChild( n, m, "mips" ) ) {
            return false;
        }
        if(!parseMips( mips, auto_gen_mips, m )) {
            SCENELOG_ERROR( log, "Failed to parse <mips>" );
            return false;
        }
        m = m->next;

        if( checkNode( m, "array" ) ) {
            if(!parseArray( array_length, m ) ) {
                SCENELOG_ERROR( log, "Failed to parse <array>" );
                return false;
            }
            m = m->next;
        }
        if( checkNode( m, "format" ) ) {
            if(!parseFormat( iformat, format, type, m ) ) {
                SCENELOG_ERROR( log, "Failed to parse <format>" );
                return false;
            }
            m = m->next;
        }
        nagAboutRemainingNodes( log, m );

        if(! image->initCube( iformat,
                              format,
                              type,
                              width,
                              array_length,
                              mips,
                              auto_gen_mips ) )
        {
            SCENELOG_ERROR( log, "Failed to create image cube." );
            return false;
        }

        while( checkNode( m, "init_from" ) ) {
            parseInitFrom( image, m );
            m = m->next;
        }
        nagAboutRemainingNodes( log, m );
        n=n->next;
    }
    else if( xmlStrEqual( n->name, BAD_CAST "create_3d" ) ) {
        size_t width, height, depth, mips, array_length=1;
        bool auto_gen_mips;
        GLenum iformat = GL_RGBA;
        GLenum type = GL_UNSIGNED_BYTE;
        GLenum format = GL_RGBA;


        xmlNodePtr m = n->children;
        if(!assertChild( n, m, "size" )) {
            return false;
        }
        if(!parseSize( &width, &height, &depth, m ) ) {
            SCENELOG_ERROR( log, "Failed to parse <size>." );
            return false;
        }
        m = m->next;

        if( !assertChild( n, m, "mips" ) ) {
            return false;
        }
        if(!parseMips( mips, auto_gen_mips, m )) {
            SCENELOG_ERROR( log, "Failed to parse <mips>" );
            return false;
        }
        m = m->next;

        if( checkNode( m, "array" ) ) {
            if(!parseArray( array_length, m ) ) {
                SCENELOG_ERROR( log, "Failed to parse <array>" );
                return false;
            }
            m = m->next;
        }
        if( checkNode( m, "format" ) ) {
            if(!parseFormat( iformat, format, type, m ) ) {
                SCENELOG_ERROR( log, "Failed to parse <format>" );
                return false;
            }
            m = m->next;
        }

        if(! image->init3D( iformat, format, type, width, height, depth, array_length, mips, auto_gen_mips ) ) {
            SCENELOG_ERROR( log, "Failed to initialize texture 3D." );
            return false;
        }

        while( checkNode( m, "init_from" ) ) {
            parseInitFrom( image, m );
            m = m->next;
        }
        nagAboutRemainingNodes( log, m );
        n=n->next;
    }

    ignoreExtraNodes( log, n );
    nagAboutRemainingNodes( log, n );
    return true;
}


    } // of namespace XML
} // of namespace Scene
