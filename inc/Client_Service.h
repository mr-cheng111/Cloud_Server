#ifndef CLIENT_CAT
#define CLIENT_CAT


#include <iostream>
#include <string>
#include <linux/udp.h>
#include <unistd.h>    // For close
#include <arpa/inet.h> // For sockaddr_in and inet_ntoa
#include <mutex>
#include <thread>
#include <atomic>
#include <queue>
#include <string.h>
#include <memory>

using namespace std;

class UDP_Server_t;
struct Data_Frame_t;

class Client_Service_t
{
private:
    std::thread *Client_Service_Thread;
    mutex Heart_Counter_Lock;
    uint8_t Heart_Counter = 25;
    
public:
    uint16_t Service_ID;
    queue<Data_Frame_t *> buffer;
    mutex Client_Connect_Flag_Change_Lock;
    mutex Client_Get_Data_Lock;
    mutex Client_Send_Data_Lock;

    atomic<bool> running;
    UDP_Server_t *Parent;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    
    Client_Service_t(uint16_t Client_ID);

    Client_Service_t(uint16_t Client_ID, UDP_Server_t *Parent_);

    ~Client_Service_t();

    void Update_Heart_Counter(void);

    static void Listen_Service_Task(Client_Service_t *Parent);
};


#endif