#include <Highlighter.h>


void Highlighter::add_words(std::vector<std::string> words, Color color) {
    m_word_lists.push_back({words, color});
}

void Highlighter::add_extension(std::string ext) {
    m_exts.push_back(ext);
}

void Highlighter::highlight(std::vector<CharView>&  m_chars) {
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

            for (HighlighterGroup group: m_word_lists) {
                for (std::string _word: group.words) {
                  if (word == _word) { col = group.color; break; }
                }
            }

            if (col.a != 0)
                for (int i = index; i <= word_end; i++)
                    m_chars[i].set_color(col);

            index = word_end;
        }
    }
}