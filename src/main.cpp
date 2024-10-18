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
    if(argc > 3)
    {
        int Port1 = *argv[1];
        int Port2 = *argv[2];
        shared_ptr<UDP_Server_t> A = make_shared<UDP_Server_t>(Port1);
        shared_ptr<UDP_Server_t> B = make_shared<UDP_Server_t>(Port2);
        weak_ptr<UDP_Server_t> A_Weak_Ptr(A);
        weak_ptr<UDP_Server_t> B_Weak_Ptr(B);
        A->Get_Ptr(B_Weak_Ptr);
        B->Get_Ptr(A_Weak_Ptr);
        while (i > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 模拟长时间运行的操作
            // i--;
        }
    }
    else
    {
        shared_ptr<UDP_Server_t> A = make_shared<UDP_Server_t>(82);
        shared_ptr<UDP_Server_t> B = make_shared<UDP_Server_t>(81);
        weak_ptr<UDP_Server_t> A_Weak_Ptr(A);
        weak_ptr<UDP_Server_t> B_Weak_Ptr(B);
        A->Get_Ptr(B_Weak_Ptr);
        B->Get_Ptr(A_Weak_Ptr);
        while (i > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 模拟长时间运行的操作
            // i--;
        }
    }



    return 0;
}
