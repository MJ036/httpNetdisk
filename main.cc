#include "CloudiskServer.h"

int main(){
    CloudiskServer cloudiskServer(1);
    cloudiskServer.loadModules();
    cloudiskServer.start(1);
    return 0;
}
