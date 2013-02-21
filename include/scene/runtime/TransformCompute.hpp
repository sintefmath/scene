#pragma once

#include <scene/Scene.hpp>

namespace Scene {
    namespace Runtime {


class TransformCompute
{
public:

    static void
    cosine( Value* dst, const Value* src );

    static void
    reciprocal( Value* dst, const Value* src );

    /** Calculate the projection matrix of a camera. */
    static void
    projection( Value* dst, const Camera* c );

    /** Calculate the inverse projection matrix of a camera. */
    static void
    projectionInverse( Value* dst, const Camera* c );

    /** Get the composition of transforms of a node. */
    static void
    nodeTransform( Value* dst, const Node* n );

    /** Get the composition of inverse transforms of a node. */
    static void
    nodeInverseTransform( Value* dst, const Node* n );

    /** Transform the z-axis direction through a matrix composition. */
    static void
    transformZAxis( Value* dst, const unsigned int N, const Value** src );


    /** Extract the translation part of a matrix composition. */
    static void
    transformOrigin( Value* dst, const unsigned int N, const Value** src );


    /** Extract the transposed upper-left 3x3 part of a matrix composition.
     *
     * \param[out] dst  Destination, should be of type float3x3 (as this method
     *                  doesn't update type).
     * \param[in]  N    The number of matrices to multiply. If N==0, this is a
     *                  no-op.
     * \param[in]  src  An array of pointers to the source values.
     */
    static void
    transposedUpper3x3( Value* dst, const unsigned int N, const Value** src );


    /** Multiply a sequence of 4x4 matrices.
     *
     * \param[out] dst  Destination, should be of type float4x4 (as this method
     *                  doesn't update type).
     * \param[in]  N    The number of matrices to multiply. If N==0, this is a
     *                  no-op, if N=1, it is a copy.
     * \param[in]  src  An array of pointers to the source values.
     */
    static void
    multiplyMatrices( Value* dst, const unsigned int N, const Value** src );


    /** Check if bounding box is inside frustum.
     *
     * This function transforms the corners of a bounding box using the matrix
     * composition A*B*C.
     *
     * The source values should be ordered as follows:
     * - bounding box min (float4, required)
     * - bounding box max (float4, required)
     * - matrix A (float4x4, required)
     * - matrix B (float4x4, optional)
     * - matrix C (float4x4, optional)
     *
     * \param[out] dst  Destination, should be of type bool (as this method
     *                  doesn't update type).
     * \param[in]  N    The number of source values. If N < 3, this is a no-op.
     * \param[in]  src  An array of pointers to the source values.
     */
    static void
    boundingBoxTest( Value* dst, const unsigned int N, const Value** src );


};

    } // of namespace Runtime
} // of namespace Scene
