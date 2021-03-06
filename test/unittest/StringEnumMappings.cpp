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
#include <scene/Scene.hpp>
#include <scene/Utils.hpp>

#define CHECK_RUNTIME_SEMANTIC(a,b) do { EXPECT_EQ( #a, Scene::runtimeSemantic( b ) ); EXPECT_EQ( b, Scene::runtimeSemantic(#a) ); } while(0)


TEST( StringEnumMap, RuntimeSemantics )
{
    CHECK_RUNTIME_SEMANTIC( FRAMEBUFFER_SIZE, Scene::RUNTIME_FRAMEBUFFER_SIZE );
    CHECK_RUNTIME_SEMANTIC( FRAMEBUFFER_SIZE_RECIPROCAL, Scene::RUNTIME_FRAMEBUFFER_SIZE_RECIPROCAL );
    CHECK_RUNTIME_SEMANTIC( MODELVIEW_MATRIX, Scene::RUNTIME_MODELVIEW_MATRIX );
    CHECK_RUNTIME_SEMANTIC( PROJECTION_MATRIX, Scene::RUNTIME_PROJECTION_MATRIX );
    CHECK_RUNTIME_SEMANTIC( PROJECTION_INVERSE_MATRIX, Scene::RUNTIME_PROJECTION_INVERSE_MATRIX );
    CHECK_RUNTIME_SEMANTIC( MODELVIEW_PROJECTION_MATRIX, Scene::RUNTIME_MODELVIEW_PROJECTION_MATRIX );
    CHECK_RUNTIME_SEMANTIC( NORMAL_MATRIX, Scene::RUNTIME_NORMAL_MATRIX );
    CHECK_RUNTIME_SEMANTIC( WORLD_FROM_OBJECT, Scene::RUNTIME_WORLD_FROM_OBJECT );
    CHECK_RUNTIME_SEMANTIC( OBJECT_FROM_WORLD, Scene::RUNTIME_OBJECT_FROM_WORLD );
    CHECK_RUNTIME_SEMANTIC( EYE_FROM_WORLD, Scene::RUNTIME_EYE_FROM_WORLD );
    CHECK_RUNTIME_SEMANTIC( WORLD_FROM_EYE, Scene::RUNTIME_WORLD_FROM_EYE );
    CHECK_RUNTIME_SEMANTIC( WORLD_FROM_CLIP, Scene::RUNTIME_WORLD_FROM_CLIP );
    CHECK_RUNTIME_SEMANTIC( LIGHT0_COLOR, Scene::RUNTIME_LIGHT0_COLOR );
    CHECK_RUNTIME_SEMANTIC( LIGHT1_COLOR, Scene::RUNTIME_LIGHT1_COLOR );
    CHECK_RUNTIME_SEMANTIC( LIGHT2_COLOR, Scene::RUNTIME_LIGHT2_COLOR );
    CHECK_RUNTIME_SEMANTIC( LIGHT3_COLOR, Scene::RUNTIME_LIGHT3_COLOR );
    CHECK_RUNTIME_SEMANTIC( LIGHT0_CONSTANT_ATT, Scene::RUNTIME_LIGHT0_CONSTANT_ATT );
    CHECK_RUNTIME_SEMANTIC( LIGHT1_CONSTANT_ATT, Scene::RUNTIME_LIGHT1_CONSTANT_ATT );
    CHECK_RUNTIME_SEMANTIC( LIGHT2_CONSTANT_ATT, Scene::RUNTIME_LIGHT2_CONSTANT_ATT );
    CHECK_RUNTIME_SEMANTIC( LIGHT3_CONSTANT_ATT, Scene::RUNTIME_LIGHT3_CONSTANT_ATT );
    CHECK_RUNTIME_SEMANTIC( LIGHT0_LINEAR_ATT, Scene::RUNTIME_LIGHT0_LINEAR_ATT );
    CHECK_RUNTIME_SEMANTIC( LIGHT1_LINEAR_ATT, Scene::RUNTIME_LIGHT1_LINEAR_ATT );
    CHECK_RUNTIME_SEMANTIC( LIGHT2_LINEAR_ATT, Scene::RUNTIME_LIGHT2_LINEAR_ATT );
    CHECK_RUNTIME_SEMANTIC( LIGHT3_LINEAR_ATT, Scene::RUNTIME_LIGHT3_LINEAR_ATT );
    CHECK_RUNTIME_SEMANTIC( LIGHT0_QUADRATIC_ATT, Scene::RUNTIME_LIGHT0_QUADRATIC_ATT );
    CHECK_RUNTIME_SEMANTIC( LIGHT1_QUADRATIC_ATT, Scene::RUNTIME_LIGHT1_QUADRATIC_ATT );
    CHECK_RUNTIME_SEMANTIC( LIGHT2_QUADRATIC_ATT, Scene::RUNTIME_LIGHT2_QUADRATIC_ATT );
    CHECK_RUNTIME_SEMANTIC( LIGHT3_QUADRATIC_ATT, Scene::RUNTIME_LIGHT3_QUADRATIC_ATT );
    CHECK_RUNTIME_SEMANTIC( LIGHT0_FALLOFF_COS, Scene::RUNTIME_LIGHT0_FALLOFF_COS );
    CHECK_RUNTIME_SEMANTIC( LIGHT1_FALLOFF_COS, Scene::RUNTIME_LIGHT1_FALLOFF_COS );
    CHECK_RUNTIME_SEMANTIC( LIGHT2_FALLOFF_COS, Scene::RUNTIME_LIGHT2_FALLOFF_COS );
    CHECK_RUNTIME_SEMANTIC( LIGHT3_FALLOFF_COS, Scene::RUNTIME_LIGHT3_FALLOFF_COS );
    CHECK_RUNTIME_SEMANTIC( LIGHT0_FALLOFF_EXPONENT, Scene::RUNTIME_LIGHT0_FALLOFF_EXPONENT );
    CHECK_RUNTIME_SEMANTIC( LIGHT1_FALLOFF_EXPONENT, Scene::RUNTIME_LIGHT1_FALLOFF_EXPONENT );
    CHECK_RUNTIME_SEMANTIC( LIGHT2_FALLOFF_EXPONENT, Scene::RUNTIME_LIGHT2_FALLOFF_EXPONENT );
    CHECK_RUNTIME_SEMANTIC( LIGHT3_FALLOFF_EXPONENT, Scene::RUNTIME_LIGHT3_FALLOFF_EXPONENT );
    CHECK_RUNTIME_SEMANTIC( LIGHT0_POS_OBJECT, Scene::RUNTIME_LIGHT0_POS_OBJECT );
    CHECK_RUNTIME_SEMANTIC( LIGHT1_POS_OBJECT, Scene::RUNTIME_LIGHT1_POS_OBJECT );
    CHECK_RUNTIME_SEMANTIC( LIGHT2_POS_OBJECT, Scene::RUNTIME_LIGHT2_POS_OBJECT );
    CHECK_RUNTIME_SEMANTIC( LIGHT3_POS_OBJECT, Scene::RUNTIME_LIGHT3_POS_OBJECT );
    CHECK_RUNTIME_SEMANTIC( LIGHT0_POS_WORLD, Scene::RUNTIME_LIGHT0_POS_WORLD );
    CHECK_RUNTIME_SEMANTIC( LIGHT1_POS_WORLD, Scene::RUNTIME_LIGHT1_POS_WORLD );
    CHECK_RUNTIME_SEMANTIC( LIGHT2_POS_WORLD, Scene::RUNTIME_LIGHT2_POS_WORLD );
    CHECK_RUNTIME_SEMANTIC( LIGHT3_POS_WORLD, Scene::RUNTIME_LIGHT3_POS_WORLD );
    CHECK_RUNTIME_SEMANTIC( LIGHT0_POS_EYE, Scene::RUNTIME_LIGHT0_POS_EYE );
    CHECK_RUNTIME_SEMANTIC( LIGHT1_POS_EYE, Scene::RUNTIME_LIGHT1_POS_EYE );
    CHECK_RUNTIME_SEMANTIC( LIGHT2_POS_EYE, Scene::RUNTIME_LIGHT2_POS_EYE );
    CHECK_RUNTIME_SEMANTIC( LIGHT3_POS_EYE, Scene::RUNTIME_LIGHT3_POS_EYE );
    CHECK_RUNTIME_SEMANTIC( LIGHT0_Z_OBJECT, Scene::RUNTIME_LIGHT0_Z_OBJECT );
    CHECK_RUNTIME_SEMANTIC( LIGHT1_Z_OBJECT, Scene::RUNTIME_LIGHT1_Z_OBJECT );
    CHECK_RUNTIME_SEMANTIC( LIGHT2_Z_OBJECT, Scene::RUNTIME_LIGHT2_Z_OBJECT );
    CHECK_RUNTIME_SEMANTIC( LIGHT3_Z_OBJECT, Scene::RUNTIME_LIGHT3_Z_OBJECT );
    CHECK_RUNTIME_SEMANTIC( LIGHT0_Z_WORLD, Scene::RUNTIME_LIGHT0_Z_WORLD );
    CHECK_RUNTIME_SEMANTIC( LIGHT1_Z_WORLD, Scene::RUNTIME_LIGHT1_Z_WORLD );
    CHECK_RUNTIME_SEMANTIC( LIGHT2_Z_WORLD, Scene::RUNTIME_LIGHT2_Z_WORLD );
    CHECK_RUNTIME_SEMANTIC( LIGHT3_Z_WORLD, Scene::RUNTIME_LIGHT3_Z_WORLD );
    CHECK_RUNTIME_SEMANTIC( LIGHT0_Z_EYE, Scene::RUNTIME_LIGHT0_Z_EYE );
    CHECK_RUNTIME_SEMANTIC( LIGHT1_Z_EYE, Scene::RUNTIME_LIGHT1_Z_EYE );
    CHECK_RUNTIME_SEMANTIC( LIGHT2_Z_EYE, Scene::RUNTIME_LIGHT2_Z_EYE );
    CHECK_RUNTIME_SEMANTIC( LIGHT3_Z_EYE, Scene::RUNTIME_LIGHT3_Z_EYE );
    CHECK_RUNTIME_SEMANTIC( LIGHT0_EYE_FROM_OBJECT, Scene::RUNTIME_LIGHT0_EYE_FROM_OBJECT );
    CHECK_RUNTIME_SEMANTIC( LIGHT1_EYE_FROM_OBJECT, Scene::RUNTIME_LIGHT1_EYE_FROM_OBJECT );
    CHECK_RUNTIME_SEMANTIC( LIGHT2_EYE_FROM_OBJECT, Scene::RUNTIME_LIGHT2_EYE_FROM_OBJECT );
    CHECK_RUNTIME_SEMANTIC( LIGHT3_EYE_FROM_OBJECT, Scene::RUNTIME_LIGHT3_EYE_FROM_OBJECT );
    CHECK_RUNTIME_SEMANTIC( LIGHT0_EYE_FROM_WORLD, Scene::RUNTIME_LIGHT0_EYE_FROM_WORLD );
    CHECK_RUNTIME_SEMANTIC( LIGHT1_EYE_FROM_WORLD, Scene::RUNTIME_LIGHT1_EYE_FROM_WORLD );
    CHECK_RUNTIME_SEMANTIC( LIGHT2_EYE_FROM_WORLD, Scene::RUNTIME_LIGHT2_EYE_FROM_WORLD );
    CHECK_RUNTIME_SEMANTIC( LIGHT3_EYE_FROM_WORLD, Scene::RUNTIME_LIGHT3_EYE_FROM_WORLD );
    CHECK_RUNTIME_SEMANTIC( LIGHT0_EYE_FROM_EYE, Scene::RUNTIME_LIGHT0_EYE_FROM_EYE );
    CHECK_RUNTIME_SEMANTIC( LIGHT1_EYE_FROM_EYE, Scene::RUNTIME_LIGHT1_EYE_FROM_EYE );
    CHECK_RUNTIME_SEMANTIC( LIGHT2_EYE_FROM_EYE, Scene::RUNTIME_LIGHT2_EYE_FROM_EYE );
    CHECK_RUNTIME_SEMANTIC( LIGHT3_EYE_FROM_EYE, Scene::RUNTIME_LIGHT3_EYE_FROM_EYE );
    CHECK_RUNTIME_SEMANTIC( LIGHT0_CLIP_FROM_OBJECT, Scene::RUNTIME_LIGHT0_CLIP_FROM_OBJECT );
    CHECK_RUNTIME_SEMANTIC( LIGHT1_CLIP_FROM_OBJECT, Scene::RUNTIME_LIGHT1_CLIP_FROM_OBJECT );
    CHECK_RUNTIME_SEMANTIC( LIGHT2_CLIP_FROM_OBJECT, Scene::RUNTIME_LIGHT2_CLIP_FROM_OBJECT );
    CHECK_RUNTIME_SEMANTIC( LIGHT3_CLIP_FROM_OBJECT, Scene::RUNTIME_LIGHT3_CLIP_FROM_OBJECT );
    CHECK_RUNTIME_SEMANTIC( LIGHT0_CLIP_FROM_WORLD, Scene::RUNTIME_LIGHT0_CLIP_FROM_WORLD );
    CHECK_RUNTIME_SEMANTIC( LIGHT1_CLIP_FROM_WORLD, Scene::RUNTIME_LIGHT1_CLIP_FROM_WORLD );
    CHECK_RUNTIME_SEMANTIC( LIGHT2_CLIP_FROM_WORLD, Scene::RUNTIME_LIGHT2_CLIP_FROM_WORLD );
    CHECK_RUNTIME_SEMANTIC( LIGHT3_CLIP_FROM_WORLD, Scene::RUNTIME_LIGHT3_CLIP_FROM_WORLD );
    CHECK_RUNTIME_SEMANTIC( LIGHT0_CLIP_FROM_EYE, Scene::RUNTIME_LIGHT0_CLIP_FROM_EYE );
    CHECK_RUNTIME_SEMANTIC( LIGHT1_CLIP_FROM_EYE, Scene::RUNTIME_LIGHT1_CLIP_FROM_EYE );
    CHECK_RUNTIME_SEMANTIC( LIGHT2_CLIP_FROM_EYE, Scene::RUNTIME_LIGHT2_CLIP_FROM_EYE );
    CHECK_RUNTIME_SEMANTIC( LIGHT3_CLIP_FROM_EYE, Scene::RUNTIME_LIGHT3_CLIP_FROM_EYE );
    CHECK_RUNTIME_SEMANTIC( LIGHT0_CLIP_FROM_CLIP, Scene::RUNTIME_LIGHT0_CLIP_FROM_CLIP );
    CHECK_RUNTIME_SEMANTIC( LIGHT1_CLIP_FROM_CLIP, Scene::RUNTIME_LIGHT1_CLIP_FROM_CLIP );
    CHECK_RUNTIME_SEMANTIC( LIGHT2_CLIP_FROM_CLIP, Scene::RUNTIME_LIGHT2_CLIP_FROM_CLIP );
    CHECK_RUNTIME_SEMANTIC( LIGHT3_CLIP_FROM_CLIP, Scene::RUNTIME_LIGHT3_CLIP_FROM_CLIP );
    CHECK_RUNTIME_SEMANTIC( LIGHT0_TEX_FROM_OBJECT, Scene::RUNTIME_LIGHT0_TEX_FROM_OBJECT );
    CHECK_RUNTIME_SEMANTIC( LIGHT1_TEX_FROM_OBJECT, Scene::RUNTIME_LIGHT1_TEX_FROM_OBJECT );
    CHECK_RUNTIME_SEMANTIC( LIGHT2_TEX_FROM_OBJECT, Scene::RUNTIME_LIGHT2_TEX_FROM_OBJECT );
    CHECK_RUNTIME_SEMANTIC( LIGHT3_TEX_FROM_OBJECT, Scene::RUNTIME_LIGHT3_TEX_FROM_OBJECT );
    CHECK_RUNTIME_SEMANTIC( LIGHT0_TEX_FROM_WORLD, Scene::RUNTIME_LIGHT0_TEX_FROM_WORLD );
    CHECK_RUNTIME_SEMANTIC( LIGHT1_TEX_FROM_WORLD, Scene::RUNTIME_LIGHT1_TEX_FROM_WORLD );
    CHECK_RUNTIME_SEMANTIC( LIGHT2_TEX_FROM_WORLD, Scene::RUNTIME_LIGHT2_TEX_FROM_WORLD );
    CHECK_RUNTIME_SEMANTIC( LIGHT3_TEX_FROM_WORLD, Scene::RUNTIME_LIGHT3_TEX_FROM_WORLD );
    CHECK_RUNTIME_SEMANTIC( LIGHT0_TEX_FROM_EYE, Scene::RUNTIME_LIGHT0_TEX_FROM_EYE );
    CHECK_RUNTIME_SEMANTIC( LIGHT1_TEX_FROM_EYE, Scene::RUNTIME_LIGHT1_TEX_FROM_EYE );
    CHECK_RUNTIME_SEMANTIC( LIGHT2_TEX_FROM_EYE, Scene::RUNTIME_LIGHT2_TEX_FROM_EYE );
    CHECK_RUNTIME_SEMANTIC( LIGHT3_TEX_FROM_EYE, Scene::RUNTIME_LIGHT3_TEX_FROM_EYE );



}
