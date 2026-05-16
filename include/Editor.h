#pragma once

#include <iostream>
#include <raylib.h>
#include <vector>
#include <string>
#include <ctype.h>
#include <algorithm>
#include <cstring>

#include <Cursor.h>
#include <ErrorView.h>
#include <CharView.h>
#include <Buffer.h>

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


struct Editor {
public:

    Editor(const Rectangle& content_rect, int cursor_w, int cursor_h, int line_height):
        m_mode(EditorMode::NORMAL),
        m_line_height(line_height),
        m_max_viewable_lines(content_rect.height / line_height),
        m_content_rect(content_rect),
        m_buffer_list{},
        m_buffer_index{0},
        m_cursor_w(cursor_w),
        m_cursor_h(cursor_h) {
        Buffer temp_buffer("", "", cursor_w, cursor_h);
        Cursor* temp_cursor = temp_buffer.get_cursor();
        temp_cursor->set_cursor_position({content_rect.x, content_rect.y});
        m_buffer_list.push_back(temp_buffer);
        m_buffer_index = m_buffer_list.size() - 1;
        m_initialized = false;
    }

    int get_line();
    int get_column();

    int get_max_viewable_lines();
    EditorMode get_editor_mode();
    void set_editor_mode(EditorMode mode);

    Cursor* get_cursor();
    void update(Rectangle content_rect);

    void report_error(const char* message);
    void report_info(const char* message);
    void report_warning(const char* message);

    void draw_errors();
    Buffer get_buffer();

    void open_file(const char* path);

    void draw_text();

    int get_start_line();
    void start_line(int iline);
private:
    void prepare_visible_chars(int start_line_number = 1);
    void syntax_highlight();
    void build_line_offsets();

    void insert_char(char c);
    void delete_char();

    Vector2 get_text_size(const char* c);

    void goto_line(int line);
    void goto_col(int col);
    void report(const char* message, ErrorType type);


    EditorMode m_mode;
    int m_line_height;
    int m_max_viewable_lines;
    std::vector<ErrorView*> m_error_messages;
    Rectangle m_content_rect;
    std::vector<CharView> m_chars;
    std::vector<int> m_line_offsets {};
    int m_render_start_index = 0;
    std::vector<Buffer> m_buffer_list;
    int m_buffer_index;
    bool m_initialized = false;
    int m_cursor_w;
    int m_cursor_h;
};