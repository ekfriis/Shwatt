import processing.core.*; 
import processing.xml.*; 

import processing.serial.*; 
import java.util.*; 

import java.applet.*; 
import java.awt.*; 
import java.awt.image.*; 
import java.awt.event.*; 
import java.io.*; 
import java.net.*; 
import java.text.*; 
import java.util.*; 
import java.util.zip.*; 
import java.util.regex.*; 

public class ShwattMonitor extends PApplet {




class State {
   int BroadcastData;
   int theta;
   int phi;
   int phiDot;
   int thetaError;
   int phiError;
   int phiDotError;
   int performance;
   int xAxis;
   int zAxis;
   int yRate;
   int xAxisNoise;
   int zAxisNoise;
   int yRateNoise;
   int xAxisBias;
   int zAxisBias;
   int yRateBias;
   int KalmanState;
   int gravity;
   int messages;
   int timeSinceLast;
   int startTime;
   public float messageRate()
   {
      if (messages > 0)
      {
         return ( 1000.f/((millis()-startTime)/messages) );
      } else return 0.0f;
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

public String inHex(int i)
{
   //java sucks
   // from mit via google
   return Integer.toHexString( 0x10000 | i).substring(1).toUpperCase();
}

public String inBits(int i)
{
   return Integer.toBinaryString(i);
}

public float intToFract(int i)
{
   Boolean isNeg = false;
   if ((i & 0x8000) > 0)
   {
      i &= ~(0x8000);
      isNeg = true;
      i *= -1;
   }

   float output = 1.0f*i/(1 << 15);
   if (isNeg)
      output *= -1.0f;
   return output;
}

public String hexAndFract(int it)
{
   String output = inHex(it);
   output += String.format("(%+1.2f)", intToFract(it));
   return output;
}


public String gravityLabel(String label, int value, int bias, int gravity)
{
   String output = String.format(label+"%+1.2f" , (value-bias)*1.0f/gravity);
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
   public void draw()
   {
      pushStyle();
      pushMatrix();
      textFont(myFont);
      noStroke();
      fill(25);
      rect(myWidth/20.f, myHeight/20.f, myWidth*(18.f/20), myHeight*(18.f/20));
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
      text(inBits(myState.KalmanState),0,0);
      popStyle();
      popMatrix();

      // heaaders
      translate(0, 30);
      pushMatrix();
      translate(myWidth/4.5f, 0);
      pushMatrix();
      scale(0.5f);
      text("raw measure", 0, 0);
      popMatrix();
      translate(myWidth/4.5f, 0);
      pushMatrix();
      scale(0.5f);
      text("measure noise", 0, 0);
      popMatrix();
      translate(myWidth/4.5f, 0);
      pushMatrix();
      scale(0.5f);
      text("measure bias", 0, 0);
      popMatrix();
      popMatrix();

      translate(0, 30);

      pushMatrix();
      fill(230, 100, 0);
      text("yRate:", 0, 0);
      fill(255);
      translate(myWidth/4.5f, 0);
      text("0x"+inHex(myState.yRate), 0, 0);
      translate(myWidth/4.5f, 0);
      text("0x"+inHex(myState.yRateNoise), 0, 0);
      translate(myWidth/4.5f, 0);
      text("0x"+inHex(myState.yRateBias), 0, 0);
      popMatrix();

      // xAxis info
      translate(0, 30);
      pushMatrix();
      fill(230, 100, 0);
      text("xAxis:", 0, 0);
      fill(255);
      translate(myWidth/4.5f, 0);
      text("0x"+inHex(myState.xAxis), 0, 0);
      translate(myWidth/4.5f, 0);
      text("0x"+inHex(myState.xAxisNoise), 0, 0);
      translate(myWidth/4.5f, 0);
      text("0x"+inHex(myState.xAxisBias), 0, 0);
      popMatrix();

      // zaXis info
      translate(0, 30);
      pushMatrix();
      fill(230, 100, 0);
      text("zAxis:", 0, 0);
      fill(255);
      translate(myWidth/4.5f, 0);
      text("0x"+inHex(myState.zAxis), 0, 0);
      translate(myWidth/4.5f, 0);
      text("0x"+inHex(myState.zAxisNoise), 0, 0);
      translate(myWidth/4.5f, 0);
      text("0x"+inHex(myState.zAxisBias), 0, 0);
      popMatrix();

      // gravity
      translate(0, 30);
      pushMatrix();
      fill(20, 100, 250);
      text("gravity:", 0, 0);
      fill(255);
      translate(myWidth/4.5f, 0);
      text("0x"+inHex(myState.gravity), 0, 0);
      translate(myWidth/4.5f, 0);
      text(gravityLabel("z:", myState.zAxis, myState.zAxisBias, myState.gravity), 0, 0);
      translate(myWidth/4.5f, 0);
      text(gravityLabel("x:", myState.xAxis, myState.xAxisBias, myState.gravity), 0, 0);
      popMatrix();

      translate(0, 30);
      pushMatrix();
      fill(100, 100, 30);
      text("Msg. Rate:", 0, 0);
      translate(myWidth/4.5f, 0);
      text(String.format("%3.1fHz", myState.messageRate()), 0, 0);
      translate(myWidth/4.5f, 0);
      text("K-Rate:", 0, 0);
      translate(myWidth/4.5f, 0);
      text(String.format("%3.1fHz", myState.messageRate()*myState.performance), 0, 0);
      popMatrix();

      translate(0, 40);
      pushMatrix();
      text("theta:", 0, 0);
      translate(myWidth/4.5f, 0);
      //text(String.format(inHex(myState.theta)"%4x(%+1.2f)",myState.theta,intToFract(myState.theta)), 0, 0);
      //text(String.format("%1.2f",intToFract(myState.theta)), 0, 0);
      text(hexAndFract(myState.theta), 0, 0);
      translate(myWidth/4.5f, 0);
      text("error:", 0, 0);
      translate(myWidth/4.5f, 0);
      text(inHex(myState.thetaError), 0, 0);
      popMatrix();

      translate(0, 40);
      pushMatrix();
      text("phi:", 0, 0);
      translate(myWidth/4.5f, 0);
      text(inHex(myState.phi), 0, 0);
      translate(myWidth/4.5f, 0);
      text("error:", 0, 0);
      translate(myWidth/4.5f, 0);
      text(inHex(myState.phiError), 0, 0);
      popMatrix();

      translate(0, 40);
      pushMatrix();
      text("phiDot:", 0, 0);
      translate(myWidth/4.5f, 0);
      text(inHex(myState.phiDot), 0, 0);
      translate(myWidth/4.5f, 0);
      text("error:", 0, 0);
      translate(myWidth/4.5f, 0);
      text(inHex(myState.phiDotError), 0, 0);
      popMatrix();

      translate(0, 40);
      text(inHex(myState.timeSinceLast), 0, 0);

      popStyle();
      popMatrix();
   }
}

class Pedal {
   float theta;
   float phi;
   float phiDot;
   float crankLength;
   float pedalWidth = 70;

   Pedal() {
      theta       = 0;
      phi         = 0;
      phiDot      = 0;
      crankLength = 165;
   }

   public void drawCrank()
   {
      pushMatrix();
      pushStyle();
      rotate(phi);
      stroke(0, 0, 200);
      strokeWeight(2);
      line(0, 0, crankLength, 0);
      strokeWeight(0.25f);
      line(-crankLength/5, 0, 0, 0);
      popMatrix();
      popStyle();
   }

   public void drawBottomBracket()
   {
      pushStyle();
      strokeWeight(2);
      fill(0);
      ellipse(0, 0, 10, 10);
      popStyle();
   }

   public void drawPedal()
   {
      pushMatrix();
      pushStyle();
      // get to edge of crank
      rotate(phi);
      translate(crankLength, 0);
      // back to regular frame
      rotate(-1.0f*phi);
      // draw reference line
      strokeWeight(0.3f);
      line(0, pedalWidth/4, 0, -pedalWidth/2.5f); // gravity
      noFill();
      arc(0, 0, pedalWidth/3, pedalWidth/3, -1*PI/2+PI/20, theta-PI/9);

      fill(0);
      rotate(theta);
      strokeWeight(5);
      stroke(200, 20, 111);
      line(-pedalWidth/2, 0, pedalWidth/2, 0);
      translate(-pedalWidth/2, 0);
      ellipse(0, 0, 5, 5);
      translate(pedalWidth, 0);
      ellipse(0, 0, 5, 5);
      popMatrix();
      popStyle();
   }
}

Pedal test = new Pedal();

PFont droidSans48;
PFont droidSans24;

State globalState = new State();
State tempState = new State(); //serial state

Serial port;

InfoBox infos;

public void setup()
{
   droidSans48 = loadFont("DroidSansMono-48.vlw");
   droidSans24 = loadFont("DroidSansMono-24.vlw");
   size(1450, 850);
   infos = new InfoBox(tempState, width/2, height, droidSans24);
   port = new Serial(this, Serial.list()[0], 19200);
   smooth();
}

public void draw()
{
   background(0);
   stroke(255);
   fill(175);

   pushMatrix();
   translate(width/4, height/2);
   scale(1.9f);
   test.drawCrank();
   test.drawBottomBracket();
   test.drawPedal();
   popMatrix();


   pushMatrix();
   translate(width/2, height*0);
   infos.draw();
   popMatrix();

   delay(10);
   test.phi += 0.01f;
   test.theta = 0.5f*sin(test.phi);
   checkForXBeeData();
   //println(tempState.theta);
}


int bytesToGet;
int bytesGot;
int byteSubCounter;
Boolean readingFrame = false;
Boolean escapeOn = false;

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
int FrameFlagShwattStatus       = (1 << 8);

int StateLength         = 6;
int StateErrorsLength   = StateLength;
int MeasuresLength      = 6;
int MeasureNoiseLength  = MeasuresLength;
int MeasureBiasesLength = MeasuresLength;
int ShwattStatusLength  = 1;
int PerformanceLength   = 7;

int HasState, HasStateErrors, HasPerformance, HasMeasures, HasMeasureNoise, HasMeasureBiases;

int StartByteBroadcastData = 15;
int StartByteState         = StartByteBroadcastData+1; //broadast data
int StartByteStateErrors   = StartByteState + StateLength*HasState;
int StartBytePerformance   = StartByteStateErrors + StateErrorsLength*HasStateErrors;
int StartByteMeasures      = StartBytePerformance + PerformanceLength*HasPerformance;
int StartByteMeasureNoise  = StartByteMeasures + MeasuresLength*HasMeasures;
int StartByteMeasureBiases = StartByteMeasureNoise + MeasureNoiseLength*HasMeasureNoise;

public void checkForXBeeData()
{
   while (port.available() > 0)
   {
      int value = port.read();
      if (value == 0x7E) {
         //println("new frame");
         readingFrame = true;
         bytesToGet = -1;
         bytesGot = -1;
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

         StartByteState         = StartByteBroadcastData+1; //broadast data
         StartByteStateErrors   = StartByteState + StateLength*HasState;
         StartBytePerformance   = StartByteStateErrors + StateErrorsLength*HasStateErrors;
         StartByteMeasures      = StartBytePerformance + PerformanceLength*HasPerformance;
         StartByteMeasureNoise  = StartByteMeasures + MeasuresLength*HasMeasures;
         StartByteMeasureBiases = StartByteMeasureNoise + MeasureNoiseLength*HasMeasureNoise;

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
            case 5 : tempState.phiDot =  ( tempState.phiDot &  0xFF00 ) |  (  value      ); byteSubCounter = -1; break; 
         }
         continue;
      }

      if (bytesGot == StartByteStateErrors+byteSubCounter && HasStateErrors > 0)
      {
         switch (byteSubCounter) {
            case 0 : tempState.thetaError  =  ( tempState.thetaError  &  0x00FF ) |  (  value << 8 );                break; 
            case 1 : tempState.thetaError  =  ( tempState.thetaError  &  0xFF00 ) |  (  value      );                break; 
            case 2 : tempState.phiError    =  ( tempState.phiError    &  0x00FF ) |  (  value << 8 );                break; 
            case 3 : tempState.phiError    =  ( tempState.phiError    &  0xFF00 ) |  (  value      );                break; 
            case 4 : tempState.phiDotError =  ( tempState.phiDotError &  0x00FF ) |  (  value << 8 );                break; 
            case 5 : tempState.phiDotError =  ( tempState.phiDotError &  0xFF00 ) |  (  value      ); byteSubCounter = -1; break; 
         }
         continue;
      }

      if (bytesGot == StartBytePerformance+byteSubCounter && HasPerformance > 0)
      {
         switch (byteSubCounter) {
            case 0 : tempState.performance  =  ( tempState.performance  &  0x00FF ) |  (  value << 8 );                break; 
            case 1 : tempState.performance  =  ( tempState.performance  &  0xFF00 ) |  (  value      );                 break; 
            case 2 : tempState.KalmanState  =  value;  break;
            case 3 : tempState.gravity  =  ( tempState.gravity & 0x00FF ) | (value << 8);  break;
            case 4 : tempState.gravity  =  ( tempState.gravity & 0xFF00 ) | (value );  break;
            case 5 : tempState.timeSinceLast = (tempState.timeSinceLast & 0x00FF) | (value << 8); break;
            case 6 : tempState.timeSinceLast = (tempState.timeSinceLast & 0xFF00) | (value); byteSubCounter = -1; break;
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
            case 5 : tempState.yRate =  ( tempState.yRate &  0xFF00 ) |  (  value      ); byteSubCounter = -1; break; 
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

      
      




















  static public void main(String args[]) {
    PApplet.main(new String[] { "--bgcolor=#ffffff", "ShwattMonitor" });
  }
}
