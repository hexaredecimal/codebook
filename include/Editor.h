#pragma once

#include <iostream>
#include <raylib.h>
#include <vector>
#include <string>
#include <ctype.h>
#include <algorithm>
#include <cstring>
#include <ErrorView.h>
#include <CharView.h>

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
        m_content_rect.x,
        m_content_rect.y - 15,
        w,
        h,
      };
      m_line_height = line_height;
    }

    Rectangle get_cursor_rect() { return m_rect; }
    Vector2 get_cursor_position() { return { m_rect.x, m_rect.y }; }
    void set_cursor_position(Vector2 pos) {
        m_rect.x = pos.x;
        m_rect.y = pos.y;
    }

    int get_line() { return m_line; }
    int get_column() { return m_col; }

    void set_line(int line) { m_line = line; }
    void set_column(int column) { m_col = column; }

    bool get_blink() { return m_blink;}
    bool set_blink(bool blink) { m_blink = blink; }


    void update(Rectangle content_rect){
        this->m_content_rect =content_rect;
        if (!m_blink) return;
        m_timer -= 1.0f;
        if (m_timer < -35.0f) {
            m_timer = 35.0f;
        }
    }

    float get_timer() { return m_timer; }
private:
    Rectangle m_rect;
    Rectangle m_content_rect;
    int m_line_height;
    int m_line = 1;
    int m_col = 1;
    bool m_blink = false;
    float m_timer = 35.0f;
};

struct Editor {
public:

    Editor(const Rectangle& content_rect, int cursor_w, int cursor_h, int line_height):
        m_cursor(content_rect, cursor_w, cursor_h, line_height),
        m_mode(EditorMode::NORMAL),
        m_line_height(line_height),
        m_max_viewable_lines(content_rect.height / line_height),
        m_file_path{},
        m_file_contents{},
        m_content_rect(content_rect){
    }

    int get_line() { return 0; /* todo */ }
    int get_column() { return 0; /* todo */ }
    void set_line() {}
    void set_column() {}

    int get_max_viewable_lines() { return m_max_viewable_lines; }
    EditorMode get_editor_mode() { return m_mode; }
    void set_editor_mode(EditorMode mode) { m_mode = mode; }

    Cursor* get_cursor() { return &m_cursor; }
    void update(Rectangle content_rect) {
      this->m_content_rect = content_rect;
      m_max_viewable_lines = content_rect.height / m_line_height;

      if (m_text_index < m_render_start_index)
          m_text_index = m_render_start_index;

      float dt = GetFrameTime();
      char c = m_file_contents[m_text_index];
      for (CharView cv: m_chars) {
          if (cv.get_source_index() == m_text_index) {
            Vector2 pos = cv.get_pos();
            m_cursor.set_cursor_position({pos.x, pos.y - 15});
            m_cursor.set_line(cv.get_line());
            m_cursor.set_column(cv.get_col());
            // std::cout << cv.get_char() << std::endl;
            break;
          }
      }

      // std::cout << "Looking at: " << c << std::endl;
      // std::cout << m_text_index << std::endl;
      m_cursor.update(content_rect);
      if (m_cursor.get_cursor_position().y > content_rect.height + m_line_height * 2)
          this->goto_line(m_cursor.get_line() - 1);

      float wheel = GetMouseWheelMove();
      if (wheel != 0.0f) {
          int delta = (wheel > 0) ? -1 : 1;
          start_line(m_start_line + delta);
      }

      if (IsKeyPressed(KEY_RIGHT)) {
        if (m_text_index < (int)m_file_contents.size())
            m_text_index++;
        int y_diff = m_cursor.get_line() - m_start_line;
        if (c == '\n' && y_diff + 1 == m_max_viewable_lines)
            this->start_line(m_start_line + 1);
      }

      if (IsKeyPressed(KEY_LEFT)) {
        // m_cursor.move_back(dummy_size);
        if (m_text_index > 0)
          m_text_index--;
        int y_diff = m_cursor.get_line() - m_start_line;
        if (y_diff + 1 == 1 && m_cursor.get_column() == 1)
            this->start_line(m_start_line - 1);
      }

      if (IsKeyPressed(KEY_UP)) {
          int y_diff = m_cursor.get_line() - m_start_line;
          if (y_diff + 1 == 1 )
              this->start_line(m_start_line - 1);

          int col = m_cursor.get_column();
          this->goto_line(m_cursor.get_line() - 1);
          this->goto_col(col);
      }

      if (IsKeyPressed(KEY_DOWN)) {
          int y_diff = m_cursor.get_line() - m_start_line;
          if (y_diff + 1 == m_max_viewable_lines)
              this->start_line(m_start_line + 1);

          int col = m_cursor.get_column();
          this->goto_line(m_cursor.get_line() + 1);
          this->goto_col(col);
      }

      std::vector<int> removals{};
      for (int i = 0; i < m_error_messages.size(); ++i) {
        ErrorView* error = m_error_messages[i];
        error->update(dt);
        if (error->get_alpha() == -5)
            removals.push_back(i);
      }

      for (int index: removals) {
        ErrorView* error = m_error_messages[index];
        m_error_messages.erase(m_error_messages.begin() + index);
        delete error;
      }
    }

    void report_error(const char* message) { this->report(message, ErrorType::ERROR); }
    void report_info(const char* message) { this->report(message, ErrorType::INFO); }
    void report_warning(const char* message) { this->report(message, ErrorType::WARNING); }


    void draw_errors() {
        for (ErrorView* error: m_error_messages) {
            error->draw();
        }
    }


    const char* get_buffer_name() {
        if (this->m_file_path.size() == 0)
            return "No file";
        return GetFileName(this->m_file_path.c_str());
    }

    void open_file(const char* path) {
        const char* source = LoadFileText(path);
        if (!source) {
            report_error((std::string("Failed to load: `") + path + "`").c_str());
            return;
        }
        m_file_path     = std::string(path);
        m_file_contents = std::string(source);
        m_text_index         = 0;
        m_render_start_index = 0;
        m_start_line         = 1;
        build_line_offsets();
        prepare_visible_chars(1);
        syntax_highlight();
    }

    void draw_text() {
        if (m_file_contents.size() == 0)
            return;

        for (CharView char_view: m_chars) {
            Vector2 pos = char_view.get_pos();
            Color color = char_view.get_color();
            DrawTextEx(GetFontDefault(),TextFormat("%c", char_view.get_char()), pos, 11, 1, color);
        }
    }

    void prepare_visible_chars(int start_line_number = 1) {
        if (m_file_contents.empty()) return;

        m_chars.clear();

        float x = m_content_rect.x + 1;
        float y = m_content_rect.y + 10;
        float right_edge = m_content_rect.x + m_content_rect.width;
        float bottom_edge = m_content_rect.y + m_content_rect.height;

        int line = start_line_number;
        int col  = 1;

        int start = m_render_start_index;

        while (start < (int)m_file_contents.size()) {
            if (y >= bottom_edge + m_line_height * 2) break;

            char c = m_file_contents[start];
            int source_index = start;
            start++;

            if (c == '\n') {
                CharView v(c, {x, y}, BLACK, line, col, source_index);
                m_chars.push_back(v);
                y += m_line_height;
                x  = m_content_rect.x + 1;
                line++;
                col = 1;
                continue;
            }

            if (c < 32) continue;

            Vector2 text_size = MeasureTextEx(GetFontDefault(), TextFormat("%c", c), 11, 1);

            if (x + text_size.x > right_edge) {
                y += m_line_height;
                x  = m_content_rect.x + 1;
                col = 1;
            }

            if (y >= m_content_rect.y) {
                CharView v(c, {x, y}, BLACK, line, col, source_index);
                m_chars.push_back(v);
            }

            x += text_size.x + 1.1f;
            col++;
        }
    }

    void syntax_highlight() {
        std::vector<const char*> keywords {
          "include", "if", "else", "define", "switch", "return", "for", "while", "do",
          "auto", "class", "struct", "namespace", "using", "typedef",
          "const", "constexpr", "enum", "union", "public", "private",
          "protected", "pragma", "this", "inline", "break", "continue",
          "goto"
        };

        std::vector<const char*> built_ins {
          "int", "float", "double", "short", "long", "void", "bool",
          "int32_t", "uint32_t", "int8_t", "uint8_t", "int16_t", "uint16_t",
          "int16_t", "uint64_t", "char", "true", "false"
        };

        for (int index = 0; index < (int)m_chars.size(); index++) {
            CharView* cv = &m_chars[index];
            char c = cv->get_char();

            if (c == '/' && index + 1 < m_chars.size() && m_chars[index + 1].get_char() == '*') {
                Color color = ColorBrightness(GREEN, -0.5);
                for (int i = index; i < (int)m_chars.size(); i++) {
                    c = m_chars[i].get_char();
                    if (c == '*' && i + 1 < m_chars.size() && m_chars[i + 1].get_char() == '/') {
                        m_chars[i].set_color(color);
                        m_chars[i + 1].set_color(color);
                        index = i + 1;
                        break;
                    }
                    m_chars[i].set_color(color);
                }
            } else if (c == '/' && index + 1 < m_chars.size() && m_chars[index + 1].get_char() == '/') {
                Color color = ColorBrightness(GREEN, -0.5);
                int line = cv->get_line();
                for (int i = index; i < (int)m_chars.size(); i++) {
                    if (m_chars[i].get_line() > line) {
                        index = i;
                        break;
                    }
                    m_chars[i].set_color(color);
                }
            } else if (c == '<') {
                for (int i = index + 1; i < (int)m_chars.size(); i++) {
                    char c = m_chars[i].get_char();
                    if (c == '>' || c == '=' || c == ' ') { index = i; break; }
                    m_chars[i].set_color(BROWN);
                }

            } else if (c == '"' || c == '\'') {
                Color col = ColorBrightness(ORANGE, 0.0f);
                cv->set_color(col);
                char start_char = c;
                char escape_symbol = '\\';
                for (int i = index + 1; i < (int)m_chars.size(); i++) {
                    char t = m_chars[i].get_char();
                    if (t == escape_symbol && i + 1 < m_chars.size()) {
                        if (m_chars[i + 1].get_char() == start_char) {
                            m_chars[i].set_color(col);
                            m_chars[i + 1].set_color(col);
                            i += 1;
                            continue;
                        }
                    }

                    if (t == escape_symbol && i + 2 < m_chars.size() && m_chars[i + 1].get_char() == escape_symbol) {
                        if (m_chars[i + 2].get_char() == start_char) {
                            std::cout <<  "Found: " << t << " and " << m_chars[i + 1].get_char() << std::endl;
                            m_chars[i].set_color(col);
                            m_chars[i + 1].set_color(col);
                            m_chars[i + 2].set_color(col);
                            i += 1;
                            continue;
                        }
                    }

                    if (t == start_char) {
                        m_chars[i].set_color(col);
                        index = i;
                        break;
                    }
                    m_chars[i].set_color(col);
                }

            } else if (isdigit(c)) {
                cv->set_color(BROWN);

            } else if (isalpha(c)) {
                int word_end = index;
                while (word_end + 1 < (int)m_chars.size() &&
                       (isalnum(m_chars[word_end + 1].get_char()) ||
                        m_chars[word_end + 1].get_char() == '_')) {
                    word_end++;
                }

                std::string word = "";
                for (int i = index; i <= word_end; i++)
                    word += m_chars[i].get_char();

                Color col = {0,0,0,0};
                for (const char* kw : keywords)
                    if (word == kw) { col = BLUE; break; }
                if (col.a == 0)
                    for (const char* t : built_ins)
                        if (word == t) { col = PURPLE; break; }

                if (col.a != 0)
                    for (int i = index; i <= word_end; i++)
                        m_chars[i].set_color(col);

                index = word_end;
            }
        }
    }

    void build_line_offsets() {
        m_line_offsets.clear();
        m_line_offsets.push_back(0);
        for (int i = 0; i < (int)m_file_contents.size(); i++) {
            char c = m_file_contents[i];
            if (c == '\n')
                m_line_offsets.push_back(i + 1);
        }
    }

    int get_start_line() { return m_start_line; }
    void start_line(int iline) {
        if (m_line_offsets.empty()) return;

        // Clamp to valid line range
        iline = std::max(1, std::min(iline, (int)m_line_offsets.size()));
        m_start_line = iline;

        // Drive rendering from the line's file offset — NOT the cursor index
        m_render_start_index = m_line_offsets[iline - 1];

        prepare_visible_chars(iline);
        syntax_highlight();
    }
private:

    void goto_line(int line) {
        for (CharView cv: m_chars) {
            if (cv.get_line() == line) {
              Vector2 pos = cv.get_pos();
              m_cursor.set_cursor_position({pos.x, pos.y - 15});
              m_cursor.set_line(cv.get_line());
              m_cursor.set_column(cv.get_col());
              m_text_index = cv.get_source_index();
              break;
            }
        }
    }


    void goto_col(int col) {
        for (CharView cv: m_chars) {
            if (cv.get_line() == m_cursor.get_line() && cv.get_col() == col) {
              Vector2 pos = cv.get_pos();
              m_cursor.set_cursor_position({pos.x, pos.y - 15});
              m_cursor.set_line(cv.get_line());
              m_cursor.set_column(cv.get_col());
              m_text_index = cv.get_source_index();
              break;
            }
        }
    }

    void report(const char* message, ErrorType type){
        ErrorView* e = new ErrorView(message, type);
        if (m_error_messages.size() == 0)
            m_error_messages.push_back(e);
        else
          m_error_messages.insert(m_error_messages.begin(), e);

        Vector2 pos = e->get_position();
        Vector2 text_size = e->get_text_size();
        pos.y += text_size.y + 5;

        for (int i = 1; i < m_error_messages.size(); ++i) {
          ErrorView* error = m_error_messages[i];
          error->set_position(pos);
          text_size = error->get_text_size();
          pos.y += text_size.y + 5;
          if (i + 1 < m_error_messages.size()) {
            ErrorView* next = m_error_messages[i+1];
            text_size = next->get_text_size();
            if (pos.y + text_size.y > GetScreenHeight())
                break;
          }
        }
    }


    Cursor m_cursor;
    EditorMode m_mode;
    int m_line_height;
    int m_max_viewable_lines;
    std::vector<ErrorView*> m_error_messages;
    std::string m_file_path;
    std::string m_file_contents;
    Rectangle m_content_rect;
    int m_text_index = 0;
    std::vector<CharView> m_chars;
    std::vector<int> m_line_offsets {};
    int m_start_line = 1;
    int m_render_start_index = 0;
};