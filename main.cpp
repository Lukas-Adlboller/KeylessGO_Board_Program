#include "mbed.h"
#include "boardprog.h"

int main()
{
  printf("\n\n[Info] Starting device...\n");

  BoardProgram boardProg;
  if(boardProg.initialize() == 0)
  {
    boardProg.start();
  }

  printf("[Info] Finished! Please restart device!\n");
  
  while(true){};
}