#include "peachos.h"
#include "stdlib.h"

int main(int argc, char** argv)
{
    print("Hello how are you!");

    void* ptr = malloc(512);
    if (ptr)
    {
        
    }


    while(1) 
    {
        if (getkey() != 0)
        {
            print("key was pressed\n");
        }
    }
    
    return 0;
}