#ifndef ENUM
#define ENUM
#include <cassert>
#include <string>

enum class PlayerAction {
    Move,
    Interact,
    PutOrPick
};

enum class PlayerDir {
    U,
    D,
    L,
    R
};

enum class ContainerKind {
    None,
    Pan,
    Pot,
    Plate,
    DirtyPlates,
};

enum class TileKind {
    None,
    Void,
    Floor,
    Wall,
    Table,
    IngredientBox,
    TrashBin,
    ChoppingStation,
    ServiceWindow,
    Stove,
    PlateReturn,
    Sink,
    PlateRack,
};

inline std::string getAction(PlayerAction action) {
    switch (action) {
    case PlayerAction::PutOrPick:
        return "PutOrPick ";
    case PlayerAction::Move:
        return "Move ";
    case PlayerAction::Interact:
        return "Interact ";
    }
}
inline std::string getDir(PlayerDir dir) {
    switch (dir) {
    case PlayerDir::U:
      return "U";
    case PlayerDir::D:
      return "D";
    case PlayerDir::L:
      return "L";
    case PlayerDir::R:
      return "R";
    }
}

inline TileKind getTileKind(char kindChar) {
    switch (kindChar) {
    case '_':
        return TileKind::Void;
    case '.':
        return TileKind::Floor;
    case '*':
        return TileKind::Table;
    case 't':
        return TileKind::TrashBin;
    case 'c':
        return TileKind::ChoppingStation;
    case '$':
        return TileKind::ServiceWindow;
    case 's':
        return TileKind::Stove;
    case 'p':
        return TileKind::PlateReturn;
    case 'k':
        return TileKind::Sink;
    case 'r':
        return TileKind::PlateRack;
    default:
        assert(0);
    }
}

inline char getAbbrev(TileKind kind) {
    switch (kind) {
    case TileKind::IngredientBox:
        return 'i';
    case TileKind::TrashBin:
        return 't';
    case TileKind::ChoppingStation:
        return 'c';
    case TileKind::ServiceWindow:
        return '$';
    case TileKind::Stove:
        return 's';
    case TileKind::PlateReturn:
        return 'p';
    case TileKind::Sink:
        return 'k';
    case TileKind::PlateRack:
        return 'r';
    default:
        assert(0);
    }
}

#endif