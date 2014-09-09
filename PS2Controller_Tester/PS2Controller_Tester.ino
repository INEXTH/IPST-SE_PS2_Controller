/*
 * Copyright (c) 2014 Innovative Experiment Co.,Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/* 
 * ตัวอย่างการใช้งานบอร์ด IPST-SE ร่วมกับ PS2 Controller
 *
 * โปรแกรมทดสอบการทำงานของ PS2 Controller
 *
 * ใช้ทดสอบ PS2 Controller โดยมีหน้าจอแสดงผลเป็นเลย์เอาท์ของปุ่มต่างๆบนตัว PS2 Controller (เป็นภาพสีเทา)
 * เมื่อกดปุ่มใดๆบน PS2 Controller ก็จะแสดงบนหน้าจอเป็นภาพว่าปุ่มนั้นๆถูกกดอยู่ (สีแตกต่างกันไปตามแต่ละปุ่ม)
 * และแสดงพิกัดของคันโยกอะนาล็อกด้วยจุดสีแดง ซึ่งจะเคลื่อนตำแหน่งไปตามการโยกคันโยก
 */

#include <ipst.h>                                // เรียกใช้งานไลบรารีสำหรับ IPST-SE
#include <PS2X_lib.h>                            // เรียกใช้งานไลบรารีสำหรับ PS2 Controller

#define PS2_DAT        16                        // กำหนดขา Data    เป็นขา 16
#define PS2_CMD        17                        // กำหนดขา Command เป็นขา 17
#define PS2_SEL        18                        // กำหนดขา Select  เป็นขา 18
#define PS2_CLK        19                        // กำหนดขา Clock   เป็นขา 19

#define GLCD_DKGRAY    colorRGB(10, 20, 10)      // กำหนดค่าสีสำหรับสีเทาเข้ม
#define GLCD_GRAY      colorRGB(27, 54, 27)      // กำหนดค่าสีสำหรับสีเทา
#define GLCD_DKGREEN   colorRGB(0, 50, 0)        // กำหนดค่าสีสำหรับสีเขียวเข้ม
#define GLCD_ORANGE    colorRGB(31, 40, 0)       // กำหนดค่าสีสำหรับสีส้ม

PS2X ps2x;                                       // ประกาศตัวแปรสำหรับ PS2 Controller

int oldLX = 0;                                   // เก็บค่าตำแหน่งพิกัด X ก่อนหน้าขอ Analog ฝั่งซ้าย
int oldLY = 0;                                   // เก็บค่าตำแหน่งพิกัด Y ก่อนหน้าขอ Analog ฝั่งซ้าย
int oldRX = 0;                                   // เก็บค่าตำแหน่งพิกัด X ก่อนหน้าขอ Analog ฝั่งขวา
int oldRY = 0;                                   // เก็บค่าตำแหน่งพิกัด Y ก่อนหน้าขอ Analog ฝั่งขวา

void setup() 
{
  delay(1000);                                   // หน่วงเวลา 1 วินาทีเพื่อรอให้บอร์ดพร้อมทำงาน
  glcdClear();                                   // เคลียร์ภาพทั้งหมดบนหน้าจอแสดงผล
  glcdMode(1);                                   // กำหนดให้หน้าจอแสดงผลเป็นแนวนอน
  glcdFillScreen(GLCD_WHITE);                    // กำหนดพื้นหลังของหน้าจอเป็นสีขาว
  setTextBackgroundColor(GLCD_WHITE);            // กำหนดพื้นหลังของตัวหนังสือเป็นสีขาว

  setTextColor(GLCD_BLACK);                      // กำหนดตัวหนังสือเป็นสีดำ
  glcd(0, 0, "Connecting");                      // แสดงข้อความเพื่อให้รู้ว่ากำลังทำการเชื่อมต่อกับ PS2 Controller
      
  initController();                              // เรียกฟังก์ชันทำการเชื่อมต่อกับ PS2 Controller
}

void loop() 
{
  ps2x.read_gamepad(false, false);               // อ่านข้อมูลจาก PS2 Controller

  checkButtonCircle();                           // เรียกฟังก์ชันเช็คว่าปุ่มวงกลมถูกกดหรือไม่
  checkButtonCross();                            // เรียกฟังก์ชันเช็คว่าปุ่มกากบาทถูกกดหรือไม่
  checkButtonSquare();                           // เรียกฟังก์ชันเช็คว่าปุ่มสี่เหลี่ยมถูกกดหรือไม่
  checkButtonTriangle();                         // เรียกฟังก์ชันเช็คว่าปุ่มสามเหลี่ยมถูกกดหรือไม่

  checkButtonUp();                               // เรียกฟังก์ชันเช็คว่าปุ่มขึ้นถูกกดหรือไม่
  checkButtonDown();                             // เรียกฟังก์ชันเช็คว่าปุ่มลงถูกกดหรือไม่
  checkButtonLeft();                             // เรียกฟังก์ชันเช็คว่าปุ่มซ้ายถูกกดหรือไม่
  checkButtonRight();                            // เรียกฟังก์ชันเช็คว่าปุ่มขวาถูกกดหรือไม่  

  checkButtonL1();                               // เรียกฟังก์ชันเช็คว่าปุ่ม L1 ถูกกดหรือไม่  
  checkButtonL2();                               // เรียกฟังก์ชันเช็คว่าปุ่ม L2 ถูกกดหรือไม่ 
  checkButtonL3();                               // เรียกฟังก์ชันเช็คว่าปุ่ม L3 ถูกกดหรือไม่ 
  checkButtonR1();                               // เรียกฟังก์ชันเช็คว่าปุ่ม R1 ถูกกดหรือไม่ 
  checkButtonR2();                               // เรียกฟังก์ชันเช็คว่าปุ่ม R2 ถูกกดหรือไม่ 
  checkButtonR3();                               // เรียกฟังก์ชันเช็คว่าปุ่ม R3 ถูกกดหรือไม่ 
  
  checkButtonSelect();                           // เรียกฟังก์ชันเช็คว่าปุ่ม Select ถูกกดหรือไม่ 
  checkButtonStart();                            // เรียกฟังก์ชันเช็คว่าปุ่ม Start ถูกกดหรือไม่ 
  
  checkAnalogL();                                // เรียกฟังก์ชันเช็คตำแหน่ง XY ของคันโยก Analog ฝั่งซ้าย
  checkAnalogR();                                // เรียกฟังก์ชันเช็คตำแหน่ง XY ของคันโยก Analog ฝั่งขวา
  
  delay(50);                                     // หน่วงเวลา 50 มิลลิวินาที
}

void initController() {                          // ฟังก์ชันสำหรับเชื่อมต่อกับ PS2 Controller
  while(true)                                    // วนการทำงานเพื่อรอการเชื่อมต่อกับ PS2 Controller
  {
    // กำหนดขาที่จะเชื่อมต่กับ PS2 Controller โดยมีการเก็บค่าที่ส่งกลับมาเป็น Integer เพื่อรู้ได้ว่าเชื่อมต่อได้หรือไม่
    int error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, false, false);
  
    if(error == 0)                               // กรณีที่เชื่อมต่อได้ ไม่มีปัญหาอะไร (Error = 0)
    {
      initGlcd();                                // เตรียมภาพบนหน้าจอ
      break;                                     // ออกจาก while(true)
    } 
    delay(500);                                  // หน่วงเวลา 500 มิลลิวินาทีเพื่อรอการเชื่อมต่อครั้งต่อไปในกรณีที่เชื่อมต่อไม่สำเร็จ
  }
}

void initGlcd()                                  // ฟังก์ชันเตรียมภาพบนหน้าจอตอนเริ่มต้นทำงาน
{
  glcdClear();                                   // เคลียร์ภาพบนหน้าจอ
  setTextColor(GLCD_RED);                        // กำหนดตัวอักษรเป็นสีแดง
  glcd(0, 7, "PlayStation 2");                   // แสดงคำว่า PlayStation 2 ที่แถวแรกสุดตรงกลาง
  setTextColor(GLCD_BLUE);                       // กำหนดตัวอักษรเป็นสีแดง
  glcd(1, 9, "Controller");                      // แสดงคำว่า Controller ที่แถวที่สองตรงกลาง
  setTextColor(GLCD_BLACK);                      // กำหนดตัวอักษรเป็นสีแดง
  glcd(2, 13, "OK");                             // แสดงคำว่า OK ที่แถวที่สามตรงกลาง
    
  drawButtonUpOff();                             // เรียกฟังก์ชันวาดภาพปุ่มขึ้นที่ยังไม่ถูกกด (เป็นภาพสีเทา)
  drawButtonDownOff();                           // เรียกฟังก์ชันวาดภาพปุ่มลงที่ยังไม่ถูกกด (เป็นภาพสีเทา)
  drawButtonLeftOff();                           // เรียกฟังก์ชันวาดภาพปุ่มซ้ายที่ยังไม่ถูกกด (เป็นภาพสีเทา)
  drawButtonRightOff();                          // เรียกฟังก์ชันวาดภาพปุ่มขวาที่ยังไม่ถูกกด (เป็นภาพสีเทา)

  drawButtonCircleOff();                         // เรียกฟังก์ชันวาดภาพปุ่มวงกลมที่ยังไม่ถูกกด (เป็นภาพสีเทา)
  drawButtonSquareOff();                         // เรียกฟังก์ชันวาดภาพปุ่มสี่เหลี่ยมที่ยังไม่ถูกกด (เป็นภาพสีเทา)
  drawButtonCrossOff();                          // เรียกฟังก์ชันวาดภาพปุ่มกากบาทที่ยังไม่ถูกกด (เป็นภาพสีเทา)
  drawButtonTriangleOff();                       // เรียกฟังก์ชันวาดภาพปุ่มสามเหลี่ยมที่ยังไม่ถูกกด (เป็นภาพสีเทา)

  drawButtonR1Off();                             // เรียกฟังก์ชันวาดภาพปุ่ม R1 ที่ยังไม่ถูกกด (เป็นภาพสีเทา)
  drawButtonR2Off();                             // เรียกฟังก์ชันวาดภาพปุ่ม R2 ที่ยังไม่ถูกกด (เป็นภาพสีเทา)
  drawButtonR3Off();                             // เรียกฟังก์ชันวาดภาพปุ่ม R3 ที่ยังไม่ถูกกด (เป็นภาพสีเทา)
  drawButtonL1Off();                             // เรียกฟังก์ชันวาดภาพปุ่ม L1 ที่ยังไม่ถูกกด (เป็นภาพสีเทา)
  drawButtonL2Off();                             // เรียกฟังก์ชันวาดภาพปุ่ม L2 ที่ยังไม่ถูกกด (เป็นภาพสีเทา)
  drawButtonL3Off();                             // เรียกฟังก์ชันวาดภาพปุ่ม L3 ที่ยังไม่ถูกกด (เป็นภาพสีเทา)
  
  drawButtonSelectOff();                         // เรียกฟังก์ชันวาดภาพปุ่ม Select ที่ยังไม่ถูกกด (เป็นภาพสีเทา)
  drawButtonStartOff();                          // เรียกฟังก์ชันวาดภาพปุ่ม Start ที่ยังไม่ถูกกด (เป็นภาพสีเทา)
  
  drawAnalogBorder();                            // เรียกฟังก์ชันวาดกรอบสี่เหลี่ยมสำหรับพื้นที่แสดงพิกัด XY ของคันโยก Analog
}

void checkButtonUp()                             // ฟังก์ชันเช็คว่าปุ่มขึ้นถูกกดหรือไม่
{
  if(ps2x.ButtonPressed(PSB_PAD_UP))             // ถ้าปุ่มขึ้นถูกกด  
  {
    drawButtonUpOn();                            // เรียกฟังก์ชันวาดภาพปุ่มขึ้นถูกกด
  }                                          
  else if(ps2x.ButtonReleased(PSB_PAD_UP))       // ถ้าปุ่มขึ้นไม่ถูกกด
  {
    drawButtonUpOff();                           // เรียกฟังก์ชันวาดภาพปุ่มขึ้นไม่ถูกกด
  }
}

void drawButtonUpOn()                            // ฟังก์ชันสำหรับวาดภาพปุ่มขึ้นที่กำลังถูกกดอยู่
{
  glcdFillRect(25, 29, 10, 12, GLCD_DKGRAY);
  glcdPixel(26, 40, GLCD_WHITE);
  glcdPixel(27, 40, GLCD_WHITE);
  glcdPixel(26, 39, GLCD_WHITE);
  glcdPixel(35, 40, GLCD_WHITE);
  glcdPixel(34, 40, GLCD_WHITE);
  glcdPixel(35, 39, GLCD_WHITE);
}

void drawButtonUpOff()                           // ฟังก์ชันสำหรับวาดภาพปุ่มขึ้นที่ไม่ถูกกด
{
  glcdFillRect(25, 29, 10, 12, GLCD_GRAY);
  glcdPixel(26, 40, GLCD_WHITE);
  glcdPixel(27, 40, GLCD_WHITE);
  glcdPixel(26, 39, GLCD_WHITE);
  glcdPixel(35, 40, GLCD_WHITE);
  glcdPixel(34, 40, GLCD_WHITE);
  glcdPixel(35, 39, GLCD_WHITE);
}

void checkButtonDown()                           // ฟังก์ชันเช็คว่าปุ่มลงถูกกดหรือไม่            
{
  if(ps2x.ButtonPressed(PSB_PAD_DOWN))           // ถ้าปุ่มลงถูกกด  
  {
    drawButtonDownOn();                          // เรียกฟังก์ชันวาดภาพปุ่มลงถูกกด
  } 
  else if(ps2x.ButtonReleased(PSB_PAD_DOWN))     // ถ้าปุ่มลงไม่ถูกกด  
  {
    drawButtonDownOff();                         // เรียกฟังก์ชันวาดภาพปุ่มลงไม่ถูกกด
  }
}

void drawButtonDownOn()                          // ฟังก์ชันสำหรับวาดภาพปุ่มลงที่กำลังถูกกดอยู่
{
  glcdFillRect(25, 60, 10, 12, GLCD_DKGRAY);
  glcdPixel(26, 60, GLCD_WHITE);
  glcdPixel(27, 60, GLCD_WHITE);
  glcdPixel(26, 61, GLCD_WHITE);
  glcdPixel(35, 60, GLCD_WHITE);
  glcdPixel(34, 60, GLCD_WHITE);
  glcdPixel(35, 61, GLCD_WHITE);
}

void drawButtonDownOff()                         // ฟังก์ชันสำหรับวาดภาพปุ่มลงที่ไม่ถูกกด
{
  glcdFillRect(25, 60, 10, 12, GLCD_GRAY);
  glcdPixel(26, 60, GLCD_WHITE);
  glcdPixel(27, 60, GLCD_WHITE);
  glcdPixel(26, 61, GLCD_WHITE);
  glcdPixel(35, 60, GLCD_WHITE);
  glcdPixel(34, 60, GLCD_WHITE);
  glcdPixel(35, 61, GLCD_WHITE);
}

void checkButtonLeft()                           // ฟังก์ชันเช็คว่าปุ่มซ้ายถูกกดหรือไม่   
{
  if(ps2x.ButtonPressed(PSB_PAD_LEFT))           // ถ้าปุ่มซ้ายถูกกด  
  {
    drawButtonLeftOn();                          // เรียกฟังก์ชันวาดภาพปุ่มซ้ายถูกกด
  } 
  else if(ps2x.ButtonReleased(PSB_PAD_LEFT))     // ถ้าปุ่มซ้ายไม่ถูกกด  
  {
    drawButtonLeftOff();                         // เรียกฟังก์ชันวาดภาพปุ่มซ้ายไม่ถูกกด
  }
}

void drawButtonLeftOn()                          // ฟังก์ชันสำหรับวาดภาพปุ่มซ้ายที่กำลังถูกกด
{
  glcdFillRect(10, 45, 12, 10, GLCD_DKGRAY);
  glcdPixel(22, 45, GLCD_WHITE);
  glcdPixel(21, 45, GLCD_WHITE);
  glcdPixel(22, 46, GLCD_WHITE);
  glcdPixel(22, 54, GLCD_WHITE);
  glcdPixel(21, 54, GLCD_WHITE);
  glcdPixel(22, 53, GLCD_WHITE);
}

void drawButtonLeftOff()                         // ฟังก์ชันสำหรับวาดภาพปุ่มซ้ายที่ไม่ถูกกด
{
  glcdFillRect(10, 45, 12, 10, GLCD_GRAY);
  glcdPixel(22, 45, GLCD_WHITE);
  glcdPixel(21, 45, GLCD_WHITE);
  glcdPixel(22, 46, GLCD_WHITE);
  glcdPixel(22, 54, GLCD_WHITE);
  glcdPixel(21, 54, GLCD_WHITE);
  glcdPixel(22, 53, GLCD_WHITE);
}


void checkButtonRight()                          // ฟังก์ชันเช็คว่าปุ่มขวาถูกกดหรือไม่    
{
  if(ps2x.ButtonPressed(PSB_PAD_RIGHT))          // ถ้าปุ่มขวาถูกกด
  {
    drawButtonRightOn();                         // เรียกฟังก์ชันวาดภาพปุ่มขวาถูกกด
  } 
  else if(ps2x.ButtonReleased(PSB_PAD_RIGHT))    // ถ้าปุ่มขวาไม่ถูกกด  
  {
    drawButtonRightOff();                        // เรียกฟังก์ชันวาดภาพปุ่มขวาไม่ถูกกด
  }
}

void drawButtonRightOn()                         // ฟังก์ชันสำหรับวาดภาพปุ่มขวาที่กำลังถูกกด
{
  glcdFillRect(38, 45, 12, 10, GLCD_DKGRAY);
  glcdPixel(39, 45, GLCD_WHITE);
  glcdPixel(40, 45, GLCD_WHITE);
  glcdPixel(39, 46, GLCD_WHITE);
  glcdPixel(39, 54, GLCD_WHITE);
  glcdPixel(40, 54, GLCD_WHITE);
  glcdPixel(39, 53, GLCD_WHITE);
}

void drawButtonRightOff()                        // ฟังก์ชันสำหรับวาดภาพปุ่มขวาที่ไม่ถูกกด
{
  glcdFillRect(38, 45, 12, 10, GLCD_GRAY);
  glcdPixel(39, 45, GLCD_WHITE);
  glcdPixel(40, 45, GLCD_WHITE);
  glcdPixel(39, 46, GLCD_WHITE);
  glcdPixel(39, 54, GLCD_WHITE);
  glcdPixel(40, 54, GLCD_WHITE);
  glcdPixel(39, 53, GLCD_WHITE);
}

void checkButtonR1()                             // ฟังก์ชันเช็คว่าปุ่ม R1 ถูกกดหรือไม่    
{
  if(ps2x.ButtonPressed(PSB_R1))                 // ถ้าปุ่ม R1 ถูกกด
  {
    drawButtonR1On();                            // เรียกฟังก์ชันวาดภาพปุ่ม R1 ถูกกด
  } 
  else if(ps2x.ButtonReleased(PSB_R1))           // ถ้าปุ่ม R1 ไม่ถูกกด
  {
    drawButtonR1Off();                           // เรียกฟังก์ชันวาดภาพปุ่ม R1 ไม่ถูกกด
  }
}

void drawButtonR1On()                            // ฟังก์ชันสำหรับวาดภาพปุ่ม R1 ที่กำลังถูกกด
{  
  setTextBackgroundColor(GLCD_DKGRAY);
  glcdFillRect(149, 10, 10, 8, GLCD_DKGRAY);
  setTextColor(GLCD_WHITE);
  glcd(1, 21, "  R1");
}

void drawButtonR1Off()                           // ฟังก์ชันสำหรับวาดภาพปุ่ม R1 ที่ไม่ถูกกด
{
  setTextBackgroundColor(GLCD_GRAY);
  setTextColor(GLCD_WHITE);
  glcdFillRect(149, 10, 10, 8, GLCD_GRAY);
  glcd(1, 21, "  R1");
}

void checkButtonR2()                             // ฟังก์ชันเช็คว่าปุ่ม R2 ถูกกดหรือไม่   
{
  if(ps2x.ButtonPressed(PSB_R2))                 // ถ้าปุ่ม R2 ถูกกด
  {
    drawButtonR2On();                            // เรียกฟังก์ชันวาดภาพปุ่ม R2 ถูกกด
  } 
  else if(ps2x.ButtonReleased(PSB_R2))           // ถ้าปุ่ม R2 ไม่ถูกกด
  {
    drawButtonR2Off();                           // เรียกฟังก์ชันวาดภาพปุ่ม R2 ไม่ถูกกด
  }
}

void drawButtonR2On()                            // ฟังก์ชันสำหรับวาดภาพปุ่ม R2 ที่กำลังถูกกด 
{
  setTextBackgroundColor(GLCD_DKGRAY);
  setTextColor(GLCD_WHITE);
  glcdFillRect(149, 0, 10, 8, GLCD_DKGRAY);
  glcd(0, 21, "  R2");
}

void drawButtonR2Off()                           // ฟังก์ชันสำหรับวาดภาพปุ่ม R2 ที่ไม่ถูกกด 
{
  setTextBackgroundColor(GLCD_GRAY);
  setTextColor(GLCD_WHITE);
  glcdFillRect(149, 0, 10, 8, GLCD_GRAY);
  glcd(0, 21, "  R2");
}

void checkButtonR3()                             // ฟังก์ชันเช็คว่าปุ่ม R3 ถูกกดหรือไม่   
{
  if(ps2x.ButtonPressed(PSB_R3))                 // ถ้าปุ่ม R3 ถูกกด 
  {
    drawButtonR3On();                            // เรียกฟังก์ชันวาดภาพปุ่ม R3 ถูกกด
  } 
  else if(ps2x.ButtonReleased(PSB_R3))           // ถ้าปุ่ม R3 ไม่ถูกกด 
  {
    drawButtonR3Off();                           // เรียกฟังก์ชันวาดภาพปุ่ม R3 ไม่ถูกกด
  }
}

void drawButtonR3On()                            // ฟังก์ชันสำหรับวาดภาพปุ่ม R3 ที่กำลังถูกกด  
{
  setTextBackgroundColor(GLCD_DKGRAY);
  setTextColor(GLCD_WHITE);
  glcdFillRect(149, 120, 10, 8, GLCD_DKGRAY);
  glcd(12, 21, "  R3");
}

void drawButtonR3Off()                           // ฟังก์ชันสำหรับวาดภาพปุ่ม R3 ที่ไม่ถูกกด  
{
  setTextBackgroundColor(GLCD_GRAY);
  setTextColor(GLCD_WHITE);
  glcdFillRect(149, 120, 10, 8, GLCD_GRAY);
  glcd(12, 21, "  R3");
}

void checkButtonL1()                             // ฟังก์ชันเช็คว่าปุ่ม L1 ถูกกดหรือไม่    
{
  if(ps2x.ButtonPressed(PSB_L1))                 // ถ้าปุ่ม L1 ถูกกด 
  {
    drawButtonL1On();                            // เรียกฟังก์ชันวาดภาพปุ่ม L1 ถูกกด
  } 
  else if(ps2x.ButtonReleased(PSB_L1))           // ถ้าปุ่ม L1 ไม่ถูกกด  
  {                                              
    drawButtonL1Off();                           // เรียกฟังก์ชันวาดภาพปุ่ม L1 ไม่ถูกกด
  }
}

void drawButtonL1On()                            // ฟังก์ชันสำหรับวาดภาพปุ่ม L1 ที่กำลังถูกกด   
{
  setTextBackgroundColor(GLCD_DKGRAY);
  setTextColor(GLCD_WHITE);
  glcd(1, 0, "  L1  ");
}

void drawButtonL1Off()                           // ฟังก์ชันสำหรับวาดภาพปุ่ม L1 ที่ไม่ถูกกด 
{
  setTextBackgroundColor(GLCD_GRAY);
  setTextColor(GLCD_WHITE);
  glcd(1, 0, "  L1  ");
}

void checkButtonL2()                             // ฟังก์ชันเช็คว่าปุ่ม L2 ถูกกดหรือไม่   
{
  if(ps2x.ButtonPressed(PSB_L2))                 // ถ้าปุ่ม L2 ถูกกด  
  {
    drawButtonL2On();                            // เรียกฟังก์ชันวาดภาพปุ่ม L2 ถูกกด
  } 
  else if(ps2x.ButtonReleased(PSB_L2))           // ถ้าปุ่ม L2 ไม่ถูกกด   
  {
    drawButtonL2Off();                           // เรียกฟังก์ชันวาดภาพปุ่ม L2 ไม่ถูกกด
  }
}

void drawButtonL2On()                            // ฟังก์ชันสำหรับวาดภาพปุ่ม L2 ที่กำลังถูกกด  
{
  setTextBackgroundColor(GLCD_DKGRAY);
  setTextColor(GLCD_WHITE);
  glcd(0, 0, "  L2  ");
}

void drawButtonL2Off()                           // ฟังก์ชันสำหรับวาดภาพปุ่ม L2 ที่ไม่ถูกกด  
{
  setTextBackgroundColor(GLCD_GRAY);
  setTextColor(GLCD_WHITE);
  glcd(0, 0, "  L2  ");
}

void checkButtonL3()                             // ฟังก์ชันเช็คว่าปุ่ม L3 ถูกกดหรือไม่   
{
  if(ps2x.ButtonPressed(PSB_L3))                 // ถ้าปุ่ม L3 ถูกกด   
  {
    drawButtonL3On();                            // เรียกฟังก์ชันวาดภาพปุ่ม L3 ถูกกด
  } 
  else if(ps2x.ButtonReleased(PSB_L3))           // ถ้าปุ่ม L3 ไม่ถูกกด    
  {
    drawButtonL3Off();                           // เรียกฟังก์ชันวาดภาพปุ่ม L3 ไม่ถูกกด
  }
}

void drawButtonL3On()                            // ฟังก์ชันสำหรับวาดภาพปุ่ม L3 ที่กำลังถูกกด   
{
  setTextBackgroundColor(GLCD_DKGRAY);
  setTextColor(GLCD_WHITE);
  glcd(12, 0, "  L3  ");
}

void drawButtonL3Off()                           // ฟังก์ชันสำหรับวาดภาพปุ่ม L3 ที่ไม่ถูกกด   
{
  setTextBackgroundColor(GLCD_GRAY);
  setTextColor(GLCD_WHITE);
  glcd(12, 0, "  L3  ");
}

void checkButtonCircle()                         // ฟังก์ชันเช็คว่าปุ่มวงกลมถูกกดหรือไม่    
{
  if(ps2x.ButtonPressed(PSB_CIRCLE))             // ถ้าปุ่มวงกลมถูกกด    
  {
    drawButtonCircleOn();                        // เรียกฟังก์ชันวาดภาพปุ่มวงกลมถูกกด
  } 
  else if(ps2x.ButtonReleased(PSB_CIRCLE))       // ถ้าปุ่มวงกลมไม่ถูกกด  
  {
    drawButtonCircleOff();                       // เรียกฟังก์ชันวาดภาพปุ่มวงกลมไม่ถูกกด
  }
}

void drawButtonCircleOn()                        // ฟังก์ชันสำหรับวาดภาพปุ่มวงกลมที่กำลังถูกกด
{
  glcdCircle(140, 48, 6, GLCD_RED);
}

void drawButtonCircleOff()                       // ฟังก์ชันสำหรับวาดภาพปุ่มวงกลมที่ไม่ถูกกด    
{
  glcdCircle(140, 48, 6, GLCD_GRAY);
}

void checkButtonCross()                          // ฟังก์ชันเช็คว่าปุ่มกากบาทถูกกดหรือไม่  
{
  if(ps2x.ButtonPressed(PSB_CROSS))              // ถ้าปุ่มกากบาทถูกกด    
  {
    drawButtonCrossOn();                         // เรียกฟังก์ชันวาดภาพปุ่มกากบาทถูกกด
  } 
  else if(ps2x.ButtonReleased(PSB_CROSS))        // ถ้าปุ่มกากบาทไม่ถูกกด  
  {
    drawButtonCrossOff();                        // เรียกฟังก์ชันวาดภาพปุ่มกากบาทไม่ถูกกด
  }
}

void drawButtonCrossOn()                         // ฟังก์ชันสำหรับวาดภาพปุ่มกากบาทที่กำลังถูกกด 
{
  glcdLine(118, 58, 130, 70, GLCD_BLUE);
  glcdLine(130, 58, 118, 70, GLCD_BLUE);
}

void drawButtonCrossOff()                        // ฟังก์ชันสำหรับวาดภาพปุ่มกากบาทที่ไม่ถูกกด    
{
  glcdLine(118, 58, 130, 70, GLCD_GRAY);
  glcdLine(130, 58, 118, 70, GLCD_GRAY);
}

void checkButtonSquare()                         // ฟังก์ชันเช็คว่าปุ่มสี่เหลี่ยมถูกกดหรือไม่ 
{
  if(ps2x.ButtonPressed(PSB_SQUARE))             // ถ้าปุ่มสี่เหลี่ยมถูกกด    
  {
    drawButtonSquareOn();                        // เรียกฟังก์ชันวาดภาพปุ่มสี่เหลี่ยมถูกกด
  } 
  else if(ps2x.ButtonReleased(PSB_SQUARE))       // ถ้าปุ่มสี่เหลี่ยมไม่ถูกกด  
  {
    drawButtonSquareOff();                       // เรียกฟังก์ชันวาดภาพปุ่มสี่เหลี่ยมไม่ถูกกด
  }
}

void drawButtonSquareOn()                        // ฟังก์ชันสำหรับวาดภาพปุ่มสี่เหลี่ยมที่กำลังถูกกด 
{
  glcdRect(102, 43, 12, 12, GLCD_MAGENTA);
}

void drawButtonSquareOff()                       // ฟังก์ชันสำหรับวาดภาพปุ่มสี่เหลี่ยมที่ไม่ถูกกด   
{
  glcdRect(102, 43, 12, 12, GLCD_GRAY);
}

void checkButtonTriangle()                       // ฟังก์ชันเช็คว่าปุ่มสามเหลี่ยมถูกกดหรือไม่ 
{
  if(ps2x.ButtonPressed(PSB_TRIANGLE))           // ถ้าปุ่มสามเหลี่ยมถูกกด 
  {
    drawButtonTriangleOn();                      // เรียกฟังก์ชันวาดภาพปุ่มสามเหลี่ยมถูกกด
  } 
  else if(ps2x.ButtonReleased(PSB_TRIANGLE))     // ถ้าปุ่มสามเหลี่ยมไม่ถูกกด 
  {
    drawButtonTriangleOff();                     // เรียกฟังก์ชันวาดภาพปุ่มสามเหลี่ยมไม่ถูกกด
  }  
}

void drawButtonTriangleOn()                      // ฟังก์ชันสำหรับวาดภาพปุ่มสามเหลี่ยมที่ถูกกด  
{
  glcdLine(117, 40, 131, 40, GLCD_DKGREEN);
  glcdLine(117, 40, 124, 28, GLCD_DKGREEN);
  glcdLine(131, 40, 124, 28, GLCD_DKGREEN);
}

void drawButtonTriangleOff()                     // ฟังก์ชันสำหรับวาดภาพปุ่มสามเหลี่ยมที่ไม่ถูกกด  
{
  glcdLine(117, 40, 131, 40, GLCD_GRAY);
  glcdLine(117, 40, 124, 28, GLCD_GRAY);
  glcdLine(131, 40, 124, 28, GLCD_GRAY);
}

void checkButtonSelect()                         // ฟังก์ชันเช็คว่าปุ่ม Select ถูกกดหรือไม่  
{
  if(ps2x.ButtonPressed(PSB_SELECT))             // ถ้าปุ่ม Select ถูกกด 
  {
    drawButtonSelectOn();                        // เรียกฟังก์ชันวาดภาพปุ่ม Select ถูกกด
  } 
  else if(ps2x.ButtonReleased(PSB_SELECT))       // ถ้าปุ่ม Select ไม่ถูกกด  
  {
    drawButtonSelectOff();                       // เรียกฟังก์ชันวาดภาพปุ่ม Select ไม่ถูกกด
  }  
}

void drawButtonSelectOn()                        // ฟังก์ชันสำหรับวาดภาพปุ่ม Select ที่ถูกกด  
{
  glcdFillRect(62, 65, 12, 9, GLCD_ORANGE);
}

void drawButtonSelectOff()                       // ฟังก์ชันสำหรับวาดภาพปุ่ม Select ที่ไม่ถูกกด  
{
  glcdFillRect(62, 65, 12, 9, GLCD_GRAY);
}

void checkButtonStart()                          // ฟังก์ชันเช็คว่าปุ่ม Start ถูกกดหรือไม่  
{
  if(ps2x.ButtonPressed(PSB_START))              // ถ้าปุ่ม Start ถูกกด 
  {
    drawButtonStartOn();                         // เรียกฟังก์ชันวาดภาพปุ่ม Start ถูกกด
  } 
  else if(ps2x.ButtonReleased(PSB_START))        // ถ้าปุ่ม Start ไม่ถูกกด   
  {
    drawButtonStartOff();                        // เรียกฟังก์ชันวาดภาพปุ่ม Start ไม่ถูกกด
  }  
}

void drawButtonStartOn()                         // ฟังก์ชันสำหรับวาดภาพปุ่ม Start ที่ถูกกด 
{
  glcdLine(84, 65, 84, 73, GLCD_ORANGE);
  glcdLine(85, 65, 85, 73, GLCD_ORANGE);
  glcdLine(86, 65, 86, 73, GLCD_ORANGE);
  glcdLine(87, 66, 87, 72, GLCD_ORANGE);
  glcdLine(88, 66, 88, 72, GLCD_ORANGE);
  glcdLine(89, 66, 89, 72, GLCD_ORANGE);
  glcdLine(90, 67, 90, 71, GLCD_ORANGE);
  glcdLine(91, 67, 91, 71, GLCD_ORANGE);
  glcdLine(92, 67, 92, 71, GLCD_ORANGE);
  glcdLine(93, 68, 93, 70, GLCD_ORANGE);
  glcdLine(94, 68, 94, 70, GLCD_ORANGE);
  glcdLine(95, 68, 95, 70, GLCD_ORANGE);
  glcdPixel(96, 69, GLCD_ORANGE);
}

void drawButtonStartOff()                        // ฟังก์ชันสำหรับวาดภาพปุ่ม Start ที่ไมถูกกด 
{
  glcdLine(84, 65, 84, 73, GLCD_GRAY);
  glcdLine(85, 65, 85, 73, GLCD_GRAY);
  glcdLine(86, 65, 86, 73, GLCD_GRAY);
  glcdLine(87, 66, 87, 72, GLCD_GRAY);
  glcdLine(88, 66, 88, 72, GLCD_GRAY);
  glcdLine(89, 66, 89, 72, GLCD_GRAY);
  glcdLine(90, 67, 90, 71, GLCD_GRAY);
  glcdLine(91, 67, 91, 71, GLCD_GRAY);
  glcdLine(92, 67, 92, 71, GLCD_GRAY);
  glcdLine(93, 68, 93, 70, GLCD_GRAY);
  glcdLine(94, 68, 94, 70, GLCD_GRAY);
  glcdLine(95, 68, 95, 70, GLCD_GRAY);
  glcdPixel(96, 69, GLCD_GRAY);
}

void checkAnalogL()                              // ฟังก์ชันสำหรับเช็คค่าจากคันโยก Analog ฝั่งซ้าย
{
  int x = ps2x.Analog(PSS_LX);                   // อ่านค่าคันโยก Analog ในแกน X
  int y = ps2x.Analog(PSS_LY);                   // อ่านค่าคันโยก Analog ในแกน Y
  
  if(x != oldLX || y != oldLY)                   // ถ้า X หรือ Y มีค่าไม่เท่ากับของเดิมที่เก็บไว้ในตัวแปร oldLX และ oldLY
  {
    oldLX = map(oldLX, 0, 255, 26, 54);          // ลบภาพเดิมออกแล้ววาดใหม่ให้เป็นพิกัด XY ล่าสุดตามคันโยก Analog ฝั่งซ้าย
    oldLY = map(oldLY, 0, 255, 82, 110);
    glcdPixel(oldLX, oldLY, GLCD_WHITE);
    glcdCircle(oldLX, oldLY, 1, GLCD_WHITE);
    oldLX = x;
    oldLY = y;
    x = map(x, 0, 255, 26, 54);
    y = map(y, 0, 255, 82, 110);
    glcdPixel(x, y, GLCD_RED);
    glcdCircle(x, y, 1, GLCD_RED);
  }
}

void checkAnalogR()                              // ฟังก์ชันสำหรับเช็คค่าจากคันโยก Analog ฝั่งขวา
{
  int x = ps2x.Analog(PSS_RX);                   // อ่านค่าคันโยก Analog ในแกน X
  int y = ps2x.Analog(PSS_RY);                   // อ่านค่าคันโยก Analog ในแกน Y
  
  if(x != oldRX || y != oldRY)                   
  {
    oldRX = map(oldRX, 0, 255, 106, 134);        // ลบภาพเดิมออกแล้ววาดใหม่ให้เป็นพิกัด XY ล่าสุดตามคันโยก Analog ฝั่งขวา
    oldRY = map(oldRY, 0, 255, 82, 110);
    glcdPixel(oldRX, oldRY, GLCD_WHITE);
    glcdCircle(oldRX, oldRY, 1, GLCD_WHITE);
    oldRX = x;
    oldRY = y;
    x = map(x, 0, 255, 106, 134);
    y = map(y, 0, 255, 82, 110);
    glcdPixel(x, y, GLCD_RED);
    glcdCircle(x, y, 1, GLCD_RED);
  }
}

void drawAnalogBorder()                          // ฟังก์ชันสำหรับวาดกรอบแสดงขอบเขตของพื้นที่ที่แสดงตำแหน่งคันโยก Analog
{
  glcdRect(24, 80, 33, 33, GLCD_GRAY);           // วาดกรอบสำหรับคันโยก Analog ฝั่งซ้าย
  glcdRect(104, 80, 33, 33, GLCD_GRAY);          // วาดกรอบสำหรับคันโยก Analog ฝั่งขวา
}


