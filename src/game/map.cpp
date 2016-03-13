#include "map.h"

using namespace game;

Map::Map(int size): data_(size)
{
    data_.resize(size);
    for (auto it=data_.begin(); it != data_.end(); it++) {
        (*it).resize(size);
    }
}

