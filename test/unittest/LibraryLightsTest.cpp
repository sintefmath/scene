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

#include <string>
#include <gtest/gtest.h>

#include <scene/DataBase.hpp>
#include <scene/Asset.hpp>
#include <scene/Light.hpp>
#include <scene/Value.hpp>
#include <scene/collada/Importer.hpp>

TEST( LibraryLights, Import )
{
    Scene::DataBase database;
    Scene::Collada::Importer importer( database );
    importer.parse( "data/unit_lib_lights.xml" );

    EXPECT_EQ( database.library<Scene::Light>().size(), 6 );


    // Library has its own asset


    Scene::Light* light_ambient = database.library<Scene::Light>().get( "light_ambient");
    ASSERT_TRUE( light_ambient != NULL );


    Scene::Value light_ambient_color = *light_ambient->color();
    EXPECT_EQ( light_ambient_color.type(), Scene::VALUE_TYPE_FLOAT3 );
    EXPECT_FLOAT_EQ( light_ambient_color.floatData()[0], 1.0f );
    EXPECT_FLOAT_EQ( light_ambient_color.floatData()[1], 0.5f );
    EXPECT_FLOAT_EQ( light_ambient_color.floatData()[2], 0.9f );



    Scene::Light* light_directional = database.library<Scene::Light>().get( "light_directional");
    ASSERT_TRUE( light_directional != NULL );

    // Directional light has its own asset


    Scene::Value light_directional_color = *light_directional->color();
    EXPECT_EQ( light_directional_color.type(), Scene::VALUE_TYPE_FLOAT3 );
    EXPECT_FLOAT_EQ( light_directional_color.floatData()[0], 1.0f );
    EXPECT_FLOAT_EQ( light_directional_color.floatData()[1], 0.5f );
    EXPECT_FLOAT_EQ( light_directional_color.floatData()[2], 0.9f );


    // Point light using default values except for the compulsory color element
    Scene::Light* light_point_default = database.library<Scene::Light>().get( "light_point_default");
    ASSERT_TRUE( light_point_default != NULL );

    Scene::Value light_point_default_color = *light_point_default->color();
    EXPECT_EQ( light_point_default_color.type(), Scene::VALUE_TYPE_FLOAT3 );
    EXPECT_FLOAT_EQ( light_point_default_color.floatData()[0], 1.0f );
    EXPECT_FLOAT_EQ( light_point_default_color.floatData()[1], 0.5f );
    EXPECT_FLOAT_EQ( light_point_default_color.floatData()[2], 0.9f );

    Scene::Value light_point_default_const_att = *light_point_default->constantAttenuation();
    EXPECT_EQ( light_point_default_const_att.type(), Scene::VALUE_TYPE_FLOAT );
    EXPECT_FLOAT_EQ( 1.f, light_point_default_const_att.floatData()[0] );

    Scene::Value light_point_default_lin_att = *light_point_default->linearAttenuation();
    EXPECT_EQ( light_point_default_lin_att.type(), Scene::VALUE_TYPE_FLOAT );
    EXPECT_FLOAT_EQ( 0.f, light_point_default_lin_att.floatData()[0] );

    Scene::Value light_point_default_quad_att = *light_point_default->quadraticAttenuation();
    EXPECT_EQ( light_point_default_quad_att.type(), Scene::VALUE_TYPE_FLOAT );
    EXPECT_FLOAT_EQ( 0.f, light_point_default_quad_att.floatData()[0] );


    // Point light with everything (including sid's) defined in the XML
    Scene::Light* light_point_specified = database.library<Scene::Light>().get( "light_point_specified");
    ASSERT_TRUE( light_point_specified != NULL );

    Scene::Value light_point_specified_color = *light_point_specified->color();
    EXPECT_EQ( light_point_specified_color.type(), Scene::VALUE_TYPE_FLOAT3 );
    EXPECT_FLOAT_EQ( light_point_specified_color.floatData()[0], 1.0f );
    EXPECT_FLOAT_EQ( light_point_specified_color.floatData()[1], 0.5f );
    EXPECT_FLOAT_EQ( light_point_specified_color.floatData()[2], 0.9f );

    Scene::Value light_point_specified_const_att = *light_point_specified->constantAttenuation();
    EXPECT_EQ( light_point_specified_const_att.type(), Scene::VALUE_TYPE_FLOAT );
    EXPECT_FLOAT_EQ( 0.1f, light_point_specified_const_att.floatData()[0] );

    Scene::Value light_point_specified_lin_att = *light_point_specified->linearAttenuation();
    EXPECT_EQ( light_point_specified_lin_att.type(), Scene::VALUE_TYPE_FLOAT );
    EXPECT_FLOAT_EQ( 0.2f, light_point_specified_lin_att.floatData()[0] );

    Scene::Value light_point_specified_quad_att = *light_point_specified->quadraticAttenuation();
    EXPECT_EQ( light_point_specified_quad_att.type(), Scene::VALUE_TYPE_FLOAT );
    EXPECT_FLOAT_EQ( 0.3f, light_point_specified_quad_att.floatData()[0] );

    EXPECT_EQ( "constant", light_point_specified->constantAttenuationSid() );
    EXPECT_EQ( "linear", light_point_specified->linearAttenuationSid() );
    EXPECT_EQ( "quadratic", light_point_specified->quadraticAttenuationSid() );


    // Spot light using default values except for the compulsory color element
    Scene::Light* light_spot_default = database.library<Scene::Light>().get( "light_spot_default");
    ASSERT_TRUE( light_spot_default != NULL );

    Scene::Value light_spot_default_color = *light_spot_default->color();
    EXPECT_EQ( light_spot_default_color.type(), Scene::VALUE_TYPE_FLOAT3 );
    EXPECT_FLOAT_EQ( light_spot_default_color.floatData()[0], 1.0f );
    EXPECT_FLOAT_EQ( light_spot_default_color.floatData()[1], 0.5f );
    EXPECT_FLOAT_EQ( light_spot_default_color.floatData()[2], 0.9f );

    Scene::Value light_spot_default_const_att = *light_spot_default->constantAttenuation();
    EXPECT_EQ( light_spot_default_const_att.type(), Scene::VALUE_TYPE_FLOAT );
    EXPECT_FLOAT_EQ( 1.f, light_spot_default_const_att.floatData()[0] );

    Scene::Value light_spot_default_lin_att = *light_spot_default->linearAttenuation();
    EXPECT_EQ( light_spot_default_lin_att.type(), Scene::VALUE_TYPE_FLOAT );
    EXPECT_FLOAT_EQ( 0.f, light_spot_default_lin_att.floatData()[0] );

    Scene::Value light_spot_default_quad_att = *light_spot_default->quadraticAttenuation();
    EXPECT_EQ( light_spot_default_quad_att.type(), Scene::VALUE_TYPE_FLOAT );
    EXPECT_FLOAT_EQ( 0.f, light_spot_default_quad_att.floatData()[0] );

    Scene::Value light_spot_default_falloff_angle = *light_spot_default->falloffAngle();
    EXPECT_EQ( Scene::VALUE_TYPE_FLOAT, light_spot_default_falloff_angle.type() );
    EXPECT_FLOAT_EQ( 3.141592653589793238462643f, light_spot_default_falloff_angle.floatData()[0] );

    Scene::Value light_spot_default_falloff_exponent = *light_spot_default->falloffExponent();
    EXPECT_EQ( Scene::VALUE_TYPE_FLOAT, light_spot_default_falloff_exponent.type() );
    EXPECT_FLOAT_EQ( 0.0f, light_spot_default_falloff_exponent.floatData()[0] );


    // Spot light with everything (including sid's) defined in the XML
    Scene::Light* light_spot_specified = database.library<Scene::Light>().get( "light_spot_specified");
    ASSERT_TRUE( light_spot_specified != NULL );

    Scene::Value light_spot_specified_color = *light_spot_specified->color();
    EXPECT_EQ( light_spot_specified_color.type(), Scene::VALUE_TYPE_FLOAT3 );
    EXPECT_FLOAT_EQ( light_spot_specified_color.floatData()[0], 1.0f );
    EXPECT_FLOAT_EQ( light_spot_specified_color.floatData()[1], 0.5f );
    EXPECT_FLOAT_EQ( light_spot_specified_color.floatData()[2], 0.9f );

    Scene::Value light_spot_specified_const_att = *light_spot_specified->constantAttenuation();
    EXPECT_EQ( light_spot_specified_const_att.type(), Scene::VALUE_TYPE_FLOAT );
    EXPECT_FLOAT_EQ( 0.1f, light_spot_specified_const_att.floatData()[0] );
    EXPECT_EQ( "constant", light_spot_specified->constantAttenuationSid() );

    Scene::Value light_spot_specified_lin_att = *light_spot_specified->linearAttenuation();
    EXPECT_EQ( light_spot_specified_lin_att.type(), Scene::VALUE_TYPE_FLOAT );
    EXPECT_FLOAT_EQ( 0.2f, light_spot_specified_lin_att.floatData()[0] );
    EXPECT_EQ( "linear", light_spot_specified->linearAttenuationSid() );

    Scene::Value light_spot_specified_quad_att = *light_spot_specified->quadraticAttenuation();
    EXPECT_EQ( light_spot_specified_quad_att.type(), Scene::VALUE_TYPE_FLOAT );
    EXPECT_FLOAT_EQ( 0.3f, light_spot_specified_quad_att.floatData()[0] );
    EXPECT_EQ( "quadratic", light_spot_specified->quadraticAttenuationSid() );

    Scene::Value light_spot_specified_falloff_angle = *light_spot_specified->falloffAngle();
    EXPECT_EQ( Scene::VALUE_TYPE_FLOAT, light_spot_specified_falloff_angle.type() );
    EXPECT_FLOAT_EQ( 0.4f, light_spot_specified_falloff_angle.floatData()[0] );
    EXPECT_EQ( "falloff_angle", light_spot_specified->falloffAngleSid() );

    Scene::Value light_spot_specified_falloff_exponent = *light_spot_specified->falloffExponent();
    EXPECT_EQ( Scene::VALUE_TYPE_FLOAT, light_spot_specified_falloff_exponent.type() );
    EXPECT_FLOAT_EQ( 0.5f, light_spot_specified_falloff_exponent.floatData()[0] );
    EXPECT_EQ( "falloff_exponent", light_spot_specified->falloffExponentSid() );


}

