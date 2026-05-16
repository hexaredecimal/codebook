#pragma once

#include <string>
#include <Cursor.h>

#include <raylib.h>

class Buffer {
public:
    Buffer(std::string file_path, std::string file_content, int w, int h):
        m_file_path{file_path},
        m_file_contents{file_content},
        m_file_name(file_path.size() == 0 ? "No file" : GetFileName(file_path.c_str())),
        m_text_index(0),
        m_start_line(1),
        m_cursor{w,h},
        m_is_dirty{false}
    {}

    void next_char() { m_text_index++; }
    void prev_char() { m_text_index--; }
    int get_text_index() { return m_text_index; }
    void set_text_index(int index) { m_text_index = index; }

    void set_start_line(int line) { m_start_line = line; }
    int get_start_line() { return m_start_line; }

    std::string get_file_contents() { return m_file_contents; }
    void insert_char(char c) {
      m_file_contents.insert(m_text_index, 1, c);
      next_char();
      if (!m_is_dirty)
          m_is_dirty = true;
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

    bool get_is_dirty() { return m_is_dirty; }
    void set_is_dirty(bool dirty) { m_is_dirty = dirty; }

private:
    std::string m_file_path;
    std::string m_file_contents;
    std::string m_file_name;
    int m_text_index = 0;
    int m_start_line = 1;
    Cursor m_cursor;
    bool m_is_dirty;
};
