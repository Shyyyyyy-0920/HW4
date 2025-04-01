#pragma once
// 这里包括了一些实体类，比如tank，bullet，enemy，map这些类的设置
// 同时地图是用一维数组来存储的，
// 这样可以节省空间，同时也方便我们进行一些操作，比如说碰撞检测等操作
// MoveCursor函数用来移动光标到指定的位置
// RandPos函数用来随机生成一个在地图范围内的位置
// 新增一个函数，只需要传入中心点坐标，即可判断周围3x3范围内是否都是空地，如果是就返回1，否则就返回0
// 新增一个函数，用于检测碰撞判断，传入两个物体（非子弹）的坐标，进行碰撞判定,如果碰撞就返回1，否则就返回0
/// \file
/// \brief This file contains the definitions, singletons, and functions of
/// the game-related types, such as `Tank`, `Bullet`, and `Map`.
///
/// The powerful "registries" are intensively used to make our life easier.
/// Please read the comments of `Registry.h` before continue.

//
//
//
//
//
#include "Registry.h"
#include "Terminal.h"

/// \brief Enums of the 2D directions.
typedef enum {
  eDirNN, // Left-down.
  eDirON, // Down.
  eDirPN, // Right-down.

  eDirNO, // Left.
  eDirOO, // Center.
  eDirPO, // Right.

  eDirNP, // Left-up.
  eDirOP, // Up.
  eDirPP, // Right-up.

  eDirInvalid, // Invalid.
} Dir;

/// \brief Enums of the map flags.
typedef enum {
  eFlagNone = ' ',  // Nothing here, passable.
  eFlagSolid = '%', // Solid, impassable and non-destructible.
  eFlagWall = '#',  // Wall, impassable but destructible.

  eFlagTank = 'T', // Tank, impassable but destructible.

  eFlagInvalid = '\0', // Invalid.
} Flag;

/// \example It is easy to create or delete a `Tank` with the help of registries, see `Registry.h`.
/// ```c
/// Tank *tank = RegNew(regTank); //! `malloc` is called here.
/// tank->pos = ...
/// ...
/// RegDelete(tank); //! `free` is called here.
/// ```
typedef struct {
  TK_REG_AUTH;   // Authorize `Tank` to make it compatible with registries, see `Registry.h`.
  Vec pos;       // Position.
  Dir dir;       // Direction.
  Color color;   // Color of the tank and its bullets.
  bool isPlayer; // Whether this tank is player or enemy.
  bool canmove;  // 新增加，用于控制人机tank一次只能行动一次
  bool canshoot; // 新增加，用于控制人机tank一次只能射一发子弹
} Tank;

/// \example It is easy to create or delete a `Bullet` with the help of registries, see `Registry.h`.
/// ```c
/// Bullet *bullet = RegNew(regBullet); //! `malloc` is called here.
/// bullet->pos = ...
/// ...
/// RegDelete(bullet); //! `free` is called here.
/// ```
typedef struct {
  TK_REG_AUTH;   // Authorize `Bullet` to make it compatible with registries, see `Registry.h`.
  Vec pos;       // Position.
  Dir dir;       // Direction.
  Color color;   // Color.
  bool isPlayer; // Whether this bullet was shot by player or enemy.
} Bullet;

typedef struct {
  // Width (x) and height (y) of the map.
  Vec size;
  // The flags of every positions of the map.
  //! Note that only static flags such as `eFlagNone` and `eFlagSolid` are included here.
  //! Dynamic flags such as `eFlagTank` are not included and will be set to `eFlagNone`.
  Flag *flags;
} Map;

//
//
//
// Define a registry for `Tank`, see `Registry.h`.
static TK_REG_DEF(Tank, regTank);

// Define a registry for `Bullet`, see `Registry.h`.
static TK_REG_DEF(Bullet, regBullet);

// The map singleton.
static Map map;

//
//
//
/// \brief Convert `pos` to its linearized index of the map.
///
/// \example ```c
/// Vec pos = {x, y};
/// Flag flag = map.flags[Idx(pos)];
/// ```
int Idx(Vec pos) {
  return pos.x + pos.y * map.size.x;
}

/// \brief Move cursor to `pos`.
///
/// \example ```c
/// MoveCursor(pos);
/// printf(TK_TEXT("Hello World!\n", TK_RED));
/// ```
void MoveCursor(Vec pos) {
  int row = map.size.y - 1 - pos.y;
  int col = pos.x * 2;
  TermMoveCursor(row, col);
}

/// \brief Randomly generate a position in map.
Vec RandPos(void) {
  return RandVec(map.size);
}

int judge3x3(Vec pos) {
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      if (map.flags[Idx(Add(pos, (Vec){i, j}))] != eFlagNone)
        return 0;
    }
  }
  return 1;
}
