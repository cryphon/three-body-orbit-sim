#include "object.h"
#include "defs.h"
#include "util.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>

Object::Object(std::vector<float> pos, std::vector<float> vel, float mass, float radius, float r, float g, float b)
    : pos(pos), vel(vel), mass(mass) {
        this->trail_color[0] = r;
        this->trail_color[1] = g;
        this->trail_color[2] = b;

        if(radius <= 0) {
            update_radius();
        }
        else {
            this->radius = radius;
        }
}

void Object::accelerate(float x, float y, float delta) {
    this->vel[0] += x * delta;
    this->vel[1] += y * delta;
}

void Object::draw() {
    glColor3f(trail_color[0], trail_color[1], trail_color[2]);
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

void Object::draw_trail() {
    if(trail.empty()) return;

    float currentColor[4];
    glGetFloatv(GL_CURRENT_COLOR, currentColor);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth(2.0f);

    glBegin(GL_LINE_STRIP);
    for(size_t i = 0; i < trail.size(); i++) {
        float alpha = static_cast<float>(i) / trail.size();
        glColor4f(trail_color[0], trail_color[1], trail_color[2], alpha);
        glVertex2f(trail[i][0], trail[i][1]);
    }
    glEnd();

    // Restore original color
    glColor4f(currentColor[0], currentColor[1], currentColor[2], 1.0f);
    glDisable(GL_BLEND);
}        

void Object::update_pos(float delta) {
    // Update position
    this->pos[0] += this->vel[0] * delta;
    this->pos[1] += this->vel[1] * delta;

    // Only add to the trail if the object has moved a significant distance
    if (trail.empty() || 
            sqrt(pow(trail.back()[0] - this->pos[0], 2) + pow(trail.back()[1] - this->pos[1], 2)) > 5.0f) {
        trail.push_back({this->pos[0], this->pos[1]});
    }

    if (trail.size() > trail_len) {
        trail.erase(trail.begin());
    }            
}

void Object::update_radius() {
    const float BASE_RADIUS = 15.0f;
    const float MIN_RADIUS = 10.0f;
    const float MAX_RADIUS = 50.0f;
    float log_mass = log10(this->mass);

    this->radius = BASE_RADIUS + (log_mass - 20.0f)* 3.0f;
    this->radius = clamp(this->radius, MIN_RADIUS, MAX_RADIUS);
}

void Object::check_bound(int bot, int top, int left, int right) {
    if(this->pos[1] < bot + radius || this->pos[1] > top - radius) {
        this->pos[1] = this->pos[1] < bot + radius ? bot + radius : top - radius;
        this->vel[1] *= -0.9f;
    }
    if(this->pos[0] < left + radius || this->pos[0] > right - radius) {
        this->pos[0] = this->pos[0] < left + radius ? left + radius : right - radius;
        this->vel[0] *= -0.9f;
    }
}
