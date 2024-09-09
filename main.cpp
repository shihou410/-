#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
const int WINDOW_WIDTH = 320;
const int WINDOW_HEIGHT = 480;

const int BLOCK_WIDTH = 18;
const int BLOCK_HEIGHT = 18;

std::string src_bg = "image/background.png";
std::string src_frame = "image/frame.png";
std::string src_blocks = "image/tiles.png";

SDL_Window *gWindow = nullptr;
SDL_Renderer *gRenderer = nullptr;

SDL_Texture *tex_bg = nullptr;
SDL_Texture *tex_frame = nullptr;
SDL_Texture *tex_blocks = nullptr;

SDL_Event eventer;
bool isRun = true;
void init();
void handleInput();
void update();
void render();
void clear();

enum class BlockColor {
  lan = 0, // 蓝色
  zi,      // 紫色
  hong,    // 红
  lv,      // 绿
  huang,   // 黄
  qing,    // 橙
  ding,    // 青
  cheng,   // 锭
  none,    // 无
};

enum class BlockType {
  Z,    // z型
  TU,   // 土型
  TIAN, // 田子型
  YI,   // 一字型
  L,    // L型
};

const int BOX_MAX = 4;

int Z[BOX_MAX][2] = {{0, 0}, {1, 0}, {1, 1}, {2, 1}};
int TU[BOX_MAX][2] = {{1, 0}, {0, 1}, {1, 1}, {2, 1}};
int TIAN[BOX_MAX][2] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
int YI[BOX_MAX][2] = {{0, 0}, {0, 1}, {0, 2}, {0, 3}};
int L[BOX_MAX][2] = {{0, 0}, {0, 1}, {0, 2}, {1, 2}};

int (*currentBlock)[2] = L;
BlockColor currentBlockColor = BlockColor::cheng;

BlockColor map[20][10] = {BlockColor::none};

int block_pos[2] = {0, 0};

int start_pos[2] = {28, 30};

// 创建随机数生成器
std::mt19937 generator;
// 创建一个均匀分布的随机数生成器，范围从 1 到 10
std::uniform_int_distribution<int> block_type_randomtor(0, 4);
std::uniform_int_distribution<int> block_color_randomtor(0, 7);

// 创建方块
void createBlock();

// 可放置
bool onPut(int bx, int by);
// 可移动
bool onMove(int bx, int by);

void rotation();

// 消除一行
void destroyLine();

int landCount = 0;
float landSpeed = 1.2f; // 秒每格
Uint32 startTick = 0;
float gameTime = 0;
int main(int, char **) {
  init();
  startTick = SDL_GetTicks();
  createBlock();
  while (isRun) {
    gameTime = (SDL_GetTicks() - startTick) / 1000.0f;
    handleInput();
    if (gameTime >= landSpeed) {
      update();
      startTick = SDL_GetTicks();
    }
    render();
  }

  clear();

  std::cout << "Hello, from game1!\n";
}

void init() {

  SDL_Init(SDL_INIT_EVERYTHING);
  IMG_Init(IMG_INIT_PNG);

  gWindow = SDL_CreateWindow("俄罗斯方块", SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
                             WINDOW_HEIGHT, SDL_WINDOW_SHOWN);

  if (gWindow == nullptr) {
    std::cout << "窗体创建失败: " << SDL_GetError() << std::endl;
    exit(-1);
  }

  gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
  if (gRenderer == nullptr) {
    std::cout << "渲染器创建失败: " << SDL_GetError() << std::endl;
    exit(-1);
  }

  tex_bg = IMG_LoadTexture(gRenderer, src_bg.c_str());
  if (tex_bg == nullptr) {
    std::cout << "背景创建失败: " << IMG_GetError() << std::endl;
    exit(-1);
  }
  tex_frame = IMG_LoadTexture(gRenderer, src_frame.c_str());
  if (tex_frame == nullptr) {
    std::cout << "背景创建失败: " << IMG_GetError() << std::endl;
    exit(-1);
  }
  tex_blocks = IMG_LoadTexture(gRenderer, src_blocks.c_str());
  if (tex_blocks == nullptr) {
    std::cout << "背景创建失败: " << IMG_GetError() << std::endl;
    exit(-1);
  }

  for (int i = 0; i < 20; i++) {
    for (int j = 0; j < 10; j++) {
      map[i][j] = BlockColor::none;
    }
  }
}

void handleInput() {
  while (SDL_PollEvent(&eventer)) {
    if (eventer.type == SDL_QUIT) {
      isRun = false;
    }

    if (eventer.type == SDL_KEYDOWN) {
      if (eventer.key.keysym.sym == SDLK_LEFT) {
        int x = block_pos[0] - 1;
        if (!onMove(x, block_pos[1])) {
          block_pos[0] = x;
        }
      }
      if (eventer.key.keysym.sym == SDLK_RIGHT) {
        int x = block_pos[0] + 1;
        if (!onMove(x, block_pos[1])) {
          block_pos[0] = x;
        }
      }
      if (eventer.key.keysym.sym == SDLK_SPACE) {
        rotation();
      }
      if (eventer.key.keysym.sym == SDLK_DOWN) {
        landSpeed = 0.1f;
      }
    }
  }
}
void update() {

  int y = block_pos[1] + 1;
  if (onPut(block_pos[0], y)) {
    for (int i = 0; i < BOX_MAX; i++) {
      int x = currentBlock[i][0] + block_pos[0];
      int y = currentBlock[i][1] + block_pos[1];
      map[y][x] = currentBlockColor;
    }
    std::cout << "落下了：" << int(currentBlockColor) << std::endl;
    destroyLine();
    createBlock();
  } else {
    block_pos[1] = y;
  }
}
void render() {
  SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
  SDL_RenderClear(gRenderer);

  // 绘制背景
  SDL_RenderCopy(gRenderer, tex_bg, nullptr, nullptr);

  // 绘制方块
  for (int i = 0; i < BOX_MAX; i++) {
    int x = currentBlock[i][0] + block_pos[0];
    int y = currentBlock[i][1] + block_pos[1];

    SDL_Rect srcrect = {int(currentBlockColor) * BLOCK_WIDTH, 0, BLOCK_WIDTH,
                        BLOCK_HEIGHT};
    SDL_Rect dstrect = {x * BLOCK_WIDTH + start_pos[0],
                        y * BLOCK_HEIGHT + start_pos[1], BLOCK_WIDTH,
                        BLOCK_HEIGHT};
    SDL_RenderCopy(gRenderer, tex_blocks, &srcrect, &dstrect);
  }

  // 绘制地图
  for (int i = 0; i < 20; i++) {
    for (int j = 0; j < 10; j++) {
      int index = int(map[i][j]);
      if (map[i][j] != BlockColor::none) {
        SDL_Rect srcrect = {index * BLOCK_WIDTH, 0, BLOCK_WIDTH, BLOCK_HEIGHT};
        SDL_Rect dstrect = {j * BLOCK_WIDTH + start_pos[0],
                            i * BLOCK_HEIGHT + start_pos[1], BLOCK_WIDTH,
                            BLOCK_HEIGHT};

        SDL_RenderCopy(gRenderer, tex_blocks, &srcrect, &dstrect);
      }
    }
  }

  // 绘制框框
  SDL_RenderCopy(gRenderer, tex_frame, nullptr, nullptr);

  SDL_RenderPresent(gRenderer);
}

void clear() {
  SDL_DestroyTexture(tex_bg);
  SDL_DestroyTexture(tex_frame);
  SDL_DestroyTexture(tex_blocks);
  SDL_DestroyRenderer(gRenderer);
  SDL_DestroyWindow(gWindow);
  IMG_Quit();
  SDL_Quit();
}

void createBlock() {
  landCount++;
  int i = block_type_randomtor(generator);
  if (int(BlockType::L) == i) {
    currentBlock = L;
  }
  if (int(BlockType::TIAN) == i) {
    currentBlock = TIAN;
  }
  if (int(BlockType::TU) == i) {
    currentBlock = TU;
  }
  if (int(BlockType::YI) == i) {
    currentBlock = YI;
  }
  if (int(BlockType::Z) == i) {
    currentBlock = Z;
  }

  currentBlockColor = BlockColor(block_color_randomtor(generator));
  block_pos[0] = 0;
  block_pos[1] = 0;
  landSpeed = 1.2f;
}

bool onMove(int bx, int by) {
  for (int i = 0; i < BOX_MAX; i++) {
    int x = currentBlock[i][0] + bx;
    int y = currentBlock[i][1] + by;

    if (map[y][x] != BlockColor::none || x < 0 || x >= 10)
      return true;
  }
  return false;
}

bool onPut(int bx, int by) {
  for (int i = 0; i < BOX_MAX; i++) {
    int x = currentBlock[i][0] + bx;
    int y = currentBlock[i][1] + by;
    if (map[y][x] != BlockColor::none || y >= 20)
      return true;
  }
  return false;
}

void destroyLine() {

  for (int i = 0; i < 20; i++) {
    bool temp = true;
    for (int j = 0; j < 10; j++) {
      if (map[i][j] == BlockColor::none) {
        temp = false;
        break;
      }
    }
    if (temp) {
      for (int j = 0; j < 10; j++) {
        map[i][j] = BlockColor::none;
        for (int t = i; t > 0; t--) {
          map[t][j] = map[t - 1][j];
        }
      }
    }
  }
}

void rotation() {
  for (int i = 0; i < BOX_MAX; i++) {
    int x = currentBlock[i][0];
    int y = currentBlock[i][1];
    currentBlock[i][0] = 2 - y;
    currentBlock[i][1] = x;
  }
}