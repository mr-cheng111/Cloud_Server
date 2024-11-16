#ifndef DATA_STRUCT
#define DATA_STRUCT

#include <iostream>

struct Data_Frame_t
{
public:
    uint8_t *Data_Buffer = nullptr;
    uint8_t Data_Length;
    Data_Frame_t(const uint16_t ID_, uint8_t CMD_, const uint8_t Data_Length_, const uint8_t *Data_)
    {
        Data_Buffer = new uint8_t(5 + Data_Length_);

        Data_Buffer[0] = ID_ & 0xff;
        Data_Buffer[1] = ID_ >> 8;
        Data_Buffer[2] = CMD_;
        Data_Buffer[3] = Data_Length_;
        Data_Buffer[4 + Data_Length_] = Data_Buffer[0] + Data_Buffer[1] + Data_Buffer[2] + Data_Buffer[3];
        
        for(uint16_t i = 0;i < Data_Length_; i++)
        {
            Data_Buffer[4 + Data_Length_] += Data_[i];
            Data_Buffer[4 + i] = Data_[i];
        }
        Data_Length = Data_Length_ + 5;
    }
    ~Data_Frame_t()
    {
        if(Data_Buffer != nullptr)
        {
            delete Data_Buffer;
        }
        Data_Buffer = nullptr;
    }
    
};


#endif