/*
 * @Author: TOTHTOT
 * @Date: 2022-04-05 18:58:40
 * @LastEditTime: 2022-04-14 17:11:32
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\实例\智能送药小车(FreeRTOS_F103C8T6)\HARDWARE\GRAYSENSOR\graysensor.c
 */
#include "graysensor.h"
#include "oled.h"
#include "usart.h"
#include "car.h"
/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

void GraySensor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //使能PA端口时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //使能PA端口时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1; //端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;          //下拉输入, 灰度传感器在没就收到信号时为高电平
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      // IO口速度为50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);                 //根据设定参数初始化

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;         //端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;     //上拉输入, 红外传感器在没就收到信号时为高电平
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // IO口速度为50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);            //根据设定参数初始化

    // GPIOA.7 中断线以及中断初始化配置
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);

    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; //下降沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure); //根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

    //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;          //使能按键所在的外部中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9; //抢占优先级0，
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        //子优先级0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           //使能外部中断通道
    NVIC_Init(&NVIC_InitStructure);
}

// 获取两个灰度传感器的值,都为低电平表示灰度传感器红线在中间
short Car_Staright_Control(void)
{
    if (GRAYSENSOR_RIGHT == 0 && GRAYSENSOR_LEFT == 0) //两边传感器都没检测到红线,灯亮,直行
    {
        // printf("Car_Staright_Control, straight \r\n");
        return 0;
    }
    else if (GRAYSENSOR_RIGHT == 1 && GRAYSENSOR_LEFT == 0) //右边传感器检测到红线,灯灭,左转
    {
        // printf("Car_Staright_Control: left\r\n");
        /* 右轮减速,左轮加速 */
        return 400;
    }
    else if (GRAYSENSOR_RIGHT == 0 && GRAYSENSOR_LEFT == 1) //左边传感器检测到红线,灯灭,右转
    {
        // printf("Car_Staright_Control: right\r\n");
        /* 右轮加速,左轮减速 */
        return -400;
    }
    /*   else if (GRAYSENSOR_RIGHT == 1 && GRAYSENSOR_LEFT == 1) //两个传感器都检测到红线,灯灭,停止
      {
          return 3;
      } */
    else
    {
        return 4;
    }
}

// 外部中断0
void EXTI1_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken;
    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == 0)
    {
#if CAR1_EN
        printf("yaowu zz\r\n");
        car_status.goose = 1;
        car_status.struct_flag = 1; //告诉OLED更新显示
#endif                              /* CAR1_EN */
#if CAR2_EN
        if (car2_receive_data.zhong_or_yuan == 1)
        {
            printf("yaowu load\r\n");
            car_status.goose = 1;
            car_status.struct_flag = 1; //告诉OLED更新显示
        }
        else if (car2_receive_data.zhong_or_yuan == 2) //因为远部发挥无需装载药物 所以无论如何药物都为1
        {
            car_status.goose = 1;
            car_status.struct_flag = 1; //告诉OLED更新显示
        }
#endif /* CAR2_EN */
    }
    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == 1)
    {
#if CAR1_EN
        printf("yaowu xz\r\n");
        car_status.goose = 0;
        car_status.struct_flag = 1; //告诉OLED更新显示
#endif                              /* CAR1_EN */
#if CAR2_EN                         //小车2中部发挥才需要判断是否有药物
        if (car2_receive_data.zhong_or_yuan == 1)
        {
            printf("yaowu xz\r\n");
            car_status.goose = 0;
            car_status.struct_flag = 1; //告诉OLED更新显示
        }
        else if (car2_receive_data.zhong_or_yuan == 2) //因为远部发挥无需装载药物 所以无论如何药物都为1
        {
            car_status.goose = 1;
            car_status.struct_flag = 1; //告诉OLED更新显示
        }
#endif /* CAR2_EN */
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken); //判断是否需要任务切换
    EXTI_ClearITPendingBit(EXTI_Line1);           //清除LINE上的中断标志位
}
