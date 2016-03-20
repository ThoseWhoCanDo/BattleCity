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

        CellType cell(int row, int col) {
            return data_[row][col];
        }

        int row_count() const {
            return data_.size();
        }
        int col_count() const {
            return data_[0].size();
        }
    private:
        typedef std::vector<CellType> MapRowType;
        typedef std::vector<MapRowType> MapDataType;
        MapDataType data_;
    };

}

#endif // MAP_H
