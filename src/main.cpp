#include <iostream>
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include "defs.h"
#include "object.h"

const double G = 6.6743e-11; // m^3 kg^-1 s^-2

GLFWwindow* start_GLFW();

double last_frame_time = 0.0;

float calculate_delta() {
    double curr_time = glfwGetTime();
    float delta = curr_time - last_frame_time;
    last_frame_time = curr_time;

    // cap delta to prevent jumping when window is moved
    if(delta > 0.1f) delta = 0.1f;

    return delta;
}


int main(void) {
    GLFWwindow* window = start_GLFW();
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);


    float center_x = SCREEN_WIDTH / 2.0f;
    float center_y = SCREEN_HEIGHT / 2.0f;

    float mass = 1.0e20;
    float scale = 300.0f;

    std::vector<Object> objs = {
        Object(
                std::vector<float>{center_x - 0.97000436f * scale, center_y + 0.24308753f * scale}, 
                std::vector<float>{0.4662036850f * scale * 0.3f, 0.4323657300f * scale * 0.3f}, 
                mass, 0, 1.0f, 0.3f, 0.3f),
        Object(
                std::vector<float>{center_x, center_y}, 
                std::vector<float>{-0.93240737f * scale * 0.3f, -0.86473146f * scale * 0.3f}, 
                mass, 0, 0.3f, 1.0f, 0.3f),
        Object(
                std::vector<float>{center_x + 0.97000436f * scale, center_y - 0.24308753f * scale}, 
                std::vector<float>{0.4662036850f * scale * 0.3f, 0.4323657300f * scale * 0.3f}, 
                mass, 0, 0.3f, 0.3f, 1.0f),
    };
    last_frame_time = glfwGetTime();
    bool first_frame = true;
    float cam_smoothing_factor = 3.0f;

    while(!glfwWindowShouldClose(window)) {        
        glClear(GL_COLOR_BUFFER_BIT);

        float delta = calculate_delta();

        for(auto& obj : objs) {
            for(auto& obj2 : objs) {
                if(&obj2 == &obj) {continue;};
                float dx = obj2.get_pos()[0] - obj.get_pos()[0];
                float dy = obj2.get_pos()[1] - obj.get_pos()[1];
                float dist = sqrt(dx*dx + dy*dy);
                std::vector<float> dir = {dx / dist, dy / dist};


                if(dist < MIN_DISTANCE) dist = MIN_DISTANCE;

                float softening = 1e4;  // Prevents infinite force at small distances
                float gforce = (G * obj.get_mass() * obj2.get_mass()) / (dist*dist + softening);
                float acc1 = gforce / obj.get_mass();

                std::vector<float> acc = {acc1 * dir[0], acc1 * dir[1]};
                obj.accelerate(acc[0], acc[1], delta);
            }

            obj.update_pos(delta);

            obj.check_bound(0, SCREEN_HEIGHT, 0, SCREEN_WIDTH);
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for(auto& obj : objs) {
            obj.draw_trail();
            obj.draw();
        }

        // Disable blending when done
        glDisable(GL_BLEND);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

GLFWwindow* start_GLFW() {
    if(!glfwInit()) {
        std::cerr << "Failed to initialize glfw; panic!" << std::endl;
        return nullptr;
    }

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "gravity_sim", NULL, NULL);
    if(!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    return window;
}
