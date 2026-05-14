#include <iostream>
#include <raylib.h>

#include <Editor.h>

void draw_header(const Texture2D);
void draw_footer(const Texture2D);
void draw_line(const Texture2D, float);
void draw_lines(const Texture2D, float);

void draw_line_numbers(const int, const Rectangle, const int, const int);
void draw_status_header(const Rectangle, const char*);
void draw_status_footer(const Rectangle, const Editor);

int main() {
  auto flag = (int)FLAG_MSAA_4X_HINT;
  SetConfigFlags(flag);

  InitWindow(600, 800, "Hello, world");
  SetTargetFPS(60);

  auto w = GetScreenWidth();
  auto h = GetScreenHeight();

  Texture2D header_texture = LoadTexture("./images/header.png");
  Texture2D footer_texture = LoadTexture("./images/footer.png");
  Texture2D line_texture = LoadTexture("./images/line.png");
  Texture2D cursor_texture = LoadTexture("./images/bicballpenblack.png");
  Texture2D mouse_texture = LoadTexture("./images/pointing_hand.png");

  Rectangle content_rect = {
    65, header_texture.height,
    header_texture.width - 81,
    h - footer_texture.height - line_texture.height * 2 - 12
  };


  Rectangle cursor_source_rect = {
    0,0,
    cursor_texture.width,
    cursor_texture.height
  };


  SetWindowSize(header_texture.width - 5, GetScreenHeight());


  Editor ed(
    content_rect,
    cursor_texture.width,
    cursor_texture.height,
    line_texture.height
  );

  ed.open_file("./src/main.cpp");
  ed.start_line(1);

  Cursor* cursor = ed.get_cursor();
  int error_count=0;
  HideCursor();

  while (!WindowShouldClose()) {
    content_rect = {
      65, header_texture.height,
      header_texture.width - 81,
      h - footer_texture.height - line_texture.height * 2 - 12
    };
    ed.update(content_rect);
    BeginDrawing();
    ClearBackground(RAYWHITE);
    draw_lines(line_texture, header_texture.height);


    draw_header(header_texture);
    // draw_status_header({0,0, header_texture.width, header_texture.height}, "$HOME");


    draw_footer(footer_texture);
    draw_status_footer({
      0,
      h - footer_texture.height,
      footer_texture.width ,
      footer_texture.height},
      ed
    );

    draw_line_numbers(ed.get_start_line(), content_rect, header_texture.height, line_texture.height);

    BeginScissorMode(content_rect.x,content_rect.y, content_rect.width, content_rect.height);
        ed.draw_text();
    EndScissorMode();

    if (cursor->get_timer() > 0)
      DrawTexturePro(cursor_texture, cursor_source_rect, cursor->get_cursor_rect(), {0,0}, 0, WHITE);

    ed.draw_errors();


    Vector2 mouse_pos = GetMousePosition();
    Rectangle source_rect = {0,0, mouse_texture.width, mouse_texture.height};
    Rectangle mouse_dest_rect {mouse_pos.x, mouse_pos.y, mouse_texture.width/2, mouse_texture.height/2};
    Rectangle mouse_shadow_dest_rect {mouse_pos.x + 5, mouse_pos.y + 5, mouse_texture.width/2, mouse_texture.height/2};

    DrawTexturePro(mouse_texture, source_rect, mouse_shadow_dest_rect, {0,0}, 0, BLACK);
    DrawTexturePro(mouse_texture, source_rect, mouse_dest_rect, {0,0}, 0, WHITE);
    // DrawRectangleLinesEx(content_rect, 1, RED);
    EndDrawing();
  }

  UnloadTexture(header_texture);
}

void draw_status_footer(const Rectangle status_rect, Editor ed) {
    Cursor* cursor = ed.get_cursor();
    int line = ed.get_start_line() + cursor->get_line() - 1;
    int column = cursor->get_column();
    const char* mode_str = TextFormat("[%s]", editor_mode_str(ed.get_editor_mode()));
    Vector2 text_size = MeasureTextEx(GetFontDefault(), mode_str, 12, 1);
    auto x = status_rect.x + status_rect.width - text_size.x - 15;
    auto y = status_rect.y  + text_size.y - 2;
    DrawText(mode_str, x, y, 12, ColorBrightness(BLUE, -0.5));

    const char* line_col_str = TextFormat("[line: %d | column: %d]", line, column);
    text_size = MeasureTextEx(GetFontDefault(), line_col_str, 12, 1);
    x = status_rect.x + status_rect.width - text_size.x;
    DrawText(line_col_str, x - text_size.x + 5, y, 12, ColorBrightness(BLUE, -0.5));

    x = 70;
    DrawText(ed.get_buffer_name(), x, y, 14, ColorBrightness(BLUE, -0.2));
}

void draw_status_header(const Rectangle status_rect, const char* buffer_name) {
    const char* opened_file = buffer_name == NULL
        ? "[NULL]" : TextFormat("[%s]", buffer_name);
    Vector2 text_size = MeasureTextEx(GetFontDefault(), opened_file, 12, 1);

    auto x = status_rect.x + status_rect.width - text_size.x - 15;

    auto y = status_rect.y + status_rect.height - text_size.y - 2;
    DrawText(opened_file, x, y, 12, BLACK);
}

void draw_line_numbers(const int line_start, const Rectangle rect, const int starty, const int line_rect_height) {
    auto gutter_rect = Rectangle { 0,0, 65, line_rect_height};
    int line_count = line_start;
    auto text_color = ColorBrightness(BLUE, 0.5);
    for (int y = starty; y <= rect.height + 2 * line_rect_height; y += line_rect_height) {
        char text[1024];
        snprintf(text, sizeof(text), "%d", line_count);
        Vector2 text_size = MeasureTextEx(GetFontDefault(), text, 12, 1);
        int x = gutter_rect.width - text_size.x - 4;
        DrawText(text, x,y + gutter_rect.height /2 , 12, text_color);
        line_count++;
    }
}

void draw_header(const Texture2D header_texture) {
    auto w = GetScreenWidth();
    auto h = GetScreenHeight();

    auto header_rect = Rectangle { 0,0, 65, header_texture.height};
    auto dest_rect = Rectangle { 0,0, 65, header_texture.height};
    DrawTexturePro(header_texture, header_rect, dest_rect, {0,0}, 0, WHITE);

    header_rect.x += header_rect.width + 1;
    dest_rect.x += header_rect.width;
    while (dest_rect.x < w) {
      DrawTexturePro(header_texture, header_rect, dest_rect, {0,0}, 0, WHITE);
      header_rect.x += header_rect.width + 1;
      dest_rect.x += header_rect.width;
    }

}

void draw_footer(const Texture2D footer_texture) {
    auto w = GetScreenWidth();
    auto h = GetScreenHeight();

    auto footer_rect = Rectangle { 0,0, 65, footer_texture.height};
    auto dest_rect = Rectangle { 0, h - footer_texture.height, 65, footer_texture.height};
    DrawTexturePro(footer_texture, footer_rect, dest_rect, {0,0}, 0, WHITE);

    footer_rect.x += footer_rect.width + 1;
    dest_rect.x += footer_rect.width;
    while (dest_rect.x < w) {
      DrawTexturePro(footer_texture, footer_rect, dest_rect, {0,0}, 0, WHITE);
      footer_rect.x += footer_rect.width + 1;
      dest_rect.x += footer_rect.width;
    }
}

void draw_line(const Texture2D line_texture, float starty) {
    auto w = GetScreenWidth();
    auto h = GetScreenHeight();

    auto line_rect = Rectangle { 0,0, 65, line_texture.height};
    auto dest_rect = Rectangle { 0, starty, 65, line_texture.height};
    DrawTexturePro(line_texture, line_rect, dest_rect, {0,0}, 0, WHITE);

    line_rect.x += line_rect.width + 1;
    dest_rect.x += line_rect.width;
    while (dest_rect.x < w) {
      DrawTexturePro(line_texture, line_rect, dest_rect, {0,0}, 0, WHITE);
      line_rect.x += line_rect.width + 1;
      dest_rect.x += line_rect.width;
    }
}


void draw_lines(const Texture2D line_texture, float starty) {
    auto w = GetScreenWidth();
    auto h = GetScreenHeight();
    while (starty < h) {
        draw_line(line_texture, starty);
        starty += line_texture.height;
    }
}
