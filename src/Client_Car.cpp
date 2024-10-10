#include "Client_Car.h"
#include "main.h"



Client_Car_t::Client_Car_t(uint16_t Client_ID)
{
    cout << "Start Client Car Thread!" << endl;
    if(Client_ID > 0)
    {
        Car_ID = Client_ID;
    }
    else
    {
        Car_ID = 65533;
    }

    running = true;
    Client_Car_Thread = new thread(Listen_Car_Task, this);

}

Client_Car_t::Client_Car_t(uint16_t Client_ID, UDP_Listen_t *Parent_)
{
    if(Client_ID > 0)
    {
        Car_ID = Client_ID;
    }
    else
    {
        Car_ID = 65533;
    }

    if(Parent_ != nullptr)
    {
        Parent = Parent_;
        this->car_addr = Parent->client_addr;
        this->car_addr_len = Parent->addr_len;
        this->Robot_Connect_Flag_Change_Lock.lock();
        this->Parent->Robot_Connect_Flag.at(Car_ID) = CONNECTING;
        this->Robot_Connect_Flag_Change_Lock.unlock();
    }
    
    running = true;
    Client_Car_Thread = new thread(Listen_Car_Task, this);

}

Client_Car_t::~Client_Car_t(void)
{
    this->running = false;
    this->Client_Car_Thread->join();
}

void Client_Car_t::Update_Heart_Counter()
{
    this->Heart_Counter_Lock.lock();
    this->Heart_Counter += 10;
    this->Heart_Counter_Lock.unlock();
}

void Client_Car_t::Listen_Car_Task(Client_Car_t *Parent)
{
    while(Parent->running)
    {
        if(Parent->Heart_Counter > 0)
        {
            Parent->Heart_Counter_Lock.lock();
            Parent->Heart_Counter -= 1;
            Parent->Heart_Counter_Lock.unlock();
            cout << unsigned(Parent->Heart_Counter) << endl;
        }
        else if(Parent->Heart_Counter == 0)
        {
            cout << "Thread Stop!" << endl;            
            if(Parent->Parent != nullptr)
            {
                Parent->Robot_Connect_Flag_Change_Lock.lock();
                Parent->Parent->Robot_Connect_Flag.at(Parent->Car_ID) = DIS_CONNECTED;
                Parent->Robot_Connect_Flag_Change_Lock.unlock();
            }
            else
            {
                cout << "Parent is nullptr." << endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}