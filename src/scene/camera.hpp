#pragma once


/**
 * @brief
 */
class Camera
{
public:
    /**
     * @brief TODO
     * TODO fix defaults
     */
    Camera(float aspect_ratio,
           const glm::vec3& pos = glm::vec3(0.0f, 5.0f, 0.0f), 
           const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
           const glm::vec3& front = glm::vec3(0.0f, 0.0f, -1.0f),
           float yaw = DEFAULT_YAW, float pitch = DEFAULT__PITCH);

    void update(float deltaTime);

    // ------------------------------------------------------------------------
    // Getters
    // ------------------------------------------------------------------------
    
    const glm::vec3& get_position() const { return position; }

    const glm::vec3& get_direction() const { return front; }

    /** @return pitch in degrees */
    float get_pitch() const { return pitch; }

    /** @return yaw in degrees */
    float get_yaw() const { return yaw; }

    // TODO make it proj view mat!
    // TODO make class variable
    glm::mat4 get_view_matrix() const 
    {
        return glm::lookAt(position, position + front, up);
    }

    glm::mat4 get_proj_matrix() const
    {
        return glm::perspective(fov, aspect_ratio, near_plane, far_plane);
    }

    /** @return Field of view in degrees */
    float get_field_of_view() const { return fov; }

    float get_near_plane_dist() const { return near_plane; }

    float get_far_plane_dist() const { return far_plane; }

    // ------------------------------------------------------------------------
    // Setters
    // ------------------------------------------------------------------------
   
    void set_position(const glm::vec3& p) { position = p; }

    void set_pitch(float v) { pitch = glm::clamp(v, MIN_PITCH, MAX_PITCH); update(); }

    void set_yaw(float v) { yaw = glm::mod(v, MAX_YAW); update(); }

    /**
     * @brief Sets the properties of projection matrix.
     * @param aspect_ratio Screen aspect ratio.
     * @param fov Field of view, in degrees.
     * @param near_plane Distance to near_plane plane.
     * @param far_plane Distance to far_plane plane.
     */
    void set_proj_mat_props(float aspect_ratio, float fov, float near_plane, float far_plane)
    {
        this->aspect_ratio = aspect_ratio;
        this->fov = glm::radians(fov);
        this->near_plane = near_plane;
        this->far_plane = far_plane;
    }

    void set_aspect_ratio(float as) { this->aspect_ratio = as; }

    /**
     * @param fov Field of view in degrees */
    void set_field_of_view(float fov) { this->fov = fov; }

    void set_near_plane_dist(float dist) { this->near_plane = dist; }

    void set_far_plane_dist(float dist) { this->far_plane = dist; }

    // ------------------------------------------------------------------------
    // Input handlers - control the camera
    // ------------------------------------------------------------------------
    
    /**
     * @brief TODO
     * @param x
     * @param y
     */
    void on_mouse_move(double x, double y);

    void on_mouse_button(int button, int action, int mods);

    void on_key_pressed(int key, int action);

    // TODO assumes GLFW_PRESS = 1, GLFW_RELEASE = 0
    void key_forward(int action)  { is_forward = action; }
    void key_backward(int action) { is_backward = action; }
    void key_right(int action)    { is_right = action; }
    void key_left(int action)     { is_left = action; }

    void key_reset(int action)
    { 
        first_cursor = true; 
        is_forward = is_backward = is_right = is_left = false;
    }

private:
    void update();

private:
    glm::vec3 position;         ///< Position of the camera
    glm::vec3 front;            ///< WHere the camera is looking at
    glm::vec3 right;            ///< Right vector of the camera
    glm::vec3 up;               ///< Up vector of the camera

    float aspect_ratio, fov, near_plane, far_plane;     ///< Properties of projection matrix

    // Looking in which direction in xz plane 
    //  0 degrees - looking in   -z direction
    //  90 degrees - looking in  -x direction
    //  180 degrees - looking in +z direction
    //  270 degrees - looking in +x direction
    float yaw;

    // positive ... looking from above the xz plane
    // negative ... looking from below the xz plane
    float pitch;
    
    int last_x, last_y;     ///< Last position of cursor on the screen
    bool first_cursor;      ///< First time the cursor is registered

    // Active movement states of the camera 
    bool is_forward, is_backward, is_right, is_left;

    // ------------------------------------------------------------------------
    // Constants - Defaults, maximums, etc.
    // ------------------------------------------------------------------------
    inline static const float DEFAULT_YAW           = 270.0f;
    inline static const float DEFAULT__PITCH        = 0.0f;
    inline static const float DEFAULT_SPEED         = 2.5f;

    inline static const float DEFAULT_FOV           = 45.0f;
    inline static const float DEFAULT_NEAR_PLANE    = 0.01f;
    inline static const float DEFAULT_FAR_PLANE     = 1000.0f;

    inline static const float MOUSE_SENSITIVITY     = 0.1f;
    inline static const float MOVE_SPEED            = 10.0f;

    inline static const float MAX_PITCH             = 89.0f;
    inline static const float MIN_PITCH             = -89.0f;
    inline static const float MAX_YAW               = 360.0f;

    inline static const glm::vec3 WORLD_UP          = glm::vec3(0.0f, 1.0f, 0.0f);
};

