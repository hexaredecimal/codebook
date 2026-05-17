#pragma once

#include <Cursor.h>
#include <CharView.h>
#include <Highlighter.h>

#include <string>
#include <raylib.h>
#include <vector>

class Buffer {
public:
 Buffer(std::string file_path, std::string file_content, int w, int h)
     : m_file_path{file_path},
       m_file_contents{file_content},
       m_file_name(file_path.size() == 0 ? "No file"
                                         : GetFileName(file_path.c_str())),
       m_text_index(0),
       m_start_line(1),
       m_cursor{w, h},
       m_highlighter{nullptr},
       m_ext{".txt"},
       m_is_dirty{false},
       m_text_selection_range{-1, -1},
       m_prev_selection_range{-1, -1} {}

 void next_char() { m_text_index++; }
 void prev_char() { m_text_index--; }
 int get_text_index() { return m_text_index; }
 void set_text_index(int index) { m_text_index = index; }

 void set_start_line(int line) { m_start_line = line; }
 int get_start_line() { return m_start_line; }

 std::string get_file_contents() { return m_file_contents; }
 void set_file_contents(std::string file) { m_file_contents = file; }
 void insert_char(char c) {
   m_file_contents.insert(m_text_index, 1, c);
   next_char();
   if (!m_is_dirty) m_is_dirty = true;
 }

    void delete_char() {
      if (m_text_index > 0) {
        m_file_contents.erase(m_text_index - 1, 1);
        prev_char();

        if (!m_is_dirty)
            m_is_dirty = true;
      }
    }

    Cursor* get_cursor() { return &m_cursor; }
    void set_file_path(std::string path) { m_file_path = path; }
    std::string get_file_path() { return m_file_path; }
    std::string get_file_name() {
        return m_file_name;
    }

    void set_highlighter(Highlighter* h) { m_highlighter = h;}

    void highlight(std::vector<CharView>& m_chars) {
        if (m_highlighter != nullptr)
            m_highlighter->highlight(m_chars);
    }

    bool get_is_dirty() { return m_is_dirty; }
    void set_is_dirty(bool dirty) { m_is_dirty = dirty; }

    void set_extension(std::string ext) { m_ext = ext; }
    std::string get_extension() { return m_ext; }

    void set_selection(int start, int end) {
      m_prev_selection_range.x = m_text_selection_range.x;
      m_prev_selection_range.y = m_text_selection_range.y;
      m_text_selection_range.x = start;
      m_text_selection_range.y = end;
    }

    Vector2 get_last_selection() { return m_prev_selection_range; }
    Vector2 get_selection() { return m_text_selection_range; }
    void clear_selection() {
      m_prev_selection_range.x = m_text_selection_range.x;
      m_prev_selection_range.y = m_text_selection_range.y;
      m_text_selection_range.x = -1;
      m_text_selection_range.y = -1;
    }

   private:
    std::string m_file_path;
    std::string m_file_contents;
    std::string m_file_name;
    Highlighter* m_highlighter;
    std::string m_ext;
    int m_text_index = 0;
    int m_start_line = 1;
    Cursor m_cursor;
    bool m_is_dirty;
    Vector2 m_text_selection_range;
    Vector2 m_prev_selection_range;
};
