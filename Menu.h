#ifndef Menu_h
#define Menu_h

#include <odroid_go.h>

#define MENU_SIZE 15

class Menu{
  public:
    Menu();
    void show();
  private:
    void draw();

    String menu[20];
    int currentItem=0;

};
#endif
