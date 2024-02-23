#include "NYCJarvis_Server.hpp"
// #include "pch.h"

extern "C"
{

    __declspec(dllexport) CustomServer* createCustomServer(int port)
    {
        return new CustomServer(port);
    }

    __declspec(dllexport) void start(CustomServer* c)
    {
        c->Start();
    }

    __declspec(dllexport) void update(CustomServer* c, size_t nMaxInfos=-1, bool bWait=false )
    {
        // std::cout<<"Update function in CLIB DLL: "<<nMaxInfos<<"\t"<<bWait<<std::endl;
        c->Update(nMaxInfos, bWait);
    }

    __declspec(dllexport) void sendInputString(CustomServer* c, char* input_text)
    {
        c->sendInputString(input_text);
    }

    __declspec(dllexport) char* getCurrentInputString(CustomServer* c)
    {
        return c->getCurrentInputString();
    }


}