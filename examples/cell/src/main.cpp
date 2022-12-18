#include "iostream"
#include "world/cell.h"
#include "world/chunk.h"
#include "world/world.h"

#include "app.h"

using namespace cell;

int main() {

    App app;
    app.init();
    app.run();
    app.cleanUp();
    
    return 0;
}