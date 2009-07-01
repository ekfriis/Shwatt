import processing.serial.*;
import processing.video.*;
import java.util.*;

MovieMaker mm;
Capture camera;

class State {
   int BroadcastData;
   int theta;
   int phi;
   int phiDot;
   int crankLength;

   int thetaError;
   int phiError;
   int phiDotError;
   int crankError;

   int debugValue;

   int performance;
   int gyroGain;
   int xAxis;
   int zAxis;
   int yRate;
   int trigPhi;
   int trigPhiDot;
   int xAxisNoise;
   int zAxisNoise;
   int yRateNoise;
   int xAxisBias;
   int zAxisBias;
   int yRateBias;
   // hmatrix
   int hMatrixTheta;
   int hMatrixPhi;
   int hMatrixPhiDot;

   int KalmanState;
   int CalibrationState;
   int TriggerState;
   int gravity;
   int messages;
   int timeSinceLast;
   int startTime;
   float messageRate()
   {
      if (messages > 0)
      {
         return ( 1000./((millis()-startTime)/messages) );
      } else return 0.0;
   }
}

/*
class OScope {
   string title;
   int currentIndex;
   color myColor;
   int xLength; //in seconds
   int minY;
   int maxY;
   int[]
   OScope(myTitle, 
*/

String inHex(int i)
{
   //java sucks
   // from mit via google
   return Integer.toHexString( 0x10000 | i).substring(1).toUpperCase();
}

String longHex(int i)
{
   return Integer.toHexString(  i).toUpperCase();
}

String inBits(int i)
{
   return Integer.toBinaryString(i);
}

float intToFract(int i)
{
   Boolean isNeg = false;
   if ((i & 0x8000) > 0)
   {
      i |= 0xFFFF0000;
      //i &= ~(0x8000);
      i = ~i;
      i += 1;
      i &= 0x0000FFFF;
      isNeg = true;
   }

   float output = 1.0*i/(1 << 15);
   if (isNeg)
      output *= -1.0;
   return output;
}

float intToAccum(int i)
{
   float output = 1.0*i/(1 << 16);
   return output;
}

String hexAndFract(int it)
{
   String output = inHex(it);
   output += String.format("(%+1.2f)", intToFract(it));
   return output;
}

String hexAndAccum(int it)
{
   String output = inHex(it);
   output += String.format("(%+3.2f)", intToAccum(it));
   return output;
}
   


String gravityLabel(String label, int value, int bias, int gravity)
{
   String output = String.format(label+"%+1.2f" , (value-bias)*1.0/gravity);
   return output;
}

class InfoBox {
   State myState;
   float myWidth;
   float myHeight;
   PFont myFont;
   InfoBox(State theState, int theWidth, int theHeight, PFont theFont)
   {
      myState = theState;
      myWidth = theWidth;
      myHeight = theHeight;
      myFont = theFont;
   }
   void draw()
   {
      pushStyle();
      pushMatrix();
      //scale(0.5);
      textFont(myFont);
      noStroke();
      fill(25);
      //rect(myWidth/20., myHeight/20., myWidth*(18./20), myHeight*(18./20));
      //fill(30, 200, 50);
      fill(255);
      translate(60, 30);
      //scale(0.5);
      text("state information", 0, 0);

      pushStyle();
      pushMatrix();
      translate(myWidth/2, 0);
      fill(255, 0, 0);
      text("flags:", 0, 0);
      translate(myWidth/5, 0);
      text(inBits(myState.CalibrationState),0,0);
      popStyle();
      popMatrix();

      // heaaders
      translate(0, 30);
      pushMatrix();
      translate(myWidth/4.5, 0);
      pushMatrix();
      scale(0.5);
      text("raw measure", 0, 0);
      popMatrix();
      translate(myWidth/4.5, 0);
      pushMatrix();
      scale(0.5);
      text("measure noise", 0, 0);
      popMatrix();
      translate(myWidth/4.5, 0);
      pushMatrix();
      scale(0.5);
      text("measure bias", 0, 0);
      popMatrix();
      popMatrix();

      translate(0, 30);

      pushMatrix();
      fill(230, 100, 0);
      text("yRate:", 0, 0);
      fill(255);
      translate(myWidth/4.5, 0);
      text("0x"+inHex(myState.yRate), 0, 0);
      translate(myWidth/4.5, 0);
      text("0x"+inHex(myState.yRateNoise), 0, 0);
      translate(myWidth/4.5, 0);
      text("0x"+inHex(myState.yRateBias), 0, 0);
      popMatrix();

      // xAxis info
      translate(0, 30);
      pushMatrix();
      fill(230, 100, 0);
      text("xAxis:", 0, 0);
      fill(255);
      translate(myWidth/4.5, 0);
      text("0x"+inHex(myState.xAxis), 0, 0);
      translate(myWidth/4.5, 0);
      text("0x"+inHex(myState.xAxisNoise), 0, 0);
      translate(myWidth/4.5, 0);
      text("0x"+inHex(myState.xAxisBias), 0, 0);
      popMatrix();

      // zaXis info
      translate(0, 30);
      pushMatrix();
      fill(230, 100, 0);
      text("zAxis:", 0, 0);
      fill(255);
      translate(myWidth/4.5, 0);
      text("0x"+inHex(myState.zAxis), 0, 0);
      translate(myWidth/4.5, 0);
      text("0x"+inHex(myState.zAxisNoise), 0, 0);
      translate(myWidth/4.5, 0);
      text("0x"+inHex(myState.zAxisBias), 0, 0);
      popMatrix();

      // trigPhi info
      translate(0, 30);
      pushMatrix();
      fill(230, 100, 0);
      text("trigPhi:", 0, 0);
      fill(255);
      translate(myWidth/4.5, 0);
      text("0x"+hexAndFract(myState.trigPhi), 0, 0);
      /*
      translate(myWidth/4.5, 0);
      text("0x"+inHex(myState.zAxisNoise), 0, 0);
      translate(myWidth/4.5, 0);
      text("0x"+inHex(myState.zAxisBias), 0, 0);
      */
      popMatrix();

      // trig Phi dot
      translate(0, 30);
      pushMatrix();
      fill(230, 100, 0);
      text("trigPhiDot:", 0, 0);
      fill(255);
      translate(myWidth/4.5, 0);
      text("0x"+hexAndFract(myState.trigPhiDot), 0, 0);
      /*
      translate(myWidth/4.5, 0);
      text("0x"+inHex(myState.zAxisNoise), 0, 0);
      translate(myWidth/4.5, 0);
      text("0x"+inHex(myState.zAxisBias), 0, 0);
      */
      popMatrix();


      // gravity
      translate(0, 30);
      pushMatrix();
      fill(20, 100, 250);
      text("gravity:", 0, 0);
      fill(255);
      translate(myWidth/4.5, 0);
      text("0x"+inHex(myState.gravity), 0, 0);
      translate(myWidth/4.5, 0);
      text(gravityLabel("z:", myState.zAxis, myState.zAxisBias, myState.gravity), 0, 0);
      translate(myWidth/4.5, 0);
      text(gravityLabel("x:", myState.xAxis, myState.xAxisBias, myState.gravity), 0, 0);
      popMatrix();

      translate(0, 30);
      pushMatrix();
      fill(100, 100, 30);
      text("Msg. Rate:", 0, 0);
      translate(myWidth/4.5, 0);
      text(String.format("%3.1fHz", myState.messageRate()), 0, 0);
      translate(myWidth/4.5, 0);
      text("K-Rate:", 0, 0);
      translate(myWidth/4.5, 0);
      text(String.format("%3.1fHz", myState.messageRate()*myState.performance), 0, 0);
      popMatrix();

      translate(0, 40);
      pushMatrix();
      text("theta:", 0, 0);
      translate(myWidth/4.5, 0);
      //text(String.format(inHex(myState.theta)"%4x(%+1.2f)",myState.theta,intToFract(myState.theta)), 0, 0);
      //text(String.format("%1.2f",intToFract(myState.theta)), 0, 0);
      text(hexAndFract(myState.theta), 0, 0);
      translate(myWidth/4.5, 0);
      text("error:", 0, 0);
      translate(myWidth/4.5, 0);
      text(hexAndAccum(myState.thetaError), 0, 0);
      popMatrix();

      translate(0, 40);
      pushMatrix();
      text("phi:", 0, 0);
      translate(myWidth/4.5, 0);
      text(hexAndFract(myState.phi), 0, 0);
      translate(myWidth/4.5, 0);
      text("error:", 0, 0);
      translate(myWidth/4.5, 0);
      text(hexAndAccum(myState.phiError), 0, 0);
      popMatrix();

      translate(0, 40);
      pushMatrix();
      text("phiDot:", 0, 0);
      translate(myWidth/4.5, 0);
      text(hexAndFract(myState.phiDot), 0, 0);
      translate(myWidth/4.5, 0);
      text("error:", 0, 0);
      translate(myWidth/4.5, 0);
      text(hexAndAccum(myState.phiDotError), 0, 0);
      popMatrix();

      translate(0, 40);
      text("Trigger: "+inBits(myState.TriggerState), 0, 0);
      translate(0, 40);
      text("Kalman: "+inBits(myState.KalmanState), 0, 0);
      translate(0, 40);
      text("Gyrogain: "+longHex(myState.gyroGain), 0, 0);
      translate(0, 40);
      //text("Crank: "+hexAndFract(myState.crankLength), 0, 0);
      text("Crank: "+hexAndAccum(myState.crankLength), 0, 0);
      translate(myWidth/2, 0);
      text("Error: "+hexAndFract(myState.crankError), 0, 0);

      popStyle();
      popMatrix();
   }
}

class Accelerometers {
   float xAxis;
   float zAxis;
   float yRate;
   float scaleFactor;
   PFont myFont;

   Accelerometers()
   {
      xAxis = 0;
      zAxis = 0;
   }
   void draw()
   {

      pushMatrix();
      pushStyle();
      scale(scaleFactor);
      // radius 1
      strokeWeight(0.2);
      //noFill();
      fill(0, 0, 0);
      stroke(240,240,240);
      ellipse(0, 0, 2*scaleFactor, 2*scaleFactor);

      stroke(0, 0, 200);
      strokeWeight(1);
      strokeCap(SQUARE);
      line(0, 0, xAxis*scaleFactor, 0);
      stroke(200, 0, 0);
      line(0, 0, 0, zAxis*scaleFactor);
      stroke(0, 200, 0);
      strokeWeight(0.5);
      line(0, 0, xAxis*scaleFactor, zAxis*scaleFactor);
      ellipse(0, 0, 2.5, 2.5);
      pushMatrix();
      translate(xAxis*scaleFactor, zAxis*scaleFactor);
      ellipse(0, 0, 1.5, 1.5);
      popMatrix();

      translate(-1.5*scaleFactor, 0);
      strokeWeight(4);
      stroke(255, 0, 255);
      line(0, 0, 0, yRate);
      popMatrix();

      fill(200);
      textFont(myFont);
      textAlign(CENTER, CENTER);

      text("+yRate", -1.5*scaleFactor*scaleFactor, 0.75*scaleFactor*scaleFactor);

      text("+x", scaleFactor*scaleFactor/2.0, 0);
      //text("+x", 10, 10);
      text("+z", 0, scaleFactor*scaleFactor/2.0);

      popStyle();
   }
}


class Pedal {
   float theta;
   float phi;
   float phiDot;
   float crankLength;
   float pedalWidth = 70;
   float triggerPhi;
   Boolean xLow;
   Boolean zLow;

   Pedal() {
      theta       = 0;
      phi         = 0;
      phiDot      = 0;
      crankLength = 165;
   }

   void drawTrigger()
   {
      pushMatrix();
      pushStyle();
      noStroke();
      fill(121, 108, 6);
      arc(0, 0, crankLength, crankLength, triggerPhi - PI/32, triggerPhi + PI/32);
      fill(0);
      ellipse(0, 0, crankLength/2, crankLength/2);

      popMatrix();
      popStyle();
   }


   void drawCrank()
   {
      pushMatrix();
      pushStyle();
      rotate(phi);
      stroke(0, 0, 200);
      strokeWeight(2);
      line(0, 0, crankLength, 0);
      strokeWeight(0.25);
      line(-crankLength/5, 0, 0, 0);
      popMatrix();
      popStyle();
   }

   void drawBottomBracket()
   {
      pushStyle();
      strokeWeight(2);
      fill(0);
      ellipse(0, 0, 10, 10);
      popStyle();
   }

   void drawPedal()
   {
      pushMatrix();
      pushStyle();
      // get to edge of crank
      rotate(phi);
      translate(crankLength, 0);
      // back to regular frame
      rotate(-1.0*phi);
      // draw reference line
      strokeWeight(0.3);
      line(0, pedalWidth/4, 0, -pedalWidth/2.5); // gravity
      noFill();
      arc(0, 0, pedalWidth/3, pedalWidth/3, -1*PI/2+PI/20, theta-PI/9);

      fill(0);
      rotate(theta);
      strokeWeight(5);
      stroke(200, 20, 111);
      line(-pedalWidth/2, 0, pedalWidth/2, 0);
      strokeWeight(1);
      line(0, 0, 0, pedalWidth/4);
      translate(-pedalWidth/2, 0);
      ellipse(0, 0, 5, 5);
      translate(pedalWidth, 0);
      ellipse(0, 0, 5, 5);
      popMatrix();
      popStyle();
   }
}

// GLOBALS

Pedal test = new Pedal();
Accelerometers myAccel = new Accelerometers();
Accelerometers expectedAccel = new Accelerometers();

PFont droidSans48;
PFont droidSans24;
PFont droidSans12;

State globalState = new State();
State tempState = new State(); //serial state

Serial port;

InfoBox infos;

void setup()
{
   tempState.gravity = 1;
   droidSans48 = loadFont("DroidSansMono-48.vlw");
   droidSans24 = loadFont("DroidSansMono-24.vlw");
   droidSans12 = loadFont("DroidSansMono-12.vlw");
   //size(1450, 850);
   size(725, 425);
   //size(640, 480);
   infos = new InfoBox(tempState, width/2, height, droidSans24);
   //port = new Serial(this, Serial.list()[0], 19200);
   port = new Serial(this, Serial.list()[0], 57600);
   myAccel.myFont = droidSans12;
   expectedAccel.myFont = droidSans12;
   // setup camera
   println(Capture.list());
   //camera = new Capture(this, width*7/16, height*7/16, 30);
   camera = new Capture(this, width*7/16, width*32/100, 30);
   /*
   mm = new MovieMaker(this, width, height, "drawing.mov",
         30, MovieMaker.H264, MovieMaker.HIGH);
         */
   mm = new MovieMaker(this, width, height, "drawing.mov",
//         20, MovieMaker.ANIMATION, MovieMaker.HIGH);
         20, MovieMaker.ANIMATION, MovieMaker.HIGH);
   
   //camera.settings();
   //println(Capture.list());
   frameRate(40);
   smooth();
}

void captureEvent(Capture myCapture) {
  myCapture.read();
}

void keyPressed() {
  if (key == ' ') {
    mm.finish();  // Finish the movie if space bar is pressed!
  }
}


void draw()
{
   checkForXBeeData();
   if( !newFrame )
      return;
   else
      newFrame = false;

   background(0);
   stroke(255);
   fill(175);
   textFont(droidSans12);

   text(String.format("%1.0f fps", frameRate), 30, 30);
   //scale(0.5);

   /*
   pushMatrix();
   translate(width/2, height*0);
   infos.draw();
   popMatrix();
   */

   pushMatrix();
   translate(width*7/16, height*1/16);
   scale(-1.0, 1.0);
   image(camera, -width*9/16, 0);
   popMatrix();

   pushMatrix();
   translate(width/4, height/2);
   //scale(1.9);
   scale(1.1);
   //test.drawTrigger();
   test.drawCrank();
   test.drawBottomBracket();
   test.drawPedal();
   popMatrix();

   pushMatrix();
   translate(width*5/8, height*7/8);
   textAlign(CENTER);
   text("measured", 0, -height*1/8);
   myAccel.scaleFactor = 7;
   myAccel.draw();
   popMatrix();

   pushMatrix();
   translate(width*7/8, height*7/8);
   textAlign(CENTER);
   text("expected", 0, -height*1/8);
   expectedAccel.scaleFactor = 7;
   expectedAccel.draw();
   popMatrix();

   mm.addFrame();  // Add window's pixels to movie



   test.phi = intToFract(tempState.phi)*PI;
   test.triggerPhi = intToFract(tempState.trigPhi)*PI;
   test.theta = intToFract(tempState.theta)*PI;
   myAccel.xAxis = (tempState.xAxis-tempState.xAxisBias)*1.0/tempState.gravity;
   myAccel.zAxis = (tempState.zAxis-tempState.zAxisBias)*1.0/tempState.gravity;
   myAccel.yRate = (tempState.yRate-tempState.yRateBias)*5.0/1024.0;

   float floatGravity = intToFract(tempState.gravity);
   float floatCrankLength = intToAccum(tempState.crankLength);
   float floatPhiDot = intToFract(tempState.phiDot);
   float floatPhi = intToFract(tempState.phi);

   /*
    * lags by 90 deg?
   expectedAccel.xAxis = (floatCrankLength*floatPhiDot*floatPhiDot*cos(-floatPhi*PI))/floatGravity;
   expectedAccel.zAxis = (-floatGravity - floatCrankLength*floatPhiDot*floatPhiDot*sin(-floatPhi*PI))/floatGravity;
   */
   /*
    * x is mirrored
   expectedAccel.xAxis = (floatCrankLength*floatPhiDot*floatPhiDot*sin(-floatPhi*PI))/floatGravity;
   expectedAccel.zAxis = (-floatGravity - floatCrankLength*floatPhiDot*floatPhiDot*cos(-floatPhi*PI))/floatGravity;
   */
   expectedAccel.xAxis = -1*(floatCrankLength*floatPhiDot*floatPhiDot*sin(-floatPhi*PI))/floatGravity;
   expectedAccel.zAxis = (-floatGravity - floatCrankLength*floatPhiDot*floatPhiDot*cos(-floatPhi*PI))/floatGravity;



   //println(tempState.theta);
}


int bytesToGet;
int bytesGot;
int byteSubCounter;
Boolean readingFrame = false;
Boolean escapeOn = false;
Boolean newFrame = false;

/*
 * FRAME DATA SPECIFICATION
 *
 */

int FrameFlagState              = (1 << 1);
int FrameFlagStateErrors        = (1 << 2);
int FrameFlagMeasures           = (1 << 3);
int FrameFlagMeasureNoise       = (1 << 4);
int FrameFlagMeasureBiases      = (1 << 5);
int FrameFlagPerformance        = (1 << 6);
int FrameFlagHmatrix            = (1 << 7);
int FrameFlagShwattStatus       = (1 << 0);

int StateLength         = 10;
//int StateErrorsLength   = StateLength;
int StateErrorsLength   = 14;
int MeasuresLength      = 10;
//int MeasureNoiseLength  = MeasuresLength;
//int MeasureBiasesLength = MeasuresLength;
int MeasureNoiseLength  = 6;
int MeasureBiasesLength = 6;
int ShwattStatusLength  = 3;
int PerformanceLength   = 10;
int HmatrixLength       = 6;

int HasState, HasStateErrors, HasPerformance, HasMeasures, HasMeasureNoise, HasMeasureBiases, HasHmatrix, HasStatus;

int StartByteBroadcastData = 15;
int StartByteState         = StartByteBroadcastData+1; //broadast data
int StartByteStateErrors   = StartByteState + StateLength*HasState;
int StartByteStatus        = StartByteStateErrors + StateErrorsLength*HasStateErrors;
int StartBytePerformance   = StartByteStatus      + ShwattStatusLength*HasStatus;
int StartByteMeasures      = StartBytePerformance + PerformanceLength*HasPerformance;
int StartByteMeasureNoise  = StartByteMeasures + MeasuresLength*HasMeasures;
int StartByteMeasureBiases = StartByteMeasureNoise + MeasureNoiseLength*HasMeasureNoise;
int StartByteHmatrix       = StartByteMeasureBiases + MeasureBiasesLength*HasMeasureBiases;

void checkForXBeeData()
{
   while (port.available() > 0)
   {
      int value = port.read();
      if (value == 0x7E) {
         //println("new frame");
         readingFrame = true;
         bytesToGet = -1;
         bytesGot = -1;
         newFrame = true;
         if (tempState.startTime == 0)
         {
            tempState.startTime = millis();
         }
         tempState.messages += 1;
         continue;
      }
      if (!readingFrame)
      {
         //print("got crap: ");
         //println(value);
         continue;
      }
      if (value == 0x7D) {
         escapeOn = true;
         continue;
      }
      if (escapeOn) {
         value = value ^ 0x20;
         escapeOn = false;
      }
      if (bytesToGet == -1 && bytesGot == -1)
      {
         bytesToGet = 256*value;
         continue;
      } else if (bytesGot == -1) {
         bytesToGet += value;
         bytesGot = 0;
         //print("Gotlength: ");
         //println(bytesToGet);
         continue;
      }

      bytesGot += 1;
      byteSubCounter += 1;

      if (bytesGot == StartByteBroadcastData)
      {
         //print("Got broadcast data:");
         //println(value);
         // SO DUMB
         HasState         = ((value & FrameFlagState)         !=  0 ) ? 1: 0 ;
         HasStateErrors   = ((value & FrameFlagStateErrors)   !=  0 ) ? 1: 0 ;
         HasPerformance   = ((value & FrameFlagPerformance)   !=  0 ) ? 1: 0 ;
         HasMeasures      = ((value & FrameFlagMeasures)      !=  0 ) ? 1: 0 ;
         HasMeasureNoise  = ((value & FrameFlagMeasureNoise)  !=  0 ) ? 1: 0 ;
         HasMeasureBiases = ((value & FrameFlagMeasureBiases) !=  0 ) ? 1: 0 ;
         HasHmatrix       = ((value & FrameFlagHmatrix   )    !=  0 ) ? 1: 0 ;
         HasStatus        = ((value & FrameFlagShwattStatus ) !=  0 ) ? 1: 0 ;

         StartByteState         = StartByteBroadcastData+1; //broadast data
         StartByteStateErrors   = StartByteState + StateLength*HasState;
         StartByteStatus        = StartByteStateErrors + StateErrorsLength*HasStateErrors;
         StartBytePerformance   = StartByteStatus      + ShwattStatusLength*HasStatus;
         StartByteMeasures      = StartBytePerformance + PerformanceLength*HasPerformance;
         StartByteMeasureNoise  = StartByteMeasures + MeasuresLength*HasMeasures;
         StartByteMeasureBiases = StartByteMeasureNoise + MeasureNoiseLength*HasMeasureNoise;
         StartByteHmatrix       = StartByteMeasureBiases + MeasureBiasesLength*HasMeasureBiases;

         byteSubCounter = -1;
         continue;
      }

      if (bytesGot == StartByteState+byteSubCounter && HasState > 0)
      {
         switch (byteSubCounter) {
            case 0 : tempState.theta  =  ( tempState.theta  &  0x00FF ) |  (  value << 8 );                break; 
            case 1 : tempState.theta  =  ( tempState.theta  &  0xFF00 ) |  (  value      );                break; 
            case 2 : tempState.phi    =  ( tempState.phi    &  0x00FF ) |  (  value << 8 );                break; 
            case 3 : tempState.phi    =  ( tempState.phi    &  0xFF00 ) |  (  value      );                break; 
            case 4 : tempState.phiDot =  ( tempState.phiDot &  0x00FF ) |  (  value << 8 );                break; 
            case 5 : tempState.phiDot =  ( tempState.phiDot &  0xFF00 ) |  (  value      );                break; 
                     /*
            case 6 : tempState.crankLength =  ( tempState.crankLength &  0x00FF ) |  (  value << 8 );                break; 
            case 7 : tempState.crankLength =  ( tempState.crankLength &  0xFF00 ) |  (  value      ); byteSubCounter = -1; break; 
            */
            case 6 : tempState.crankLength  =  ( tempState.crankLength  &  0x00FFFFFF ) |  (  value << 24 );  break; 
            case 7 : tempState.crankLength  =  ( tempState.crankLength  &  0xFF00FFFF ) |  (  value << 16 );  break; 
            case 8 : tempState.crankLength  =  ( tempState.crankLength  &  0xFFFF00FF ) |  (  value << 8  );  break; 
            case 9 : tempState.crankLength  =  ( tempState.crankLength  &  0xFFFFFF00 ) |  (  value       );  byteSubCounter = -1; break; 
         }
         continue;
      }

      if (bytesGot == StartByteStateErrors+byteSubCounter && HasStateErrors > 0)
      {
         switch (byteSubCounter) {

            case 0 : tempState.thetaError  =  ( tempState.thetaError  &  0x00FFFFFF ) |  (  value << 24 );  break; 
            case 1 : tempState.thetaError  =  ( tempState.thetaError  &  0xFF00FFFF ) |  (  value << 16 );  break; 
            case 2 : tempState.thetaError  =  ( tempState.thetaError  &  0xFFFF00FF ) |  (  value << 8  );  break; 
            case 3 : tempState.thetaError  =  ( tempState.thetaError  &  0xFFFFFF00 ) |  (  value       );  break; 
            case 4 : tempState.phiError  =  ( tempState.phiError  &  0x00FFFFFF ) |  (  value << 24 );  break; 
            case 5 : tempState.phiError  =  ( tempState.phiError  &  0xFF00FFFF ) |  (  value << 16 );  break; 
            case 6 : tempState.phiError  =  ( tempState.phiError  &  0xFFFF00FF ) |  (  value << 8  );  break; 
            case 7 : tempState.phiError  =  ( tempState.phiError  &  0xFFFFFF00 ) |  (  value       );  break; 
            case 8 : tempState.phiDotError  =  ( tempState.phiDotError  &  0x00FFFFFF ) |  (  value << 24 );  break; 
            case 9 : tempState.phiDotError  =  ( tempState.phiDotError  &  0xFF00FFFF ) |  (  value << 16 );  break; 
            case 10 : tempState.phiDotError  =  ( tempState.phiDotError  &  0xFFFF00FF ) |  (  value << 8  );  break; 
            case 11: tempState.phiDotError  =  ( tempState.phiDotError  &  0xFFFFFF00 ) |  (  value       );  break; 

            case 12 : tempState.crankError =  ( tempState.crankError &  0x00FF ) |  (  value << 8 );                break; 
            case 13 : tempState.crankError =  ( tempState.crankError &  0xFF00 ) |  (  value      ); byteSubCounter = -1; break; 
/*
            case 0 : tempState.thetaError  =  ( tempState.thetaError  &  0x00FF ) |  (  value << 8 );                break; 
            case 1 : tempState.thetaError  =  ( tempState.thetaError  &  0xFF00 ) |  (  value      );                break; 
            case 2 : tempState.phiError    =  ( tempState.phiError    &  0x00FF ) |  (  value << 8 );                break; 
            case 3 : tempState.phiError    =  ( tempState.phiError    &  0xFF00 ) |  (  value      );                break; 
            case 4 : tempState.phiDotError =  ( tempState.phiDotError &  0x00FF ) |  (  value << 8 );                break; 
            case 5 : tempState.phiDotError =  ( tempState.phiDotError &  0xFF00 ) |  (  value      );  break; 
            case 6 : tempState.crankError =  ( tempState.crankError &  0x00FF ) |  (  value << 8 );                break; 
            case 7 : tempState.crankError =  ( tempState.crankError &  0xFF00 ) |  (  value      ); byteSubCounter = -1; break; 
            */
         }
         continue;
      }

      if (bytesGot == StartByteStatus+byteSubCounter && HasStatus > 0)
      {
         switch (byteSubCounter) {
            case 0 : tempState.KalmanState  =  value;  break;
            case 1 : tempState.TriggerState =  value;  break;
            case 2 : tempState.CalibrationState = value; byteSubCounter = -1; break;
         }
      }


      if (bytesGot == StartBytePerformance+byteSubCounter && HasPerformance > 0)
      {
         switch (byteSubCounter) {
            case 0 : tempState.performance  =  ( tempState.performance  &  0x00FF ) |  (  value << 8 );                break; 
            case 1 : tempState.performance  =  ( tempState.performance  &  0xFF00 ) |  (  value      );                 break; 
            case 2 : tempState.gyroGain  =  ( tempState.gyroGain  &  0x00FFFFFF ) |  (  value << 24 );  break; 
            case 3 : tempState.gyroGain  =  ( tempState.gyroGain  &  0xFF00FFFF ) |  (  value << 16 );  break; 
            case 4 : tempState.gyroGain  =  ( tempState.gyroGain  &  0xFFFF00FF ) |  (  value << 8  );  break; 
            case 5 : tempState.gyroGain  =  ( tempState.gyroGain  &  0xFFFFFF00 ) |  (  value       );  break; 
            case 6 : tempState.gravity  =  ( tempState.gravity & 0x00FF ) | (value << 8);  break;
            case 7 : tempState.gravity  =  ( tempState.gravity & 0xFF00 ) | (value );  break;
            case 8 : tempState.timeSinceLast = (tempState.timeSinceLast & 0x00FF) | (value << 8); break;
            case 9 : tempState.timeSinceLast = (tempState.timeSinceLast & 0xFF00) | (value); byteSubCounter = -1; break;
         }
         continue;
      }

      if (bytesGot == StartByteMeasures+byteSubCounter && HasMeasures > 0)
      {
         switch (byteSubCounter) {
            case 0 : tempState.xAxis =  ( tempState.xAxis &  0x00FF ) |  (  value << 8 );                break; 
            case 1 : tempState.xAxis =  ( tempState.xAxis &  0xFF00 ) |  (  value      );                break; 
            case 2 : tempState.zAxis =  ( tempState.zAxis &  0x00FF ) |  (  value << 8 );                break; 
            case 3 : tempState.zAxis =  ( tempState.zAxis &  0xFF00 ) |  (  value      );                break; 
            case 4 : tempState.yRate =  ( tempState.yRate &  0x00FF ) |  (  value << 8 );                break; 
            case 5 : tempState.yRate =  ( tempState.yRate &  0xFF00 ) |  (  value      );                break; 
            case 6 : tempState.trigPhi =  ( tempState.trigPhi &  0x00FF ) |  (  value << 8 );            break; 
            case 7 : tempState.trigPhi =  ( tempState.trigPhi &  0xFF00 ) |  (  value      );            break; 
            case 8 : tempState.trigPhiDot =  ( tempState.trigPhiDot &  0x00FF ) |  (  value << 8 );                break; 
            case 9 : tempState.trigPhiDot =  ( tempState.trigPhiDot &  0xFF00 ) |  (  value      ); byteSubCounter = -1; break; 
         }
         continue;
      }

      if (bytesGot == StartByteMeasureNoise+byteSubCounter && HasMeasureNoise > 0)
      {
         switch (byteSubCounter) {
            case 0 : tempState.xAxisNoise =  ( tempState.xAxisNoise &  0x00FF ) |  (  value << 8 );                break; 
            case 1 : tempState.xAxisNoise =  ( tempState.xAxisNoise &  0xFF00 ) |  (  value      );                break; 
            case 2 : tempState.zAxisNoise =  ( tempState.zAxisNoise &  0x00FF ) |  (  value << 8 );                break; 
            case 3 : tempState.zAxisNoise =  ( tempState.zAxisNoise &  0xFF00 ) |  (  value      );                break; 
            case 4 : tempState.yRateNoise =  ( tempState.yRateNoise &  0x00FF ) |  (  value << 8 );                break; 
            case 5 : tempState.yRateNoise =  ( tempState.yRateNoise &  0xFF00 ) |  (  value      ); byteSubCounter = -1; break; 
         }
         continue;
      }

      if (bytesGot == StartByteMeasureBiases+byteSubCounter && HasMeasureBiases > 0)
      {
         switch (byteSubCounter) {
            case 0 : tempState.xAxisBias =  ( tempState.xAxisBias &  0x00FF ) |  (  value << 8 );                break; 
            case 1 : tempState.xAxisBias =  ( tempState.xAxisBias &  0xFF00 ) |  (  value      );                break; 
            case 2 : tempState.zAxisBias =  ( tempState.zAxisBias &  0x00FF ) |  (  value << 8 );                break; 
            case 3 : tempState.zAxisBias =  ( tempState.zAxisBias &  0xFF00 ) |  (  value      );                break; 
            case 4 : tempState.yRateBias =  ( tempState.yRateBias &  0x00FF ) |  (  value << 8 );                break; 
            case 5 : tempState.yRateBias =  ( tempState.yRateBias &  0xFF00 ) |  (  value      ); byteSubCounter = -1; break; 
         }
         continue;
      }

      if (bytesGot == StartByteHmatrix+byteSubCounter && HasHmatrix > 0)
      {
         switch (byteSubCounter) {
            case 0 : tempState.hMatrixTheta =  ( tempState.hMatrixTheta &  0x00FF ) |  (  value << 8 );                break; 
            case 1 : tempState.hMatrixTheta =  ( tempState.hMatrixTheta &  0xFF00 ) |  (  value      ); break; 
            case 2 : tempState.hMatrixPhi =  ( tempState.hMatrixPhi &  0x00FF ) |  (  value << 8 );                break; 
            case 3 : tempState.hMatrixPhi =  ( tempState.hMatrixPhi &  0xFF00 ) |  (  value      );                break; 
            case 4 : tempState.hMatrixPhiDot =  ( tempState.hMatrixPhiDot &  0x00FF ) |  (  value << 8 );                break; 
            case 5 : tempState.hMatrixPhiDot =  ( tempState.hMatrixPhiDot &  0xFF00 ) |  (  value      ); byteSubCounter = -1; break; 
         }
         continue;
      }

      //print(value);
      //print(":");
      if (bytesGot == bytesToGet)
      {
         //println("");
         readingFrame = false;
         //println("done");
         continue;
      }
   }
}

      
      



















