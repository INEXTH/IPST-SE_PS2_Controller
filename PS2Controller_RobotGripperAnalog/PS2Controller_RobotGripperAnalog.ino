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
 * ตัวอย่างการใช้งานบอร์ด IPST-SE ร่วมกับ PS2 Controller แบบใช้คันโยกอะนาล็อกเพื่อควบคุม Gripper
 *
 * โปรแกรมควบคุมหุ่นยนต์ IPST-SE แบบมี Gripper สำหรับการทำภารกิจที่ต้องหนีบจับวัตถุ
 *
 * ควบคุมหุ่นยนต์ IPST-SE ด้วยปุ่มทิศทางขึ้น ลง ซ้าย ขวา โดยที่ปุ่มขึ้นจะเป็นการเดินหน้า
 * และปุ่มลงจะเป็นการถอยหลัง ส่วนปุ่มซ้ายและขวาจะเป็นการหมุนตัวไปในทิศทางนั้นๆ
 * สำหรับการควบคุมแขนจับจะให้กดปุ่ม R1 หรือ R2 ค้าง โดยที่ R1 เป็นการควบคุมเซอร์โวที่ใช้หนีบจับวัตถุ
 * ซึ่งจะควบคุมองศาด้วยคันโยกอะนาล็อกฝั่งขวาในแนวแกน X หรือโยกคันโยกในแนวนอน (กด R1 ค้างแล้วโยกคันโยกซ้าย-ขวา)
 * ส่วน R2 เป็นการควบคุมเซอร์โวที่ใช้ยกแขนจับ ซึ่งจะควบคุมองศาด้วยคันโยกอะนาล็อกฝั่งขวาในแนวแกน Y 
 * (กด R2 ค้างแล้วโยกขึ้นโยกขึ้น-ลง)
 * 
 * เพิ่มเติม - ต่อเซอร์โวสำหรับหนีบจับวัตถุเข้าที่ช่อง SV0 และต่อเซอรืโวสำหรับยกแขนจับเข้าที่ช่อง SV1
 */

#include <ipst.h>                                    // เรียกใช้งานไลบรารีสำหรับ IPST-SE
#include <PS2X_lib.h>                                // เรียกใช้งานไลบรารีสำหรับ PS2 Controller

#define PS2_DAT        16                            // กำหนดขา Data    เป็นขา 16
#define PS2_CMD        17                            // กำหนดขา Command เป็นขา 17
#define PS2_SEL        18                            // กำหนดขา Select  เป็นขา 18
#define PS2_CLK        19                            // กำหนดขา Clock   เป็นขา 19

PS2X ps2x;                                           // ประกาศตัวแปรสำหรับ PS2 Controller

int servoGrab = 90;                                  // ตัวแปรสำหรับเก็บองศาของเซอร์โวที่ใช้หนีบวัตถุ
int servoLift = 90;                                  // ตัวแปรสำหรับเก็บองศาของเซอร์โวที่ใช้ยกแขนจับ

boolean isR1 = false;                                // สภานะของปุ่ม R1
boolean isR2 = false;                                // สภานะของปุ่ม R2

void setup()
{
  delay(1000);                                       // หน่วงเวลา 1 วินาทีเพื่อรอให้บอร์ดพร้อมทำงาน
  glcdClear();                                       // เคลียร์ภาพทั้งหมดบนหน้าจอแสดงผล
  glcdMode(1);                                       // กำหนดให้หน้าจอแสดงผลเป็นแนวนอน
    
  servo(0, servoGrab);                               // กำหนดองศาเริ่มต้นให้กับเซอร์โวที่ใช้หนีบวัตถุ
  servo(1, servoLift);                               // กำหนดองศาเริ่มต้นให้กับเซอร์โวที่ใชยกแขนจับ
  delay(200);                                        // หน่วงเวลา 200 มิลลิวินาที
  
  glcd(0, 0, "Connecting");                      // แสดงข้อความเพื่อให้รู้ว่ากำลังทำการเชื่อมต่อกับ PS2 Controller
      
  while(true)                                    // วนการทำงานเพื่อรอการเชื่อมต่อกับ PS2 Controller
  {
    // กำหนดขาที่จะเชื่อมต่กับ PS2 Controller โดยมีการเก็บค่าที่ส่งกลับมาเป็น Integer เพื่อรู้ได้ว่าเชื่อมต่อได้หรือไม่
    int error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, false, false);
  
    if(error == 0)                               // กรณีที่เชื่อมต่อได้ ไม่มีปัญหาอะไร (Error = 0)
    {
      glcd(0, 0, "OK           ");               // แสดงข้อความว่าเชื่อมต่อกับ PS2 Controller เรียบร้อยแล้ว
      delay(1000);                               // หน่วงเวลา 1 วินาที
      glcdClear();                               // เคลียร์ภาพทั้งหมดบนหน้าจอแสดงผล
      break;                                     // ออกจาก while(true)
    } 
    delay(500);                                  // หน่วงเวลา 500 มิลลิวินาทีเพื่อรอการเชื่อมต่อครั้งต่อไปในกรณีที่เชื่อมต่อไม่สำเร็จ
  }
}

void loop()
{
  ps2x.read_gamepad(false, false);                   // อ่านข้อมูลจาก PS2 Controller
    
  if(ps2x.Button(PSB_R1))                            // ถ้าปุ่ม R1 ถูกกด
    isR1 = true;                                     // กำหนดสถานะของ isR1 เป็น True
  else                                               // ถ้าปุ่ม R1 ไม่ถูกกด
    isR1 = false;                                    // กำหนดสถานะของ isR1 เป็น False
    
  if(ps2x.Button(PSB_R2))                            // ถ้าปุ่ม R2 ถูกกด 
    isR2 = true;                                     // กำหนดสถานะของ isR2 เป็น True
  else                                               // ถ้าปุ่ม R2 ไม่ถูกกด
    isR2 = false;                                    // กำหนดสถานะของ isR2 เป็น False
    
  if(ps2x.Button(PSB_TRIANGLE))                      // เมื่อปุ่มสามเหลี่ยมถูกกด
  {
    servo(0, -1);                                    // ผ่อนเซอร์โวทั้งสองตัว
    servo(1, -1);
  } 
  
  if(ps2x.Button(PSB_PAD_UP))                        // ถ้าปุ่มขึ้นถูกกด
    fd(90);                                          // สั่งงานให้หุ่นยนต์เดินหน้า
  else if(ps2x.Button(PSB_PAD_DOWN))                 // ถ้าปุ่มลงถูกกด
    bk(90);                                          // สั่งงานให้หุ่นยนต์ถอยหลัง
  else if(ps2x.Button(PSB_PAD_LEFT))                 // ถ้าปุ่มซ้ายถูกกด
    sl(40);                                          // สั่งงานให้หุ่นยนต์เลี้ยวซ้าย
  else if(ps2x.Button(PSB_PAD_RIGHT))                // ถ้าปุ่มขวาถูกกด
    sr(40);                                          // สั่งงานให้หุ่นยนต์เลี้ยวขวา
  else                                               // ถ้าไม่มีปุ่มทิศทางใดๆถูกกด
    ao();                                            // หุ่นยนต์หยุดเคลื่อนที่
    
  if(isR1)                                           // เมื่อสถานะของ isR1 เป็น True (ถูกกด)
  {
    int a = ps2x.Analog(PSS_RX);                     // เก็บค่าแกน X จากคันโยก Analog ฝั่งขวาไว้ในตัวแปร a
    int b = 0;                                       // ประกาศตัวแปร b ให้มีค่าเริ่มต้นเป็น 0
    glcd(3, 0, "%d  ", a);                           // แสดงค่าจากตัวแปร a ขึ้นบนหน้าจอ
    
    if(a > 140)                                      // ถ้า a มากกว่า 140 
      b = map(a, 140, 255, 0, 3);                    // เฉลี่ยค่าที่อยู่ในช่วง 140 ถึง 255 ให้ออกมาเป็น 0 ถึง 3 แล้วเก็บไว้ในตัวแปร b
    else if(a < 124) 
      b = map(a, 0, 140, -3, 0);                     // เฉลี่ยค่าที่อยู่ในช่วง 140 ถึง 255 ให้ออกมาเป็น -3 ถึง 0 แล้วเก็บไว้ในตัวแปร b
      
    servoGrab += b;                                  // เพิ่มองศาของเซอร์โวสำหรับหนีบวัตถุตามค่าจากตัวแปร b
    servoGrab = (servoGrab > 140) ? 140 : servoGrab; // กำหนดให้ค่าองศาสูงสุดได้แค่ 140
    servoGrab = (servoGrab < 40) ? 40 : servoGrab;   // กำหนดให้ค่าองศาต่ำสุดได้แค่ 40
    servo(0, servoGrab);                             // กำหนดให้เซอร์โวหมุนไปที่องศาตามค่าที่เก็บไว้ในตัวแปร servoGrab
  } 
  else if(isR2)                                      // เมื่อสถานะของ isR2 เป็น True (ถูกกด)
  {
    int a = ps2x.Analog(PSS_RY);                     // เก็บค่าแกน Y จากคันโยก Analog ฝั่งขวาไว้ในตัวแปร a
    int b = 0;                                       // ประกาศตัวแปร b ให้มีค่าเริ่มต้นเป็น 0
    glcd(3, 0, "%d  ", a);                           // แสดงค่าจากตัวแปร a ขึ้นบนหน้าจอ
    
    if(a > 140)                                      // ถ้า a มากกว่า 140 
      b = map(a, 140, 255, 0, 3);                    // เฉลี่ยค่าที่อยู่ในช่วง 140 ถึง 255 ให้ออกมาเป็น 0 ถึง 3 แล้วเก็บไว้ในตัวแปร b
    else if(a < 124) 
      b = map(a, 0, 140, -3, 0);                     // เฉลี่ยค่าที่อยู่ในช่วง 140 ถึง 255 ให้ออกมาเป็น -3 ถึง 0 แล้วเก็บไว้ในตัวแปร b
      
    servoLift += (b * -1);                           // เพิ่มองศาของเซอร์โวสำหรับหนีบวัตถุตามค่าจากตัวแปร b (ค่าจาก b จะเป็นทิศทางตรงข้ามกัน)
    servoLift = (servoLift > 135) ? 135 : servoLift; // กำหนดให้ค่าองศาสูงสุดได้แค่ 135
    servoLift = (servoLift < 35) ? 35 : servoLift;   // กำหนดให้ค่าองศาต่ำสุดได้แค่ 35
    servo(1, servoLift);                             // กำหนดให้เซอร์โวหมุนไปที่องศาตามค่าที่เก็บไว้ในตัวแปร servoLift
  }
  
  delay(50);                                         // หน่วงเวลา 50 มิลลิวินาที
}
