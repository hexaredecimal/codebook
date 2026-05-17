#pragma once

#include <raylib.h>

#include <functional>
#include <iostream>
#include <string>

enum class PopupItemKind { ITEM, SEPARATOR };

class PopupItem {
 public:
  using PopupItemFunction = std::function<void(void)>;

  PopupItem(std::string str, PopupItemKind kind)
      : m_text{str}, m_rect{}, m_func{nullptr}, m_kind(kind) {
    m_line_texture = LoadTexture("./images/popup_menu_line.png");
  }

  std::string get_text() { return m_text; }
  void set_text(std::string text) { m_text = text; }

  void set_pos(Vector2 pos) {
    m_rect.x = pos.x;
    m_rect.y = pos.y;
  }

  void set_size(Vector2 pos) {
    m_rect.width = pos.x;
    m_rect.height = pos.y;
  }

  bool draw() {
    if (m_kind == PopupItemKind::SEPARATOR) {
      m_rect.height = 3;
      Rectangle rect = {m_rect.x + 5, m_rect.y, m_rect.width - 5,
                        m_rect.height};
      DrawRectangleRec(rect, BLACK);
      return false;
    }

    Rectangle src_rect = {0, 0, m_line_texture.width, m_line_texture.height};
    Rectangle dest_rect{m_rect.x + 2, m_rect.y + 2, m_rect.width,
                        m_rect.height};

    bool is_hovered = CheckCollisionPointRec(GetMousePosition(), dest_rect);
    Color tint = WHITE;
    Color text_color = BLACK;
    if (is_hovered) {
      tint = ColorBrightness(BLACK, 0.5);
      text_color = ColorBrightness(WHITE, 0.5);
    }

    DrawTexturePro(m_line_texture, src_rect, dest_rect, {0, 0}, 0, BLACK);
    DrawTexturePro(m_line_texture, src_rect, m_rect, {0, 0}, 0, tint);

    int text_x = m_rect.x + 5;
    int text_y = m_rect.y + 5;
    int font_size = 14;
    for (char c : m_text) {
      const char* text = TextFormat("%c", c);
      int w = MeasureText(text, font_size);
      DrawText(text, text_x, text_y, font_size, text_color);
      text_x += w + 1;
    }

    if (is_hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        m_func != nullptr) {
      m_func();
      return true;
    }

    return false;
  }

  Rectangle get_rectangle() { return m_rect; }
  void set_on_click(PopupItemFunction func) { m_func = func; }

 private:
  std::string m_text;
  Rectangle m_rect;
  Texture2D m_line_texture;
  PopupItemFunction m_func;
  PopupItemKind m_kind;
};
