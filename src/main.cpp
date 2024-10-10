#include <iostream>
#include <csignal>
#include <linux/udp.h>
#include <queue>
#include "Client_Car.h"
#include "main.h"

using namespace std;
uint32_t i = 10000;

int main(int argc, char **argv)
{

    cout << "Hello World!" << endl;
    // Client_Car_t *A = new Client_Car_t(1);
    UDP_Listen_t *A = new UDP_Listen_t();
    while (i > 0) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 模拟长时间运行的操作
        i--;
    }
    delete A;
    

    return 0;
}
