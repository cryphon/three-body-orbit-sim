#include <iostream>
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 800
#define PI 3.141592653589
#define MIN_DISTANCE 10000.0f

const double G = 6.6743e-11; // m^3 kg^-1 s^-2

GLFWwindow* start_GLFW();

double last_frame_time = 0.0;

float camera_x = 0.0f;
float camera_y = 0.0f;

const float GRID_SPACING = 150.0f;
const float GRID_ALPHA = 0.3f;

float clamp(float value, float minVal, float maxVal) {
    return std::max(minVal, std::min(value, maxVal));
}


class Object {
    public:
        std::vector<float> pos;
        std::vector<float> vel;
        float mass;
        float radius;
        int res = 100;
        std::vector<std::vector<float>> trail;
        int trail_len = 500;
        float trail_color[3] = {1.0f, 1.0f, 1.0f};

        Object(std::vector<float> pos, std::vector<float> vel, float mass, float radius = 0, float r = 1.0f, float g = 1.0f, float b = 1.0f)
            : pos(pos), vel(vel), mass(mass) {
                this->pos = pos;
                this->vel = vel;
                this->mass = mass;

                this->trail_color[0] = r;
                this->trail_color[1] = g;
                this->trail_color[2] = b;

                if(radius <= 0) {
                    update_radius();
                } else {
                    this->radius = radius;
                }
            }

        void accelerate(float x, float y, float delta) {
            this->vel[0] += x * delta;
            this->vel[1] += y * delta; 
        }

        void draw_trail() {

            if(trail.empty()) return;

            // Save current color
            float currentColor[4];
            glGetFloatv(GL_CURRENT_COLOR, currentColor);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_LINE_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
            glLineWidth(2.0f);

            glBegin(GL_LINE_STRIP);
            for(size_t i = 0; i < trail.size(); i++) {
                // Calculate alpha based on position in trail
                float alpha = static_cast<float>(i) / trail.size();

                glColor4f(trail_color[0], trail_color[1], trail_color[2], alpha);


                glVertex2f(trail[i][0], trail[i][1]);
            }
            glEnd();

            // Restore original color
            glColor4f(currentColor[0], currentColor[1], currentColor[2], 1.0f);
            glDisable(GL_BLEND);
        }        

        void update_pos(float delta) {
            // Update position
            this->pos[0] += this->vel[0] * delta;
            this->pos[1] += this->vel[1] * delta;

            // Only add to the trail if the object has moved a significant distance
            if (trail.empty() || 
                    sqrt(pow(trail.back()[0] - this->pos[0], 2) + pow(trail.back()[1] - this->pos[1], 2)) > 5.0f) {
                trail.push_back({this->pos[0], this->pos[1]});
            }

            // Limit trail size
            if (trail.size() > trail_len) {
                trail.erase(trail.begin());
            }            
        }

        void update_radius() {
            const float BASE_RADIUS = 15.0f;
            const float MIN_RADIUS = 10.0f;
            const float MAX_RADIUS = 50.0f;

            float log_mass = log10(this->mass);

            this->radius = BASE_RADIUS + (log_mass - 20.0f)* 3.0f;
            this->radius = clamp(this->radius, MIN_RADIUS, MAX_RADIUS);
        }

        void check_bound(int bot, int top, int left, int right) {
            if(this->pos[1] < bot + radius || this->pos[1] > top - radius) {
                this->pos[1] = this->pos[1] < bot + radius ? bot + radius : top - radius;
                this->vel[1] *= -0.9f;
            }
            if(this->pos[0] < left + radius || this->pos[0] > right - radius) {
                this->pos[0] = this->pos[0] < left + radius ? left + radius : right - radius;
                this->vel[0] *= -0.9f;
            }
        }

        void draw() {
            glColor3f(0.949f, 0.949f, 0.949f);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2d(pos[0], pos[1]);
            for(int i = 0; i <= res; i++) {
                float angle = 2.0f * PI * (static_cast<float>(i) / res);
                float x = pos[0] + cos(angle) * this->radius;
                float y = pos[1] + sin(angle) * this->radius;
                glVertex2d(x, y);
            }
            glEnd();
        }

        };


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
                mass, 0, 0.3f, 0.3f, 1.0f)
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
                float dx = obj2.pos[0] - obj.pos[0];
                float dy = obj2.pos[1] - obj.pos[1];
                float dist = sqrt(dx*dx + dy*dy);
                std::vector<float> dir = {dx / dist, dy / dist};


                if(dist < MIN_DISTANCE) dist = MIN_DISTANCE;

                float softening = 1e4;  // Prevents infinite force at small distances
                float gforce = (G * obj.mass * obj2.mass) / (dist*dist + softening);
                float acc1 = gforce / obj.mass;

                std::vector<float> acc = {acc1 * dir[0], acc1 * dir[1]};
                obj.accelerate(acc[0], acc[1], delta);
            }

            obj.update_pos(delta);

            obj.check_bound(0, SCREEN_HEIGHT, 0, SCREEN_WIDTH);
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for(auto& obj : objs) {
            // Draw trail first
            obj.draw_trail();

            // Set color based on mass
            float mass_ratio = obj.mass / (7.35 * pow(10, 22));
            if(mass_ratio > 1.0f) mass_ratio = 1.0f;
            glColor3f(0.2f + 0.8f * mass_ratio, 0.2f + 0.8f * (1.0f - mass_ratio), 0.2f);
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
