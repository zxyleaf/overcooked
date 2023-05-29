#include <enum.h>
#include <framework.h>
#include <string>
#include <iostream>
#include <sstream>
#include <string>
#include <cassert>
#include <vector>
#include <algorithm>
#include <cmath>

const int INF = 0x3f3f3f;
const double esp = 0.45;
/* 按照读入顺序定义 */
int width, height;
char Map[20 + 5][20 + 5];
int bfsMap[20 + 5][20 + 5];
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
int StoveNum = 0;
int PlateNum = 0, dirtyPlateNum = 0;
//PlayerDir sinkPlaceDir, servicePlaceDir, PlateReturnDir, chopPlaceDir, stovePlaceDir, plateRackPlaceDir;
std::pair<int, int> servicePlace_int, PlateReturn_int, sinkPlace_int, chopPlace_int, stovePlace_int[20 + 5], plateRackPlace_int;
std::pair<double, double> sinkPlace, plateRackPlace, PlateReturn, chopPlace, servicePlace, StovePlace[20 + 5];
int isWashing = 3;
Mission orderMission[20 + 5];

void stringToLoc(int id, const std::string& name, std::pair<int, int> &int_loc, std::pair<double, double> &double_loc) {
  if (name == "chop") {
    int_loc = chopPlace_int;
    double_loc = chopPlace;
    Players[id].mission.shouldInterat = 1;
  }
  else if (name == "pan") {
    for (int i = 0; i < entityCount; i++) {
      if (Entity[i].containerKind == ContainerKind::Pan) {
        double_loc = findValidLocation((int )Entity[i].x, (int )Entity[i].y);
        int_loc.first = (int )Entity[i].x;
        int_loc.second = (int )Entity[i].y;
        break ;
      }
    }
    //Players[id].mission.shouldInterat = 1;
  }
  else if (name == "pot") {
    for (int i = 0; i < entityCount; i++) {
      if (Entity[i].containerKind == ContainerKind::Pot) {
        double_loc = findValidLocation((int )Entity[i].x, (int )Entity[i].y);
        int_loc.first = (int )Entity[i].x;
        int_loc.second = (int )Entity[i].y;
        std::cerr << "pot at " << double_loc.first << "  " << double_loc.second << std::endl;
        break ;
      }
    }
    //Players[id].mission.shouldInterat = 1;
  }
  else if (name == "Plate") {
    for (int j = 0; j < entityCount; j++) {
      if (Entity[j].containerKind == ContainerKind::Plate && Entity[j].PlayerId == id) {
        double_loc = findValidLocation((int )Entity[j].x, (int )Entity[j].y);
        int_loc.first = (int )Entity[j].x;
        int_loc.second = (int )Entity[j].y;
        return ;
      }
    }
    for (int j = 0; j < entityCount; j++) {
      if (Entity[j].containerKind == ContainerKind::Plate && Entity[j].PlayerId == 3) {
        Entity[j].PlayerId = id;
        double_loc = findValidLocation((int )Entity[j].x, (int )Entity[j].y);
        int_loc.first = (int )Entity[j].x;
        int_loc.second = (int )Entity[j].y;
        break;
      }
    }
  }
  else if (name == "ServiceWindow") {
    int_loc = servicePlace_int;
    double_loc = servicePlace;
  }
  else {
    for (int j = 0; j < IngredientCount; j++) {
      if (Ingredient[j].name == name) {
        int_loc = std::make_pair(Ingredient[j].x, Ingredient[j].y);
        double_loc = Ingredient[j].availableLoc;
      }
    }
  }
}
bool isIngredient(const std::string &name) {
  for (int i = 0; i < IngredientCount; i++) {
    if (Ingredient[i].name == name) {
      return true;
    }
  }
  return false;
}
void init_read()
{
    std::string s;
    std::stringstream ss;

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
        Players[i].targetPlace = " ";
        Players[i].mission.allDone = 0;
        Players[i].mission.finish = 0;
        Players[i].mission.shouldInterat = 0;
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
    for (int i = 0; i < 20; i++) {
        Entity[i].PlayerId = 3;
    }
}

bool frame_read(int nowFrame)
{
    std::string s;
    std::stringstream ss;
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
                4 0 @  : fish  ; 105 / 150
                @ Pot : rice  ; 3 / 250

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
            if (s == "Pan") {
                Entity[i].containerKind = ContainerKind::Pan;
            }
            else if (s == "Pot") {
                Entity[i].containerKind = ContainerKind::Pot;
            }
            else if (s == "Plate")
                Entity[i].containerKind = ContainerKind::Plate, PlateNum++;
            else if (s == "DirtyPlates")
            {
                Entity[i].containerKind = ContainerKind::DirtyPlates;
                tmp >> Entity[i].sum;
                dirtyPlateNum = Entity[i].sum;
            }
            else
                Entity[i].entity.push_back(s);
        }
    }
    return false;
}

std::pair<double, double> findValidLocation(int y, int x) {
    std::pair<double, double> ret;
    double bia = 0.5;
    if (getTileKind(Map[x][y]) == TileKind::Sink) {
        bia = 0.35;
    }
    double bia_add = bia + (double )1;
    if (x + 1 < width && getTileKind(Map[x + 1][y]) == TileKind::Floor) {
        ret = std::make_pair((double)y + bia, (double)x + bia_add);
    }
    else if (x - 1 >= 0 && getTileKind(Map[x - 1][y]) == TileKind::Floor) {
        ret = std::make_pair((double)y + bia, (double)x - bia);
    }
    else if (y + 1 < height && getTileKind(Map[x][y + 1]) == TileKind::Floor) {
        ret = std::make_pair((double)y + bia_add, (double)x + bia);
    }
    else if (y - 1 >= 0 && getTileKind(Map[x][y - 1]) == TileKind::Floor) {
        ret = std::make_pair((double)y - bia, (double)x + bia);
    }
    return ret;
}
int numOfSC(int i) {
    int numOfs = 0;
    for (const std::string& name : totalOrder[i].recipe) {
        if (name[0] == 's' && name[2] == 'f') {
            numOfs++;
        }
    }
    return numOfs;
}
int numOfS_rice(int i) {
    int numOfs = 0;
    for (const std::string& name : totalOrder[i].recipe) {
        if (name[0] == 's' && name[2] == 'r') {
            numOfs++;
        }
    }
    return numOfs;
}
int numOfS(int i) {
    int numOfs = 0;
    for (const std::string& name : totalOrder[i].recipe) {
        if (name[0] == 's') {
            numOfs++;
        }
    }
    return numOfs;
}
bool hasPlate(int i) {
    std::string val;
    std::stack <std::string> temp;
    bool ret = false;
    while(!orderMission[i].Places.empty())
    {
        val = orderMission[i].Places.top();
        if (val == "Plate") {
            ret = true;
        }
        temp.push(val);
        orderMission[i].Places.pop();
    }
    //复原
    while(!temp.empty())
    {
        val = temp.top();
        temp.pop();
        orderMission[i].Places.push(val);
    }
    return ret;
}
void init() {
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++) {
            if (getTileKind(Map[i][j]) == TileKind::Floor) {
                bfsMap[i][j] = 1;
            }
            else
                bfsMap[i][j] = 0;
        }
    for (int i = 0; i < IngredientCount; i++) {
        Ingredient[i].availableLoc = findValidLocation(Ingredient[i].x, Ingredient[i].y);
    }
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++) {
            if (getTileKind(Map[i][j]) == TileKind::ChoppingStation) {
                chopPlace = findValidLocation(j, i);
                chopPlace_int = std::make_pair(j, i);
            }
            else if (getTileKind(Map[i][j]) == TileKind::ServiceWindow) {
                servicePlace = findValidLocation(j, i);
                servicePlace_int = std::make_pair(j, i);
            }
            else if (getTileKind(Map[i][j]) == TileKind::Stove) {
                StovePlace[++StoveNum] = findValidLocation(j, i);
                stovePlace_int[StoveNum] = std::make_pair(j, i);
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
                plateRackPlace_int = std::make_pair(j, i);
            }
        }
    for (int i = 0; i < totalOrderCount; i++) {
        orderMission[i].targetLoc.push(servicePlace);
        orderMission[i].targetCubeLoc.push(servicePlace_int);
        orderMission[i].Places.emplace("ServiceWindow");
        orderMission[i].allDone = 1;
        orderMission[i].action.push(PlayerAction::PutOrPick);
        int numberOfS = numOfS(i);
        int numberOfSC = numOfSC(i);
        int numberOfS_rice = numOfS_rice(i);
        int recipeLen = (int )totalOrder[i].recipe.size();
        int kind = 0;
        if (numberOfS == 1 && numberOfS_rice == 1) {
            kind = 1; // 只有一个s 并且是米饭  最后肯定去pot
            int idx = 0;
            for (const std::string& name : totalOrder[i].recipe) {
                if (name[0] == 's') {
                  std::swap(totalOrder[i].recipe[idx], totalOrder[i].recipe[recipeLen - 1]);
                  break;
                }
                idx++;
            }
            if (recipeLen >= 3)
                std::swap(totalOrder[i].recipe[recipeLen - 3], totalOrder[i].recipe[recipeLen - 2]);
        }
        else if (numberOfS == 1 && numberOfSC == 1) {
            kind = 3; // 只有一个s 是鱼
            int idx = 0;
            for (const std::string& name : totalOrder[i].recipe) {
                if (name[0] == 's') {
                  std::swap(totalOrder[i].recipe[idx], totalOrder[i].recipe[recipeLen - 1]);
                  break;
                }
                idx++;
            }
        }
        else if (numberOfS == 2 && numberOfSC == 1) {
            kind = 2; // 有两个s 一个是rice 一个是fish
            int idx = 0;
            for (const std::string& name : totalOrder[i].recipe) {
                if (name[0] == 's') {
                  std::swap(totalOrder[i].recipe[idx], totalOrder[i].recipe[0]);
                  break;
                }
                idx++;
            }
            idx = 0;
            for (const std::string& name : totalOrder[i].recipe) {
                if (idx != 0 && name[0] == 's') {
                  std::swap(totalOrder[i].recipe[idx], totalOrder[i].recipe[1]);
                  break;
                }
                idx++;
            }
            if (totalOrder[i].recipe[1][2] == 'f') {
                std::swap(totalOrder[i].recipe[0], totalOrder[i].recipe[1]);
            }
        }
        int recipeCur = 1;
        for (const std::string& name : totalOrder[i].recipe) {
            std::cerr << "name :" << name << std::endl;
            orderMission[i].recipe.push_back(name);
            std::string tempName = name;
            bool single = true;
            while (!isIngredient(tempName)) {
                single = false;
                for (int j = 0; j < recipeCount; j++) {
                  if (Recipe[j].nameAfter == tempName) {
                    int end = 0;
                    for (std::string::iterator it = Recipe[j].kind.begin();
                         it != Recipe[j].kind.end(); it++) {
                      if (*it == '-' && *(it + 1) == '>')
                        break;
                      end++;
                    }
                    std::string kindName = Recipe[j].kind.substr(1, end - 1);
                    if (name == tempName && kindName == "chop") {
                      if (kind == 1) {
                        std::cerr << "current " << recipeCur << " recipeCur " << recipeCur << std::endl;
                        orderMission[i].Places.emplace("pot");
                        orderMission[i].allDone++;
                      }
                      orderMission[i].Places.emplace("Plate");
                      orderMission[i].allDone++;

                    }
                    orderMission[i].Places.emplace(kindName);
                    orderMission[i].allDone++;

                    if (name == tempName && kindName != "chop" && recipeCur == 1) {
                      if (kind == 2) {
                        orderMission[i].Places.emplace("pot");
                        orderMission[i].allDone++;
                      }
                      orderMission[i].Places.emplace("Plate");
                      orderMission[i].allDone++;
                      orderMission[i].Places.emplace(kindName);
                      orderMission[i].allDone++;
                    }
                    else if (name == tempName && kindName != "chop") {

                    }
                    tempName = Recipe[j].nameBefore;
                    break;
                  }
                }
            }
            if (single) {
                if (kind == 1 && recipeCur == 1) {
                  orderMission[i].Places.emplace("pot");
                  orderMission[i].allDone++;
                }

                  orderMission[i].Places.emplace("Plate");
                  orderMission[i].allDone++;
            }
            for (int j = 0; j < IngredientCount; j++) {
                if (Ingredient[j].name == tempName) {
                   int tempX = Ingredient[j].x;
                   int tempY = Ingredient[j].y;
                   orderMission[i].targetLoc.push(Ingredient[j].availableLoc);
                   orderMission[i].targetCubeLoc.emplace(std::make_pair(tempX, tempY));
                   orderMission[i].Places.emplace(Ingredient[j].name);
                   orderMission[i].allDone++;
                   orderMission[i].action.push(PlayerAction::PutOrPick);
                }
            }
            recipeCur++;
        }
        /*std::string val;
        while(!orderMission[i].Places.empty())
        {
            val=orderMission[i].Places.top();
            std::cerr << val << " ! " << std::endl;
            orderMission[i].Places.pop();
        }*/
    }
}
bool inTheSameCube(int x1, int y1, double x2, double y2, PlayerDir dir) {
    int x = (int)x2;
    int y = (int)y2;
    switch (dir) {
    case PlayerDir::D:
        y = y + 1;
        break;
    case PlayerDir::U:
        y = y - 1;
        break;
    case PlayerDir::R:
        x = x + 1;
        break;
    case PlayerDir::L:
        x = x - 1;
        break;
    default:
        break ;
    }
    //std::cerr << "compare" << x1 << x << y1 << y << std::endl;

    if (x1 == x && y1 == y) {
        return true;
    }
    return false;
}
int times = 0;
bool tooClose() {
    double x1 = Players[0].x;
    double y1 = Players[0].y;
    double x2 = Players[1].x;
    double y2 = Players[1].y;
    if (Players[0].stay <= 0 && Players[1].stay <= 0 && fabs(x1 - x2) <= (double )1 && fabs(y1 - y2) <= (double )1) {
        return true;
    }
    return false;
}
bool isBlocked(int id, int anotherOne, std::pair<int, int>) {
    if (Players[id].mission.Places.top() == Players[anotherOne].targetPlace && Players[anotherOne].targetPlace == "chop") {
        return true;
    }
    if (Players[id].mission.Places.top() == "pot") {
        if (Players[id].mission.Places.top() == Players[anotherOne].targetPlace) {
            return true;
        }
        for (int i = 0; i < entityCount; i++) {
            //std::cerr << "Entity[i].containerKind " << (int )Entity[i].containerKind << " " << Entity[i].currentFrame << std::endl;
            if (Entity[i].containerKind == ContainerKind::Pot && Entity[i].currentFrame > 0 && Players[id].containerKind != ContainerKind::Plate) {
                return true;
            }
        }
    }
    if (Players[id].mission.Places.top() == "pan") {
        if (Players[id].mission.Places.top() == Players[anotherOne].targetPlace) {
            return true;
        }
        for (int i = 0; i < entityCount; i++) {
            if (Entity[i].containerKind == ContainerKind::Pan && Entity[i].currentFrame > 0 && Players[id].containerKind != ContainerKind::Plate) {
                return true;
            }
        }
    }
    return false;
}
int adjust = 0;
bool isFinish(int id) {
    auto temp = Players[id].mission.Places.top();
    Players[id].mission.Places.pop();
    if (Players[id].mission.Places.top() == "ServiceWindow") {
        Players[id].mission.Places.push(temp);
        return true;
    }
    Players[id].mission.Places.push(temp);
    return false;
}
std::pair<std::string, std::string> dealWithAction() {
    std::string ret[2];
    if (tooClose()) {
        times++;
    }
//    if (times >= 120) {
//        if (Players[1].stay <= 0)
//            ret[1] = "Move U";
//        if (Players[0].stay <= 0)
//            ret[0] = "Move L";
//        adjust++;
//        if (adjust == 18) {
//            times = 0;
//            adjust = 0;
//        }
//        return std::make_pair(ret[0], ret[1]);
//    }
    for (int i = 0; i < k; i++) {
        int anotherOne = (1 + i) % k;
        if (Players[i].containerKind == ContainerKind::Plate) {
            for (int j = 0; j < 20; j++) { //拿到之后，就没有plate这个实体了
                if (Entity[j].PlayerId == i)
                   Entity[j].PlayerId = 3;
            }
        }
        if (isWashing != anotherOne && Players[anotherOne].stay > 0 && tooClose()) {
            ret[i] = "Move ";
        }
        else if (Players[i].stay > 0) {
            if (Players[i].stay % 180 == 0) {
                //std::cerr << "stay and interact " << Players[i].x << " " << Players[i].y << std::endl;
                 ret[i] = getAction(PlayerAction::Interact);
                 ret[i] += getDir(Players[i].targetDir);
            }
            else if (Players[i].stay % 180 == 1 && isWashing == i) {
                 //std::cerr << "usedPlateNum -- " << usedPlateNum << std::endl;
                 usedPlateNum--;
                 ret[i] = "Move ";
            }
            else
                ret[i] = "Move ";
            Players[i].stay--;

            if (Players[i].stay == 0 && isWashing == i) {
                isWashing = 3;
            }
            else if (Players[i].stay == 0) {
                std::cerr << "is 3 " << Players[i].mission.Places.top() << std::endl;
                Players[i].mission.finish = 3; // 切完了拿起来
                ret[i] = getAction(PlayerAction::PutOrPick);
                ret[i] += getDir(Players[i].targetDir);
            }
        }
        else if (Players[i].OnTheWay != PlayerDir::None) {
            ret[i] = getAction(PlayerAction::Move);
            PlayerDir tempDir = dealWithDir(i, Players[i].targetLocation.first, Players[i].targetLocation.second, Players[i].x, Players[i].y);
            ret[i] += getDir(tempDir);
            if (tempDir == PlayerDir::None) {
                Players[i].OnTheWay = PlayerDir::None;
                //Players[i].targetDir = PlayerDir::None;
                Players[i].targetLocation.first = 25; // 之后不需要位置了 但还需要方向
                Players[i].targetLocation.second = 25;
                //std::cerr << "part arrived " << Players[i].targetPlace << std::endl;
                if (Players[i].finish == 1) {
                    Players[i].finish = 0;
                    Players[i].toEnd = 1;
                }
                if (Players[i].containerKind == ContainerKind::Plate && Players[i].toEnd == 1) {
                    Players[i].finish = 0;
                    Players[i].over = 1;
                    //arrived!!!
                }
                if (Players[i].containerKind == ContainerKind::DirtyPlates) {
                    ret[i] = ret[i] = getAction(PlayerAction::PutOrPick);
                    ret[i] += getDir(Players[i].targetDir);
                    Players[i].stay = 180 * Players[i].sum;
                }

            }
        }
        else if (/*i == 1 && */Players[i].OrderId == INF && dirtyPlateNum > 0 && isWashing == 3) {
            /* todo: 洗盘子 (暂时只有一个人) */
            ret[i] = addTarget(i, PlateReturn, PlateReturn_int.first, PlateReturn_int.second);
            isWashing = i;
        }
        else if (/*i == 1 && */isWashing == i) {
            if (Players[i].containerKind == ContainerKind::DirtyPlates) {
                ret[i] = addTarget(i, sinkPlace, sinkPlace_int.first, sinkPlace_int.second);
            }
            else {
                ret[i] = ret[i] = getAction(PlayerAction::PutOrPick);
                ret[i] += getDir(Players[i].targetDir);
            }
        }
        else if (/*i == 0 && */Players[i].OrderId == INF && usedPlateNum < totalPlateNum) {
            for (int j = 0; j < orderCount; j++) {
                if (!Order[j].PlayerId && Order[j].validFrame > 60) {
                    //std::cerr << "totalPlateNum = " << totalPlateNum << "usedPlateNum" << usedPlateNum << std::endl;
                    bool findMission;
                    int MissionId = -1;
                    for (int missionId = 0; missionId < totalOrderCount; missionId++) {
                        findMission = true;
                        for (const auto& tempRecipe : Order[j].recipe) {
                            auto result = find(orderMission[missionId].recipe.begin(), orderMission[missionId].recipe.end(), tempRecipe);
                            if ( result == orderMission[missionId].recipe.end()) {
                                findMission = false;
                            }
                        }
                        if (findMission && Order[j].recipe.size() == orderMission[missionId].recipe.size()) {
                            MissionId = missionId;
                            break;
                        }
                    }
                    assert(MissionId != -1);
                    Players[i].mission = orderMission[MissionId];
                    Players[i].OrderId = j;
                    Order[j].PlayerId = i;
                    usedPlateNum++;
                    std::cerr << "Players[i].OrderId = " << Players[i].OrderId  << " MissionId = "<< MissionId << std::endl;
                    for (int tempIngredient = 0; tempIngredient < IngredientCount; tempIngredient++) {
                        if (Players[i].mission.Places.top() == Ingredient[tempIngredient].name) {
                            ret[i] = addTarget(i, Ingredient[tempIngredient].availableLoc, Ingredient[tempIngredient].x, Ingredient[tempIngredient].y);
                            Players[i].targetPlace = Players[i].mission.Places.top();
                        }
                    }
                    /*bool serviceIngredient = false;
                    for (int tempIngredient = 0; tempIngredient < IngredientCount; tempIngredient++) {
                        if (Order[j].recipe[Players[i].OrderIdx] == Ingredient[tempIngredient].name) {
                          std::cerr << "compare" << i << " to " << anotherOne << std::endl;
                          if (inTheSameCube(Ingredient[tempIngredient].x, Ingredient[tempIngredient].y, Players[anotherOne].targetLocation.first, Players[anotherOne].targetLocation.second, Players[anotherOne].targetDir)) {
                              break;
                          }
                          usedPlateNum++;
                          Players[i].OrderId = j;
                          Players[i].OrderIdx = 0;
                          Order[j].PlayerId = i;
                          serviceIngredient = true;
                          ret[i] = addTarget(i, Ingredient[tempIngredient].availableLoc, Ingredient[tempIngredient].x, Ingredient[tempIngredient].y);
                          break;
                        }
                    }
                    */
                    break;
                }
            }
        }
        else if (/*i == 0 && */Players[i].mission.allDone != 0) {
            std::cerr << "id" << i <<  "  in mission " << Players[i].mission.Places.top() <<" finish is " << Players[i].mission.finish << " allDone =" << Players[i].mission.allDone << std::endl;
            if (Players[i].mission.finish == 0) {
                std::cerr << "id" << i << "in 0" << std::endl;
                if (Players[i].mission.Places.top() == "pot" && Players[i].containerKind == ContainerKind::Plate) {
                    for (int j = 0; j < entityCount; j++) {
                        if (Entity[j].containerKind == ContainerKind::Pot) { // 把东西从锅里拿出来
                            if (Entity[j].currentFrame > Entity[j].totalFrame) {
                                ret[i] = getAction(PlayerAction::PutOrPick);
                                ret[i] += getDir(Players[i].targetDir);
                                Players[i].targetPlace = " ";
                                Players[i].mission.finish = 4;
                                Players[i].mission.shouldInterat = 0;
                                //std::cerr << "take out of the pot" << std::endl;
                                break;
                            }
                        }
                    }
                }
                else if (Players[i].mission.Places.top() == "pan" && Players[i].containerKind == ContainerKind::Plate) {
                    for (int j = 0; j < entityCount; j++) {
                        if (Entity[j].containerKind == ContainerKind::Pan) { // 把东西从锅里拿出来
                            if (Entity[j].currentFrame > Entity[j].totalFrame) {
                                ret[i] = getAction(PlayerAction::PutOrPick);
                                ret[i] += getDir(Players[i].targetDir);
                                Players[i].targetPlace = " ";
                                Players[i].mission.finish = 4;
                                Players[i].mission.shouldInterat = 0;
                                //std::cerr << "take out of the pan" << std::endl;
                                break;
                            }
                        }
                    }
                }
                else {
                    ret[i] = getAction(PlayerAction::PutOrPick);
                    ret[i] += getDir(Players[i].targetDir);
                    //if (Players[i].containerKind == ContainerKind::Plate)
                        Players[i].mission.finish = 1; // 1 已经放下了
                    //else {
                    //    std::cerr << "do not have plate " <<  Players[i].mission.Places.top() << std::endl;
                    //}
                }

            }
            else if (Players[i].mission.finish == 1 && Players[i].mission.shouldInterat) {
                std::cerr << "id" << i << "in 1,1" << std::endl;
                ret[i] = getAction(PlayerAction::Interact);
                ret[i] += getDir(Players[i].targetDir);
                Players[i].mission.finish = 2; // 2 已经开始交互了
                Players[i].mission.shouldInterat = 0;
            }
            else if (Players[i].mission.finish == 2) {
                std::cerr << "id" << i << "in 2" << std::endl;
                if (Players[i].mission.Places.top() == "chop") {
                    Players[i].stay = 180 * Players[i].sum;
                    ret[i] = "Move ";
                }
                else {
                    Players[i].mission.finish = 3;
                }
                Players[i].mission.allDone--;
                Players[i].mission.Places.pop();
            }
            else if (Players[i].mission.finish == 3) {
                std::cerr << "id" << i << "in 3 for" << Players[i].mission.Places.top() << std::endl;
                std::pair<int, int> int_loc;
                std::pair<double, double> double_loc;
                stringToLoc(i ,Players[i].mission.Places.top(), int_loc, double_loc);
                std::cerr << "target " << int_loc.first << " " << int_loc.second << std::endl;
                Players[i].targetPlace = Players[i].mission.Places.top();
                ret[i] = addTarget(i, double_loc, int_loc.first, int_loc.second);
                Players[i].mission.finish = 0;
                Players[i].mission.shouldInterat = 0;
            }
            else if (Players[i].mission.finish == 1 && Players[i].mission.Places.top() == "Plate") {
                std::cerr << "id" << i << "in 1,plate" << std::endl;
                if (Players[i].containerKind == ContainerKind::Plate) {
                    //if (isFinish(i)) {
                        Players[i].mission.finish = 4;
                        ret[i] = "Move";
                        continue ;
//                    }
//                    Players[i].mission.Places.pop();
//                    Players[i].mission.allDone--;
//                    Players[i].mission.finish = 0;
//                    std::pair<int, int> int_loc;
//                    std::pair<double, double> double_loc;
//                    stringToLoc(i, Players[i].mission.Places.top(), int_loc, double_loc);
//                    ret[i] = addTarget(i, double_loc, int_loc.first, int_loc.second);
                }
                std::cerr << "without plate " << std::endl;
                if (isFinish(i)) {
                    ret[i] = getAction(PlayerAction::PutOrPick);
                    ret[i] += getDir(Players[i].targetDir);
                    Players[i].mission.finish = 4;
                }
                else {
                    std::cerr << "find Ingredient" << std::endl;
                    Players[i].mission.Places.pop();
                    Players[i].mission.allDone--;
                    Players[i].mission.finish = 0;
                    bool isIn = false;
                    for (int tempIngredient = 0; tempIngredient < IngredientCount; tempIngredient++) {
                        if (Players[i].mission.Places.top() == Ingredient[tempIngredient].name) {
                            ret[i] = addTarget(i, Ingredient[tempIngredient].availableLoc, Ingredient[tempIngredient].x, Ingredient[tempIngredient].y);
                            Players[i].targetPlace = Players[i].mission.Places.top();
                            isIn = true;
                            break;
                        }
                    }
                    if (!isIn) {
                        ret[i] = getAction(PlayerAction::PutOrPick);
                        ret[i] += getDir(Players[i].targetDir);
                        Players[i].mission.finish = 3;
                    }
                    std::cerr << "isIn" << isIn << std::endl;
                }
            }
            else if (Players[i].mission.Places.top() == "ServiceWindow") {
                std::cerr << "id" << i << "in service" << std::endl;
                Players[i].mission.allDone--;
                Players[i].mission.finish = 0;
                Players[i].mission.Places.pop();
                Players[i].targetPlace = " ";
                Players[i].mission.shouldInterat = 0;
                assert(Players[i].mission.allDone == 0);
                ret[i] = getAction(PlayerAction::PutOrPick);
                ret[i] += getDir(Players[i].targetDir);
                Players[i].OrderId = INF;
                for (int j = 0; j < 20; j++) {
                    if (Entity[j].PlayerId == i)
                        Entity[j].PlayerId = 3;
                }
                for (int j = 0; j < 20; j++) {
                    if (Order[j].PlayerId== i)
                        Order[j].PlayerId = 0;
                }
                continue ;
            }
            else if (Players[i].mission.shouldInterat == 0) {
                std::cerr << "id" << i << "in last  " << Players[i].mission.Places.top() <<  std::endl;
                Players[i].mission.allDone--;
                Players[i].mission.Places.pop();
                std::pair<int, int> int_loc;
                std::pair<double, double> double_loc;
                stringToLoc(i ,Players[i].mission.Places.top(), int_loc, double_loc);
                if (isBlocked(i, anotherOne, int_loc)) {
                    std::cerr << Players[i].mission.Places.top()  << " vs "  << Players[anotherOne].targetPlace << std::endl;
                    ret[i] = "Move ";
                    Players[i].mission.shouldInterat = 0;
                    Players[i].mission.allDone++;
                    Players[i].mission.Places.push(Players[i].mission.Places.top());
                }
                else {
                    std::cerr << "double_loc" << double_loc.first << " " << double_loc.second << "int_loc" << int_loc.first << " "<< int_loc.second << std::endl;
                    ret[i] = addTarget(i, double_loc, int_loc.first, int_loc.second);
                    if (Players[i].mission.Places.top() != "Plate")
                        Players[i].targetPlace = Players[i].mission.Places.top();
                    else
                        Players[i].targetPlace = " ";
                    std::cerr << "id" << i << "in last after  " << Players[i].mission.Places.top() << "dir = " << (int )Players[i].targetDir <<  std::endl;
                    Players[i].mission.finish = 0;

                }

            }
            else {
                std::cerr << "really????" << std::endl;
            }
        }
        else if (Players[i].OrderId != INF && PlateNum > 0 && Players[i].finish == 0 && Players[i].over == 0) {
            assert(0);
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
        else if (/*i == 0 && */Players[i].OrderId != INF && PlateNum > 0 && Players[i].finish == 1 && Players[i].toEnd == 0 && Players[i].over == 0) {
            Players[i].mission.allDone--;
            Players[i].mission.Places.pop();
            std::pair<int, int> int_loc;
            std::pair<double, double> double_loc;
            //std::cerr << "begin an order " << std::endl;
            stringToLoc(i, Players[i].mission.Places.top(), int_loc, double_loc);
            ret[i] = addTarget(i, double_loc, int_loc.first, int_loc.second);

            /*for (int j = 0; j < entityCount; j++) {
                if (Entity[j].containerKind == ContainerKind::Plate && Entity[j].PlayerId == 3) {
                    Entity[j].PlayerId = i;
                    ret[i] = addTarget(i, findValidLocation(Entity[j].x, Entity[j].y), Entity[j].x, Entity[j].y);
                    break;
                }
            }*/
        }
        else if (Players[i].OrderId != INF && Players[i].over == 1) {
            assert(0);
            if (Players[i].containerKind != ContainerKind::Plate) {
                Players[i].OrderId = INF;
                Players[i].finish = 0;
                Players[i].toEnd = 0;
                Players[i].over = 0;
                Players[i].mission.finish = 0;
                Players[i].mission.allDone = 0;
                for (int j = 0; j < 20; j++) {
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
        else if (i == 0) {
            int idx = 0;
            for (int j = 0; j < totalOrderCount; j++) {
                for (const auto& temp : totalOrder[j].recipe) {
                    for (int In = 0; In < IngredientCount; In++) {
                        if (temp == Ingredient[In].name) {
                          idx = In;
                          break;
                        }
                    }
                    if (idx != 0)
                        break;
                }
            }
            ret[i] = addTarget(i, Ingredient[idx].availableLoc, Ingredient[idx].x, Ingredient[idx].y);
        }
//        else if (i == 1) {
//            ret[i] = addTarget(i, PlateReturn, PlateReturn_int.first, PlateReturn_int.second);
//        }
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
    PlayerDir tempDir = dealWithDir(id, Players[id].targetLocation.first, Players[id].targetLocation.second, Players[id].x, Players[id].y);
    Players[id].OnTheWay = tempDir;
    ret += getDir(tempDir);
    return ret;
}

int vx[10] = {1, 0, -1, 0, 1, 1, -1, -1}; // vx  vy用来计算一个节点周围上下左右4个节点
int vy[10] = {0, 1, 0, -1, 1, -1, 1, -1};
void bfs(int id, int targetX, int targetY, int tempX, int tempY) {
    if (targetX == tempX && targetY == tempY) {
        return;
    }
    int temp_another[128];
    int path[128]; // 存每个节点的父节点，即路径
    bool vis[25][25]; // 判断某节点是否已经被访问过
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            vis[i][j] = false;
            path[i * width + j] = 0;
            temp_another[i * width + j] = 0;
            if (bfsMap[i][j] == 2) {
                bfsMap[i][j] = 1;
            }
        }
    }

    int Route_size = 0;
    if (!Players[(id + 1) % k].route.empty()) {
        Route_size = Players[(id + 1) % k].route.size();
    }
    int temp_x = (int )Players[(id + 1) % k].x;
    int temp_y = (int )Players[(id + 1) % k].y;
    std::queue<std::pair<int, int>> temp_queue;
    for(int i = 0; i < Route_size; i++) {
        std::cerr << "first " << Players[(id + 1) % k].route.front().first << " second " <<Players[(id + 1) % k].route.front().second << std::endl;
        temp_another[i] = bfsMap[Players[(id + 1) % k].route.front().second][Players[(id + 1) % k].route.front().first];
        if (Players[(id + 1) % k].stay >= 0 || (temp_x != Players[(id + 1) % k].route.front().second && temp_y != Players[(id + 1) % k].route.front().first))
            bfsMap[Players[(id + 1) % k].route.front().second][Players[(id + 1) % k].route.front().first] = 2, std::cerr << "in " << std::endl;
        Players[(id + 1) % k].route.push(Players[(id + 1) % k].route.front());
        temp_queue.push(Players[(id + 1) % k].route.front());
        Players[(id + 1) % k].route.pop();
    }

    temp_another[Route_size] = bfsMap[temp_y][temp_x];
    bfsMap[temp_y][temp_x] = 2;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            std::cerr << bfsMap[i][j];
        }
        std::cerr << std::endl;
    }
    std::queue<node> q;
    node start = node(tempY, tempX, tempX + width * tempY);
    q.push(start);
    vis[tempY][tempX] = true;
    while (!q.empty())
    {
        node u = q.front();
        q.pop();
        path[u.id] = u.parent;
        //std::cerr << "u.id = " << u.id << " " <<  path[u.id] << std::endl;
        for (int i = 0; i < 8; i++)
        {
            int tx = u.x + vx[i];
            int ty = u.y + vy[i];
            // std::cerr << "i = " << i << "bfsMap[tx][ty]" << bfsMap[tx][ty] << " " << tx << " " << ty << "vis " << vis[tx][ty] << std::endl;
            if (tx >= 0 && ty >= 0 && tx < 10 && ty < 10 && bfsMap[tx][ty] != 0 && bfsMap[tx][ty] != 2 && !vis[tx][ty])
            {
                if (i == 4) {
                    if (bfsMap[tx - 1][ty] == 0 || bfsMap[tx][ty - 1] == 0)
                        continue ;
                    if (bfsMap[tx - 1][ty] == 2 && bfsMap[tx][ty - 1] == 2)
                        continue ;
                    if (id == 1 && (bfsMap[tx - 1][ty] == 2 || bfsMap[tx][ty - 1] == 2))
                        continue ;
                }
                else if (i == 5) {
                    if (bfsMap[tx - 1][ty] == 0 || bfsMap[tx][ty + 1] == 0)
                        continue ;
                    if (bfsMap[tx - 1][ty] == 2 && bfsMap[tx][ty + 1] == 2)
                        continue ;
                    if (id == 1 && (bfsMap[tx - 1][ty] == 2 || bfsMap[tx][ty + 1] == 2))
                        continue ;
                }
                else if (i == 6) {
                    if (bfsMap[tx + 1][ty] == 0 || bfsMap[tx][ty - 1] == 0)
                        continue ;
                    if (bfsMap[tx + 1][ty] == 2 && bfsMap[tx][ty - 1] == 2)
                        continue ;
                    if (id == 1 && (bfsMap[tx + 1][ty] == 2 || bfsMap[tx][ty - 1] == 2))
                        continue ;
                }
                else if (i == 7) {
                    if (bfsMap[tx + 1][ty] == 0 || bfsMap[tx][ty + 1] == 0)
                        continue ;
                    if (bfsMap[tx + 1][ty] == 2 && bfsMap[tx][ty + 1] == 2)
                        continue ;
                    if (id == 1 && (bfsMap[tx + 1][ty] == 2 || bfsMap[tx][ty + 1] == 2))
                        continue ;
                }
                vis[tx][ty] = true;
                node v = node(tx, ty, ty + width * tx);
                v.parent = u.id;
                // std::cerr << "in bfsMap[tx][ty]" << bfsMap[tx][ty] << " " << tx << " " << ty << "\n";
                //std::cerr << "in " << v.x <<" " << v.y << " " << v.id<<std::endl;
                q.push(v);
            }
        }
    }
    std::vector<int> ans;
    int p = targetX + targetY * width;
    while (p)
    {
        //std::cerr << "p = " << p << std::endl;
        ans.push_back(p);
        p = path[p];
    }
    for(int i = 0; i < Route_size; i++) {
        bfsMap[temp_queue.front().second][temp_queue.front().first] = temp_another[i];
        temp_queue.pop();
    }
    bfsMap[temp_y][temp_x] = temp_another[Route_size];
    assert(Players[id].route.empty());
    for (int i = (int)ans.size() - 2; i >= 0; i--)
    {
        std::cerr << ans[i] << " -> ";
        Players[id].route.emplace(ans[i] % width, ans[i] / width);
    }
    std::cerr << std::endl;
}
PlayerDir dealWithDir(int id, double targetX, double targetY, double tempX, double tempY) {
       std::pair<int, int> temp;
    double temp_esp = esp;
    if (id == isWashing && Players[id].route.front().first == (int )sinkPlace.first) {
        temp_esp = 0.05;
    }
    if (fabs(targetX - tempX) <= temp_esp && fabs(targetY - tempY) <= temp_esp) {
        if (Players[id].route.empty() == 0) {
            Players[id].route.pop();
        }
        return PlayerDir::None;
    }
    if ((Players[id].X_Velocity - 4.2) > 0.00005 || (Players[id].Y_Velocity - 4.2) > 0.00005) {
        return PlayerDir::STOP;
    }
    if (Players[id].route.empty()) {
        std::cerr<< "id " << id << "targetX " << targetX << " " << targetY << "temp "<< tempX << " " << tempY << std::endl;
        bfs(id, (int )targetX, (int )targetY, (int )tempX, (int )tempY);
        if (Players[id].route.empty()) {
            std::cerr << "should stop " << targetX <<" " << targetY << std::endl;
            return PlayerDir::STOP;
        }
        temp = Players[id].route.front();
        targetX = (double )temp.first;
        targetY = (double )temp.second;
        targetX = targetX + 0.5;
        targetY = targetY + 0.5;
        std::cerr << "after targetX " << targetX << " " << targetY << "temp "<< tempX << " " << tempY << std::endl;
        if (fabs(targetX - tempX) <= temp_esp && fabs(targetY - tempY) <= temp_esp) {
            //std::cerr << "arrived at " << tempX << " " << tempY << std::endl;
            Players[id].route.pop();
            if (Players[id].route.empty()) {
                return PlayerDir::None;
            }
        }
    }
    else {
        assert(Players[id].route.empty() == 0);
        temp = Players[id].route.front();
        targetX = temp.first;
        targetY = temp.second;
        targetX = targetX + 0.5;
        targetY = targetY + 0.5;
        std::cerr << "targetX " << fabs(targetX - tempX) << " " << fabs(targetY - tempY) << "temp "<< tempX << " " << tempY << std::endl;
        if (fabs(targetX - tempX) <= temp_esp && fabs(targetY - tempY) <= temp_esp) {
            std::cerr << "arrived at " << tempX << " " << tempY << std::endl;
            Players[id].route.pop();
            if (Players[id].route.empty()) {
                return PlayerDir::None;
            }
            else {
                assert(Players[id].route.empty() == 0);
                temp = Players[id].route.front();
                targetX = temp.first;
                targetY = temp.second;
                targetX = targetX + 0.5;
                targetY = targetY + 0.5;
            }

        }
        std::cerr << "another to " << targetX << " " << targetY << std::endl;
    }
    if (fabs(targetX - tempX) <= temp_esp && fabs(targetY - tempY) <= temp_esp) {
        assert(0);
        return PlayerDir::None;
    }
//    if (fabs(targetX - tempX) <= 0.81 && fabs(targetY - tempY) <= 0.81 && (fabs(Players[id].X_Velocity) > 2.5 || fabs(Players[id].Y_Velocity) > 2.5)) {
//        return PlayerDir::STOP;
//    }
//    if (id == 1 && fabs(targetX - tempX) <= 0.81 && fabs(targetY - tempY) <= 0.81 && (fabs(Players[id].X_Velocity) > 1.0 || fabs(Players[id].Y_Velocity) > 1.0)) {
//        return PlayerDir::STOP;
//    }
    if (fabs(targetX - tempX) <= temp_esp && getTileKind(Map[(int) tempY][(int)tempX]) == TileKind::Floor) {
        if (targetY - tempY > temp_esp) {
            if (Players[id].Y_Velocity < 3)
                return PlayerDir::D;
            else
                return PlayerDir::STOP;
        }
        else if (targetY - tempY < temp_esp) {
            return PlayerDir::U;
        }
    }
    else if (fabs(targetY - tempY) <= temp_esp) {
        if (targetX - tempX > temp_esp) {
            return PlayerDir::R;
        }
        else if (targetX - tempX < temp_esp) {
            return PlayerDir::L;
        }
    }
    else {
        if (targetX - tempX > temp_esp && targetY - tempY > temp_esp) {
            return PlayerDir::DR;
        }
        else if (targetX - tempX > temp_esp && tempY - targetY > temp_esp) {
            return PlayerDir::UR;
        }
        else if (tempX - targetX > temp_esp && targetY - tempY > temp_esp) {
            return PlayerDir::DL;
        }
        else if (tempX - targetX > temp_esp && tempY - targetY > temp_esp) {
            return PlayerDir::UL;
        }
    }
    if (targetX - tempX > temp_esp) {
        return PlayerDir::R;
    }
    else if (targetY - tempY > temp_esp) {
        return PlayerDir::D;
    }
    else if (targetX - tempX < temp_esp) {
        return PlayerDir::L;
    }
    else if (targetY - tempY < temp_esp) {
        return PlayerDir::U;
    }
    if (targetX - tempX > temp_esp && targetY - tempY > temp_esp) {
        return PlayerDir::DR;
    }
    else if (targetX - tempX > temp_esp && tempY - targetY > temp_esp) {
        return PlayerDir::UR;
    }
    else if (tempX - targetX > temp_esp && targetY - tempY > temp_esp) {
        return PlayerDir::DL;
    }
    else if (tempX - targetX > temp_esp && tempY - targetY > temp_esp) {
        return PlayerDir::UL;
    }
    else if (fabs(targetX - tempX) <= temp_esp && targetY - tempY > temp_esp) {
        return PlayerDir::D;
    }
    else if (fabs(targetX - tempX) <= temp_esp && tempY - targetY > temp_esp) {
        return PlayerDir::U;
    }
    else if (targetX - tempX > esp && fabs(targetY - tempY) <= esp) {
        return PlayerDir::R;
    }
    else if (tempX - targetX > esp && fabs(targetY - tempY) <= esp) {
        return PlayerDir::L;
    }
    else {
        assert(0);
        return PlayerDir::None;
    }
}
