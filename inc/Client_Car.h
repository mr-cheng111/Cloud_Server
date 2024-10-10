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

class UDP_Listen_t;

class Client_Car_t
{
private:
    std::thread *Client_Car_Thread;
    mutex Heart_Counter_Lock;
    uint8_t Heart_Counter = 25;

    
public:
    uint16_t Car_ID;
    queue<uint8_t *> buffer;
    mutex Robot_Connect_Flag_Change_Lock;
    atomic<bool> running;
    UDP_Listen_t *Parent;
    struct sockaddr_in car_addr;
    socklen_t car_addr_len = sizeof(car_addr);
    
    Client_Car_t(uint16_t Client_ID);

    Client_Car_t(uint16_t Client_ID, UDP_Listen_t *Parent_);

    ~Client_Car_t();
    void Update_Heart_Counter(void);

    static void Listen_Car_Task(Client_Car_t *Parent);
};


#endif