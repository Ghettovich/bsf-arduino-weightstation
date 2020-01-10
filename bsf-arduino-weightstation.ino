// Paint application - Demonstate both TFT and Touch Screen
#include <stdint.h>
#include <SeeedTouchScreen.h>
#include <TFTv2.h>
#include <SPI.h>

#include <EtherSia.h>

int etherSS = 53;

/** W5100 Ethernet Interface */
EtherSia_ENC28J60 ether(etherSS);

/** Define UDP socket with ether and port */
UDPSocket udp(ether, 6678);
const char * serverIP = "fd54:d174:8676:1:653f:56d7:bd7d:c238";

int ColorPaletteHigh = 60;
int color = WHITE;  //Paint brush color
unsigned int colors[4] = {RED, BLUE, YELLOW, GRAY1};

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// The 2.8" TFT Touch shield has 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM); //init TouchScreen port pins

static void setPinDefinitions() {
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  pinMode(etherSS, OUTPUT);
  digitalWrite(etherSS, HIGH);
}

static void initTFTAndDrawButtons() {
  // TFT
  Tft.TFTinit();  //init TFT library
  Serial.begin(115200);
  //Draw the pallet
  for (int i = 0; i < 4; i++)
  {
    Tft.fillRectangle(i * 60, 0, 60, ColorPaletteHigh, colors[i]);
  }

  Tft.drawString("KIES COMPONENT", 40, 80, 2, WHITE);
}

static void initEthernetAdapter() {
  // Ethernet adapter
  MACAddress macAddress("9e:b3:19:c7:1b:10");
  Serial.println("[BSF-WeightStation]");
  macAddress.println();

  //  // Start Ethernet
  if (ether.begin(macAddress) == false) {
    Serial.println("Failed to configure Ethernet");
  }

//  if (udp.setRemoteAddress(serverIP, 6678)) {
//    Serial.print("Remote address: ");
//    udp.remoteAddress().println();
//  }

  Serial.print("Our link-local address is: ");
  ether.linkLocalAddress().println();
  Serial.print("Our global address is: ");
  ether.globalAddress().println();
}

void setup()
{
  setPinDefinitions();
  initTFTAndDrawButtons();
  initEthernetAdapter();

  Serial.println("Ready.");
}

void loop()
{
  ether.receivePacket();

  if (udp.havePacket()) {
    Serial.print("Received UDP from: ");
    udp.packetSource().println();

    Serial.print("Packet length: ");
    Serial.println(udp.payloadLength(), DEC);

    if (udp.payloadEquals("hoi")) {
      Serial.println("** received hoi **");
      udp.sendReply("ok");
      Tft.drawString("1000 g", 40, 170, 4, WHITE);
    }
  }
  
  // a point object holds x y and z coordinates.
  Point p = ts.getPoint();

  //map the ADC value read to into pixel co-ordinates

  p.x = map(p.x, TS_MINX, TS_MAXX, 0, 240);
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, 320);

  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!

  if (p.z > __PRESURE) {
    // Detect component select change
    if (p.y < ColorPaletteHigh + 2)
    {
      Tft.fillRectangle(0, 115, 240, 30, BLACK);
      if ((p.x / 60) == 0) {
        Tft.drawString("COMPONENT 1", 20, 120, 3, RED);
      }
      else if ((p.x / 60) == 1) {
        Tft.drawString("COMPONENT 2", 20, 120, 3, BLUE);
      }
      else if ((p.x / 60) == 2) {
        Tft.drawString("COMPONENT 3", 20, 120, 3, YELLOW);
      }
      else if ((p.x / 60) == 3) {
        Tft.drawString("COMPONENT 4", 20, 120, 3, GRAY1);
      }
    }
  }

}
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
