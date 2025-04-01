#pragma once

/// \file
/// \brief This file contains the game lifecycle and logics.
/// There are 5 important functions:
/// `GameInit`, `GameInput`, `GameUpdate`, `GameTerminate`, and
/// the most important: `GameLifecycle`.
/// Please read the corresponding comments to understand their relationships.

//
//
//
//
//
#include "Config.h"
#include "Renderer.h"

#include <time.h>

typedef struct {
  char keyHit; // The keyboard key hit by the player at this frame.
} Game;

// The game singleton.
static Game game;

// The keyboard key "ESC".
static const char keyESC = '\033';

//
//
//
/// \brief Configure the scene (See `Scene.h`) with `config` (See `Config.h`), and
/// perform initializations including:
/// 1. Terminal setup.
/// 2. Memory allocations.
/// 3. Map and object generations.
/// 4. Rendering of the initialized scene.
///
/// \note This function should be called at the very beginning of `GameLifecycle`.
void GameInit(void) {
  // Setup terminal.
  TermSetupGameEnvironment();
  TermClearScreen();

  // Configure scene.
  map.size = config.mapSize;
  int nEnemies = config.nEnemies;
  int nSolids = config.nSolids;
  int nWalls = config.nWalls;

  // 初始化场景.
  RegInit(regTank);
  RegInit(regBullet);

  map.flags = (Flag *)malloc(sizeof(Flag) * map.size.x * map.size.y);
  for (int y = 0; y < map.size.y; ++y)
    for (int x = 0; x < map.size.x; ++x) {
      Vec pos = {x, y};

      Flag flag = eFlagNone;
      if (x == 0 || y == 0 || x == map.size.x - 1 || y == map.size.y - 1) // 确立边界为墙壁，无法穿过无法破坏
        flag = eFlagSolid;

      map.flags[Idx(pos)] = flag; // 其他地方都是空地
    }
  for (int i = 0; i < nWalls; i++) { // 随机添加墙壁，数量由config.nWalls决定
    Vec pos = RandPos();
    if (judge3x3(pos)) { // 只能在空地上添加墙壁
      for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
          Vec pos_copy = pos;
          pos_copy.x = pos.x + i;
          pos_copy.y = pos.y + j; // 设置为墙壁
          map.flags[Idx(pos_copy)] = eFlagWall;
        }
      }
    }
  }
  for (int i = 0; i < nSolids; i++) { // 随机添加坚固物体，数量由config.nSolids决定
    Vec pos = RandPos();
    if (judge3x3(pos)) { // 只能在空地上添加坚固物体
      for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
          Vec pos_copy = pos;
          pos_copy.x = pos.x + i;
          pos_copy.y = pos.y + j;
          map.flags[Idx(pos_copy)] = eFlagSolid; // 设置为墙壁
        }
      }
    }
  }
  { // 此为玩家操控的tank
    Tank *tank = RegNew(regTank);
    tank->pos = (Vec){6, 2}; // 玩家坦克的初始位置，(x, y) 代表第x列第y行
    tank->dir = eDirPO;      // 初始运动方向
    tank->color = TK_GREEN;  // 玩家坦克的颜色，绿色
    tank->isPlayer = true;   // 是否为玩家坦克，true代表玩家坦克
  }
  // Initialize renderer.
  renderer.csPrev = (char *)malloc(sizeof(char) * map.size.x * map.size.y);
  renderer.colorsPrev = (Color *)malloc(sizeof(Color) * map.size.x * map.size.y);
  renderer.cs = (char *)malloc(sizeof(char) * map.size.x * map.size.y);
  renderer.colors = (Color *)malloc(sizeof(Color) * map.size.x * map.size.y);

  for (int i = 0; i < map.size.x * map.size.y; ++i) {
    renderer.csPrev[i] = renderer.cs[i] = ' ';
    renderer.colorsPrev[i] = renderer.colors[i] = TK_NORMAL;
  }

  // Render scene.
  for (int y = 0; y < map.size.y; ++y)
    for (int x = 0; x < map.size.x; ++x) {
      Vec pos = {x, y};
      RdrPutChar(pos, map.flags[Idx(pos)], TK_AUTO_COLOR);
    }
  RdrRender();
  RdrFlush();
}

//
//
//
/// \brief Read input from the player.
///
/// \note This function should be called in the loop of `GameLifecycle` before `GameUpdate`.
void GameInput(void) {
  game.keyHit = kbhit_t() ? (char)getch_t() : '\0';
}

//
//
//
/// \brief Perform all tasks required for a frame update, including:
/// 1. Game logics of `Tank`s, `Bullet`s, etc.
/// 2. Rerendering all objects in the scene.
///
/// \note This function should be called in the loop of `GameLifecycle` after `GameInput`.
void GameUpdate(void) {
  RdrClear();

  // TODO: You may need to delete or add codes here.
  // 这里目前是更新tank的移动方向
  for (RegIterator it = RegBegin(regTank); it != RegEnd(regTank); it = RegNext(it)) {
    Tank *tank = RegEntry(regTank, it);
    // printf("%d\n", (int)RegSize(regTank)); // 输出当前tank的数量，便于调试
    if (game.keyHit == 'w' && tank->isPlayer) { // 玩家按w键向上移动
      tank->dir = eDirOP;                       // 朝上
      ++tank->pos.y;
      if (map.flags[Idx(tank->pos) + map.size.x] != eFlagNone ||
          map.flags[Idx(tank->pos) + map.size.x - 1] != eFlagNone ||
          map.flags[Idx(tank->pos) + map.size.x + 1] != eFlagNone)
        --tank->pos.y;
    } else if (game.keyHit == 's' && tank->isPlayer) { // 玩家按s键向下移动
      tank->dir = eDirON;                              // 朝下
      --tank->pos.y;
      if (map.flags[Idx(tank->pos) - map.size.x] != eFlagNone ||
          map.flags[Idx(tank->pos) - map.size.x - 1] != eFlagNone ||
          map.flags[Idx(tank->pos) - map.size.x + 1] != eFlagNone)
        ++tank->pos.y;
    } else if (game.keyHit == 'a' && tank->isPlayer) { // 玩家按a键向左移动
      tank->dir = eDirNO;                              // 朝左
      --tank->pos.x;
      if (map.flags[Idx(tank->pos) - 1] != eFlagNone || map.flags[Idx(tank->pos) - 1 - map.size.x] != eFlagNone ||
          map.flags[Idx(tank->pos) + map.size.x - 1] != eFlagNone)
        ++tank->pos.x;
    } else if (game.keyHit == 'd' && tank->isPlayer) { // 玩家按d键向右移动
      tank->dir = eDirPO;                              // 朝右
      ++tank->pos.x;
      if (map.flags[Idx(tank->pos) + 1] != eFlagNone || map.flags[Idx(tank->pos) + 1 - map.size.x] != eFlagNone ||
          map.flags[Idx(tank->pos) + map.size.x + 1] != eFlagNone)
        --tank->pos.x;
    }
    if (game.keyHit == 'k' && tank->isPlayer) {
      { // 此为player坦克射出的子弹
        Bullet *bullet = RegNew(regBullet);
        bullet->pos = tank->pos;
        bullet->dir = tank->dir;
        bullet->color = TK_BLUE;
        bullet->isPlayer = true;
        if (bullet->dir == eDirOP) // 上
          bullet->pos.y += 1;
        else if (bullet->dir == eDirON) // 下
          bullet->pos.y -= 1;
        else if (bullet->dir == eDirNO) // 左
          bullet->pos.x -= 1;
        else if (bullet->dir == eDirPO) // 右
          bullet->pos.x += 1;
      }
    }
  }
  // 接下来写子弹的更新逻辑
  for (RegIterator it = RegBegin(regBullet); it != RegEnd(regBullet); it = RegNext(it)) {
    Bullet *bullet = RegEntry(regBullet, it);
    if (bullet->dir == eDirOP) {
      ++bullet->pos.y;
    } else if (bullet->dir == eDirON) {
      --bullet->pos.y;
    } else if (bullet->dir == eDirNO) {
      --bullet->pos.x;
    } else if (bullet->dir == eDirPO) {
      ++bullet->pos.x;
    }
    if (map.flags[Idx(bullet->pos)] != eFlagNone) {
      if (map.flags[Idx(bullet->pos)] == eFlagWall) {
        map.flags[Idx(bullet->pos)] = eFlagNone;
        RdrClear();
      }
      RegDelete(bullet);
    }
  }
  RdrRender();
  RdrFlush();
}

//
//
//
/// \brief Terminate the game and free all the resources.
///
/// \note This function should be called at the very end of `GameLifecycle`.
void GameTerminate(void) {
  while (RegSize(regTank) > 0)
    RegDelete(RegEntry(regTank, RegBegin(regTank)));

  while (RegSize(regBullet) > 0)
    RegDelete(RegEntry(regBullet, RegBegin(regBullet)));

  free(map.flags);

  free(renderer.csPrev);
  free(renderer.colorsPrev);
  free(renderer.cs);
  free(renderer.colors);

  TermClearScreen();
}

//
//
//
/// \brief Lifecycle of the game, defined by calling the 4 important functions:
/// `GameInit`, `GameInput`, `GameUpdate`, and `GameTerminate`.
///
/// \note This function should be called by `main`.
void GameLifecycle(void) {
  GameInit(); // 游戏初始化操作，必须要进行

  double frameTime = (double)1000 / (double)config.fps; // 帧率，每帧的时间间隔，单位为毫秒
  clock_t frameBegin = clock();                         // 记录程序开始运行时间

  while (true) {
    // printf(TK_TEXT("Hello World%c\n", TK_YELLOW), '!');

    GameInput(); // 玩家输入
    if (game.keyHit == keyESC)
      break; // 玩家按Esc可以直接退出

    GameUpdate(); // 这里实现逻辑上更新

    while (((double)(clock() - frameBegin) / CLOCKS_PER_SEC) * 1000.0 < frameTime - 0.5)
      Daze();
    frameBegin = clock();
  }

  GameTerminate();
}
