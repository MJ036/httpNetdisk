#include "CloudiskServer.h"

int main(){
    CloudiskServer cloudiskServer(1);
    cloudiskServer.loadModules();
    cloudiskServer.start(1234);
    return 0;
}
