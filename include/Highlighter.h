#pragma once

#include <vector>
#include <CharView.h>
#include <ctype.h>
#include <functional>
#include <string>


using HighlighterFunction = std::function<void(std::vector<CharView>&, int*)>;
class Highlighter {
public:
    Highlighter():
        m_word_lists{},
        m_func{nullptr}    {}

    void add_words(std::vector<std::string>, Color);
    void highlight(std::vector<CharView>&);
    void add_extension(std::string);

    std::vector<std::string> get_extensions() { return m_exts; }

    void set_highter_function(HighlighterFunction func) { m_func; }
private:
    struct HighlighterGroup {
    public:
        std::vector<std::string> words;
        Color color;
    };

    std::vector<HighlighterGroup> m_word_lists;
    std::vector<std::string> m_exts;
    HighlighterFunction m_func;
};
