//
// High High School Adventure for M5 Series
//

#include <LGFX.h>
#include <Keyboard.h>
#include <SD.h>
#include <SPI.h>
//#include <Wire.h>
#include <graph.h>
#include <vscroll.h>
#include <lineeditor.h>
#include <zwords.h>
#include <zmap.h>
#include <zrule.h>
#include <zobjdata.h>
#include <zuserdata.h>
#include <zsystem.h>
#include <dialog.h>

//static ZSystem game = ZSystem::getInstance(); // game system

void 
setup() 
{
  Serial1.begin(115200); // Serial1 for debug
  Serial.begin(115200); // Serial for USB
  rp2040.begin(0); // initialize core 0 only for now
#if defined(LCD28)||defined(LCD35)
  // 同じSPIにSDとLCDがぶら下がっているとき、両方のCSピンをOFF(HIGH)にすると安定するらしい。
  pinMode(22, OUTPUT); // SD CS pin
  pinMode(9, OUTPUT); // LCD CS pin
  digitalWrite(22, HIGH);// OFF
  digitalWrite(9, HIGH); // OFF
#endif
  ZSystem &game = ZSystem::getInstance(); // game system
  // initialize SD interface
  //Serial1.println("HHSAdv PicoCalc Start ...");
#if defined(PICOCALC)
  SPI.setTX(19);
  SPI.setSCK(18);
  SPI.setRX(16);
  SPI.setCS(17);
  //SPI.begin(true);
  //gpio_set_dir(17, true);
  while (false == SD.begin(17, SPI))
  {
    //Serial1.println("SD Wait ...");
    delay(500);
  }
#elif defined(LCD28)||defined(LCD35)
  // init SD
  SPI1.setTX(11);
  SPI1.setSCK(10);
  SPI1.setRX(12);
  //SPI1.setCS(22); // バスを共有しエイルからか、ここでSPI1.setCS()すると固まる。
  SPI1.begin();
  while (false == SD.begin(22, SPI1))
  {
    Serial1.println("SD Wait ...");
    delay(500);
  }
#endif
  //Serial1.println("Initializing system ...");
  game.loadDict("/HHSAdv/highds.com");
  game.loadMap("/HHSAdv/map.dat");
  game.loadObjs("/HHSAdv/thin.dat");
  game.loadUser("/HHSAdv/data.dat");
  game.loadRules("/HHSAdv/rule.dat");
  game.loadMsgs("/HHSAdv/msg.dat");
  //Serial1.println("Go to title ...");
  game.title(); // draw title
}

void 
loop() 
{
  ZSystem::getInstance().loop();
}