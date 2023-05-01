#include <enum.h>
#include <framework.h>
#include <string>
#include <iostream>
#include <sstream>
#include <string>
#include <cassert>
#include <vector>
#include <string>
#include <cmath>

const int INF = 0x3f3f3f;
const double esp = 5e-2;
/* 按照读入顺序定义 */
int width, height;
char Map[20 + 5][20 + 5];
int IngredientCount;
struct Ingredient Ingredient[20 + 5];
int recipeCount;
struct Recipe Recipe[20 + 5];
int totalTime, randomizeSeed, totalOrderCount;
struct Order totalOrder[20 + 5];
int orderCount;
struct Order Order[20 + 5];
int k;
struct Player Players[2 + 5];
int entityCount;
struct Entity Entity[20 + 5];
int remainFrame, Fund;
int totalPlateNum = 0, usedPlateNum = 0;
int StoveNum = 0, chopNum = 0;
int PlateNum = 0, dirtyPlateNum = 0;
std::pair<int, int> servicePlace_int, PlateReturn_int, sinkPlace_int;
std::pair<double, double> sinkPlace, plateRackPlace, PlateReturn, chopPlace[20 + 5], servicePlace, StovePlace[20 + 5];
int isWashing = 3;


void init_read()
{
    std::string s;
    std::stringstream ss;
    int frame;

    /* 读取初始地图信息 */
    std::getline(std::cin, s, '\0');
    ss << s;

    /* 若按照该读入，访问坐标(x, y)等价于访问Map[y][x],你可按照自己的习惯进行修改 */
    ss >> width >> height;
    std::cerr << "Map size: " << width << "x" << height << std::endl;
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            ss >> Map[i][j];

    /* 读入原料箱：位置、名字、以及采购单价 */
    ss >> IngredientCount;
    for (int i = 0; i < IngredientCount; i++)
    {
        ss >> s;
        assert(s == "IngredientBox");
        ss >> Ingredient[i].x >> Ingredient[i].y >> Ingredient[i].name >> Ingredient[i].price;
    }

    /* 读入配方：加工时间、加工前的字符串表示、加工容器、加工后的字符串表示 */
    ss >> recipeCount;
    for (int i = 0; i < recipeCount; i++)
    {
        ss >> Recipe[i].time >> Recipe[i].nameBefore >> Recipe[i].kind >> Recipe[i].nameAfter;
    }

    /* 读入总帧数、当前采用的随机种子、一共可能出现的订单数量 */
    ss >> totalTime >> randomizeSeed >> totalOrderCount;

    /* 读入订单的有效帧数、价格、权重、订单组成 */
    for (int i = 0; i < totalOrderCount; i++)
    {
        ss >> totalOrder[i].validFrame >> totalOrder[i].price >> totalOrder[i].frequency;
        getline(ss, s);
        std::stringstream tmp(s);
        while (tmp >> s)
            totalOrder[i].recipe.push_back(s);
    }

    /* 读入玩家信息：初始坐标 */
    ss >> k;
    assert(k == 2);
    for (int i = 0; i < k; i++)
    {
        ss >> Players[i].x >> Players[i].y;
        Players[i].containerKind = ContainerKind::None;
        Players[i].entity.clear();
        Players[i].OnTheWay = PlayerDir::None;
        Players[i].targetLocation = std::make_pair(Players[i].x, Players[i].y);
        Players[i].targetDir = PlayerDir::None;
        Players[i].OrderId = INF;
        Players[i].OrderIdx = 0;
        Players[i].finish = 0;
        Players[i].toEnd = 0;
        Players[i].over = 0;
        Players[i].stay = 0;
        Players[i].sum = 1;
    }

    /* 读入实体信息：坐标、实体组成 */
    ss >> entityCount;
    for (int i = 0; i < entityCount; i++)
    {
        ss >> Entity[i].x >> Entity[i].y >> s;
        Entity[i].entity.push_back(s);
        Entity[i].PlayerId = 3;
        if (s == "Plate") {
            totalPlateNum++;
        }
    }
}

bool frame_read(int nowFrame)
{
    std::string s;
    std::stringstream ss;
    int frame;
    std::getline(std::cin, s, '\0');
    ss.str(s);
    /*
      如果输入流中还有数据，说明游戏已经在请求下一帧了
      这时候我们应该跳过当前帧，以便能够及时响应游戏。
    */
    if (std::cin.rdbuf()->in_avail() > 0)
    {
        std::cerr << "Warning: skipping frame " << nowFrame
             << " to catch up with the game" << std::endl;
        return true;
    }
    ss >> s;
    assert(s == "Frame");
    int currentFrame;
    ss >> currentFrame;
    assert(currentFrame == nowFrame);
    ss >> remainFrame >> Fund;
    /* 读入当前的订单剩余帧数、价格、以及配方 */
    ss >> orderCount;
    for (int i = 0; i < orderCount; i++)
    {
        ss >> Order[i].validFrame >> Order[i].price;
        Order[i].recipe.clear();
        Order[i].PlayerId = 0;
        getline(ss, s);
        std::stringstream tmp(s);
        while (tmp >> s)
        {
            Order[i].recipe.push_back(s);
        }
    }
    ss >> k;
    assert(k == 2);
    /* 读入玩家坐标、x方向速度、y方向速度、剩余复活时间 */
    for (int i = 0; i < k; i++)
    {
        ss >> Players[i].x >> Players[i].y >> Players[i].X_Velocity >> Players[i].Y_Velocity >> Players[i].live;
        getline(ss, s);
        std::stringstream tmp(s);
        Players[i].containerKind = ContainerKind::None;
        Players[i].entity.clear();
        Players[i].sum = 1;
        while (tmp >> s)
        {
            /*
                若若该玩家手里有东西，则接下来一个分号，分号后一个空格，空格后为一个实体。
                以下是可能的输入（省略前面的输入）：
                 ;  : fish
                 ; @  : fish
                 ; @ Plate : fish
                 ; Plate
                 ; DirtyPlates 1
                ...
            */

            /* 若你不需要处理这些，可直接忽略 */
            if (s == ";" || s == ":" || s == "@" || s == "*")
                continue;

            /* 
                Todo: 其他容器
            */
            if (s == "Plate")
                Players[i].containerKind = ContainerKind::Plate;
            else if (s == "DirtyPlates") {
                Players[i].containerKind = ContainerKind::DirtyPlates;
                tmp >> Players[i].sum;
            }
            else
                Players[i].entity.push_back(s);
        }
    }

    ss >> entityCount;
    PlateNum = 0;
    dirtyPlateNum = 0;
    /* 读入实体坐标 */
    for (int i = 0; i < entityCount; i++)
    {
        ss >> Entity[i].x >> Entity[i].y;
        getline(ss, s);
        std::stringstream tmp(s);
        Entity[i].containerKind = ContainerKind::None;
        Entity[i].entity.clear();
        Entity[i].currentFrame = Entity[i].totalFrame = 0;
        Entity[i].sum = 1;
        while (tmp >> s)
        {
            /*
                读入一个实体，例子：
                DirtyPlates 2
                fish
                DirtyPlates 1 ; 15 / 180

            */

            /* 若你不需要处理这些，可直接忽略 */
            if (s == ":" || s == "@" || s == "*")
                continue;
            if (s == ";")
            {
                tmp >> Entity[i].currentFrame >> s >> Entity[i].totalFrame;
                assert(s == "/");
                break;
            }
            
            /* 
                Todo: 其他容器
            */
            if (s == "Plate")
                Entity[i].containerKind = ContainerKind::Plate, PlateNum++;
            else if (s == "DirtyPlates")
            {
                Entity[i].containerKind = ContainerKind::DirtyPlates, dirtyPlateNum++;
                tmp >> Entity[i].sum;
            }
            else
                Entity[i].entity.push_back(s);
        }
    }
    return false;
}

std::pair<double, double> findValidLocation(int y, int x) {
    std::pair<double, double> ret;
    if (x + 1 < width && getTileKind(Map[x + 1][y]) == TileKind::Floor) {
        ret = std::make_pair((double)y + 0.5, (double)x + 1.5);
    }
    else if (x - 1 >= 0 && getTileKind(Map[x - 1][y]) == TileKind::Floor) {
        ret = std::make_pair((double)y + 0.5, (double)x - 0.5);
    }
    else if (y + 1 < height && getTileKind(Map[x][y + 1]) == TileKind::Floor) {
        ret = std::make_pair((double)y + 1.5, (double)x + 0.5);
    }
    else if (y - 1 >= 0 && getTileKind(Map[x][y - 1]) == TileKind::Floor) {
        ret = std::make_pair((double)y - 0.5, (double)x + 0.5);
    }
    return ret;
}

void init() {
    for (int i = 0; i < IngredientCount; i++) {
        Ingredient[i].availableLoc = findValidLocation(Ingredient[i].x, Ingredient[i].y);
    }
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++) {
            if (getTileKind(Map[i][j]) == TileKind::ChoppingStation) {
                chopPlace[++chopNum] = findValidLocation(j, i);
            }
            else if (getTileKind(Map[i][j]) == TileKind::ServiceWindow) {
                servicePlace = findValidLocation(j, i);
                servicePlace_int = std::make_pair(j, i);
            }
            else if (getTileKind(Map[i][j]) == TileKind::Stove) {
                StovePlace[++StoveNum] = findValidLocation(j, i);
            }
            else if (getTileKind(Map[i][j]) == TileKind::PlateReturn) {
                PlateReturn = findValidLocation(j, i);
                PlateReturn_int = std::make_pair(j, i);
            }
            else if (getTileKind(Map[i][j]) == TileKind::Sink) {
                sinkPlace = findValidLocation(j, i);
                sinkPlace_int = std::make_pair(j, i);
            }
            else if (getTileKind(Map[i][j]) == TileKind::PlateRack) {
                plateRackPlace = findValidLocation(j, i);
            }
        }
}

int times = 0;
bool tooClose() {
    double x1 = Players[0].x;
    double y1 = Players[0].y;
    double x2 = Players[1].x;
    double y2 = Players[1].y;
    if (fabs(x1 - x2) <= (double )1 && fabs(y1 - y2) <= (double )1) {
        return true;
    }
    return false;
}
int adjust = 0;
std::pair<std::string, std::string> dealWithAction() {
    std::string ret[2];
    if (tooClose()) {
        times++;
        //std::cerr << "times: " << times << std::endl;
    }
    if (times >= 60) {
        ret[1] = "Move UR";
        ret[0] = "Move DL";
        adjust++;
        if (adjust == 18) {
            times = 0;
            adjust = 0;
        }
        return std::make_pair(ret[0], ret[1]);
    }
    for (int i = 0; i < k; i++) {
        if (Players[i].stay > 0) {
            if (Players[i].stay % 180 == 0) {
                 ret[i] = getAction(PlayerAction::Interact);
                 ret[i] += getDir(Players[i].targetDir);
            }
            else if (Players[i].stay % 180 == 1) {
                 usedPlateNum--;
                 ret[i] = "Move ";
            }
            else
                ret[i] = "Move ";
            Players[i].stay--;
            if (Players[i].stay == 0) {
                isWashing = 3;
            }
        }
        else if (Players[i].OnTheWay != PlayerDir::None) {
            ret[i] = getAction(PlayerAction::Move);
            PlayerDir tempDir = dealWithDir(Players[i].targetLocation.first, Players[i].targetLocation.second, Players[i].x, Players[i].y);
            ret[i] += getDir(tempDir);
            if (tempDir == PlayerDir::None) {
                Players[i].OnTheWay = PlayerDir::None;
                if (Players[i].finish == 1) {
                    Players[i].finish = 0;
                    Players[i].toEnd = 1;
                }
                if (Players[i].containerKind == ContainerKind::Plate && Players[i].toEnd == 1) {
                    Players[i].finish = 0;
                    Players[i].over = 1;
                    //std::cerr << "arrived!!!" << std::endl;
                }
                if (Players[i].containerKind == ContainerKind::DirtyPlates) {
                    ret[i] = ret[i] = getAction(PlayerAction::PutOrPick);
                    ret[i] += getDir(Players[i].targetDir);
                    Players[i].stay = 180 * Players[i].sum;
                    //std::cerr << "stay!!! " << Players[i].stay << std::endl;
                    //isWashing = 3;
                }
            }
        }
        else if (Players[i].OrderId == INF && dirtyPlateNum > 0 && isWashing == 3) {
            /* todo: 洗盘子 (暂时只有一个人) */
            ret[i] = addTarget(i, PlateReturn, PlateReturn_int.first, PlateReturn_int.second);
            isWashing = i;
        }
        else if (isWashing == i) {
            if (Players[i].containerKind == ContainerKind::DirtyPlates) {
                ret[i] = addTarget(i, sinkPlace, sinkPlace_int.first, sinkPlace_int.second);
            }
            else {
                ret[i] = ret[i] = getAction(PlayerAction::PutOrPick);
                ret[i] += getDir(Players[i].targetDir);
            }
        }
        else if (Players[i].OrderId == INF && usedPlateNum < totalPlateNum) {
            for (int j = 0; j < orderCount; j++) {
                if (!Order[j].PlayerId && Order[j].validFrame > 60) {
                    usedPlateNum++;
                    Players[i].OrderId = j;
                    Players[i].OrderIdx = 0;
                    Order[j].PlayerId = i;
                    bool serviceIngredient = false;
                    for (int tempIngredient = 0; tempIngredient < IngredientCount; tempIngredient++) {
                        if (Order[j].recipe[Players[i].OrderIdx] == Ingredient[tempIngredient].name) {
                          serviceIngredient = true;
                          ret[i] = addTarget(i, Ingredient[tempIngredient].availableLoc, Ingredient[tempIngredient].x, Ingredient[tempIngredient].y);
                          break;
                        }
                    }
                    if (!serviceIngredient) {
                        assert(0);
                        /* todo: 不是原料、需要加工 */
                    }
                    break;
                }
            }
        }
        else if (Players[i].OrderId != INF && PlateNum > 0 && Players[i].finish == 0 && Players[i].over == 0) {
            if (Players[i].containerKind == ContainerKind::Plate) {
                Players[i].finish = 1;
                ret[i] = "Move ";
                continue ;
            }
            ret[i] = getAction(PlayerAction::PutOrPick);
            ret[i] += getDir(Players[i].targetDir);
            if (Players[i].toEnd == 0)
                Players[i].finish = 1;
        }
        else if (Players[i].OrderId != INF && PlateNum > 0 && Players[i].finish == 1 && Players[i].toEnd == 0 && Players[i].over == 0) {
            for (int j = 0; j < entityCount; j++) {
                if (Entity[j].containerKind == ContainerKind::Plate && Entity[j].PlayerId == 3) {
                    Entity[j].PlayerId = i;
                    ret[i] = addTarget(i, findValidLocation(Entity[j].x, Entity[j].y), Entity[j].x, Entity[j].y);
                    break;
                }
            }
        }
        else if (Players[i].OrderId != INF && Players[i].toEnd == 1 && Players[i].over == 0) {
            ret[i] = addTarget(i, servicePlace, servicePlace_int.first, servicePlace_int.second);
        }
        else if (Players[i].OrderId != INF && Players[i].over == 1) {
            if (Players[i].containerKind != ContainerKind::Plate) {
                Players[i].OrderId = INF;
                Players[i].finish = 0;
                Players[i].toEnd = 0;
                Players[i].over = 0;
                for (int j = 0; j < entityCount; j++) {
                    Entity[j].PlayerId = 3;
                }
                for (int j = 0; j < orderCount; j++) {
                    Order[j].PlayerId = 0;
                }
                continue ;
            }
            ret[i] = getAction(PlayerAction::PutOrPick);
            ret[i] += getDir(Players[i].targetDir);
        }
        else {
            ret[i] = addTarget(i, Ingredient[1].availableLoc, Ingredient[1].x, Ingredient[1].y);
        }
    }
    return std::make_pair(ret[0], ret[1]);
}

std::string addTarget(int id, std::pair<double, double> tempTarget, int x, int y) {
    std::string ret;
    Players[id].targetLocation = tempTarget;
    if (x >= tempTarget.first) {
        Players[id].targetDir = PlayerDir::R;
    }
    else if (y >= tempTarget.second) {
        Players[id].targetDir = PlayerDir::D;
    }
    else if (x + 1 <= tempTarget.first) {
        Players[id].targetDir = PlayerDir::L;
    }
    else {
        Players[id].targetDir = PlayerDir::U;
    }
    ret = getAction(PlayerAction::Move);
    PlayerDir tempDir = dealWithDir(Players[id].targetLocation.first, Players[id].targetLocation.second, Players[id].x, Players[id].y);
    Players[id].OnTheWay = tempDir;
    ret += getDir(tempDir);
    return ret;
}

PlayerDir dealWithDir(double targetX, double targetY, double tempX, double tempY) {
    if (targetX - tempX > esp && targetY - tempY > esp) {
        return PlayerDir::DR;
    }
    else if (targetX - tempX > esp && tempY - targetY > esp) {
        return PlayerDir::UR;
    }
    else if (tempX - targetX > esp && targetY - tempY > esp) {
        return PlayerDir::DL;
    }
    else if (tempX - targetX > esp && tempY - targetY > esp) {
        return PlayerDir::UL;
    }
    else if (fabs(targetX - tempX) <= esp && targetY - tempY > esp) {
        return PlayerDir::D;
    }
    else if (fabs(targetX - tempX) <= esp && tempY - targetY > esp) {
        return PlayerDir::U;
    }
    else if (targetX - tempX > esp && fabs(targetY - tempY) <= esp) {
        return PlayerDir::R;
    }
    else if (tempX - targetX > esp && fabs(targetY - tempY) <= esp) {
        return PlayerDir::L;
    }
    else {
        return PlayerDir::None;
    }
}