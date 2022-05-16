/*
 * @Author: TOTHTOT
 * @Date: 2022-04-05 18:58:40
 * @LastEditTime: 2022-04-14 17:11:32
 * @LastEditors: Please set LastEditors
 * @Description: ��koroFileHeader�鿴���� ��������: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\ʵ��\������ҩС��(FreeRTOS_F103C8T6)\HARDWARE\GRAYSENSOR\graysensor.c
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

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //ʹ��PA�˿�ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //ʹ��PA�˿�ʱ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1; //�˿�����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;          //��������, �Ҷȴ�������û���յ��ź�ʱΪ�ߵ�ƽ
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      // IO���ٶ�Ϊ50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);                 //�����趨������ʼ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;         //�˿�����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;     //��������, ���⴫������û���յ��ź�ʱΪ�ߵ�ƽ
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // IO���ٶ�Ϊ50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);            //�����趨������ʼ��

    // GPIOA.7 �ж����Լ��жϳ�ʼ������
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);

    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; //�½��ش���
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure); //����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���

    //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;          //ʹ�ܰ������ڵ��ⲿ�ж�ͨ��
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9; //��ռ���ȼ�0��
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        //�����ȼ�0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           //ʹ���ⲿ�ж�ͨ��
    NVIC_Init(&NVIC_InitStructure);
}

// ��ȡ�����Ҷȴ�������ֵ,��Ϊ�͵�ƽ��ʾ�Ҷȴ������������м�
short Car_Staright_Control(void)
{
    if (GRAYSENSOR_RIGHT == 0 && GRAYSENSOR_LEFT == 0) //���ߴ�������û��⵽����,����,ֱ��
    {
        // printf("Car_Staright_Control, straight \r\n");
        return 0;
    }
    else if (GRAYSENSOR_RIGHT == 1 && GRAYSENSOR_LEFT == 0) //�ұߴ�������⵽����,����,��ת
    {
        // printf("Car_Staright_Control: left\r\n");
        /* ���ּ���,���ּ��� */
        return 400;
    }
    else if (GRAYSENSOR_RIGHT == 0 && GRAYSENSOR_LEFT == 1) //��ߴ�������⵽����,����,��ת
    {
        // printf("Car_Staright_Control: right\r\n");
        /* ���ּ���,���ּ��� */
        return -400;
    }
    /*   else if (GRAYSENSOR_RIGHT == 1 && GRAYSENSOR_LEFT == 1) //��������������⵽����,����,ֹͣ
      {
          return 3;
      } */
    else
    {
        return 4;
    }
}

// �ⲿ�ж�0
void EXTI1_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken;
    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == 0)
    {
#if CAR1_EN
        printf("yaowu zz\r\n");
        car_status.goose = 1;
        car_status.struct_flag = 1; //����OLED������ʾ
#endif                              /* CAR1_EN */
#if CAR2_EN
        if (car2_receive_data.zhong_or_yuan == 1)
        {
            printf("yaowu load\r\n");
            car_status.goose = 1;
            car_status.struct_flag = 1; //����OLED������ʾ
        }
        else if (car2_receive_data.zhong_or_yuan == 2) //��ΪԶ����������װ��ҩ�� �����������ҩ�ﶼΪ1
        {
            car_status.goose = 1;
            car_status.struct_flag = 1; //����OLED������ʾ
        }
#endif /* CAR2_EN */
    }
    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == 1)
    {
#if CAR1_EN
        printf("yaowu xz\r\n");
        car_status.goose = 0;
        car_status.struct_flag = 1; //����OLED������ʾ
#endif                              /* CAR1_EN */
#if CAR2_EN                         //С��2�в����Ӳ���Ҫ�ж��Ƿ���ҩ��
        if (car2_receive_data.zhong_or_yuan == 1)
        {
            printf("yaowu xz\r\n");
            car_status.goose = 0;
            car_status.struct_flag = 1; //����OLED������ʾ
        }
        else if (car2_receive_data.zhong_or_yuan == 2) //��ΪԶ����������װ��ҩ�� �����������ҩ�ﶼΪ1
        {
            car_status.goose = 1;
            car_status.struct_flag = 1; //����OLED������ʾ
        }
#endif /* CAR2_EN */
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken); //�ж��Ƿ���Ҫ�����л�
    EXTI_ClearITPendingBit(EXTI_Line1);           //���LINE�ϵ��жϱ�־λ
}
