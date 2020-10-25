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

    const glm::vec3& get_position() const { return position; }

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

    void on_key_pressed(float deltaTime, int key, int scancode, int action, int mods);

    void update(float deltaTime);

    // TODO UNUSED
    void move_forward(float dt)  { position += front * (MOVE_SPEED * dt); }
    void move_backward(float dt) { position -= front * (MOVE_SPEED * dt); }
    void move_right(float dt)    { position += right * (MOVE_SPEED * dt); }
    void move_left(float dt)     { position -= right * (MOVE_SPEED * dt); }

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
    void set_field_of_view(float fov) { this->fov = glm::radians(fov); }
    void set_near_plane_dist(float dist) { this->near_plane = dist; }
    void set_far_plane_dist(float dist) { this->far_plane = dist; }


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
    
    // 
    // positive ... looking from above the xz plane
    // negative ... looking from below the xz plane
    float pitch;
    

    int last_x, last_y;     ///< Last position of cursor on the screen

    bool is_forward, is_backward, is_right, is_left;    ///< Update of movement states

    // ------------------------------------------------------------------------
    // Constants - Defaults, maximums, etc.
    // ------------------------------------------------------------------------
    static const float DEFAULT_YAW;
    static const float DEFAULT__PITCH;
    static const float DEFAULT_SPEED;
    static const float DEFAULT_ZOOM;
    static const float DEFAULT_FOV;
    static const float DEFAULT_NEAR_PLANE;
    static const float DEFAULT_FAR_PLANE;

    static const float MOUSE_SENSITIVITY;
    static const float ZOOM_SENSITIVITY;
    static const float MOVE_SPEED;

    static const float MAX_PITCH;
    static const float MIN_PITCH;
};

