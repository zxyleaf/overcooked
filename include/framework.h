#ifndef FRAMEWORK
#define FRAMEWORK

#include <stack>
#include <string>
#include <vector>
#include <enum.h>
#include <queue>
struct Ingredient
{
    int x, y, price;
    std::string name;
    int direction;
    std::pair<double, double> availableLoc;
};

struct Recipe
{
    int time;
    std::string nameBefore, nameAfter;
    std::string kind;
};

struct Order {
    int validFrame;
    int price;
    int frequency;
    int PlayerId;
    std::vector<std::string> recipe;
};

struct Mission {
    std::vector<std::string> recipe;
    std::stack<std::string> Places;
    std::stack<std::pair<double, double>> targetLoc;
    std::stack<std::pair<int, int>> targetCubeLoc;
    std::stack<PlayerAction> action;
    int shouldInterat;
    int finish;
    int allDone;
};
struct Player {
    double x, y;
    double X_Velocity;
    double Y_Velocity;
    int live;
    int OrderId;
    int OrderIdx;
    int finish;
    int toEnd;
    int over;
    int stay;
    int sum;
    std::string targetPlace;
    std::pair<double, double> targetLocation;
    PlayerDir targetDir;
    PlayerDir OnTheWay;
    ContainerKind containerKind;
    std::vector<std::string> entity;
    Mission mission;
    std::queue<std::pair<int, int>> route;
};

struct Entity
{
    double x, y;
    ContainerKind containerKind;
    std::vector<std::string> entity;
    int currentFrame, totalFrame;
    int PlayerId;
    int sum;
};


struct node {
    int x;
    int y;
    int id;
    int parent = 0;
    node(int tx, int ty, int tid) {
      x = tx;
      y = ty;
      id = tid;
    }
};

/* 初始化时的读入。 */
void init_read();

/* 相关预处理 */
void init();

/* 每一帧的读入；返回：是否跳帧。 */
bool frame_read(int nowFrame);

std::pair<std::string, std::string> dealWithAction();
PlayerDir dealWithDir(int id, double targetX, double targetY, double tempX, double tempY);

std::pair<double, double> findValidLocation(int x, int y);
std::string addTarget(int id, std::pair<double, double> tempTarget, int x, int y);
#endif