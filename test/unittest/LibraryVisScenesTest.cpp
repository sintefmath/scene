#include <string>
#include <gtest/gtest.h>

#include <scene/DataBase.hpp>
#include <scene/Asset.hpp>
#include <scene/VisualScene.hpp>
#include <scene/Value.hpp>
#include <scene/collada/Importer.hpp>

static std::string test_document =
"<?xml version=\"1.0\"?>"
"<COLLADA>"
"  <asset>"
"    <created>2006-06-21T21:23:22Z</created>"
"    <modified>2006-06-21T21:23:22Z</modified>"
"  </asset>"
"  <library_visual_scenes>"
"    <asset>"
"      <created>2011-01-02T01:02:03Z</created>"
"      <modified>2011-01-02T01:02:04Z</modified>"
"    </asset>"
"    <visual_scene id=\"vis_scene\">"
"      <asset>"
"	<created>2011-01-02T03:04:05Z</created>"
"	<modified>2011-01-02T04:05:06Z</modified>"
"      </asset>"
"      <node id=\"vis_scene_node_0\" />"
"      <node id=\"vis_scene_node_1\" />"
"      <evaluate_scene id=\"eval0_id\" sid=\"eval0_sid\" enabled=\"FALSE\" >"
"	<render camera_node=\"#app_camera_node\">"
"	  <extra>"
"           <technique profile=\"scene\">"
"	      <light_node index=\"0\" ref=\"#light0\" />"
"	      <light_node index=\"2\" ref=\"#light2\" />"
"           </technique>"
"	  </extra>"
"	</render>"
"      </evaluate_scene>"
"      <evaluate_scene id=\"eval1_id\" sid=\"eval1_sid\" enabled=\"TRUE\">"
"	<asset>"
"	  <created>2011-01-02T05:06:07Z</created>"
"	  <modified>2011-01-02T06:07:08Z</modified>"
"	</asset>"
"      </evaluate_scene>"
"      <extra>"
"	Ignore me"
"      </extra>"
"    </visual_scene>"
"  </library_visual_scenes>"
"</COLLADA>";


TEST( LibraryVisualScenes, Import )
{
    Scene::DataBase database;
    Scene::Collada::Importer importer( database );
    importer.parseMemory( test_document.c_str() );

    EXPECT_EQ( database.library<Scene::VisualScene>().size(), 1 );
    Scene::VisualScene* vis_scene = database.library<Scene::VisualScene>().get( "vis_scene" );
    ASSERT_TRUE( vis_scene != NULL );
    EXPECT_EQ( "vis_scene", vis_scene->id() );

    // Check that the nodes have been imported
    Scene::Node* scene_nodes = database.library<Scene::Node>().get( vis_scene->nodesId() );
    ASSERT_TRUE( scene_nodes != NULL );
    ASSERT_EQ( 2, scene_nodes->children() );
    EXPECT_EQ( scene_nodes, scene_nodes->child(0)->parent() );
    EXPECT_EQ( scene_nodes, scene_nodes->child(1)->parent() );

    // Check ID of evaluate scene
    EXPECT_EQ( 2, vis_scene->evaluateScenes() );
    EXPECT_EQ( "eval0_id", vis_scene->evaluateScene(0)->id() );
    EXPECT_EQ( "eval1_id", vis_scene->evaluateScene(1)->id() );


    ASSERT_TRUE( vis_scene->evaluateScene(0)->renderItems() > 0 );
    EXPECT_EQ( "app_camera_node", vis_scene->evaluateScene(0)->renderItem(0)->cameraNodeId() );
    EXPECT_EQ( "light0", vis_scene->evaluateScene(0)->renderItem(0)->lightNodeId(0) );
    EXPECT_EQ( "", vis_scene->evaluateScene(0)->renderItem(0)->lightNodeId(1) );
    EXPECT_EQ( "light2", vis_scene->evaluateScene(0)->renderItem(0)->lightNodeId(2) );
}

