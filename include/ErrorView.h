#pragma once
#include <raylib.h>

enum class ErrorType {
    INFO,
    WARNING,
    ERROR
};

class ErrorView {
public:
    ErrorView(const char* message, ErrorType type):
      m_timer(1.f),
      m_pos({10, GetScreenHeight() * 70.0f / 100.0f}),
      m_message(message),
      m_alpha(1.0f),
      m_text_size(MeasureTextEx(GetFontDefault(), m_message, 14, 1)),
      m_type(type){}

    Vector2 get_position() { return m_pos; }
    void set_position(Vector2 pos) { m_pos = pos; }
    int get_alpha() { return m_alpha; }

    float get_timer() { return m_timer; }
    Vector2 get_text_size() { return m_text_size; }

    void update(float dt) {
        this->m_timer -= dt;
        if (this->m_timer < 0) {
            this->m_pos.y += m_text_size.y * dt;
            this->m_alpha -= 0.01;
        }

        if (this->m_alpha <= 0.0f) {
            this->m_pos = {0,0};
        }
    }

    void draw() {
        if (this->m_alpha <= 0.0f)
            return;

        auto color = WHITE;
        if (m_type == ErrorType::ERROR)
            color = RED;
        if (m_type == ErrorType::WARNING)
            color = ORANGE;
        if (m_type == ErrorType::INFO)
            color = GREEN;

        DrawText(TextFormat("[Error]: %s", this->m_message), this->m_pos.x, this->m_pos.y, 14, Fade(color, this->m_alpha));
    }


private:
    Vector2 m_pos;
    float m_timer;
    const char* m_message;
    float m_alpha;
    Vector2 m_text_size;
    ErrorType m_type;
};
