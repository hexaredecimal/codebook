#pragma once

#include <PopupItem.h>
#include <raylib.h>

#include <iostream>
#include <vector>

class PopupMenu {
 public:
  PopupMenu() : m_show{false}, m_pos{} {
    m_header_texture = LoadTexture("./images/popup_menu_header.png");
    m_footer_texture = LoadTexture("./images/popup_menu_footer.png");
  }

  PopupItem* add_item(std::string text) {
    PopupItem item(text, PopupItemKind::ITEM);
    m_menu_items.push_back(item);
    int last = m_menu_items.size() - 1;
    return &m_menu_items[last];
  }

  void add_separator() {
    PopupItem item("", PopupItemKind::SEPARATOR);
    m_menu_items.push_back(item);
  }

  void update(Rectangle content_rect) {
    if (m_show || m_menu_items.size() == 0) {
      return;
    }

    Vector2 mouse_pos = GetMousePosition();
    m_pos = mouse_pos;
    m_show = CheckCollisionPointRec(mouse_pos, content_rect) &&
             IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
  }

  void draw() {
    if (!m_show || m_menu_items.size() == 0) return;

    Rectangle src_rect = {0, 0, m_header_texture.width,
                          m_header_texture.height};
    Rectangle dest_rect = {m_pos.x, m_pos.y, 150, 20};

    Vector2 mouse_pos = GetMousePosition();
    bool is_hovered = CheckCollisionPointRec(mouse_pos, dest_rect);
    Vector2 pos = m_pos;
    DrawTexturePro(m_header_texture, src_rect, {pos.x + 2, pos.y + 2, 150, 20},
                   {0, 0}, 0, BLACK);
    DrawTexturePro(m_header_texture, src_rect, dest_rect, {0, 0}, 0, WHITE);

    int x = dest_rect.x;
    int y = dest_rect.y + dest_rect.height;

    int h = GetScreenHeight();
    for (PopupItem item : m_menu_items) {
      item.set_pos({x, y});
      item.set_size({dest_rect.width, dest_rect.height});
      if (item.draw()) m_show = false;
      Rectangle item_bound = item.get_rectangle();
      y += item_bound.height;
      if (y > h) break;
    }

    dest_rect.y = y;
    pos.y = y;
    src_rect.width = m_footer_texture.width;
    src_rect.height = m_footer_texture.height;
    DrawTexturePro(m_footer_texture, src_rect, {pos.x + 2, pos.y + 2, 150, 20},
                   {0, 0}, 0, BLACK);
    DrawTexturePro(m_footer_texture, src_rect, dest_rect, {0, 0}, 0, WHITE);

    dest_rect.y += dest_rect.height;

    bool is_supported_button = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) ||
                               IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) ||
                               IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE);
    float scroll_wheel = GetMouseWheelMove();
    if ((!is_hovered && is_supported_button) || scroll_wheel != 0.0f)
      m_show = false;
  }

 private:
  bool m_show;
  Vector2 m_pos;
  Texture2D m_header_texture;
  Texture2D m_footer_texture;

  std::vector<PopupItem> m_menu_items;
};
