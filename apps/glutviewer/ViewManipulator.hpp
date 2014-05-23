#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

/** View manipulator agnostic of windowing system.
 *
 * Converts mouse interactions to rotations, translations, zoom, and so on.
 * Supports classical 'examiner interaction' (ROTATE, SPIN, PAN, DOLLY, and
 * ZOOM) and 'first-person view' (ORIENT, PAN, FOLLOW, and ZOOM).
 *
 * \author Erik W. Bjønnes <erik.bjonnes@sintef.no>
 * \author Christopher Dyken <christopher.dyken@sintef.no>
 */
class ViewManipulator
{
public:
    
    /** View manipulator states. */
    enum State {
        NONE,   ///< Nothing happening.
        ROTATE, ///< Rotation around a pivot point.
        SPIN,   ///< Spinning around a pivot point.
        PAN,    ///< Translation along camera X and Y.
        DOLLY,  ///< Translation along camera Z, maintains pivot point.
        FOLLOW, ///< Translation along camera Z, adjusting for up dir, moves pivot.
        ZOOM,   ///< Zoom.
        ORIENT  ///< Rotation around viewer position.
    };
    
    /** Projection types. */
    enum ProjectionType
    {
        PROJECTION_PERSPECTIVE, ///< Use perspective projection.
        PROJECTION_ORTHOGRAPHIC ///< Use orthographic projection.
    };
    

    /** Creates new view manipulator that views the bounding box. */
    ViewManipulator(const glm::vec3 &bb_min, const glm::vec3 &bb_max);
    
    /** Sets new camera state.
     *
     * Called by the window manager to set a new camera state.
     * Saves mouse position, and reset elapsed time.
     * Saves camera state
     * \param int state state of camera, NONE, SPIN, etc.
     * \param int x, y mouse position
     */
    void
    startMotion(int state, int x, int y);
    
    void
    motion(int x, int y );

    /** If state is SPIN, calculates new positions and call calculateMatrices() */
    void
    update();
    
    void
    endMotion(int x, int y );
    
    /** Sets the projection type to be used for the viewer */
    void
    setProjectionType(ProjectionType pt);
    
    /** Get the current projection type. */
    ProjectionType
    getProjectionType() const
    { return m_projection; }
    
    /** Sets the window size to be used by the viewer */
    void setWindowSize(int w, int h);
    
    /** Sets the orientation of the camera to that of the matrix. */
    void setOrientation(const glm::mat4 &m);
    
    /** Sets the orientation of the camera to that of the quaternion. */
    void setOrientation(const glm::quat &q);
    
    /** Sets the view volume of the viewer to an axis aligned bounding box give by the input parameters.
             *
             * \param[in] bb_min the min corner of the AABB.
             * \param[in] bb_max the max corner of the AABB.
             * Calls calculateMatrices()
             */
    void setViewVolume(const glm::vec3 &bb_min, const glm::vec3 &bb_max);
    
    /** Sets the camera from an affine transformation and a projection matrix.
     *
     * It uses the rotation and translation parts of the modelview matrix to
     * position and orient the camera, and uses the angle between the top and
     * bottom frustum planes to deduce the field-of-view along y.
     *
     * If guess_bbox is enabled, it positions the center of interest in the
     * center between the near and far planes and sets the bounding box to be an
     * axis aligned box centered around this point with side lengths equal to
     * the distance between the near and far planes.
     *
     * \param modelview  An homogeneous affine transform matrix that encodes the
     *                   position and orientation of the camera.
     * \param projection An homogeneous projection matrix.
     *
     * \author Christopher Dyken, <christopher.dyken@sintef.no>
     */
    void
    setCamera( const glm::mat4& modelview, const glm::mat4& projection, const bool guess_bbox = false );
    
    
    /** Updates the view volume, but doesn't move the camera. */
    void updateViewVolume(const glm::vec3 &bb_min, const glm::vec3 bb_max);
    
    /** Sets the viewer to have the entire AABB in view.
     *
     * Updates camera center of interest and distance.
     */
    void viewAll();
    
    /** Get current modelview matrix. */
    const glm::mat4&
    getModelviewMatrix() const
    { return m_camera_state_curr.m_model_view; }
    
    /** Get current modelview inverse matrix. */
    const glm::mat4&
    getModelviewInverseMatrix() const
    { return m_camera_state_curr.m_model_view_inverse; }
    
    /** Get current Projection matrix. */
    const glm::mat4&
    getProjectionMatrix() const
    { return m_camera_state_curr.m_projection; }
    
    /** Get current Projection inverse matrix.*/
    const glm::mat4& getProjectionInverseMatrix() const
    { return m_camera_state_curr.m_projection_inverse; }
    
    /** Get current modelview-projection matrix. */
    const glm::mat4& getModelviewProjectionMatrix() const
    { return m_camera_state_curr.m_model_view_projection; }
    
    /** Get current modelview-projection inverse matrix. */
    const glm::mat4& getModelviewProjectionInverseMatrix() const
    { return m_camera_state_curr.m_model_view_projection_inverse; }
    
    /** Get camera orientation. */
    const glm::quat&
    getOrientation() const
    { return m_camera_state_curr.m_orientation; }
    
    /** Get center of interest. */
    const glm::vec3&
    getCenterOfInterest() const
    { return m_camera_state_curr.m_center_of_interest; }
    
    /** Get current window size. */
    const glm::vec2&
    getWindowSize() const
    { return m_win_size; }
    
    /** Get current aspect ratio. */
    float getAspectRatio() const
    { return m_win_size[0]/m_win_size[1]; }
    
    /** Get current field of view in xz-plane. */
    float
    getFieldOfViewX() const
    { // FIXME: this is probably incorrect.
        return getAspectRatio()*m_camera_state_curr.m_fov;
    }
    
    /** Get current field of view in yz-plane. */
    float getFieldOfViewY() const
    { return m_camera_state_curr.m_fov; }
    

    /** Get current near- and far-plane. */
    glm::vec2
    getNearFarPlanes() const
    { return glm::vec2( m_camera_state_curr.m_near, m_camera_state_curr.m_far ); }
    
    
    /** Get current camera position. */
    glm::vec3 getCurrentViewPoint() const;
    
    

    /** Set the field of view
     * \param fov in degrees
     * Converts the input fov to radians and stores it in the class.
     */
    void setFOV(const float fov)
    { m_camera_state_curr.m_fov = fov; }
    

protected:

    /** Struct keeping track of the camera states. */
    struct CameraState
    {
        glm::mat4 m_model_view;
        glm::mat4 m_model_view_inverse;
        glm::mat4 m_projection;
        glm::mat4 m_projection_inverse;
        glm::mat4 m_model_view_projection;
        glm::mat4 m_model_view_projection_inverse;
        glm::quat m_orientation;
        glm::vec3 m_center_of_interest;
        float     m_near;
        float     m_far;
        float     m_distance;
        float     m_fov;
    };

    glm::vec2       m_mouse_state_init;
    glm::vec2       m_mouse_state_prev;
    glm::vec2       m_mouse_state_curr;
    CameraState     m_camera_state_init;
    CameraState     m_camera_state_curr;
    ProjectionType  m_projection;
    int             m_state;
    glm::vec3       m_spin_axis;
    glm::vec3       m_up_axis;
    float           m_spin_speed;
    glm::vec3       m_bbox_min;
    glm::vec3       m_bbox_max;
    glm::vec2       m_win_size;
    
    /** Calculates the matrices used by the viewer. */
    void calculateMatrices();
    
    /** If true gives the axis and angle between the two points.
     *
     * \param axis between points, return value
     * \param angle between the points, return value
     * \param wc1 point in window coordinates
     * \param wc2 second point in window coordinates
     */
    bool
    trackball(glm::vec3 &axis, float &angle, const glm::vec2 &wc1, const glm::vec2 &wc2);

    
    /** Checks if any of the elements of a quaternion is NaN by checking if any element != itself
             *
             * \param Quaternion to check
             */
    inline bool checkNaN(glm::quat q)
    {
        return ((q.x != q.x) || (q.y != q.y) || (q.z != q.z) || (q.w != q.w));
    }
    
    glm::vec3
    mousePosOnInterestPlaneAsObjectCoords( glm::vec2 mouse_pos);


};
