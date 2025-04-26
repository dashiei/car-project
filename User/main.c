#include "stm32f10x.h"                  // Device header
#include "LEDSEG.h"
#include "Delay.h"
#include "robot.h"
#include "Incontrol.h"
#include "Irtracking.h"
#include "Irobstacle.h"
#include "Key.h"
#include "Serial.h"

unsigned char temp = 1;    // 模式切换标志位

void Robot_Traction()      // 红外循迹功能函数
{
	if(Left_Irtracking_Get() == 0 && Right_Irtracking_Get() == 0)
	{
		makerobo_run(70,0);   // 两个传感器都未检测到黑线,小车前进
	}
	else if(Left_Irtracking_Get() == 1 && Right_Irtracking_Get() == 0) // 左传感器检测到黑线
	{
		makerobo_Left(70,0);    // 小车左转
	}
	else if(Left_Irtracking_Get() == 0 && Right_Irtracking_Get() == 1) // 右传感器检测到黑线
	{
		makerobo_Right(70,0); // 小车右转
	}			
	else if(Left_Irtracking_Get() == 1 && Right_Irtracking_Get() == 1) // 两个传感器都检测到黑线,停车
	{
	 makerobo_brake(0); // 停止
	}
}

void Robot_Avoidance()     // 红外避障功能函数
{
	if(Left_Irobstacle_Get() == 1 && Right_Irobstacle_Get() == 1)
			{
				makerobo_run(70,0);   // 两侧都未检测到障碍物,小车前进
			}
			else if(Left_Irobstacle_Get() == 1 && Right_Irobstacle_Get() == 0) // 右侧检测到障碍物
			{
				makerobo_Left(70,400);  // 左转400ms
			}
			else if(Left_Irobstacle_Get() == 0 && Right_Irobstacle_Get() == 1) // 左侧检测到障碍物
			{
				makerobo_Right(70,400); // 右转400ms
			}			
			else if(Left_Irobstacle_Get() == 0 && Right_Irobstacle_Get() == 0) // 两侧同时检测到障碍物
			{
				makerobo_brake(400);          // 停止400ms
				makerobo_Spin_Right(70,900);  // 原地右转900ms
			}
}

void ControlCar_Ircontrol() // 红外遥控控制功能函数
{
  uint8_t buf[2];
	uint8_t data_code=0; 
	
	if(IR_Receiveflag == 1) // 接收到红外信号
		{
			IR_Receiveflag = 0; // 清除标志位
			printf("接收到的数据: %0.8X\r\n",IR_Receivecode);	// 打印接收到的数据
			data_code=IR_Receivecode>>8;
			IR_Receivecode = 0; // 清除接收到的数据
			
			buf[0] = data_code/16;
			buf[1] = data_code%16;
			
		  printf("buf[0]:%d\r\n",buf[0]);
		  printf("buf[1]:%d\r\n",buf[1]);
		}
    if(buf[0] == 11 && buf[1] == 1)
		{
			makerobo_run(70,2000);  // 前进2秒
		}
		else if(buf[0] == 13 && buf[1] == 4)
		{
			makerobo_back(70,2000); // 后退2秒
		}
		else if(buf[0] == 9 && buf[1] == 1)
		{
			makerobo_Spin_Left(70,2000); // 左转2秒
		}
		else if(buf[0] == 14 && buf[1] == 1)
		{
			makerobo_Spin_Right(70,2000); // 右转2秒
		}
		else if(buf[0] == 8 && buf[1] == 1)
		{
			makerobo_brake(0); // 停止
		}
		else
		{
			makerobo_brake(0); // 停止
		}
}

void Irscan()
{
	uint8_t Tim=0,Ok=0,Data,Num=0;
	while(1)
	{
	  if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)==1)
		{
			 Tim=IRremote_Counttime();// 计算此次高电平时间

			 if(Tim>=250) break;// 超过时间退出

			 if(Tim>=200 && Tim<250)
			 {
			 	Ok=1;// 接收到起始信号
			 }
			 else if(Tim>=60 && Tim<90)
			 {
			 	Data=1;// 接收到数据1
			 }
			 else if(Tim>=10 && Tim<50)
			 {
			 	Data=0;// 接收到数据0
			 }

			 if(Ok==1)
			 {
			 	IR_Receivecode<<=1;
				IR_Receivecode+=Data;

				if(Num>=32)
				{
					IR_Receiveflag=1;
				  break;
				}
			 }
			 Num++;
		}
		ControlCar_Ircontrol();
	}
	EXTI_ClearITPendingBit(EXTI_Line8);	
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  // 设置中断优先级分组2
	Key_Init();
	IRremote_Init();           // 红外遥控初始化
	Irtracking_Init();         // 红外循迹初始化
	Irobstacle_Init();         // 红外避障初始化
	LEDSEG_Init();
	Serial_Init();             // 串口初始化
	robot_Init();              // 小车初始化
	while (1)
	{
	  if(Key_GetNum() == 1)   // 按键按下
		{
			temp ++;            // 模式切换
		}
		if(temp > 3)
		{
			temp = 1;          // 模式循环
		}
		switch(temp)
		{
			case 1: Digital_Display(1);Robot_Traction();EXTI_DeInit();break;  // 模式1:循迹模式
			case 2: Digital_Display(2);Robot_Avoidance();break;              // 模式2:避障模式
			case 3: Digital_Display(3);Irscan();break;                       // 模式3:遥控模式
		}
	}
}