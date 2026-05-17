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

    if (file_contents.size() == 0) {
      current_buffer->set_text_index(0);
      current_buffer->set_file_contents(" ");
      start_line(1);
      return;
    }

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
      if (current_buffer->get_text_index() > 0) current_buffer->prev_char();

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

void Editor::report_error(std::string message) { this->report(message, ErrorType::ERROR); }
void Editor::report_info(std::string message) { this->report(message, ErrorType::INFO); }
void Editor::report_warning(std::string message) { this->report(message, ErrorType::WARNING); }


void Editor::insert_char(char c) {
    if (!m_initialized)
        m_initialized = true;

    Buffer* buffer = &m_buffer_list[m_buffer_index];
    buffer->insert_char(c);
    build_line_offsets();
    prepare_visible_chars(buffer->get_start_line());
    buffer->highlight(m_chars);
}

void Editor::delete_char() {
    Buffer* buffer = &m_buffer_list[m_buffer_index];
    if (buffer->get_text_index() > 0) {
        if (!m_initialized)
            m_initialized = true;

        buffer->delete_char();
        build_line_offsets();
        Cursor* cursor = buffer->get_cursor();
        CharView c = cursor->get_current_char();
        if (c.get_col() == 1 && c.get_line() == buffer->get_start_line()) {
            start_line(buffer->get_start_line() - m_max_viewable_lines / 2);
        } else {
            prepare_visible_chars(buffer->get_start_line());
            buffer->highlight(m_chars);
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
        std::string err = std::string("Failed to load: `") + path + "`";
        report_error(err);
        return;
    }

    if (!m_initialized) {
        m_buffer_list.clear();
    }

    m_buffer_list.push_back(Buffer(std::string(path), std::string(source), m_cursor_w, m_cursor_h));
    int last = m_buffer_list.size() - 1;
    Buffer* this_buffer = &m_buffer_list[last];
    Cursor* cursor = this_buffer->get_cursor();
    cursor->set_cursor_position({m_content_rect.x, m_content_rect.y});

    const char* raw_ext = GetFileExtension(path);

    if (raw_ext) {
      std::string ext = std::string(raw_ext);
        this_buffer->set_extension(ext);
      Highlighter* highlighter = CommonSyntax::get_highligher(ext);
      if (highlighter != nullptr)
          this_buffer->set_highlighter(highlighter);
    }

    m_render_start_index = 0;
    build_line_offsets();
    prepare_visible_chars(1);

    Buffer current_buffer = m_buffer_list[m_buffer_index];
    current_buffer.highlight(m_chars);
}

void Editor::draw_text() {
    Buffer* buffer = &m_buffer_list[m_buffer_index];
    std::string contents = buffer->get_file_contents();

    if (contents.size() == 0)
        return;

    Vector2 mouse_pos = GetMousePosition();
    static Vector2 dragOffset = {0};
    static int drag_start_index = -1;
    static int drag_end_index = -1;

    Vector2 selection = buffer->get_selection();

    for (CharView char_view : m_chars) {
      const char* text = TextFormat("%c", char_view.get_char());
      Vector2 pos = char_view.get_pos();
      Color color = char_view.get_color();
      DrawTextEx(GetFontDefault(), text, pos, 11, 1, color);
      int source_index = char_view.get_source_index();

      Vector2 size = this->get_text_size(text);
      Rectangle text_rect = {pos.x, pos.y - m_line_height / 2, size.x,
                             m_line_height};

      if (selection.x != -1 && selection.y != -1) {
        if (source_index >= selection.x && source_index <= selection.y) {
          DrawRectangleRec({text_rect.x, text_rect.y, text_rect.width + 1.1f,
                            text_rect.height},
                           ColorAlpha(BLUE, 0.2));
        }
      }

      bool is_hovering_text = CheckCollisionPointRec(mouse_pos, text_rect);
      if (is_hovering_text && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        this->goto_line(char_view.get_line());
        this->goto_col(char_view.get_col() + 1);
        if (selection.x == -1 && selection.y == -1) {
          m_dragging = true;
          drag_start_index = char_view.get_source_index();
        } else {
          m_dragging = false;
          drag_start_index = -1;
          drag_end_index = -1;
          buffer->clear_selection();
        }
      }

      if (m_dragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON) &&
          is_hovering_text) {
        drag_end_index = char_view.get_source_index();
        if (drag_end_index < drag_start_index)
          buffer->set_selection(drag_end_index, drag_start_index);
        else
          buffer->set_selection(drag_start_index, drag_end_index);
      }

      if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        if (!m_dragging) return;

        if (is_hovering_text) drag_end_index = char_view.get_source_index();

        if (drag_end_index < drag_start_index)
          buffer->set_selection(drag_end_index, drag_start_index);
        else if (drag_end_index > drag_start_index)
          buffer->set_selection(drag_start_index, drag_end_index);
        else {
          drag_start_index = -1;
          drag_start_index = -1;
          buffer->clear_selection();
        }
        m_dragging = false;
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
    buffer->highlight(m_chars);
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

void Editor::report(std::string message, ErrorType type){
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

void Editor::new_buffer() {
  Cursor* cursor = m_buffer_list[m_buffer_index].get_cursor();
  Rectangle rect = cursor->get_cursor_rect();
  Buffer temp_buffer("", "//Temp buffer\nDo what ever you want here\n...",
                     rect.width, rect.height);
  Cursor* temp_cursor = temp_buffer.get_cursor();
  temp_cursor->set_cursor_position({m_content_rect.x, m_content_rect.y});
  m_buffer_list.push_back(temp_buffer);
  m_buffer_index = m_buffer_list.size() - 1;
  start_line(1);
}

void Editor::next_buffer() {
  m_buffer_index++;
  if (m_buffer_index >= m_buffer_list.size()) {
    m_buffer_index = m_buffer_list.size() - 1;
  }
  Buffer buffer = m_buffer_list[m_buffer_index];
  start_line(buffer.get_start_line());
}

void Editor::prev_buffer() {
  m_buffer_index--;
  if (m_buffer_index <= 0) {
    m_buffer_index = 0;
  }
  Buffer buffer = m_buffer_list[m_buffer_index];
  start_line(buffer.get_start_line());
}

void Editor::select_all() {
  Buffer* buffer = &m_buffer_list[m_buffer_index];
  buffer->set_selection(0, buffer->get_file_contents().size() - 1);
}

void Editor::copy() {
  Buffer buffer = m_buffer_list[m_buffer_index];
  Vector2 selection = buffer.get_last_selection();

  std::stringstream ss;
  for (CharView char_view : m_chars) {
    int source_index = char_view.get_source_index();
    if (selection.x != -1 && selection.y != -1) {
      if (source_index >= selection.x && source_index <= selection.y) {
        ss << char_view.get_char();
      }
    }
  }

  std::string copied = ss.str();
  SetClipboardText(copied.c_str());
  std::stringstream msg_ss;
  msg_ss << "Copied " << ss.str().length() << " characters into the clipboard"
         << std::endl;

  report_info(msg_ss.str());
}

void Editor::cut() {
  Buffer* buffer = &m_buffer_list[m_buffer_index];
  Vector2 selection = buffer->get_last_selection();

  if (selection.x == -1 && selection.y == -1) {
    return;
  }

  std::stringstream ss;
  for (CharView char_view : m_chars) {
    int source_index = char_view.get_source_index();
    if (source_index >= selection.x && source_index <= selection.y) {
      ss << char_view.get_char();
    }
  }

  std::string m_text = buffer->get_file_contents();
  m_text.erase(selection.x, selection.y - selection.x + 1);
  buffer->set_file_contents(m_text);
  buffer->clear_selection();

  buffer->set_is_dirty(true);
  start_line(buffer->get_start_line());

  std::string copied = ss.str();
  SetClipboardText(copied.c_str());
  std::stringstream msg_ss;
  msg_ss << "Cut " << ss.str().length() << " characters into the clipboard"
         << std::endl;

  report_info(msg_ss.str());
}

void Editor::paste() {
  Buffer* buffer = &m_buffer_list[m_buffer_index];
  const char* paste = GetClipboardText();

  if (paste == NULL) return;

  std::string _paste = paste;
  std::cout << paste << std::endl;
  for (char text : _paste) {
    insert_char(text);
  }
  std::stringstream msg_ss;
  msg_ss << "Pasted " << _paste.length() << " characters into the buffer"
         << std::endl;
  report_info(msg_ss.str());
}