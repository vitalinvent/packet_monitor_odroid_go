#include "Menu.h"

Menu::Menu(){
  menu[0]="Time to update screen [250]";
  menu[1]="---";
  menu[2]="---";
  menu[3]="---";
  menu[4]="Sound [ON|OFF]";
  menu[5]="---";
  menu[6]="---";
  menu[7]="---";
  menu[8]="---";
  menu[9]="---";
  menu[10]="---";
  menu[11]="---";
  menu[12]="---";
  menu[13]="---";
  menu[14]="---";
  menu[15]="Exit";
}

void Menu::show(){
  boolean exitMenu=false;
  boolean pressed=false;
  while(!exitMenu){

//    if(GO.BtnA.isPressed() || GO.BtnB.isPressed() || GO.BtnMenu.isPressed() 
//    || GO.BtnVolume.isPressed() || GO.BtnSelect.isPressed() || 
//    GO.BtnStart.isPressed()) {pressed=true;}
//    
    if (GO.JOY_Y.wasAxisPressed() == 2) {
          currentItem++;
          pressed=true;
    }
    
    if (GO.JOY_Y.wasAxisPressed() == 1) {
          currentItem--;
          pressed=true;
    }

    if (currentItem>MENU_SIZE){ currentItem=0;}
    if (currentItem<0){ currentItem=MENU_SIZE;}

    if (GO.BtnMenu.wasPressed()) {
          exitMenu=true;
    }

    if (GO.BtnA.wasPressed()) {
          if (menu[currentItem]=="Exit"){
            exitMenu=true;
          }
    }

    if (pressed==true){
      GO.lcd.clearDisplay();
      draw();
    }

//    if (GO.JOY_X.wasAxisPressed() == 2) {
//        left;
//    }
//    if (GO.JOY_X.wasAxisPressed() == 1) {
//        right;
//    }
    
    delay(250);
  }
}

void Menu::draw(){
    GO.lcd.setTextSize(1);
    GO.lcd.clearDisplay();
    GO.lcd.drawString("   ---== Menu ==--- ",0,0); 
  for(int i=0;i<MENU_SIZE;i++){
    if (currentItem==i){
      GO.lcd.setTextColor(LIGHTGREY);
    } else{
      GO.lcd.setTextColor(DARKGREY);
    }
    GO.lcd.drawString(menu[i],0,((i+1)*10)+1); 
  }
    GO.lcd.drawString(" 0<-- 0 -->0",0,231); 
}
