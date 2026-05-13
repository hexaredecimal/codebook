#pragma once

#include <iostream>
#include <raylib.h>

enum class EditorMode {
    NORMAL,
    INSERT,
    VISUAL,
};

inline const char* editor_mode_str(const EditorMode mode) {
    if (mode == EditorMode::NORMAL)
        return "NORMAL";
    if (mode == EditorMode::INSERT)
        return "INSERT";
    if (mode == EditorMode::VISUAL)
        return "VISUAL";
    return "NORMAL";
}

struct Cursor {
public:
    Cursor(const Rectangle& content_rect, int w, int h, int line_height){
      m_content_rect = content_rect;
      m_rect = {
        m_content_rect.x + 2,
        m_content_rect.y - 15,
        w,
        h,
      };
      m_line_height = line_height;
    }

    Rectangle get_cursor_rect() { return m_rect; }
    Vector2 get_cursor_position() { return { m_rect.x, m_rect.y }; }

    void move_forward(int size) {
        m_rect.x += size + 2;
        if (m_rect.x > m_content_rect.width)
            move_down();
    }

    void move_back(int size) {
        if (m_rect.x - size < m_content_rect.x)
            return;

        m_rect.x -= size - 2;
    }

    void move_down() {
        if (m_rect.y + m_line_height > m_content_rect.height + 2)
            return;
        m_rect.y += m_line_height;
        m_rect.x = m_content_rect.x + 2;
    }

private:
    Rectangle m_rect;
    Rectangle m_content_rect;
    int m_line_height;
};

struct Editor {
public:

    Editor(const Rectangle& content_rect, int cursor_w, int cursor_h, int line_height):
        m_cursor(content_rect, cursor_w, cursor_h, line_height),
        m_mode(EditorMode::NORMAL),
        m_line_height(line_height),
        m_max_viewable_lines(content_rect.height / line_height){
    }

    int get_line() { return 0; /* todo */ }
    int get_column() { return 0; /* todo */ }
    void set_line() {}
    void set_column() {}

    int get_max_viewable_lines() { return m_max_viewable_lines; }
    EditorMode get_editor_mode() { return m_mode; }
    void set_editor_mode(EditorMode mode) { m_mode = mode; }

    Cursor get_cursor() { return m_cursor; }
private:
    Cursor m_cursor;
    EditorMode m_mode;
    int m_line_height;
    int m_max_viewable_lines;
};