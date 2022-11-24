#include "iostream"
#include "world/cell.h"
#include "world/chunk.h"
#include "world/world.h"


using namespace cell;

int main() {

    std::vector<Cell> cells = {Cell(0,0,0, 1,1,1, 0), Cell(16,0,0, 17, 1, 1, 0)};

    World first_world;
    first_world.loadChunk(glm::ivec3(0,0,0), cells);
    first_world.loadChunk(glm::ivec3(255,0,0), cells);

    const Chunk* chunk = first_world.getChunkAt(glm::ivec3(1,1,1));

    if(chunk != nullptr) {

        const Cell* c = chunk->getCell(15, 0, 0);

        if(c != nullptr)
            std::cout << *c << "\n";
    }

    return 0;
}