

#include "rtspserverapp.h"
#include <iostream>

int main(int argc, const char* argv[]) {

    RtspServerApp app(argc, argv);

    IF_NOTOK(app.Init()) {
        std::cout << "app init failed" << std::endl;
    }

    std::cout << app.GetArgvByIndex(1) << std::endl;

    app.Run();

    return 0;

}