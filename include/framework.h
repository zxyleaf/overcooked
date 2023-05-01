#ifndef FRAMEWORK
#define FRAMEWORK

#include <string>
#include <vector>
#include <enum.h>

struct Ingredient
{
    int x, y, price;
    std::string name;
    std::pair<double, double> availableLoc;
};

struct Recipe
{
    int time;
    std::string nameBefore, nameAfter;
    std::string kind;
};

struct Order
{
    int validFrame;
    int price;
    int frequency;
    int PlayerId;
    std::vector<std::string> recipe;
};

struct Player
{
    double x, y;
    double X_Velocity;
    double Y_Velocity;
    int live;
    int OrderId;
    int OrderIdx;
    std::pair<double, double> targetLocation;
    PlayerDir OnTheWay;
    ContainerKind containerKind;
    std::vector<std::string> entity;
};

struct Entity
{
    double x, y;
    ContainerKind containerKind;
    std::vector<std::string> entity;
    int currentFrame, totalFrame;
    int sum;
};


/* 初始化时的读入。 */
void init_read();

/* 相关预处理 */
void init();

/* 每一帧的读入；返回：是否跳帧。 */
bool frame_read(int nowFrame);

std::pair<std::string, std::string> dealWithAction();
PlayerDir dealWithDir(double targetX, double targetY, double tempX, double tempY);

std::pair<double, double> findValidLocation(int x, int y);
#endif