#include <Editor.h>




int Editor::get_line() { return m_buffer_list[m_buffer_index].get_cursor()->get_line(); }
int Editor::get_column() { return m_buffer_list[m_buffer_index].get_cursor()->get_column(); }

int Editor::get_max_viewable_lines() { return m_max_viewable_lines; }
EditorMode Editor::get_editor_mode() { return m_mode; }
void Editor::set_editor_mode(EditorMode mode) { m_mode = mode; }

Cursor* Editor::get_cursor() { return m_buffer_list[m_buffer_index].get_cursor(); }

void Editor::update(Rectangle content_rect) {
    this->m_content_rect = content_rect;
    m_max_viewable_lines = content_rect.height / m_line_height;

    Buffer* current_buffer = &m_buffer_list[m_buffer_index];
    if (current_buffer->get_text_index() < m_render_start_index)
        current_buffer->set_text_index(m_render_start_index);

    Cursor* cursor = current_buffer->get_cursor();
    float dt = GetFrameTime();
    std::string file_contents = current_buffer->get_file_contents();
    char c = file_contents[current_buffer->get_text_index()];
    for (CharView cv: m_chars) {
        if (cv.get_source_index() == current_buffer->get_text_index()) {
            Vector2 pos = cv.get_pos();
            cursor->set_cursor_position({pos.x, pos.y - 15});
            cursor->set_current_char(cv);
            // std::cout << cv.get_char() << std::endl;
            break;
        }
    }

    // std::cout << "Looking at: " << c << std::endl;
    cursor->update();
    if (cursor->get_cursor_position().y > content_rect.height + m_line_height * 2)
        this->goto_line(cursor->get_line() - 1);

    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        int delta = (wheel > 0) ? -1 : 1;
        start_line(current_buffer->get_start_line() + delta);
    }


    int ch = GetCharPressed();

    while (ch > 0) {
        if (ch >= 32 && ch != 127)
            insert_char((char)ch);;
        ch = GetCharPressed();
    }

    if (IsKeyPressed(KEY_ENTER))
        insert_char('\n');

    if (IsKeyPressed(KEY_TAB))
        insert_char('\t');

    if (IsKeyPressed(KEY_BACKSPACE))
        delete_char();

    std::string contents = current_buffer->get_file_contents();

    if (IsKeyPressed(KEY_RIGHT)) {
        if (current_buffer->get_text_index() < (int)contents.size())
            current_buffer->next_char();
        int y_diff = cursor->get_line() - current_buffer->get_start_line();
        if (c == '\n' && y_diff + 1 == m_max_viewable_lines)
            this->start_line(current_buffer->get_start_line() + 1);
    }

    if (IsKeyPressed(KEY_LEFT)) {
        if (current_buffer->get_text_index() > 0)
            current_buffer->prev_char();

        int y_diff = cursor->get_line() - current_buffer->get_start_line();
        if (y_diff + 1 == 1 && cursor->get_column() == 1)
            this->start_line(current_buffer->get_start_line() - 1);
    }

    if (IsKeyPressed(KEY_UP)) {
        int y_diff = cursor->get_line() - current_buffer->get_start_line();
        if (y_diff + 1 == 1 )
            this->start_line(current_buffer->get_start_line() - 1);

        int col = cursor->get_column();
        this->goto_line(cursor->get_line() - 1);
        this->goto_col(col);
    }

    if (IsKeyPressed(KEY_DOWN)) {
        int y_diff = cursor->get_line() - current_buffer->get_start_line();
        if (y_diff + 1 == m_max_viewable_lines)
            this->start_line(current_buffer->get_start_line() + 1);

        int col = cursor->get_column();
        this->goto_line(cursor->get_line() + 1);
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

void Editor::report_error(const char* message) { this->report(message, ErrorType::ERROR); }
void Editor::report_info(const char* message) { this->report(message, ErrorType::INFO); }
void Editor::report_warning(const char* message) { this->report(message, ErrorType::WARNING); }


void Editor::insert_char(char c) {
    Buffer* buffer = &m_buffer_list[m_buffer_index];
    buffer->insert_char(c);
    build_line_offsets();
    prepare_visible_chars(buffer->get_start_line());
    syntax_highlight();
}

void Editor::delete_char() {
    Buffer* buffer = &m_buffer_list[m_buffer_index];
    if (buffer->get_text_index() > 0) {
        buffer->delete_char();
        build_line_offsets();
        Cursor* cursor = buffer->get_cursor();
        CharView c = cursor->get_current_char();
        if (c.get_col() == 1 && c.get_line() == buffer->get_start_line()) {
            start_line(buffer->get_start_line() - m_max_viewable_lines / 2);
        } else {
            prepare_visible_chars(buffer->get_start_line());
            syntax_highlight();
        }

    }
}

Vector2 Editor::get_text_size(const char* c) {
    return MeasureTextEx(GetFontDefault(), c, 11, 1);
}

void Editor::draw_errors() {
    for (ErrorView* error: m_error_messages) {
        error->draw();
    }
}


Buffer Editor::get_buffer() {
    return m_buffer_list[m_buffer_index];
}

void Editor::open_file(const char* path) {
    const char* source = LoadFileText(path);
    if (!source) {
        report_error((std::string("Failed to load: `") + path + "`").c_str());
        return;
    }

    if (!m_initialized) {
        m_buffer_list.clear();
    }

    m_buffer_list.push_back(Buffer(std::string(path), std::string(source), m_cursor_w, m_cursor_h));
    int last = m_buffer_list.size() - 1;
    Cursor* cursor = m_buffer_list[last].get_cursor();
    cursor->set_cursor_position({m_content_rect.x, m_content_rect.y});

    m_render_start_index = 0;
    build_line_offsets();
    prepare_visible_chars(1);
    syntax_highlight();
}

void Editor::draw_text() {
    Buffer* buffer = &m_buffer_list[m_buffer_index];
    std::string contents = buffer->get_file_contents();

    if (contents.size() == 0)
        return;

    Vector2 mouse_pos = GetMousePosition();
    for (CharView char_view: m_chars) {
        const char* text = TextFormat("%c", char_view.get_char());
        Vector2 pos = char_view.get_pos();
        Color color = char_view.get_color();
        DrawTextEx(GetFontDefault(),text, pos, 11, 1, color);

        Vector2 size = this->get_text_size(text);
        Rectangle text_rect = { pos.x, pos.y - m_line_height / 2, size.x, m_line_height};
        if (CheckCollisionPointRec(mouse_pos, text_rect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            this->goto_line(char_view.get_line());
            this->goto_col(char_view.get_col() + 1);
            //DrawRectangleLinesEx(text_rect, 1, RED);
        }
    }

}

void Editor::prepare_visible_chars(int start_line_number) {
    Buffer* buffer = &m_buffer_list[m_buffer_index];
    std::string contents = buffer->get_file_contents();
    if (contents.empty()) return;

    m_chars.clear();

    float x = m_content_rect.x + 1;
    float y = m_content_rect.y + 10;
    float right_edge = m_content_rect.x + m_content_rect.width;
    float bottom_edge = m_content_rect.y + m_content_rect.height;

    int line = start_line_number;
    int col  = 1;

    int start = m_render_start_index;

    while (start < (int)contents.size()) {
        if (y >= bottom_edge + m_line_height * 2) break;

        char c = contents[start];
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

        Vector2 text_size = this->get_text_size(TextFormat("%c", c));

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

    if (start >= contents.size()) {
        CharView v(' ', {x, y}, BLACK, line, col, start);
        m_chars.push_back(v);
    }
}

void Editor::syntax_highlight() {
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

void Editor::build_line_offsets() {
    Buffer* buffer = &m_buffer_list[m_buffer_index];
    std::string contents = buffer->get_file_contents();
    m_line_offsets.clear();
    m_line_offsets.push_back(0);
    for (int i = 0; i < (int)contents.size(); i++) {
        char c = contents[i];
        if (c == '\n')
            m_line_offsets.push_back(i + 1);
    }
}

int Editor::get_start_line() {
    return m_buffer_list[m_buffer_index].get_start_line();
}

void Editor::start_line(int iline) {
    if (m_line_offsets.empty()) return;

    // Clamp to valid line range
    iline = std::max(1, std::min(iline, (int)m_line_offsets.size()));
    Buffer* buffer = &m_buffer_list[m_buffer_index];
    buffer->set_start_line(iline);

    // Drive rendering from the line's file offset — NOT the cursor index
    m_render_start_index = m_line_offsets[iline - 1];

    prepare_visible_chars(iline);
    syntax_highlight();
}

void Editor::goto_line(int line) {
    Buffer* buffer = &m_buffer_list[m_buffer_index];
    Cursor* cursor = buffer->get_cursor();
    for (CharView cv: m_chars) {
        if (cv.get_line() == line) {
            Vector2 pos = cv.get_pos();
            cursor->set_cursor_position({pos.x, pos.y - 15});
            cursor->set_current_char(cv);
            buffer->set_text_index(cv.get_source_index());
            break;
        }
    }
}


void Editor::goto_col(int col) {
    Buffer* buffer = &m_buffer_list[m_buffer_index];
    Cursor* cursor = buffer->get_cursor();
    for (CharView cv: m_chars) {
        if (cv.get_line() == cursor->get_line() && cv.get_col() == col) {
            Vector2 pos = cv.get_pos();
            cursor->set_cursor_position({pos.x, pos.y - 15});
            cursor->set_current_char(cv);
            buffer->set_text_index(cv.get_source_index());
            break;
        }
    }
}

void Editor::report(const char* message, ErrorType type){
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
