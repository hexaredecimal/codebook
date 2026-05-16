#pragma once

#include <raylib.h>
#include <CharView.h>

struct Cursor {
public:
    Cursor(int w, int h)
        : m_current_char{0, {}, BLACK, 1, 1, 0} {
        m_rect = {};
        m_rect.width = w;
        m_rect.height = h;
    }

    Rectangle get_cursor_rect() { return m_rect; }
    Vector2 get_cursor_position() { return { m_rect.x, m_rect.y }; }
    void set_cursor_position(Vector2 pos) {
        m_rect.x = pos.x;
        m_rect.y = pos.y;
    }

    void set_cursor_size(int w, int h) {
        m_rect.x = w;
        m_rect.y = h;
    }

    int get_line() { return m_line; }
    int get_column() { return m_col; }

    void set_line(int line) { m_line = line; }
    void set_column(int column) { m_col = column; }

    bool get_blink() { return m_blink;}
    bool set_blink(bool blink) { m_blink = blink; }


    void update(){
        if (!m_blink) return;
        m_timer -= 1.0f;
        if (m_timer < -35.0f) {
            m_timer = 35.0f;
        }
    }

    void set_current_char(CharView c) {
        m_current_char = c;
        set_line(c.get_line());
        set_column(c.get_col());
    }
    CharView get_current_char() { return m_current_char; }

    float get_timer() { return m_timer; }
private:
    Rectangle m_rect;
    int m_line = 1;
    int m_col = 1;
    bool m_blink = false;
    float m_timer = 35.0f;
    CharView m_current_char;
};
