#include <fstream>
#include <boost/lexical_cast.hpp>
#include <scene/DataBase.hpp>
#include <scene/Parameter.hpp>
#include <scene/Geometry.hpp>
#include <scene/SourceBuffer.hpp>
#include <scene/collada/Importer.hpp>

#include <gtest/gtest.h>

// These tests are outdated and must be rewritten
#if 0

class BuilderImportTestStatic
{
public:
    BuilderImportTestStatic()
    {
        Scene::XML::Importer importer( m_db );
        importer.parse( "data/testdataset1.xml" );
    }

    Scene::DataBase  m_db;

};


static BuilderImportTestStatic*  instance = NULL;


class BuilderImportTest : public ::testing::Test
{
protected:
    virtual
    void SetUp()
    {
        if( instance == NULL ) {
            instance = new BuilderImportTestStatic;
        }
    }
};

TEST_F( BuilderImportTest, testeffect1_GLSL )
{
    // check existence
    const Scene::Effect* e = instance->m_db.library<Scene::Effect>().get( "testeffect1" );
    EXPECT_FALSE( e == NULL );
    ASSERT_EQ( e->id(), "testeffect1" );

    // check parameters
    ASSERT_EQ( e->parameters(), 1 );
    const Scene::Parameter* p = e->parameter( 0 );
    ASSERT_EQ( p->sid(), "ambient" );





    /*
    const Scene::DataBase& db1 = m_instance->m_db_1;
    const Scene::DataBase& db2 = m_instance->m_db_2;

    ASSERT_EQ( db1.materials(), db2.materials() );
*/
}


class BuilderImportGeometryTest : public ::testing::Test
{
protected:
    virtual
    void SetUp()
    {
        if( instance == NULL ) {
            instance = new BuilderImportTestStatic;
        }
        m_g = instance->m_db.library<Scene::Geometry>().get( "my_geometry_foo" );
        EXPECT_TRUE( m_g == NULL );
        m_g = instance->m_db.library<Scene::Geometry>().get( "geometry_default" );
        EXPECT_FALSE( m_g == NULL );
    }

    void
    checkPrimitiveSet( const size_t p_ix,
                       const Scene::PrimitiveType prim_type,
                       const size_t prim_count )
    {
        if( m_g == NULL ) {
            return;
        }

        ASSERT_LE( p_ix, m_g->primitiveSets() );
        const Scene::Primitives& p = m_g->primitiveSet( p_ix );
        ASSERT_EQ( prim_type, p.m_type );
        ASSERT_EQ( prim_count, p.m_count );
        ASSERT_EQ( 1, p.m_index_elements );
        ASSERT_EQ( 0, p.m_index_offset_vertex );
        ASSERT_EQ( "dummymaterial", p.m_material_symbol );
    }

    void
    checkUnindexed( const size_t p_ix )
    {
        if( m_g == NULL ) {
            return;
        }
        ASSERT_LE( p_ix, m_g->primitiveSets() );
        const Scene::Primitives& p = m_g->primitiveSet( p_ix );
        ASSERT_EQ( "", p.m_index_source_id );
    }

    void
    checkIndexed( const size_t p_ix,
                  const int* indices,
                  size_t     indices_n )
    {
        if( m_g == NULL ) {
            return;
        }
        ASSERT_LE( p_ix, m_g->primitiveSets() );
        const Scene::Primitives& p = m_g->primitiveSet( p_ix );

        ASSERT_EQ( m_g->id() + "_indices_" + boost::lexical_cast<std::string>( p_ix ),
                   p.m_index_source_id );
        ASSERT_EQ( "dummymaterial", p.m_material_symbol );

        const Scene::SourceBuffer* s = instance->m_db.library<Scene::SourceBuffer>().get( p.m_index_source_id );
        EXPECT_FALSE( s == NULL );

        ASSERT_EQ( indices_n, s->elementCount());

        const int* g_indices = s->intData();
        EXPECT_FALSE( g_indices == NULL );

        for(size_t i=0; i<indices_n; i++ ) {
            ASSERT_EQ( indices[i], g_indices[i] );
        }
    }

    const Scene::Geometry* m_g;
};



TEST_F( BuilderImportGeometryTest, unindexed_triangles )
{
    checkPrimitiveSet( 0, Scene::PRIMITIVE_TRIANGLES, 1 );
    checkUnindexed( 0 );
}

TEST_F( BuilderImportGeometryTest, indexed_triangles )
{
    const int indices[3] = {0, 2, 1 };

    checkPrimitiveSet( 1, Scene::PRIMITIVE_TRIANGLES, 1 );
//    checkIndexed( 1, indices, 3 );
}

/*
TEST_F( BuilderImportGeometryTest, unindexed_patches )
{
    checkPrimitiveSet( 2, Scene::PRIMITIVE_TRIANGLES, 1 );
    checkUnindexed( 0 );
}
*/
/*
TEST_F( BuilderImportGeometryTest, indexed_triangles )
{
    const int indices[3] = {0, 2, 1 };

    checkPrimitiveSet( 1, Scene::PRIMITIVE_TRIANGLES, 1 );
    checkIndexed( 1, indices, 3 );
}
*/
#endif
