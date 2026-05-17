#include <CommonSyntax.h>
#include <iostream>

std::vector<Highlighter> CommonSyntax::MAP{};

void CommonSyntax::init() {
    Highlighter c_cpp_h = CommonSyntax::c_cpp();
    CommonSyntax::MAP.push_back(c_cpp_h);

    Highlighter python = CommonSyntax::py();
    CommonSyntax::MAP.push_back(python);
}

Highlighter* CommonSyntax::get_highligher(std::string& ext) {
    int size = CommonSyntax::MAP.size();
    for (int i = 0; i < size; ++i) {
        Highlighter* it = &CommonSyntax::MAP[i];
        for (std::string ext_ : it->get_extensions()) {
            if (ext_ == ext)
                return it;
        }
    }

    return nullptr;
}

Highlighter CommonSyntax::py() {
    Highlighter python;
    python.add_extension(".py");

    python.add_words({
       "False","None","True","and","as","assert","async","await",
        "break","class","continue","def","del","elif","else","except",
        "finally","for","from","global","if","import",
        "in","is","lambda","nonlocal","not","or","pass",
        "raise","return","try","while","with","yield"}, BLUE);

    python.set_highter_function([](std::vector<CharView>& m_chars, int *index_ptr) {
      int index = *index_ptr;
      CharView* cv = &m_chars[index];
      char c = cv->get_char();

      if (c == '#') {
          Color color = ColorBrightness(GREEN, -0.5);
          int line = cv->get_line();
          for (int i = index; i < (int)m_chars.size(); i++) {
              if (m_chars[i].get_line() > line) {
                  index = i - 1;
                  break;
              }
              m_chars[i].set_color(color);
          }
        return true;
      }

      return false;
    });
    return python;
}

Highlighter CommonSyntax::c_cpp() {
    Highlighter c_cpp_h;
    c_cpp_h.add_extension(".c");
    c_cpp_h.add_extension(".h");
    c_cpp_h.add_extension(".cpp");
    c_cpp_h.add_extension(".hpp");
    c_cpp_h.add_extension(".cxx");
    c_cpp_h.add_extension(".hxx");

    c_cpp_h.add_words(
        {"include",   "if",     "else",      "define", "endif",    "extern",
         "switch",    "return", "for",       "while",  "do",       "auto",
         "class",     "struct", "namespace", "using",  "typedef",  "const",
         "constexpr", "enum",   "union",     "public", "private",  "protected",
         "pragma",    "this",   "inline",    "break",  "continue", "goto",
         "static",    "ifndef", "ifdef"},
        BLUE);

    c_cpp_h.add_words({
        "int", "float", "double", "short", "long", "void", "bool",
       "int32_t", "uint32_t", "int8_t", "uint8_t", "int16_t", "uint16_t",
       "int16_t", "uint64_t", "char", "true", "false"
    }, PURPLE);

    c_cpp_h.add_words({"std", "string", "vector", "shared_ptr", "unique_ptr",
         "unordered_map", "cout", "endl"}, ColorBrightness(PINK, -0.3));

    c_cpp_h.set_highter_function([](std::vector<CharView>& m_chars, int *index_ptr) {
      int index = *index_ptr;
      CharView* cv = &m_chars[index];
      char c = cv->get_char();

      if (c == '/' && index + 1 < m_chars.size() && m_chars[index + 1].get_char() == '*') {
          Color color = ColorBrightness(GREEN, -0.5);
          for (int i = index; i < (int)m_chars.size(); i++) {
              c = m_chars[i].get_char();
              if (c == '*' && i + 1 < m_chars.size() && m_chars[i + 1].get_char() == '/') {
                  m_chars[i].set_color(color);
                  m_chars[i + 1].set_color(color);
                  *index_ptr = i + 1;
                  break;
              }
              m_chars[i].set_color(color);
          }
          return true;
      } else if (c == '/' && index + 1 < m_chars.size() && m_chars[index + 1].get_char() == '/') {
          Color color = ColorBrightness(GREEN, -0.5);
          int line = cv->get_line();
          for (int i = index; i < (int)m_chars.size(); i++) {
              if (m_chars[i].get_line() > line) {
                *index_ptr = i - 1;
                break;
              }
              m_chars[i].set_color(color);
          }
          return true;
      }
      return false;
    });

    return c_cpp_h;
}