#ifndef MAP_H
#define MAP_H

#include <vector>

namespace game {

    class Map
    {
    public:
        enum CellType {
            Floor,
            Clay,
            Wall,
            Rock,
            Water,
            Custom = 1000
        };

        Map(int size);
    private:
        typedef std::vector<CellType> MapRowType;
        typedef std::vector<MapRowType> MapDataType;
        MapDataType data_;
    };

}

#endif // MAP_H
