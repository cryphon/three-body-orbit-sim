#include <iostream>
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <random>

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 800
#define PI 3.141592653589
#define MIN_DISTANCE 1.0f
const double G = 6.7943e-11; // m^3 kg^-1 s^-2

GLFWwindow* start_GLFW();

double last_frame_time = 0.0;

float camera_x = 0.0f;
float camera_y = 0.0f;

float clamp(float value, float minVal, float maxVal) {
    return std::max(minVal, std::min(value, maxVal));
}


class Object {
    public:
        std::vector<float> pos;
        std::vector<float> vel;
        float mass;
        float density = 0.08375; // HYDROGEN
        float radius;
        int res = 1000;
        float restitution = 0.7f;
        float max_vel = 550.0f;
        
        Object(std::vector<float> pos, std::vector<float> vel, float mass, float density, float initial_radius = 0) {
            this->pos = pos;
            this->vel = vel;
            this->mass = mass;

             // Calculate the radius based on mass and density if not specified
            if (initial_radius <= 0) {
                update_radius();
            } else {
                this->radius = initial_radius;
            }        
        }

        void clamp_vel() {
            // Calculate current velocity magnitude
            float vel_mag = sqrt(vel[0] * vel[0] + vel[1] * vel[1]);
            
            // If magnitude exceeds max_velocity, scale it down
            if (vel_mag > max_vel) {
                float scale = max_vel / vel_mag;
                vel[0] *= scale;
                vel[1] *= scale;
            }
        }

        void accelerate(float x, float y, float delta) {
            this->vel[0] += x * delta;
            this->vel[1] += y * delta; 

            //clamp_vel();
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


         void update_radius() {
            // Use a simpler scaling approach that ensures visibility
            const float BASE_RADIUS = 15.0f;
            const float MIN_RADIUS = 10.0f;
            const float MAX_RADIUS = 50.0f;
            
            // Use log scale for mass to handle large differences
            float log_mass = log10(this->mass);
            
            // Map to reasonable screen sizes
            this->radius = BASE_RADIUS + (log_mass - 20.0f) * 3.0f;
            
            // Clamp to reasonable limits
            this->radius = clamp(this->radius, MIN_RADIUS, MAX_RADIUS);
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
                    float overlap = (radius + o.radius) - dist;
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
        }};


float calculate_delta() {
    double curr_time = glfwGetTime();
    float delta = curr_time - last_frame_time;
    last_frame_time = curr_time;

    // cap delta to prevent jumping when window is moved
    if(delta > 0.1f) delta = 0.1f;

    return delta;
}


void gravity(std::vector<Object> objs, float delta) {
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
                obj.accelerate(acc[0], acc[1], delta * 1000);
            }

            // obj.accelarate(0, -9.81);
            obj.update_pos(delta);

            obj.check_bound(0, SCREEN_HEIGHT, 0, SCREEN_WIDTH);
        }


}



int main(void) {
    GLFWwindow* window = start_GLFW();
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);


    float center_x = SCREEN_WIDTH / 2.0f;
    float center_y = SCREEN_HEIGHT / 2.0f;




    std::vector<Object> objs = {};
    last_frame_time = glfwGetTime();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> pos_dist_x(100.0f, SCREEN_WIDTH - 100.0f);
    std::uniform_real_distribution<float> pos_dist_y(100.0f, SCREEN_HEIGHT - 100.0f);
    std::uniform_real_distribution<float> vel_dist(-20.0f, 20.0f);
    std::uniform_real_distribution<float> mass_dist(1e21, 1e23);
    std::uniform_real_distribution<float> density_dist(500.0f, 5000.0f);

    // Generate N objects
    int num_objects = 100; // Change this value for more objects
    for (int i = 0; i < num_objects; i++) {
        float x = pos_dist_x(gen);
        float y = pos_dist_y(gen);
        float vx = vel_dist(gen);
        float vy = vel_dist(gen);
        float mass = mass_dist(gen);
        float density = density_dist(gen);

        objs.push_back(Object({x, y}, {vx, vy}, mass, density, 10));
    }

    while(!glfwWindowShouldClose(window)) {        
        glClear(GL_COLOR_BUFFER_BIT);

        float delta= calculate_delta();

                for(auto& obj : objs) {

            obj.accelerate(0, -9.81, delta);
            obj.update_pos(delta);

            obj.check_bound(0, SCREEN_HEIGHT, 0, SCREEN_WIDTH);
        }

        for(size_t i = 0; i < objs.size(); i++) {
            for(size_t j = i + 1; j < objs.size(); j++) {
                objs[i].check_collision(objs[j]);
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
