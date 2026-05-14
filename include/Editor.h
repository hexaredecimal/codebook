#pragma once

#include <iostream>
#include <raylib.h>
#include <vector>
#include <string>
#include <ctype.h>
#include <algorithm>
#include <cstring>
#include <ErrorView.h>
#include <CharView.h>.h>

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
        m_content_rect.x + 3,
        m_content_rect.y - 15,
        w,
        h,
      };
      m_line_height = line_height;
    }

    Rectangle get_cursor_rect() { return m_rect; }
    Vector2 get_cursor_position() { return { m_rect.x, m_rect.y }; }

    int get_line() { return m_line; }
    int get_column() { return m_col; }

    bool get_blink() { return m_blink;}
    bool set_blink(bool blink) { m_blink = blink; }

    void move_forward(int size) {

        if (m_rect.x + size> m_content_rect.width)
          if (m_rect.y + m_line_height > m_content_rect.height + 2)
              return;


        m_rect.x += size + m_rect.width;
        m_col++;
        if (m_rect.x > m_content_rect.width) {
            move_down();
        }
    }

    void move_back(int size) {
        m_rect.x -= size + m_rect.width;
        if (m_col > 1)
          m_col--;
        else
            m_rect.x = m_content_rect.x + 3;
    }

    bool move_down() {
        if (m_rect.y + m_line_height > m_content_rect.height + m_line_height)
            return false;
        m_rect.y += m_line_height;
        m_rect.x = m_content_rect.x + 3;
        m_line++;
        m_col = 1;
        return true;
    }


    bool move_up() {
        if (m_rect.y - m_line_height < m_content_rect.y - m_line_height)
            return false;;
        m_rect.y -= m_line_height;
        m_rect.x = m_content_rect.x + 3;
        m_line--;
        return true;
    }

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

      float dt = GetFrameTime();
      char c = m_file_contents[m_text_index];
      // std::cout << "Looking at: " << c << std::endl;
      int dummy_size = MeasureText(TextFormat("%c", c), 11);
      m_cursor.update(content_rect);

      if (IsKeyPressed(KEY_RIGHT)) {
        if (c == '\n')
          m_cursor.move_down();
        else
          m_cursor.move_forward(dummy_size);
        m_text_index++;
      }

      if (IsKeyPressed(KEY_LEFT)) {
        m_cursor.move_back(dummy_size);
        if (m_text_index > 0)
          m_text_index--;
      }

      if (IsKeyPressed(KEY_UP)) {
        m_cursor.move_up();
      }

      if (IsKeyPressed(KEY_DOWN)) {
        bool can_move = m_cursor.move_down();
        std::cout << "Before: " << this->get_start_line() << std::endl;
        if (!can_move) {
          this->start_line(this->get_start_line() + 1);
        }
        std::cout << "After: " << this->get_start_line() << std::endl;
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
        if (source == NULL) {
            this->report_error((std::string("Failed to load file: `") + std::string(path) + std::string("`")).c_str());
            return;
        }

        this->m_file_path = std::string(path);
        this->m_file_contents = std::string(source);
        this->build_line_offsets();
        this->prepare_visible_chars();
        this->syntax_highlight();
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
        if (m_file_contents.size() == 0) return;

        float x = m_content_rect.x + 1;
        float y = m_content_rect.y + 10;

        m_chars.clear();
        int start = m_text_index;
        int line = start_line_number;
        int col = 1;

        while (true) {
            if (start >= (int)m_file_contents.size()) break;

            char c = m_file_contents[start];
            int source_index = start;
            start++;

            Vector2 text_size = MeasureTextEx(GetFontDefault(), TextFormat("%c", c), 11, 1);

            if (c == '\n' || (x + text_size.x >= m_content_rect.width && y < m_content_rect.height)) {
                y += m_line_height;
                x = m_content_rect.x + 1;
                line++;
                col = 1;
                continue;
            }

            if (x + text_size.x < m_content_rect.width) {
                CharView v(c, {x, y}, BLACK, line, col, source_index);
                m_chars.push_back(v);
                x += text_size.x + 1.1f;
                col++;
            }

            if (y + m_line_height >= m_content_rect.height + m_line_height * 4) break;
        }
    }

    void syntax_highlight() {
        std::vector<const char*> keywords {
          "include", "if", "else", "define", "switch", "return", "for", "while", "do",
          "auto", "class", "struct", "namespace", "using", "typedef",
          "const", "constexpr", "enum", "union"
        };

        std::vector<const char*> types {
          "int", "float", "double", "short", "long", "void", "bool",
          "int32_t", "uint32_t", "int8_t", "uint8_t", "int16_t", "uint16_t",
          "int16_t", "uint64_t", "char"
        };

        for (int index = 0; index < (int)m_chars.size(); index++) {
            CharView* cv = &m_chars[index];
            char c = cv->get_char();

            if (c == '<') {
                for (int i = index + 1; i < (int)m_chars.size(); i++) {
                    char c = m_chars[i].get_char();
                    if (c == '>' || c == '=') { index = i; break; }
                    m_chars[i].set_color(BROWN);
                }

            } else if (c == '"') {
                Color col = ColorBrightness(ORANGE, 0.0f);
                cv->set_color(col);
                for (int i = index + 1; i < (int)m_chars.size(); i++) {
                    if (m_chars[i].get_char() == '"') {
                        m_chars[i].set_color(ORANGE);
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
                    for (const char* t : types)
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
            if (m_file_contents[i] == '\n')
                m_line_offsets.push_back(i + 1);
        }
    }

    int get_start_line() { return m_start_line; }
    void start_line(int iline) {
        iline = std::max(1, std::min(iline, (int)m_line_offsets.size() - 1));
        m_start_line = iline;
        if (iline - 1 <= m_line_offsets.size())
            build_line_offsets();

        m_text_index = m_line_offsets[iline - 1];
        prepare_visible_chars(iline);
        syntax_highlight();
    }
private:

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
};