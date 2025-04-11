#pragma once
// Rdeputchar的作用是将字符c渲染到指定位置pos，并使用color指定的颜色进行渲染。
//  它会根据传入的参数来确定渲染的颜色，如果color是TK_AUTO_COLOR，则会根据字符c的类型来自动 deduce 颜色。
// Rdrclear的作用是清除当前帧中的所有对象，通常在渲染新帧之前调用。
// RdrRender的作用是渲染当前帧中的所有对象，它会遍历所有的坦克和子弹，并使用RdrPutChar函数将它们渲染到屏幕上。
// RdrFlush的作用是绘制到屏幕上
/// ```c
/// RdrClear();
/// // Update objects in the scene here.
/// RdrRender();
/// RdrFlush();
/// ```
/// \file
/// \brief This file introduces a renderer which
/// helps us visualize objects in the scene.

//
//
//
//
//
#include "Scene.h"

/// \brief Let the renderer automatically deduce the color.
#define TK_AUTO_COLOR ((const char *)(NULL))

/// \brief The error message for undefined behaviors.
#define TK_INVALID_COLOR "Invalid Color"

/// \example Consequently call these functions to render a frame.
/// ```c
/// RdrClear();
/// // Update objects in the scene here.
/// RdrRender();
/// RdrFlush();
/// ```
///
/// \details The renderer is implemented by
/// imitating the double buffering technique.
/// See https://en.wikipedia.org/wiki/Multiple_buffering for more details.
typedef struct {
  char *csPrev;      // Characters of the previous frame.
  Color *colorsPrev; // Character colors of the previous frame.
  char *cs;          // Characters of the current frame.
  Color *colors;     // Character colors of the current frame.
} Renderer;

// The renderer singleton.
static Renderer renderer;

//
//
//
/// \brief Render character `c` at position `pos` with color `color`.
///
/// \example ```c
/// // Explicitly specify the color.
/// RdrPutChar(pos, 'o', TK_RED);
/// // Let the renderer automatically deduce the color.
/// RdrPutChar(pos, eFlagWall, TK_AUTO_COLOR);
/// ```
void RdrPutChar(Vec pos, char c, Color color) {
  renderer.cs[Idx(pos)] = c;

  if (color == TK_AUTO_COLOR) {
    Flag flag = (Flag)c;
    color = flag == eFlagNone    ? TK_NORMAL
            : flag == eFlagSolid ? TK_BLUE
            : flag == eFlagWall  ? TK_WHITE

            : flag == eFlagTank ? TK_INVALID_COLOR
                                : TK_INVALID_COLOR;
  }
  renderer.colors[Idx(pos)] = color;
}

//
//
//
/// \brief Clear all the objects in the scene from the frame.
void RdrClear(void) {
  // Clear tanks.
  for (RegIterator it = RegBegin(regTank); it != RegEnd(regTank); it = RegNext(it)) {
    Tank *tank = RegEntry(regTank, it);
    Vec pos = tank->pos;

    for (int y = -1; y <= 1; ++y)
      for (int x = -1; x <= 1; ++x)
        RdrPutChar(Add(pos, (Vec){x, y}), map.flags[Idx(pos)], TK_AUTO_COLOR);
  }

  // Clear bullets.
  for (RegIterator it = RegBegin(regBullet); it != RegEnd(regBullet); it = RegNext(it)) {
    Bullet *bullet = RegEntry(regBullet, it);
    Vec pos = bullet->pos;

    RdrPutChar(pos, map.flags[Idx(pos)], TK_AUTO_COLOR);
  }
}

/// \brief 按照不同位置不同方向来绘制坦克，需要接受这个坦克的方向，中心位置和这个坦克颜色
void Tankappr(int dir, Vec pos, Color color) {
  if (dir == 7) { // 下
    for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
        RdrPutChar(Add(pos, (Vec){i, j}), '@', color);
        if (i == 0) {
          if (j == 0)
            RdrPutChar(Add(pos, (Vec){0, 0}), 'O', color);
          else if (j == -1)
            RdrPutChar(Add(pos, (Vec){0, -1}), 'X', color);
          else if (j == 1)
            RdrPutChar(Add(pos, (Vec){0, 1}), '|', color);
        }
      }
    }
  } else if (dir == 3) {
    for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
        RdrPutChar(Add(pos, (Vec){i, j}), '@', color);
        if (j == 0) {
          if (i == 0)
            RdrPutChar(Add(pos, (Vec){0, 0}), 'O', color);
          else if (i == -1)
            RdrPutChar(Add(pos, (Vec){-1, 0}), '-', color);
          else if (i == 1)
            RdrPutChar(Add(pos, (Vec){1, 0}), 'X', color);
        }
      }
    }
  } else if (dir == 5) {
    for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
        RdrPutChar(Add(pos, (Vec){i, j}), '@', color);
        if (j == 0) {
          if (i == 0)
            RdrPutChar(Add(pos, (Vec){0, 0}), 'O', color);
          else if (i == -1)
            RdrPutChar(Add(pos, (Vec){-1, 0}), 'X', color);
          else if (i == 1)
            RdrPutChar(Add(pos, (Vec){1, 0}), '-', color);
        }
      }
    }
  } else if (dir == 1) { // 上
    for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
        RdrPutChar(Add(pos, (Vec){i, j}), '@', color);
        if (i == 0) {
          if (j == 0)
            RdrPutChar(Add(pos, (Vec){0, 0}), 'O', color);
          else if (j == 1)
            RdrPutChar(Add(pos, (Vec){0, 1}), 'X', color);
          else if (j == -1)
            RdrPutChar(Add(pos, (Vec){0, -1}), '|', color);
        }
      }
    }
  }
}

/// \brief Render all the objects in the scene to the frame.
void RdrRender(void) {
  // Render tanks.
  for (RegIterator it = RegBegin(regTank); it != RegEnd(regTank); it = RegNext(it)) {
    Tank *tank = RegEntry(regTank, it);
    Vec pos = tank->pos;
    Color color = tank->color;

    // TODO: You may need to delete or add codes here.
    Tankappr(tank->dir, pos, color); // Render the tank's appearance based on its direction and position.
  }

  // Render bullets.
  for (RegIterator it = RegBegin(regBullet); it != RegEnd(regBullet); it = RegNext(it)) {
    Bullet *bullet = RegEntry(regBullet, it);
    Vec pos = bullet->pos;
    Color color = bullet->color;

    RdrPutChar(pos, 'o', color);
  }
}

/// \brief Flush the previously rendered frame to screen to
/// make it truly visible.
void RdrFlush(void) {
  char *csPrev = renderer.csPrev;
  Color *colorsPrev = renderer.colorsPrev;
  const char *cs = renderer.cs;
  const Color *colors = renderer.colors;

  for (int y = 0; y < map.size.y; ++y)
    for (int x = 0; x < map.size.x; ++x) {
      Vec pos = {x, y};
      int i = Idx(pos);

      if (cs[i] != csPrev[i] || colors[i] != colorsPrev[i]) {
        MoveCursor(pos);
        printf(TK_TEXT("%c", TK_RUNTIME_COLOR), colors[i], cs[i]);

        csPrev[i] = cs[i];
        colorsPrev[i] = colors[i];
      }
    }
}
