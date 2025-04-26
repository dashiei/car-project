#include "stm32f10x.h"                  // STM32F10x外设库头文件
#include "Delay.h"                      // 延时函数头文件

/**
 * @brief  按键初始化函数
 * @param  无
 * @retval 无
 * @note   配置PA15引脚为上拉输入模式
 */
void Key_Init(void)
{
    // 使能GPIOA的时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    // 定义GPIO初始化结构体
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;    // 设置为上拉输入模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;       // 选择PA15引脚
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 设置GPIO速度为50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);           // 初始化GPIO
}

/**
 * @brief  获取按键状态函数
 * @param  无
 * @retval 返回按键值：0表示未按下，1表示按下
 * @note   采用消抖处理，检测按键按下和松开
 */
uint8_t Key_GetNum(void)
{
    uint8_t KeyNum = 0;
    // 检测到PA15引脚为低电平（按键按下）
    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15) == 0)
    {
        Delay_ms(20);    // 延时20ms进行消抖
        // 等待按键松开
        while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15) == 0);
        Delay_ms(20);    // 延时20ms进行消抖
        KeyNum = 1;      // 按键按下，返回值置1
    }
    return KeyNum;       // 返回按键状态
}