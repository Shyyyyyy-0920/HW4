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
          map.flags[Idx(Add(pos, (Vec){i, j}))] = eFlagWall;
        }
      }
    }
  }
  for (int i = 0; i < nSolids; i++) { // 随机添加坚固物体，数量由config.nSolids决定
    Vec pos = RandPos();
    if (judge3x3(pos)) { // 只能在空地上添加坚固物体
      for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
          map.flags[Idx(Add(pos, (Vec){i, j}))] = eFlagSolid; // 设置为墙壁
        }
      }
    }
  }
  { // 此为玩家操控的tank
    Tank *tank = RegNew(regTank);
    tank->pos = (Vec){2, 2}; // 玩家坦克的初始位置，(x, y) 代表第x列第y行
    tank->dir = eDirPO;      // 初始运动方向
    tank->color = TK_GREEN;  // 玩家坦克的颜色，绿色
    tank->isPlayer = true;   // 是否为玩家坦克，true代表玩家坦克
    tank->canmove = true;    // 用于鉴定此此能否移动
    tank->canshoot = true;   // 用于人机的间隔开火
  }
  { // 此为敌人tank
    Tank *tank_enemy = RegNew(regTank);
    tank_enemy->pos = (Vec){10, 12}; // 玩家坦克的初始位置，(x, y) 代表第x列第y行
    tank_enemy->dir = eDirPO;        // 初始运动方向
    tank_enemy->color = TK_RED;      // 玩家坦克的颜色，绿色
    tank_enemy->isPlayer = false;    // 是否为玩家坦克，true代表玩家坦克
    tank_enemy->canmove = true;      // 用于鉴定此此能否移动
    tank_enemy->canshoot = true;     // 用于人机的间隔开火
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
  // 这里目前是更新tank(不管是玩家还是人机)的移动方向
  for (RegIterator it = RegBegin(regTank); it != RegEnd(regTank); it = RegNext(it)) {
    Tank *tank = RegEntry(regTank, it);
    // 先把更新前的tank位置给恢复了
    for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
        map.flags[Idx(Add(tank->pos, (Vec){i, j}))] = eFlagNone;
        RdrClear(); // 这个非常重要，否则无法删除之前画的东西！！！！（吸取教训）
      }
    }
    int randintIndex = rand() % 4; // 人机tank的四个方向移动
    // printf("%d\n", (int)RegSize(regTank)); // 输出当前tank的数量，便于调试
    if (((game.keyHit == 'w' || game.keyHit == 'W') && tank->isPlayer) ||
        (!tank->isPlayer && randintIndex == 0 && tank->canmove)) { // 玩家按w键向上移动或者人机tank向上移动
      tank->dir = eDirOP;                                          // 朝上
      if (!tank->isPlayer)                                         // 如果这是人机坦克，那么就不能运动了
        tank->canmove = false;
      ++tank->pos.y;
      if (map.flags[Idx(tank->pos) + map.size.x] != eFlagNone ||
          map.flags[Idx(tank->pos) + map.size.x - 1] != eFlagNone ||
          map.flags[Idx(tank->pos) + map.size.x + 1] != eFlagNone)
        --tank->pos.y; // 障碍物判定
    } else if (((game.keyHit == 's' || game.keyHit == 'S') && tank->isPlayer) ||
               (!tank->isPlayer && randintIndex == 1 && tank->canmove)) { // 玩家按s键向下移动或者人机tank向下移动
      tank->dir = eDirON;                                                 // 朝下
      if (!tank->isPlayer)                                                // 如果这是人机坦克，那么就不能运动了
        tank->canmove = false;
      --tank->pos.y;
      if (map.flags[Idx(tank->pos) - map.size.x] != eFlagNone ||
          map.flags[Idx(tank->pos) - map.size.x - 1] != eFlagNone ||
          map.flags[Idx(tank->pos) - map.size.x + 1] != eFlagNone)
        ++tank->pos.y;
    } else if (((game.keyHit == 'a' || game.keyHit == 'A') && tank->isPlayer) ||
               (!tank->isPlayer && randintIndex == 2 && tank->canmove)) { // 玩家按a键向左移动或者人机坦克想左移动
      tank->dir = eDirNO;                                                 // 朝左
      if (!tank->isPlayer)                                                // 如果这是人机坦克，那么就不能运动了
        tank->canmove = false;
      --tank->pos.x;
      if (map.flags[Idx(tank->pos) - 1] != eFlagNone || map.flags[Idx(tank->pos) - 1 - map.size.x] != eFlagNone ||
          map.flags[Idx(tank->pos) + map.size.x - 1] != eFlagNone)
        ++tank->pos.x;
    } else if (((game.keyHit == 'd' || game.keyHit == 'D') && tank->isPlayer) ||
               (!tank->isPlayer && randintIndex == 3 && tank->canmove)) { // 玩家按d键向右移动或者人机tank想右移动
      tank->dir = eDirPO;                                                 // 朝右
      if (!tank->isPlayer)                                                // 如果这是人机坦克，那么就不能运动了
        tank->canmove = false;
      ++tank->pos.x;
      if (map.flags[Idx(tank->pos) + 1] != eFlagNone || map.flags[Idx(tank->pos) + 1 - map.size.x] != eFlagNone ||
          map.flags[Idx(tank->pos) + map.size.x + 1] != eFlagNone)
        --tank->pos.x;
    }
    if (((game.keyHit == 'k' || game.keyHit == 'K') && tank->isPlayer) || (!tank->isPlayer && tank->canshoot)) {
      { // 此为player坦克射出的子弹或者人机tank发出子弹
        Bullet *bullet = RegNew(regBullet);
        bullet->pos = tank->pos;
        bullet->dir = tank->dir;
        if (tank->isPlayer) {
          bullet->color = TK_BLUE;
          bullet->isPlayer = true;
        } else {
          bullet->color = TK_RED;
          bullet->isPlayer = false;
          tank->canshoot = false; // 一发子弹过后禁止射击了
        }
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
    // 试一下把更新后tank的位置周围打上标记
    for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
        map.flags[Idx(Add(tank->pos, (Vec){i, j}))] = eFlagTank;
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
    if (map.flags[Idx(bullet->pos)] != eFlagNone) { // 做子弹碰撞消失的判断，同时可以消灭可消灭物
      if (map.flags[Idx(bullet->pos)] == eFlagWall) {
        map.flags[Idx(bullet->pos)] = eFlagNone;
        RdrClear();
      } else if (map.flags[Idx(bullet->pos)] == eFlagTank) {
        // 这里写消灭坦克的逻辑，要保证只要击中t，那么这一块坦克就消失了
        Vec flag_pos = {0, 0};
        for (RegIterator it = RegBegin(regTank); it != RegEnd(regTank); it = RegNext(it)) { // 先枚举所有tank类
          Tank *tank = RegEntry(regTank, it);
          for (int i = -1; i <= 1; i++) {
            for (
                int j = -1; j <= 1;
                j++) { // 判断以那个tank中心的pos周围九格是否存在与子弹pos重合的，要是有，那么需要把那一块变为空地，并且
                       // 还要把tank和子弹从tank类里删除
              if (Eq(Add(tank->pos, (Vec){i, j}), bullet->pos)) {
                map.flags[Idx(tank->pos)] = eFlagNone;
                RdrClear();
                RegDelete(tank);
                flag_pos = tank->pos;
                break;
              }
            }
          }
          if (flag_pos.x != 0 && flag_pos.y != 0) {
            for (int i = -1; i <= 1; i++) {
              for (int j = -1; j <= 1; j++) {
                map.flags[Idx(Add(tank->pos, (Vec){i, j}))] = eFlagNone; // 删去这个周围的所有T标志物
                RdrClear();
              }
            }
            flag_pos = (Vec){0, 0}; // 重新归位
          }
        }
      }
      // 上面是消灭tank的逻辑
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
  clock_t aitank_move_begin = clock();
  while (true) {
    // printf(TK_TEXT("Hello World%c\n", TK_YELLOW), '!');

    GameInput(); // 玩家输入
    if (game.keyHit == keyESC)
      break; // 玩家按Esc可以直接退出

    GameUpdate(); // 这里实现逻辑上更新

    while (((double)(clock() - frameBegin) / CLOCKS_PER_SEC) * 1000.0 < frameTime - 0.5)
      Daze();
    frameBegin = clock();
    // 最后要将人机tank的canmove恢复
    if ((double)(clock() - aitank_move_begin) > 600) {
      for (RegIterator it = RegBegin(regTank); it != RegEnd(regTank); it = RegNext(it)) {
        Tank *tank = RegEntry(regTank, it);
        if (!tank->isPlayer) {
          tank->canmove = true;  // 允许运动
          tank->canshoot = true; // 允许射击
        }
      }
      aitank_move_begin = clock();
    }
  }
  GameTerminate();
}
