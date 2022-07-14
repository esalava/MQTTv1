#include<stdio.h>   
#include <unistd.h>

int main() 
{ 
   while(1){
    sleep(3);
    printf("CPU_USAGE\n");
   }

   return 0; 
}