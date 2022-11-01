/*******************************************************************************
* UART �շ����̣���ѯ��
*
* ��������ʾʹ�� USART2 �շ�����
*
* UART1 ��Ϊ���Դ���
* ���� debug.h ������  UART1 Ϊ Debug ���ڣ�����
* #define DEBUG   DEBUG_UART1
*
* ����������MRS
*******************************************************************************/

#include "debug.h"

/*******************************************************************************
* Function Name  : USARTx_CFG
* Description    : Initializes the USART peripheral.
* ����	��	���ڳ�ʼ��
* Input          : None
* Return         : None
*******************************************************************************/
void USARTx_CFG(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	//����ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	/* USART2 TX-->PA2  RX-->PA3 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;           //RX����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;                    // ������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;     // ����λ 8
	USART_InitStructure.USART_StopBits = USART_StopBits_1;          // ֹͣλ 1
	USART_InitStructure.USART_Parity = USART_Parity_No;             // ��У��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // ��Ӳ������
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; //ʹ�� RX �� TX

	USART_Init(USART2, &USART_InitStructure);

	USART_Cmd(USART2, ENABLE);                                        //����UART
}

/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Return         : None
*******************************************************************************/
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	Delay_Init();
	USART_Printf_Init(115200);
	printf("SystemClk:%d \t---- From debug : UART%d\r\n",SystemCoreClock,DEBUG);

	USARTx_CFG();           /* USART INIT */
	int i = 0;
	char str[]="Loop back from USART2.\r\n";     //����һ����ʾ��
	while(str[i]){
	    while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);    //�ȴ� �ϴη��� ����
	    USART_SendData(USART2, str[i]);                                 //��������
	    i++;
	}
	Delay_Ms(500);
	int recv;
	while(1){
	    //���ڻػ����ѽ��յ�������ԭ������
	    while(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET);   //�ȴ���������
	    recv = USART_ReceiveData(USART2);                               //��ȡ���յ�������
        while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);    //�ȴ� �ϴη��� ����
        USART_SendData(USART2, recv);                                   //��������

	}

}

