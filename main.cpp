#include "server.hpp"

int main (){
    
    Server server("0.0.0.0", 8443);

    server.run();
    
    return 0;
}
