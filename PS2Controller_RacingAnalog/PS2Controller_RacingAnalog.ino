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
 * ควบคุมหุ่นยนต์ IPST-SE แบบเกมขับรถโดยใช้คันโยกอะนาล็อก
 *
 * ควบคุมหุ่นยนต์ IPST-SE ด้วยการใช้ปุ่มกากบาทและวงกลมสำหรับสั่งให้หุ่นยนต์เคลื่อนที่ไปข้างหน้า 
 * โดยที่ปุ่มกากบาทมีความเร็ว 80% และปุ่มวงกลมมีความเร็ว 100% และปุ่มสี่เหลี่ยมสำหรับถอยหลัง (ความเร็ว 80%)
 * ในการควบคุมทิศทางจะใช้คันโยกอะนาล็อกฝั่งซ้ายมือเพื่อเลี้ยว โดยสามารถโยกมากน้อยเพื่อกำหนดว่าเลี้ยวมากน้อยแค่ไหน
 */


#include <ipst.h>                                 // เรียกใช้งานไลบรารีสำหรับ IPST-SE
#include <PS2X_lib.h>                             // เรียกใช้งานไลบรารีสำหรับ PS2 Controller

#define PS2_DAT        16                         // กำหนดขา Data    เป็นขา 16
#define PS2_CMD        17                         // กำหนดขา Command เป็นขา 17
#define PS2_SEL        18                         // กำหนดขา Select  เป็นขา 18
#define PS2_CLK        19                         // กำหนดขา Clock   เป็นขา 19

#define NONE           0                          // แทน 0 เป็นสถานะที่หุ่นยนต์ไม่เคลื่อนที่ ด้วยการตั้งชื่อตัวแปรว่า NONE
#define FORWARD        1                          // แทน 1 เป็นสถานะที่หุ่นยนต์เคลื่อนที่เดินหน้า ด้วยการตั้งชื่อตัวแปรว่า FORWARD
#define BACKWARD       2                          // แทน 2 เป็นสถานะที่หุ่นยนต์เคลื่อนที่ถอยหลัง ด้วยการตั้งชื่อตัวแปรว่า BACKWARD

PS2X ps2x;                                        // ประกาศตัวแปรสำหรับ PS2 Controller

byte turboSpeed = 100;                            // กำหนดความเร็วเมื่อกดปุ่มวงกลมเป็น 100%
byte maxSpeed = 80;                               // กำหนดความเร็วเมื่อกดปุ่มกากบาทหรือสี่เหลี่ยมเป็น 80%
byte minSpeed = 30;                               // กำหนดความเร็วต่ำสุดเมื่อมีการเลี้ยว โดยให้ความเร็วของล้อฝั่งที่จะเลี้ยวเป็น 30% 
byte currentSpeed = maxSpeed;                     // ตัวแปรสำหรับเก็บความเร็ว ณ ตอนนั้น โดยเปลี่ยนไปตามการกดปุ่มเคลื่อนที่ กำหนดให้มีค่าเริ่มต้นตามที่กำหนดใน maxSpeed

byte moving = NONE;                               // ประกาศตัวแปรสำหรับเก็บสถานะการเคลื่อนที่ โดยกำหนดค่าเริ่มต้นเป็น NONE หรือไม่เคลื่อนที่
byte left = 80, right = 80;                       // ประกาศตัวแปร left และ right สำหรับใช้กำหนดความเร็วในการเคลื่อนที่ของล้อแต่ละฝั่ง โดยมีความเร็วเป็น 80%


void setup()
{
  delay(1000);                                    // หน่วงเวลา 1 วินาทีเพื่อรอให้บอร์ดพร้อมทำงาน
  glcdClear();                                    // เคลียร์ภาพทั้งหมดบนหน้าจอแสดงผล
  glcdMode(1);                                   // กำหนดให้หน้าจอแสดงผลเป็นแนวนอน
  
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
  ps2x.read_gamepad(false, false);                // อ่านข้อมูลจาก PS2 Controller
  
  if(ps2x.Button(PSB_CROSS))                      // ถ้าปุ่มกากบาทถูกกด
  {
    moving = FORWARD;                             // กำหนดสถานะการเคลื่อนที่เป็น FORWARD
    currentSpeed = maxSpeed;                      // กำหนดความเร็วปัจจุบันเป็นค่าจากตัวแปร maxSpeed
  } 
  else if(ps2x.Button(PSB_CIRCLE))                // ถ้าปุ่มวงกลมถูกกด
  {
    moving = FORWARD;                             // กำหนดสถานะการเคลื่อนที่เป็น FORWARD
    currentSpeed = turboSpeed;                    // กำหนดความเร็วปัจจุบันเป็นค่าจากตัวแปร turboSpeed
  } 
  else if(ps2x.Button(PSB_SQUARE))                // ถ้าปุ่มสี่เหลี่ยมถูกกด
  {
    moving = BACKWARD;                            // กำหนดสถานะการเคลื่อนที่เป็น BACKWARD
    currentSpeed = maxSpeed;                      // กำหนดความเร็วปัจจุบันเป็นค่าจากตัวแปร maxSpeed
  } 
  else                                            // ถ้าไม่มีปุ่มไหนถูกกด
  {
    moving = NONE;                                // กำหนดสถานะการเคลื่อนที่เป็น NONE
  }
    
    
  int x = ps2x.Analog(PSS_LX);                    // อ่านค่าแกน X จากคันโยก Analog ฝั่งซ้าย มาเก็บไว้ในตัวแปร x
  left = currentSpeed;                            // กำหนดความเร็วของล้อฝั่งซ้ายตามค่าจากตัวแปร currentSpeed
  right = currentSpeed;                           // กำหนดความเร็วของล้อฝั่งขวาตามค่าจากตัวแปร currentSpeed

  if(x > 130)                                           // ถ้าค่าแกน X มากกว่า 130 
    right = map(x, 130, 255, currentSpeed, minSpeed);   // เฉลี่ยค่าที่อยู่ในช่วง 130 ถึง 255 ให้ออกมาเป็น currentSpeed ถึง minSpeed(30) แล้วเก็บไว้ในตัวแปร right
  else if( x < 125)
    left = map(x, 125, 0, currentSpeed, minSpeed);      // เฉลี่ยค่าที่อยู่ในช่วง 125 ถึง 0 ให้ออกมาเป็น currentSpeed ถึง minSpeed(30) แล้วเก็บไว้ในตัวแปร left
  
  if(moving == FORWARD)                           // ถ้าสถานะการเคลื่อนที่เป็น FORWARD
    fd2(left, right);                             // เคลื่อนที่ไปข้างหน้าโดยกำหนดความเร็วของล้อแต่ละฝั่งตามตัวแปรแยกกัน
  else if(moving == BACKWARD)                     // ถ้าสถานะการเคลื่อนที่เป็น BACKWARD
    bk2(left, right);                             // เคลื่อนที่ไปข้างหลังโดยกำหนดความเร็วของล้อแต่ละฝั่งตามตัวแปรแยกกัน
  else                                            // ถ้าสถานะการเคลื่อนที่ไม่ใช่ FORWARD และ BACKWARD (เป็น NONE)
    ao();                                         // หยุดเคลื่อนที่
  
  delay(50);                                      // หน่วงเวลา 50 มิลลิวินาที
}
