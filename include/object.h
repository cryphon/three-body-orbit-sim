#ifndef OBJECT_H
#define OBJECT_H
#include <vector>

class Object {
    private:
        std::vector<float> pos;
        std::vector<float> vel;
        float mass;
        float radius;
        int res;

        std::vector<std::vector<float>> trail;
        int trail_len = 500;
        float trail_color[3] = {1.0f, 1.0f, 1.0f};

    public:
        Object(std::vector<float> pos, std::vector<float> vel, float mass, float radius = 0, float r = 1.0f, float g = 1.0f, float b = 1.0f);

        void accelerate(float x, float y, float delta);
        void draw();
        void draw_trail();    
        void update_pos(float delta);
        void update_radius(); 
        void check_bound(int bot, int top, int left, int right);

        std::vector<float> get_pos() { return this->pos; };
        std::vector<float> get_vel() { return this->vel; };
        float get_mass() {return this->mass; };

    };

#endif // OBJECT_H
