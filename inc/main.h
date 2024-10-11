#ifndef MAIN_H
#define MAIN_H

#include "Client_Service.h"
#include <iostream>
#include <algorithm>
#include <mutex>
#include <memory>
#include <thread>
#include <atomic>
#include <linux/udp.h>
#include <unistd.h>    //  For close
#include <arpa/inet.h> //  For sockaddr_in and inet_ntoa


#define PORT 80             //  监听的端口
#define BUF_SIZE 1024       //  缓冲区大小
#define CLIENT_NUM 15000    // 机器人最大数量, 同时也是ID范围

using namespace std;

enum
{
    CONNECTED = 0,
    CONNECTING,
    DIS_CONNECTED
};

struct Data_Frame_t
{
public:
    Data_Frame_t( const uint16_t ID_, uint8_t CMD_, const uint8_t Data_Length_, const uint8_t *Data_)
    {
        ID_L = ID_ & 0xff;
        ID_H = ID_ >> 8;
        CMD = CMD_;
        Data_Length = Data_Length_;
        Data = new uint8_t(Data_Length);
        copy_n(Data_, Data_Length, Data);
        Tail = ID_L + ID_H + CMD + Data_Length;
        for(uint16_t i = 0;i < Data_Length + 4; i++)
        {
            Tail += Data[i];
        }
    }
    uint8_t ID_L;
    uint8_t ID_H;
    uint8_t CMD;
    uint8_t Data_Length;
    uint8_t *Data;
    uint8_t Tail;
};


class UDP_Server_t
{

private:
    int sockfd;
    uint8_t buffer[BUF_SIZE];
    thread *UDP_Server_Thread;
    thread *ID_Detect_Thread;
    weak_ptr<UDP_Server_t> Des;
    uint16_t Port;

public:
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    array<shared_ptr<Client_Service_t>, CLIENT_NUM> Client_Queue;
    array<atomic<uint8_t>, CLIENT_NUM> Client_Connect_Flag;
    atomic<bool> running = false;

    UDP_Server_t(uint16_t Port_)
    {
        //  创建 UDP 套接字
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
        {
            std::cerr << "Socket creation failed" << std::endl;
            exit(EXIT_FAILURE);
        }

        //  填充服务器地址结构
        memset(&server_addr, 0, sizeof(server_addr)); //  清零
        server_addr.sin_family = AF_INET;             //  设置地址族为 IPv4
        server_addr.sin_addr.s_addr = INADDR_ANY;     //  监听所有接口
        server_addr.sin_port = htons(Port);           //  设置端口号

        //  绑定
        if(bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
        {
            std::cerr << "Bind failed" << std::endl;
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        this->Port = Port_;
        running = true;
        UDP_Server_Thread = new thread(UDP_Server_t_Task, this);
        ID_Detect_Thread  = new thread(ID_Detect_Task, this);
    }

    ~UDP_Server_t()
    {
        close(sockfd);
        this->running = false;
        this->UDP_Server_Thread->join();
        this->ID_Detect_Thread->join();
    }

    void Get_Ptr(std::weak_ptr<UDP_Server_t> F)
    {
        Des = F;
    }

    static void UDP_Server_t_Task(UDP_Server_t *Parent)
    {
        
        while(Parent->running)
        {
            try 
            {
                ssize_t n = recvfrom(Parent->sockfd, Parent->buffer, BUF_SIZE, MSG_WAITALL, (struct sockaddr *)&Parent->client_addr, &Parent->addr_len);
                if (n < 0)
                {
                    throw std::runtime_error("Receive failed"); //  抛出异常
                }
                
                // 获取当前输入的ID
                uint16_t Temp_ID = (uint16_t) (static_cast<uint8_t>(Parent->buffer[1]) << 8 | static_cast<uint8_t>(Parent->buffer[0]));
                // 获取当前输入的控制标志
                uint8_t CMD_Flag = Parent->buffer[2];
                // 获取当前数据长度
                uint8_t Data_Length = Parent->buffer[3];
                // 获取当前数据的求和校验
                uint8_t Data_Tail = Parent->buffer[n - 1];
                if(CMD_Flag == 1)
                {
                    // 服务对象不存在
                    if(Parent->Client_Queue.at(Temp_ID) == nullptr)
                    {
                        // 为此ID机器创建消息队列
                        Parent->Client_Queue.at(Temp_ID) =  make_shared<Client_Service_t>(Temp_ID, Parent);
                        cout << "Client:" << Temp_ID << "，登录成功！" << endl;
                        Parent->Client_Queue.at(Temp_ID)->Client_Connect_Flag_Change_Lock.lock();
                        Parent->Client_Connect_Flag.at(Temp_ID) = CONNECTED;
                        Parent->Client_Queue.at(Temp_ID)->Client_Connect_Flag_Change_Lock.unlock();
                    }
                    else 
                    {
                        // 当前状态为未连接
                        Parent->Client_Queue.at(Temp_ID)->Client_Connect_Flag_Change_Lock.lock();
                        if(Parent->Client_Connect_Flag.at(Temp_ID) == DIS_CONNECTED)
                        {
                            Parent->Client_Queue.at(Temp_ID)->Client_Connect_Flag_Change_Lock.unlock();
                            Parent->Client_Queue.at(Temp_ID).reset();
                            Parent->Client_Queue.at(Temp_ID) =  make_shared<Client_Service_t>(Temp_ID, Parent);
                            cout << "Client:" << Temp_ID << "重新登录成功！" << endl;
                        }
                        // 当前为已连接
                        else if(Parent->Client_Connect_Flag.at(Temp_ID) == CONNECTED)
                        {
                            Parent->Client_Queue.at(Temp_ID)->Client_Connect_Flag_Change_Lock.unlock();
                            // 将数据放入队列
                            Data_Frame_t *Data = new Data_Frame_t(Temp_ID, CMD_Flag, Data_Length, Parent->buffer + 4);
                            if(Data->Tail == Data_Tail)
                            {
                                Parent->Client_Queue.at(Temp_ID)->Client_Get_Data_Lock.lock();
                                Parent->Client_Queue.at(Temp_ID)->buffer.push(Data);
                                Parent->Client_Queue.at(Temp_ID)->Client_Get_Data_Lock.unlock();
                            }

                            /***************TODO：测试发送到客户端逻辑**************/
                            
                            if(std::shared_ptr<UDP_Server_t> Des = Parent->Des.lock())
                            {
                                if(Des->Client_Queue.at(Temp_ID) != nullptr)
                                {
                                    try
                                    {
                                        if(Des->Client_Queue.at(Temp_ID)->Client_Connect_Flag_Change_Lock.try_lock())
                                        {
                                            if(Des->Client_Connect_Flag.at(Temp_ID) == CONNECTED )
                                            {
                                                Des->Client_Queue.at(Temp_ID)->Client_Connect_Flag_Change_Lock.unlock();
                                                
                                                Parent->Client_Queue.at(Temp_ID)->Client_Get_Data_Lock.lock();
                                                Data_Frame_t *Temp_Data = Parent->Client_Queue.at(Temp_ID)->buffer.front();
                                                Parent->Client_Queue.at(Temp_ID)->buffer.pop();
                                                Parent->Client_Queue.at(Temp_ID)->Client_Get_Data_Lock.unlock();
                                                Des->Client_Queue.at(Temp_ID)->Client_Send_Data_Lock.lock();
                                                Des->Send_Data2Client(Des->Client_Queue.at(Temp_ID)->addr, \
                                                                      Des->Client_Queue.at(Temp_ID)->addr_len,\
                                                                      Temp_Data);
                                                Des->Client_Queue.at(Temp_ID)->Client_Send_Data_Lock.unlock();
                                                cout << "成功发送！" << endl;
                                            }
                                        }
                                        else
                                        {
                                            Des->Client_Queue.at(Temp_ID)->Client_Connect_Flag_Change_Lock.unlock();
                                            throw std::runtime_error("Can't get lock failed."); //  抛出异常
                                        }
                                    }
                                    catch(const std::runtime_error& e)
                                    {
                                        std::cerr << "Failed: " << e.what() << '\n';
                                    }
                                    catch(...)
                                    {
                                        std::cerr << "Caught an unknown exception!" << endl;
                                    }
                                }
                                else
                                {
                                    cout << "未找到接受客户端！" << endl;
                                }
                            }
                            else
                            {
                                cout << "未找到监听程序！" << endl;
                            }
                            // Parent->Send_Response_Heart(Temp_ID, \
                            //                             Parent->Client_Queue.at(Temp_ID)->addr, \
                            //                             Parent->Client_Queue.at(Temp_ID)->addr_len, \
                            //                             CMD_Flag);
                            Parent->Client_Queue.at(Temp_ID)->Update_Heart_Counter();
                        }
                        // 当前为正在连接
                        else if(Parent->Client_Connect_Flag.at(Temp_ID) == CONNECTING)
                        {
                            Parent->Client_Queue.at(Temp_ID)->Client_Connect_Flag_Change_Lock.unlock();
                        }
                    }
                }
                // else if(CMD_Flag == 0)
                // {
                //     // 服务对象不存在
                //     if(Parent->Client_Queue.at(Temp_ID) == nullptr)
                //     {
                //         // 为此ID机器创建消息队列
                //         Parent->Client_Queue.at(Temp_ID) =  make_shared<Client_Service_t>(Temp_ID, Parent);
                //         cout << "Client:" << Temp_ID << "，登录成功！" << endl;
                //         Parent->Client_Queue.at(Temp_ID)->Client_Connect_Flag_Change_Lock.lock();
                //         Parent->Client_Connect_Flag.at(Temp_ID) = CONNECTED;
                //         Parent->Client_Queue.at(Temp_ID)->Client_Connect_Flag_Change_Lock.unlock();
                //     }
                //     else
                //     {
                //         // 当前状态为未连接
                //         Parent->Client_Queue.at(Temp_ID)->Client_Connect_Flag_Change_Lock.lock();
                //         if(Parent->Client_Connect_Flag.at(Temp_ID) == DIS_CONNECTED)
                //         {
                //             Parent->Client_Queue.at(Temp_ID)->Client_Connect_Flag_Change_Lock.unlock();
                //             Parent->Client_Queue.at(Temp_ID).reset();
                //             Parent->Client_Queue.at(Temp_ID) =  make_shared<Client_Service_t>(Temp_ID, Parent);
                //             cout << "Client:" << Temp_ID << "重新登录成功！" << endl;
                //         }
                //         // 当前为已连接
                //         else if(Parent->Client_Connect_Flag.at(Temp_ID) == CONNECTED)
                //         {
                //             Parent->Client_Queue.at(Temp_ID)->Client_Connect_Flag_Change_Lock.unlock();
                //             Parent->Send_Response_Heart(Temp_ID, \
                //                                         Parent->Client_Queue.at(Temp_ID)->addr, \
                //                                         Parent->Client_Queue.at(Temp_ID)->addr_len, \
                //                                         CMD_Flag);
                //             Parent->Client_Queue.at(Temp_ID)->Update_Heart_Counter();
                //         }
                //         // 当前为正在连接
                //         else if(Parent->Client_Connect_Flag.at(Temp_ID) == CONNECTING)
                //         {
                //             Parent->Client_Queue.at(Temp_ID)->Client_Connect_Flag_Change_Lock.unlock();
                //         }
                //     }
                // }
            }
            catch (const std::runtime_error& e) 
            {
                std::cerr << "Caught an exception: " << e.what() << std::endl;
            } 
            catch (...)
            {
                std::cerr << "Caught an unknown exception!" << std::endl;
            }
        }
    }

    static void ID_Detect_Task(UDP_Server_t *Parent)
    {
        while(Parent->running)
        {
            for(uint32_t i = 0;i < CLIENT_NUM; i++)
            {
                // 检测是否创建机器人服务节点
                if(Parent->Client_Queue.at(i) != nullptr)
                {
                    // 将连接表加锁
                    Parent->Client_Queue.at(i)->Client_Connect_Flag_Change_Lock.lock();
                    // 如果连接状态为DIS_CONNECTED，则此时清理对应的服务节点
                    if(Parent->Client_Connect_Flag.at(i) == DIS_CONNECTED)
                    {
                        Parent->Client_Queue.at(i)->Client_Connect_Flag_Change_Lock.unlock(); 
                        Parent->Client_Queue.at(i).reset();
                        cout << "delete the Object, Object ID is: " << i << endl;
                    }
                    else
                    {
                        Parent->Client_Queue.at(i)->Client_Connect_Flag_Change_Lock.unlock();
                    }
                }
            }
        }
    }

    /*
    ** ID：目标设备的ID
    ** client_addr：目标设备的地址
    ** addr_len：地址长度
    ** CMD：目标设备类型
    */
    void Send_Response_Heart(const uint16_t ID, const struct sockaddr_in& client_addr, socklen_t addr_len, uint8_t CMD)
    {
        uint8_t Data = 'A';
        Data_Frame_t *Send_Data = new Data_Frame_t(ID, CMD, sizeof(Data), &Data);

        // 向客户端发送反馈指令
        ssize_t sent = sendto(sockfd, (uint8_t *)Send_Data, sizeof(Send_Data), MSG_CONFIRM, (const struct sockaddr *)&client_addr, addr_len);
        if (sent < 0)
        {
            std::cerr << Port << "发送失败" << std::endl;
        }
        else
        {
            std::cout << Port << "向设备： " << ID << " 发送心跳包反馈" << std::endl;
        }
    }

    /*
    ** ID：目标设备的ID
    ** client_addr：目标设备的地址
    ** addr_len：地址长度
    ** Data：数据缓冲区
    ** CMD：目标设备类型
    */
    void Send_Data2Client(const uint16_t ID, const struct sockaddr_in& client_addr, socklen_t addr_len, uint8_t *Data, uint8_t CMD)
    {
        // 向客户端发送控制指令
        Data_Frame_t *Send_Data = new Data_Frame_t(ID, CMD, sizeof(Data), Data);

        ssize_t sent = sendto(sockfd, Send_Data, sizeof(Send_Data), MSG_CONFIRM, (const struct sockaddr *)&client_addr, addr_len);
        if (sent < 0)
        {
            std::cerr << Port << "发送失败" << std::endl;
        }
        else
        {
            std::cout << Port << "向设备： " << ID << " 发送控制指令: " << Data << std::endl;
        }
    }
    void Send_Data2Client(const struct sockaddr_in& client_addr, socklen_t addr_len ,Data_Frame_t *Send_Data)
    {
        // 向客户端发送控制指令

        ssize_t sent = sendto(sockfd, Send_Data, sizeof(Send_Data), MSG_CONFIRM, (const struct sockaddr *)&client_addr, addr_len);
        if (sent < 0)
        {
            std::cerr << Port << "发送失败" << std::endl;
        }
        else
        {
            std::cout << Port << "向设备： " << unsigned((uint16_t) (Send_Data->ID_H << 8 | Send_Data->ID_L)) << " 发送控制指令" << std::endl;
        }
    }
};









#endif