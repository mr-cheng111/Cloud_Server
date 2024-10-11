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

    shared_ptr<UDP_Server_t> A = make_shared<UDP_Server_t>(1234);
    // shared_ptr<UDP_Server_t> B = make_shared<UDP_Server_t>(81);

    // weak_ptr<UDP_Server_t> A_Weak_Ptr(A);
    // weak_ptr<UDP_Server_t> B_Weak_Ptr(B);


    // A->Get_Ptr(B_Weak_Ptr);
    // B->Get_Ptr(A_Weak_Ptr);

    while (i > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 模拟长时间运行的操作
        i--;
    }
    A.reset();
    // B.reset();
    

    return 0;
}
