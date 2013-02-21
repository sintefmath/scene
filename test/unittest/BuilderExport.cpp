#include <fstream>
#include <scene/DataBase.hpp>
#include <scene/Geometry.hpp>
#include <scene/SourceBuffer.hpp>
#include <scene/collada/Importer.hpp>
#include <scene/collada/Exporter.hpp>
#include <gtest/gtest.h>

namespace Scene {
    namespace Collada {

    // These tests are outdated and must be rewritten
#if 0

class BuilderExportTestStatic
{
public:
    BuilderExportTestStatic()
    {
        Scene::XML::Importer importer( m_db_1 );
        importer.parse( "data/testdataset1.xml" );
//         importer.parse( "data/material_phong.xml" );
//         importer.parse( "data/geometry_cube.xml" );

         Scene::XML::Exporter exporter( m_db_1 );
         m_collada_node = exporter.create( Scene::PROFILE_BRIDGE |
                                           Scene::PROFILE_CG |
                                           Scene::PROFILE_COMMON |
                                           Scene::PROFILE_GLES |
                                           Scene::PROFILE_GLES2 |
                                           Scene::PROFILE_GLSL );

         Scene::XML::Importer reimporter( m_db_2 );
         reimporter.parseCollada( m_collada_node );

         xmlDocPtr doc = xmlNewDoc( BAD_CAST "1.0" );
         xmlDocSetRootElement( doc, m_collada_node );
         xmlChar* buf = NULL;
         int size;
         xmlDocDumpFormatMemory( doc, &buf, &size, 1 );
         if( size > 0 ) {
             std::ofstream dump( "unit_builder_dump.xml" );
             dump << std::string( (const char*)&buf[0], (const char*)&buf[size] );
             dump.close();
         }
         if( buf != NULL ) {
             xmlFree( buf );
         }
         xmlFreeDoc( doc );
    }


    Scene::DataBase  m_db_1;
    Scene::DataBase  m_db_2;
    xmlNodePtr       m_collada_node;
    Scene::Geometry* m_geometry;

    static BuilderExportTestStatic* m_instance;


};


static BuilderExportTestStatic* instance = NULL;


class BuilderTest : public ::testing::Test
{
protected:


    virtual
    void SetUp()
    {
        if( instance == NULL ) {
            instance = new BuilderExportTestStatic;
        }

    }




};

TEST_F( BuilderTest, Materials )
{
    const Scene::DataBase& db1 = instance->m_db_1;
    const Scene::DataBase& db2 = instance->m_db_2;

    ASSERT_EQ( db1.library<Scene::Material>().size(), db2.library<Scene::Material>().size() );
}

TEST_F( BuilderTest, SourceBuffers )
{
    const Scene::DataBase& db1 = instance->m_db_1;
    const Scene::DataBase& db2 = instance->m_db_2;

    ASSERT_EQ( db1.library<Scene::SourceBuffer>().size(),
               db2.library<Scene::SourceBuffer>().size() );

    for( size_t i=0; i<db1.library<Scene::SourceBuffer>().size(); i++ ) {
        const Scene::SourceBuffer* b1 = db1.library<Scene::SourceBuffer>().get( i );
        const Scene::SourceBuffer* b2 = db2.library<Scene::SourceBuffer>().get( b1->id() );
        ASSERT_FALSE( b1 == NULL );

        ASSERT_EQ( b1->elementType(), b2->elementType() );
        ASSERT_EQ( b1->elementCount(), b2->elementCount() );


        if( b1->elementType() == ELEMENT_FLOAT ) {
            const float* d1 = b1->floatData();
            const float* d2 = b2->floatData();
            for(size_t j=0; j<b1->elementCount(); j++) {
                ASSERT_EQ( d1[j], d2[j] );
            }
        }
        else if( b1->elementType() == ELEMENT_INT ) {
            const int* d1 = b1->intData();
            const int* d2 = b2->intData();
            for(size_t j=0; j<b1->elementCount(); j++) {
                ASSERT_EQ( d1[j], d2[j] );
            }
        }

    }

    // Check contents
}


TEST_F( BuilderTest, Geometries )
{

    size_t geo1_n = instance->m_db_1.library<Scene::Geometry>().size();
    size_t geo2_n = instance->m_db_2.library<Scene::Geometry>().size();
    ASSERT_EQ( geo1_n, geo2_n );

    for(size_t i=0; i<geo1_n; i++) {
        const Geometry* g1 = instance->m_db_1.library<Scene::Geometry>().get(i);
        const Geometry* g2 = instance->m_db_2.library<Scene::Geometry>().get( g1->id() );
        ASSERT_FALSE( g2 == NULL );

        ASSERT_EQ( g1->id(), g2->id() );
        ASSERT_EQ( g1->assetInfo().created().string(), g2->assetInfo().created().string() );
        ASSERT_EQ( g1->assetInfo().modified().string(), g2->assetInfo().modified().string() );

        for( size_t i=0; i<Scene::VERTEX_SEMANTIC_N; i++ ) {
            const Geometry::VertexInput& s1 = g1->vertexInput( (Scene::VertexSemantic)i );
            const Geometry::VertexInput& s2 = g2->vertexInput( (Scene::VertexSemantic)i );
            ASSERT_EQ( s1.m_enabled, s2.m_enabled );
            if( s1.m_enabled ) {
                ASSERT_EQ( s1.m_source_buffer_id, s2.m_source_buffer_id );
                ASSERT_EQ( s1.m_components, s2.m_components );
                ASSERT_EQ( s1.m_count, s2.m_count );
                ASSERT_EQ( s1.m_offset, s2.m_offset );
                ASSERT_EQ( s1.m_stride, s2.m_stride );
            }

        }
    }
}
#endif

    }
}
