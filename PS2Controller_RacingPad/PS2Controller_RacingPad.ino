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
 * ควบคุมหุ่นยนต์ IPST-SE แบบเกมขับรถโดยใชปุ่มทิศทาง
 *
 * ควบคุมหุ่นยนต์ IPST-SE ด้วยการใช้ปุ่มกากบาทและวงกลมสำหรับสั่งให้หุ่นยนต์เคลื่อนที่ไปข้างหน้า 
 * โดยที่ปุ่มกากบาทมีความเร็ว 80% และปุ่มวงกลมมีความเร็ว 100% และปุ่มสี่เหลี่ยมสำหรับถอยหลัง (ความเร็ว 80%)
 * ในการควบคุมทิศทางจะใชปุ่มทิศทางซ้ายขวาคล้ายกับการเล่นเกมขับรถ
 */

#include <ipst.h>                                 // เรียกใช้งานไลบรารีสำหรับ IPST-SE
#include <PS2X_lib.h>                             // เรียกใช้งานไลบรารีสำหรับ PS2 Controller

#define PS2_DAT        18                         // กำหนดขา Data    เป็นขา 18
#define PS2_CMD        20                         // กำหนดขา Command เป็นขา 20
#define PS2_SEL        19                         // กำหนดขา Select  เป็นขา 19
#define PS2_CLK        17                         // กำหนดขา Clock   เป็นขา 17

#define NONE       0                              // แทน 0 เป็นสถานะที่หุ่นยนต์ไม่เคลื่อนที่ ด้วยการตั้งชื่อตัวแปรว่า NONE
#define FORWARD    1                              // แทน 1 เป็นสถานะที่หุ่นยนต์เคลื่อนที่เดินหน้า ด้วยการตั้งชื่อตัวแปรว่า FORWARD
#define BACKWARD   2                              // แทน 2 เป็นสถานะที่หุ่นยนต์เคลื่อนที่ถอยหลัง ด้วยการตั้งชื่อตัวแปรว่า BACKWARD

PS2X ps2x;                                        // ประกาศตัวแปรสำหรับ PS2 Controller

byte moving = NONE;                               // ประกาศตัวแปรสำหรับเก็บสถานะการเคลื่อนที่ โดยกำหนดค่าเริ่มต้นเป็น NONE หรือไม่เคลื่อนที่
byte left = 80, right = 80;                       // ประกาศตัวแปร left และ right สำหรับใช้กำหนดความเร็วในการเคลื่อนที่ของล้อแต่ละฝั่ง โดยมีความเร็วเป็น 80%

void setup()
{
  delay(1000);                                    // หน่วงเวลา 1 วินาทีเพื่อรอให้บอร์ดพร้อมทำงาน
  glcdClear();                                    // เคลียร์ภาพทั้งหมดบนหน้าจอแสดงผล
  
  // กำหนดขาที่จะเชื่อมต่กับ PS2 Controller โดยมีการเก็บค่าที่ส่งกลับมาเป็น Integer เพื่อรู้ได้ว่าเชื่อมต่อได้หรือไม่
  int error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, false, false);
  
  if(error == 0)                                  // กรณีที่เชื่อมต่อได้ ไม่มีปัญหาอะไร (Error = 0)
  {
    glcd(0, 0, "Controller found");               // แสดงหน้าจอว่าเชื่อมต่อกับ PS2 Controller ได้
  } 
  else                                            // ถ้าไม่สามารถเชื่อมต่อกับ PS2 Controller 
  {
    glcd(0, 0, "Controller not found");           // แสดงหน้าจอว่าไม่สามารถเชื่อมต่อกับ PS2 Controller ได้
    glcd(1, 0, "Check your controller");
    glcd(2, 0, "And restart to try again");
    while(true);                                  // วนไม่จำกัด เพื่อให้ผู้ใช้รีเซตบอร์ดเพื่อเชื่อมต่อใหม่
  }  
}

void loop()
{
  ps2x.read_gamepad(false, false);                // อ่านข้อมูลจาก PS2 Controller

  if(ps2x.Button(PSB_CROSS))                      // ถ้าปุ่มกากบาทถูกกด
  {            
    moving = FORWARD;                             // กำหนดสถานะการเคลื่อนที่เป็น FORWARD
    left = 50;                                    // กำหนดความเร็วของล้อฝั่งซ้ายเป็น 50%
    right = 50;                                   // กำหนดความเร็วของล้อฝั่งขวาเป็น 50%
  } 
  else if(ps2x.Button(PSB_CIRCLE))                // ถ้าปุ่มวงกลมถูกกด
  {
    moving = FORWARD;                             // กำหนดสถานะการเคลื่อนที่เป็น FORWARD
    left = 100;                                   // กำหนดความเร็วของล้อฝั่งซ้ายเป็น 100%
    right = 100;                                  // กำหนดความเร็วของล้อฝั่งขวาเป็น 100%
  } 
  else if(ps2x.Button(PSB_SQUARE))                // ถ้าปุ่มสี่เหลี่ยมถูกกด
  {
    moving = BACKWARD;                            // กำหนดสถานะการเคลื่อนที่เป็น BACKWARD
    left = 60;                                    // กำหนดความเร็วของล้อฝั่งซ้ายเป็น 60%
    right = 60;                                   // กำหนดความเร็วของล้อฝั่งขวาเป็น 60%
  } 
  else                                            // ถ้าไม่มีปุ่มไหนถูกกด
  {
    moving = NONE;                                // กำหนดสถานะการเคลื่อนที่เป็น NONE
  }
    
  if(ps2x.Button(PSB_PAD_LEFT))                   // เมื่อกดปุ่มซ้าย
  {
    left = left / 2;                              // ความเร็วของล้อฝั่งซ้ายลดลงครึ่งหนึ่ง
  } 
  else if(ps2x.Button(PSB_PAD_RIGHT))             // เมื่อกดปุ่มขวา
  {
    right = right / 2;                            // ความเร็วของล้อฝั่งขวาลดลงครึ่งหนึ่ง
  } 
  
  if(moving == FORWARD)                           // ถ้าสถานะการเคลื่อนที่เป็น FORWARD
    fd2(left, right);                             // เคลื่อนที่ไปข้างหน้าโดยกำหนดความเร็วของล้อแต่ละฝั่งตามตัวแปรแยกกัน
  else if(moving == BACKWARD)                     // ถ้าสถานะการเคลื่อนที่เป็น BACKWARD
    bk2(left, right);                             // เคลื่อนที่ไปข้างหลังโดยกำหนดความเร็วของล้อแต่ละฝั่งตามตัวแปรแยกกัน
  else                                            // ถ้าสถานะการเคลื่อนที่ไม่ใช่ FORWARD และ BACKWARD (เป็น NONE)
    ao();                                         // หยุดเคลื่อนที่
  
  delay(50);                                      // หน่วงเวลา 50 มิลลิวินาที
}
