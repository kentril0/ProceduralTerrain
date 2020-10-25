#include "core/pch.hpp"
#include "camera.hpp"
#include <GLFW/glfw3.h>


const float Camera::DEFAULT_YAW       = -90.0f;
const float Camera::DEFAULT_PITCH     = 0.0f;
const float Camera::DEFAULT_SPEED     = 2.5f;
const float Camera::DEFAULT_ZOOM      = 45.0f;
const float Camera::MOUSE_SENSITIVITY = 0.1f;
const float Camera::ZOOM_SENSITIVITY  = 45.1f;
const float Camera::MOVE_SPEED        = 10.0f;

const float Camera::DEFAULT_FOV       = 45.0f;
const float Camera::DEFAULT_NEAR      = 0.01f;
const float Camera::DEFAULT_FAR       = 1000.0f;

const float Camera::MAX_PITCH         = 89.0f;
const float Camera::MIN_PITCH         = -89.0f;

// TODO
#define KEY_FORWARD     GLFW_KEY_W
#define KEY_BACKWARD    GLFW_KEY_S
#define KEY_LEFT        GLFW_KEY_A
#define KEY_RIGHT       GLFW_KEY_D


Camera::Camera(float aspect_ratio, const glm::vec3& pos, const glm::vec3& up,
               const glm::vec3& front,
               float yaw, float pitch)
  : position(pos), 
    front(front),
    up(up), 
    aspect_ratio(aspect_ratio),
    fov(glm::radians(DEFAULT_FOV)),
    near(DEFAULT_NEAR), far(DEFAULT_FAR),
    yaw(yaw), pitch(pitch),
    last_x(0), last_y(0),
    is_forward(false),
    is_backward(false),
    is_right(false),
    is_left(false)
{
    
}


void Camera::on_mouse_move(double x, double y)
{
    // Calculate the offset movement between the last and current frame
    float dx = float(x - last_x) * MOUSE_SENSITIVITY;
    float dy = float(last_y - y) * MOUSE_SENSITIVITY;

    last_x = static_cast<int>(x);
    last_y = static_cast<int>(y);

    pitch += dy;
    if (pitch > MAX_PITCH)
        pitch = MAX_PITCH;
    else if (pitch < MIN_PITCH)
        pitch = MIN_PITCH;

    yaw = glm::mod(yaw + dx, 360.0f);

    // calculate the new front vector
    front.x = cosf(glm::radians(yaw)) * cosf(glm::radians(pitch));
    front.y = sinf(glm::radians(pitch));
    front.z = sinf(glm::radians(yaw)) * cosf(glm::radians(pitch));

    front = glm::normalize(front);

    // re-calculate the right and up vector
    // normalize because their length gets closer to 0 the more you look up or down
    //  -> results in slower movement
    right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
    up    = glm::normalize(glm::cross(right, front));
}

void Camera::update(float dt)
{
    float velocity = MOVE_SPEED * dt;
    if (is_forward)
        position += front * velocity;
    if (is_backward)
        position -= front * velocity;
    if (is_right)
        position += right * velocity;
    if (is_left)
        position -= right * velocity;
}

void Camera::on_mouse_button(int button, int action, int mods)
{

}

void Camera::on_key_pressed(float dt, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        if (key == KEY_FORWARD)
            is_forward = true;
        else if (key == KEY_BACKWARD)
            is_backward = true;
        else if (key == KEY_RIGHT)
            is_right = true;
        else if (key == KEY_LEFT)
            is_left = true;
    }
    else if (action == GLFW_RELEASE)
    {
        if (key == KEY_FORWARD)
            is_forward = false;
        else if (key == KEY_BACKWARD)
            is_backward = false;
        else if (key == KEY_RIGHT)
            is_right = false;
        else if (key == KEY_LEFT)
            is_left = false;
    }
}

