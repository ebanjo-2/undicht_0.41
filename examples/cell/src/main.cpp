#include "iostream"

#include "app.h"

using namespace cell;

int main() {

    App app;
    app.init();
    app.run();
    app.cleanUp();
    
    return 0;
}