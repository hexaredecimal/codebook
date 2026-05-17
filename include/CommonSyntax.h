#pragma once

#include <Highlighter.h>


class CommonSyntax {
public:
    static void init();
    static Highlighter* get_highligher(std::string&);

private:
    static Highlighter c_cpp();
    static Highlighter py();

    static std::vector<Highlighter> MAP;
};

