#include <iostream>
#include "app.h"

int main() {

    HelloWorldApp app;

    app.init();
    app.run();
    app.cleanUp();

    return 0;
}