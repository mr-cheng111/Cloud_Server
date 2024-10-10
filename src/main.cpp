#include <iostream>
#include <csignal>
#include <linux/udp.h>
#include <queue>
#include "main.h"

using namespace std;
uint32_t i = 10000;

int main(int argc, char **argv)
{

    cout << "Hello World!" << endl;

    UDP_Server_t *A = new UDP_Server_t();
    while (i > 0) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 模拟长时间运行的操作
        i--;
    }
    delete A;
    

    return 0;
}
