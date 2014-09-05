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
 * สำหรับการควบคุมแขนจับจะใช้ปุ่ม R1 R2 L1 และ L2 โดยที่ R1 และ R2 จะควบคุมเซอร์โวสำหรับหนีบจับวัตถุ
 * โดยที่กด R1 เพื่อหยีบวัตถุ และกด R2 เพื่อปล่อยวัตถ ส่วน L1 และ L2 จะใช้ควบคุมเซอร์โวสำหรับยกแขนจับ
 * โดยที่กด L1 เพื่อปล่อยแขนจับให้ต่ำลง และกด L2 เพื่อยกแขนจับขึ้น 
 * 
 * เพิ่มเติม - ต่อเซอร์โวสำหรับหนีบจับวัตถุเข้าที่ช่อง SV0 และต่อเซอรืโวสำหรับยกแขนจับเข้าที่ช่อง SV1
 */

#include <ipst.h>                                // เรียกใช้งานไลบรารีสำหรับ IPST-SE
#include <PS2X_lib.h>                            // เรียกใช้งานไลบรารีสำหรับ PS2 Controller

#define PS2_DAT        18                        // กำหนดขา Data    เป็นขา 18
#define PS2_CMD        20                        // กำหนดขา Command เป็นขา 20
#define PS2_SEL        19                        // กำหนดขา Select  เป็นขา 19
#define PS2_CLK        17                        // กำหนดขา Clock   เป็นขา 17

PS2X ps2x;                                       // ประกาศตัวแปรสำหรับ PS2 Controller

int servoGrab = 90;                              // ตัวแปรสำหรับเก็บองศาของเซอร์โวที่ใช้หนีบวัตถุ
int servoLift = 90;                              // ตัวแปรสำหรับเก็บองศาของเซอร์โวที่ใช้ยกแขนจับ

boolean isR1 = false;                            // สภานะของปุ่ม R1
boolean isR2 = false;                            // สภานะของปุ่ม R2

boolean isL1 = false;                            // สภานะของปุ่ม L1
boolean isL2 = false;                            // สภานะของปุ่ม L2

void setup()
{
  delay(1000);                                   // หน่วงเวลา 1 วินาทีเพื่อรอให้บอร์ดพร้อมทำงาน
  glcdClear();                                   // เคลียร์ภาพทั้งหมดบนหน้าจอแสดงผล
  glcdMode(1);                                   // กำหนดให้หน้าจอแสดงผลเป็นแนวนอน
  
  servo(0, servoGrab);                           // กำหนดองศาเริ่มต้นให้กับเซอร์โวที่ใช้หนีบวัตถุ
  servo(1, servoLift);                           // กำหนดองศาเริ่มต้นให้กับเซอร์โวที่ใชยกแขนจับ
  delay(200);                                    // หน่วงเวลา 200 มิลลิวินาที
  
  // กำหนดขาที่จะเชื่อมต่กับ PS2 Controller โดยมีการเก็บค่าที่ส่งกลับมาเป็น Integer เพื่อรู้ได้ว่าเชื่อมต่อได้หรือไม่
  int error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, false, false);
  
  if(error == 0)                                 // กรณีที่เชื่อมต่อได้ ไม่มีปัญหาอะไร (Error = 0)
  {
    glcd(0, 0, "Controller found");              // แสดงหน้าจอว่าเชื่อมต่อกับ PS2 Controller ได้
  } 
  else                                           // ถ้าไม่สามารถเชื่อมต่อกับ PS2 Controller 
  {
    glcd(0, 0, "Controller not found");          // แสดงหน้าจอว่าไม่สามารถเชื่อมต่อกับ PS2 Controller ได้
    glcd(1, 0, "Check your controller");
    glcd(2, 0, "And restart to try again");
    while(true);                                 // วนไม่จำกัด เพื่อให้ผู้ใช้รีเซตบอร์ดเพื่อเชื่อมต่อใหม่
  }
}

void loop()
{
  ps2x.read_gamepad(false, false);               // อ่านข้อมูลจาก PS2 Controller
  
  if(ps2x.Button(PSB_R1))                        // ถ้าปุ่ม R1 ถูกกด
    isR1 = true;                                 // กำหนดสถานะของ isR1 เป็น True
  else                                           // ถ้าปุ่ม R1 ไม่ถูกกด
    isR1 = false;                                // กำหนดสถานะของ isR1 เป็น False
    
  if(ps2x.Button(PSB_R2))                        // ถ้าปุ่ม R2 ถูกกด 
    isR2 = true;                                 // กำหนดสถานะของ isR2 เป็น True
  else                                           // ถ้าปุ่ม R2 ไม่ถูกกด
    isR2 = false;                                // กำหนดสถานะของ isR2 เป็น False
    
  if(ps2x.Button(PSB_L1))                        // ถ้าปุ่ม L1 ถูกกด 
    isL1 = true;                                 // กำหนดสถานะของ isL1 เป็น True
  else                                           // ถ้าปุ่ม L1 ไม่ถูกกด
    isL1 = false;                                // กำหนดสถานะของ isL1 เป็น False
    
  if(ps2x.Button(PSB_L2))                        // ถ้าปุ่ม L2 ถูกกด 
    isL2 = true;                                 // กำหนดสถานะของ isL2 เป็น True
  else                                           // ถ้าปุ่ม L2 ไม่ถูกกด
    isL2 = false;                                // กำหนดสถานะของ isL2 เป็น False
    
  if(ps2x.Button(PSB_TRIANGLE))                  // เมื่อปุ่มสามเหลี่ยมถูกกด
  {
    servo(0, -1);                                // ผ่อนเซอร์โวทั้งสองตัว
    servo(1, -1);
  } 
  
  if(ps2x.Button(PSB_PAD_UP))                    // ถ้าปุ่มขึ้นถูกกด
    fd(90);                                      // สั่งงานให้หุ่นยนต์เดินหน้า
  else if(ps2x.Button(PSB_PAD_DOWN))             // ถ้าปุ่มลงถูกกด
    bk(90);                                      // สั่งงานให้หุ่นยนต์ถอยหลัง
  else if(ps2x.Button(PSB_PAD_LEFT))             // ถ้าปุ่มซ้ายถูกกด
    sl(40);                                      // สั่งงานให้หุ่นยนต์เลี้ยวซ้าย
  else if(ps2x.Button(PSB_PAD_RIGHT))            // ถ้าปุ่มขวาถูกกด
    sr(40);                                      // สั่งงานให้หุ่นยนต์เลี้ยวขวา
  else                                           // ถ้าไม่มีปุ่มทิศทางใดๆถูกกด
    ao();                                        // หุ่นยนต์หยุดเคลื่อนที่
  
  if(isR1)                                       // เมื่อสถานะของ isR1 เป็น True (ถูกกด) : หนีบวัตถุ
  {  
    servoGrab += 2;                              // องศาของเซอร์โวเพิ่มขึ้น 2 องศา
    servoGrab = (servoGrab > 140) ? 140 : servoGrab; // ให้ค่าองศาของเซอร์โวสูงสุดแค่ 140 องศา
    servo(0, servoGrab);                         // กำหนดองศาของเซอร์โวที่เก็บไว้ในตัวแปร servoGrab
  } 
  else if(isR2)                                  // เมื่อสถานะของ isR2 เป็น True (ถูกกด) : ปล่อยวัตถุ
  {
    servoGrab -= 2;                              // องศาของเซอร์โวลดลง 2 องศา
    servoGrab = (servoGrab < 40) ? 40 : servoGrab;   // ให้ค่าองศาของเซอร์โวต่ำสุดแค่ 40 องศา
    servo(0, servoGrab);                         // กำหนดองศาของเซอร์โวสำหรับหนีบของที่เก็บไว้ในตัวแปร servoGrab
  } 
  else if(isL1)                                  // เมื่อสถานะของ isL1 เป็น True (ถูกกด) : ปล่อยแขนจับให้ต่ำลง
  {
    servoLift += 2;                              // องศ่าของเซอร์โวเพิ่มขึ้น 2 องศา
    servoLift = (servoLift > 135) ? 135 : servoLift; // ให้ค่าองศาของเซอร์โวสูงสุดแค่ 135 องศา
    servo(1, servoLift);                         // กำหนดองศาของเซอร์โวสำหรับยกแขนจับที่เก็บไว้ในตัวแปร servoLift
  } 
  else if(isL2)                                  // เมื่อสถานะของ isL2 เป็น True (ถูกกด) : ยกแขนจับขึ้น
  {
    servoLift -= 2;                              // องศาของเซอร์โวลดลง 2 องศา
    servoLift = (servoLift < 35) ? 35 : servoLift;   // ให้ค่าองศาของเซอร์โวต่ำสุดแค่ 35 องศา
    servo(1, servoLift);                         // กำหนดองศาของเซอร์โวสำหรับยกแขนจับที่เก็บไว้ในตัวแปร servoLift
  }
  
  delay(50);                                     // หน่วงเวลา 50 มิลลิวินาที
}
