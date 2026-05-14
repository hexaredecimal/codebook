#pragma once
#include <raylib.h>

class CharView {
public:
    CharView(char c, Vector2 pos, Color color, int line, int col, int source_index):
        m_char(c),
        m_pos(pos),
        m_color(color),
        m_line(line),
        m_col(col),
        m_source_index(source_index){}

    char get_char() { return m_char; }
    Vector2 get_pos() { return m_pos; }
    Color get_color() { return m_color; }
    int get_line() { return m_line; }
    int get_col() { return m_col; }
    int get_source_index() { return m_source_index; }

    void set_color(Color color) { m_color = color; }

private:
    char m_char;
    Vector2 m_pos;
    Color m_color;
    int m_line;
    int m_col;
    int m_source_index = 0;
};
