#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class Camera{
    public:
        glm::vec3 position;
        glm::vec3 forward;
        glm::vec3 up;
        
        bool firstMouse = true;
        float yaw   = -90.0f;
        float pitch =  0.0f;
        float lastX =  1600.0f / 2.0;
        float lastY =  1200.0 / 2.0;
        float fov;

        float nearClip = 0.1f;
        float farClip = 1000.0f;

        Camera(glm::vec3 position, glm::vec3 forward, glm::vec3 up, float fov){
            this->position = position;
            this->forward = forward;
            this->up = up;
            this->fov = fov;
        }

        glm::mat4 GetPerspectiveMatrix(int screenWidth, int screenHeight){
            return glm::perspective(glm::radians(fov), (float)screenWidth / (float)screenHeight, nearClip, farClip);
        }

        glm::mat4 GetViewMatrix(){
            return glm::lookAt(position, position + forward, up);
        }

        void ProcessInput(GLFWwindow* window, float deltaTime){
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                glfwSetWindowShouldClose(window, true);

            float cameraSpeed;
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
                cameraSpeed = static_cast<float>(50 * deltaTime);
            }
            else {
                cameraSpeed = static_cast<float>(10 * deltaTime);
            }
            
            // WASD
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                position += cameraSpeed * forward;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                position -= cameraSpeed * forward;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                position -= glm::normalize(glm::cross(forward, up)) * cameraSpeed;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                position += glm::normalize(glm::cross(forward, up)) * cameraSpeed;

            // Up and Down
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
                position += glm::vec3(0,1,0) * cameraSpeed;
            if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
                position += glm::vec3(0,-1,0) * cameraSpeed;
        }

        void ProcessMouseCallback(GLFWwindow* window, double xposIn, double yposIn){
            float xpos = static_cast<float>(xposIn);
            float ypos = static_cast<float>(yposIn);

            if (firstMouse)
            {
                lastX = xpos;
                lastY = ypos;
                firstMouse = false;
            }

            float xoffset = xpos - lastX;
            float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
            lastX = xpos;
            lastY = ypos;

            float sensitivity = 0.1f; // change this value to your liking
            xoffset *= sensitivity;
            yoffset *= sensitivity;

            yaw += xoffset;
            pitch += yoffset;

            // make sure that when pitch is out of bounds, screen doesn't get flipped
            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;

            glm::vec3 front;
            front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            front.y = sin(glm::radians(pitch));
            front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            forward = glm::normalize(front);
        }

        void ProcessScrollCallback(GLFWwindow* window, double xoffset, double yoffset){
            fov -= (float)yoffset;
            if (fov < 1.0f)
                fov = 1.0f;
            if (fov > 45.0f)
                fov = 45.0f;
        }
};

#endif