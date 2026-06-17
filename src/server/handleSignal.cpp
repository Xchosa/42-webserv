//#include <csignal>
//#include <iostream>


//void signalHandler(int sig) {
//    std::cout << "Interrupt handle " << sig << std::endl;
    
//    // Optionally exit the program after handling
//    exit(sig);
//}

//int main() {
    
//    // Handle signal
//    signal(SIGINT, signalHandler);

//    // Loop that waits for the signal
//    while (true) {
//        std::cout << "Geeks\n";
//    }
//    return 0;
//}

//# server loop is infinite, set falt at run(), to return normal 
//for epoll_wait errno = EINTR => interrupted by signal -> check for shutdown requested 