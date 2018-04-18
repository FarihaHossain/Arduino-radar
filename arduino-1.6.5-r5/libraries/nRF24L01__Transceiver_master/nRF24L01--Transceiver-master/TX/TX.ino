#include <SPI.h>
#include <nRF24L01p.h>

//edit this:
nRF24L01p transceiver(10,9);//CSN,CE
byte PIN_LED = 2;
//dont edit this:
int numberInt=1;
char* TX_ADDRESS = "Fritz";
char* RX_ADDRESS = "Peter";
boolean initialised=false;

void setup(){
  pinMode(PIN_LED, OUTPUT);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  transceiver.channel(53);
  transceiver.TXaddress(TX_ADDRESS);
  transceiver.RXaddress(RX_ADDRESS);
  transceiver.init();
  //initial transmit (reset
  blink(numberInt);
  transceiver.txPL(numberInt);
  transceiver.send(SLOW);
  transceiver.init();
  initialised=true; 
}

void loop(){
  if(transceiver.available()){ //if received data
    initialised=false;
    //receive
    transceiver.read();
    transceiver.rxPL(numberInt);
    //show some love
    blink(numberInt);
    numberInt++;
    //answer
    transceiver.txPL(numberInt);
    transceiver.send(SLOW);
  }
  else{
    if(!initialised){//no more data available but had data before: reinit.
      transceiver.init();
      initialised=true; 
    }
  }
}

void blink(int times){
    for (int i=0;i < times; i++){
      digitalWrite(PIN_LED, HIGH);
      delay(100);
      digitalWrite(PIN_LED, LOW);
      delay(150);
    }
}

