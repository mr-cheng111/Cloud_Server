#include "Client_Service.h"
#include "main.h"



Client_Service_t::Client_Service_t(uint16_t Client_ID)
{
    cout << "Start Client Service Thread!" << endl;
    if(Client_ID > 0)
    {
        Service_ID = Client_ID;
    }
    else
    {
        Service_ID = 65533;
    }

    running = true;
    Client_Service_Thread = new thread(Listen_Service_Task, this);

}

Client_Service_t::Client_Service_t(uint16_t Client_ID, UDP_Server_t *Parent_)
{
    if(Client_ID > 0)
    {
        Service_ID = Client_ID;
    }
    else
    {
        Service_ID = 65533;
    }

    if(Parent_ != nullptr)
    {
        Parent = Parent_;
        this->addr = Parent->client_addr;
        this->addr_len = Parent->addr_len;
        this->Client_Connect_Flag_Change_Lock.lock();
        this->Parent->Client_Connect_Flag.at(Service_ID) = CONNECTING;
        this->Client_Connect_Flag_Change_Lock.unlock();
    }
    
    running = true;
    Client_Service_Thread = new thread(Listen_Service_Task, this);

}

Client_Service_t::~Client_Service_t(void)
{
    this->running = false;
    this->Client_Service_Thread->join();
}

void Client_Service_t::Update_Heart_Counter()
{
    this->Heart_Counter_Lock.lock();
    if(this->Heart_Counter <= 100)
    {
        this->Heart_Counter += 10;
    }
    this->Heart_Counter_Lock.unlock();
}

void Client_Service_t::Listen_Service_Task(Client_Service_t *Parent)
{
    while(Parent->running)
    {
        if(Parent->Heart_Counter > 0)
        {
            Parent->Heart_Counter_Lock.lock();
            Parent->Heart_Counter -= 1;
            Parent->Heart_Counter_Lock.unlock();
            // cout << unsigned(Parent->Heart_Counter) << endl;
        }
        else if(Parent->Heart_Counter == 0)
        {
            cout << "Thread Stop!" << endl;            
            if(Parent->Parent != nullptr)
            {
                Parent->Client_Connect_Flag_Change_Lock.lock();
                Parent->Parent->Client_Connect_Flag.at(Parent->Service_ID) = DIS_CONNECTED;
                Parent->Client_Connect_Flag_Change_Lock.unlock();
                // cout << unsigned(Parent->Heart_Counter) << endl;
            }
            else
            {
                cout << "Parent is nullptr." << endl;
            }
        }
    
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}