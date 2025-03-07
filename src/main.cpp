#include <iostream>
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 800
#define PI 3.141592653589
#define MIN_DISTANCE 1.0f
const double G = 6.7943e-11; // m^3 kg^-1 s^-2

GLFWwindow* start_GLFW();

double last_frame_time = 0.0;

float camera_x = 0.0f;
float camera_y = 0.0f;

const float GRID_SPACING = 50.0f;
const float GRID_ALPHA = 0.3f;

float clamp(float value, float minVal, float maxVal) {
    return std::max(minVal, std::min(value, maxVal));
}


class Object {
    public:
        std::vector<float> pos;
        std::vector<float> vel;
        float mass;
        float density = 0.08375; // HYDROGEN
        //float radius = pow(((3 * this->mass / this->density) / (4 * PI)), (1.0f / 3.0f));
        float radius;
        int res = 100;
        
        Object(std::vector<float> pos, std::vector<float> vel, float mass, float radius = 15) {
            this->pos = pos;
            this->vel = vel;
            this->mass = mass;
            this->radius = radius;
        }

        void accelerate(float x, float y, float delta) {
            this->vel[0] += x * delta;
            this->vel[1] += y * delta; 
        }

        void update_pos(float delta) {
            this->pos[0] += this->vel[0] * delta;
            this->pos[1] += this->vel[1] * delta;
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


        void check_collision(Object& o) {
            if(this == &o) return;

            float dx = o.pos[0] - pos[0];
            float dy = o.pos[1] - pos[1];
            float dist = sqrt(dx*dx + dy*dy);

            // check if objects are colliding
            if(dist < (radius + o.radius)) {
                    // collision response
                    float overlap = (radius + o.radius) - dist + 100;
                    float nx = dx / dist; // normalized x direction
                    float ny = dy / dist; // normalized y direction

                    // move objects apart based on mass
                    float total = mass + o.mass;
                    float ratio1 = o.mass / total;
                    float ratio2 = mass / total;

                    pos[0] -= nx * overlap * ratio1;
                    pos[1] -= ny * overlap * ratio1;
                    o.pos[0] += nx * overlap * ratio2;
                    o.pos[1] += nx * overlap * ratio2;

                    // elastic collision response
                    float v1x = vel[0];
                    float v1y = vel[1];
                    float v2x = o.vel[0];
                    float v2y = o.vel[1];

                    // calculate new vel (elastic collision)
                    vel[0] = (v1x * (mass - o.mass) + 2 * o.mass * v2x) / total;
                    vel[1] = (v1y * (mass - o.mass) + 2 * o.mass * v2y) / total;
                    o.vel[0] = (v2x * (o.mass - mass) + 2 * mass * v1x) / total;
                    o.vel[1] = (v2y * (o.mass - mass) + 2 * mass * v1y) / total;

                    // add dampening
                    vel[0] *= 0.95f;
                    vel[1] *= 0.95f;
                    o.vel[0] *= 0.95f;
                    o.vel[1] *= 0.95f;
            }
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


    std::vector<Object> objs = {
        Object(std::vector<float>{0.0f, 0.0f}, std::vector<float>{0.0f, 0.0f}, 7.35 * pow(10, 22), 25),
        Object(std::vector<float>{0.0f, SCREEN_HEIGHT}, std::vector<float>{0.0f, 0.0f}, 7.35 * pow(10, 22), 25),
        Object(std::vector<float>{SCREEN_WIDTH, 0.0f}, std::vector<float>{0.0f, 0.0f}, 7.35 * pow(10, 22), 25),
        Object(std::vector<float>{SCREEN_WIDTH, SCREEN_HEIGHT}, std::vector<float>{0.0f, 0.0f}, 7.35 * pow(10, 22), 25),
        Object(std::vector<float>{SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2}, std::vector<float>{0.0f, 0.0f}, 7.35 * pow(10, 22), 25),
    };

    last_frame_time = glfwGetTime();

    while(!glfwWindowShouldClose(window)) {        
        glClear(GL_COLOR_BUFFER_BIT);

        float delta= calculate_delta();

        for(auto& obj : objs) {
            for(auto& obj2 : objs) {
                if(&obj2 == &obj) {continue;};
                float dx = obj2.pos[0] - obj.pos[0];
                float dy = obj2.pos[1] - obj.pos[1];
                float dist = sqrt(dx*dx + dy*dy);
                std::vector<float> dir = {dx / dist, dy / dist};
                dist *= 1000;


                if(dist < MIN_DISTANCE) dist = MIN_DISTANCE;
                
                float gforce = (G * obj.mass * obj2.mass) / (dist*dist);
                float acc1 = gforce / obj.mass;

                std::vector<float> acc = {acc1 * dir[0], acc1 * dir[1]};
                obj.accelerate(acc[0], acc[1], delta);
            }

            // obj.accelarate(0, -9.81);
            obj.update_pos(delta);

            obj.check_bound(0, SCREEN_HEIGHT, 0, SCREEN_WIDTH);
        }

        for(size_t i = 0; i < objs.size(); i++) {
            for(size_t j = i + 1; j < objs.size(); j++) {
                //objs[i].check_collision(objs[j]);
            }
        }

        for(auto& obj : objs) {
            obj.draw();
        }

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
