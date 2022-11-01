/*******************************************************************************
* UART DMA �ػ�����
*
*
* ��������ʾ USART2 ��� DMA ʹ��
* �޷��ͻ��棬ʹ�����ݵ�ַ�͵ط���
* ���ջ���Ϊ���λ�����
* DMA �շ������� CPU �������
*
* UART1 ��Ϊ���Դ���
* ���� debug.h ������  UART1 Ϊ Debug ���ڣ�����
* #define DEBUG   DEBUG_UART1
*
* ����������MRS
*******************************************************************************/

#include "debug.h"

/* Global define */
#define RXBUF_SIZE 1024     // DMA buffer size ; DMA ���ջ���Ĵ�С
#define size(a)   (sizeof(a) / sizeof(*(a)))

/* Global Variable */
u8 RxBuffer[RXBUF_SIZE]={0};	// DMA ���ջ���


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

	/* USART2 TX-->A2  RX-->A3 */
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
	DMA_Cmd(DMA1_Channel6, ENABLE);                                  //�������� DMA
	USART_Cmd(USART2, ENABLE);                                        //����UART
}

/*******************************************************************************
* Function Name  : DMA_INIT
* Description    : Configures the DMA.
* ����	��	DMA ��ʼ��
* Input          : None
* Return         : None
*******************************************************************************/
void DMA_INIT(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	// TX DMA ��ʼ��
	DMA_DeInit(DMA1_Channel7);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART2->DATAR);        // DMA �����ַ����ָ���Ӧ������
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)RxBuffer;                   // DMA �ڴ��ַ��ָ���ͻ��������׵�ַ��δָ���Ϸ���ַ������ DMA ��� hardfault
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                     // ���� : ���� ��Ϊ �յ㣬�� �ڴ� ->  ����
	DMA_InitStructure.DMA_BufferSize = 0;                                   // ��������С,��Ҫ DMA ���͵����ݳ���,Ŀǰû�����ݿɷ�
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        // �����ַ����������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 // �ڴ��ַ����������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // ��������λ��8λ(Byte)
 	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         // �ڴ�����λ��8λ(Byte)
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           // ��ͨģʽ�������������ѭ������
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;                 // ���ȼ����
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                            // ʵ��Ϊ M2P,���Խ��� M2M
	DMA_Init(DMA1_Channel7, &DMA_InitStructure);

	// RX DMA ��ʼ�������λ������Զ�����
	DMA_DeInit(DMA1_Channel6);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART2->DATAR);
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)RxBuffer;                   // ���ջ�����
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                      // ���� : ���� ��Ϊ Դ���� �ڴ� <- ����
	DMA_InitStructure.DMA_BufferSize = RXBUF_SIZE;                          // ����������Ϊ RXBUF_SIZE
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                         // ѭ��ģʽ�����ɻ��λ�����
	DMA_Init(DMA1_Channel6, &DMA_InitStructure);

}



/*******************************************************************************
* Function Name  :  uartWriteBlocking
* Description    :  send data via USART2			��USART2�������ݣ�����ʽ
* Input          :  char * data          data to send	Ҫ���͵����ݵ��׵�ַ
*                   uint16_t num         number of data	���ݳ���
* Return         :  RESET                USART2 busy,failed to send	����ʧ��
*                   SET                  send success				���ͳɹ�
*******************************************************************************/
FlagStatus uartWriteBlocking(char * data , uint16_t num)
{
    //�ȴ��ϴη������
	while(DMA_GetCurrDataCounter(DMA1_Channel7) != 0){
	}

    DMA_ClearFlag(DMA2_FLAG_TC8);
	DMA_Cmd(DMA1_Channel7, DISABLE );           // �� DMA �����
	DMA1_Channel7->MADDR = (uint32_t)data;      // ���ͻ�����Ϊ data
	DMA_SetCurrDataCounter(DMA1_Channel7,num);  // ���û���������
	DMA_Cmd(DMA1_Channel7, ENABLE);             // �� DMA
	return SET;
}

/*******************************************************************************
* Function Name  :  uartWriteStrBlocking
* Description    :  send string via USART2	��USART2�����ַ���������ʽ
* Input          :  char * str          string to send
* Return         :  RESET                USART2 busy,failed to send	����ʧ��
*                   SET                  send success				���ͳɹ�
*******************************************************************************/
FlagStatus uartWriteStrBlocking(char * str)
{
    uint16_t num = 0;
    while(str[num])num++;           // �����ַ�������
    return uartWriteBlocking(str,num);
}


/*******************************************************************************
* Function Name  :  uartRead
* Description    :  read some bytes from receive buffer �ӽ��ջ���������һ������
* Input          :  char * buffer        buffer to storage the data	������Ŷ������ݵĵ�ַ
*                   uint16_t num         number of data to read		Ҫ�����ֽ���
* Return         :  int                  number of bytes read		����ʵ�ʶ������ֽ���
*******************************************************************************/
uint16_t rxBufferReadPos = 0;       //���ջ�������ָ��
uint32_t uartRead(char * buffer , uint16_t num)
{
    uint16_t rxBufferEnd = RXBUF_SIZE - DMA_GetCurrDataCounter(DMA1_Channel6); //���� DMA ����β��λ��
    uint16_t i = 0;

    if (rxBufferReadPos == rxBufferEnd){
        // �����ݣ�����
        return 0;
    }

    while (rxBufferReadPos!=rxBufferEnd && i < num){
        buffer[i] = RxBuffer[rxBufferReadPos];
        i++;
        rxBufferReadPos++;
        if(rxBufferReadPos >= RXBUF_SIZE){
            // ��ָ�볬�����ջ�����������
            rxBufferReadPos = 0;
        }
    }
    return i;
}

/*******************************************************************************
* Function Name  :  uartReadByte
* Description    :  read one byte from UART buffer	�ӽ��ջ��������� 1 �ֽ�����
* Input          :  None
* Return         :  char    read data				���ض���������(������Ҳ����0)
*******************************************************************************/
char uartReadByte()
{
    char ret;
    uint16_t rxBufferEnd = RXBUF_SIZE - DMA_GetCurrDataCounter(DMA1_Channel6);//���� DMA ����β��λ��
    if (rxBufferReadPos == rxBufferEnd){
        // �����ݣ�����
        return 0;
    }
    ret = RxBuffer[rxBufferReadPos];
    rxBufferReadPos++;
    if(rxBufferReadPos >= RXBUF_SIZE){
        // ��ָ�볬�����ջ�����������
        rxBufferReadPos = 0;
    }
    return ret;
}
/*******************************************************************************
* Function Name  :  uartAvailable
* Description    :  get number of bytes Available to read from the UART buffer	��ȡ�������пɶ����ݵ�����
* Input          :  None
* Return         :  uint16_t    number of bytes Available to read				���ؿɶ���������
*******************************************************************************/
uint16_t uartAvailable()
{
    uint16_t rxBufferEnd = RXBUF_SIZE - DMA_GetCurrDataCounter(DMA1_Channel6);//���� DMA ����β��λ��
    // ����ɶ��ֽ�
    if (rxBufferReadPos <= rxBufferEnd){
        return rxBufferEnd - rxBufferReadPos;
    }else{
        return rxBufferEnd +RXBUF_SIZE -rxBufferReadPos;
    }
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

	DMA_INIT();
	USARTx_CFG();                                                 	/* USART INIT */
	USART_DMACmd(USART2,USART_DMAReq_Tx|USART_DMAReq_Rx,ENABLE);	//���� USART2 DMA ���պͷ���
	uartWriteStrBlocking("From USART2 DMA.\r\n");
	Delay_Ms(500);
	int period = 500; // ���ڻش������ ��λΪ����
	while(1){
	//���ֽ��շ�
		while(!uartAvailable()); 	//�ȴ�RX����
		//�����ѽ��յ������ݷ��ͳ�ȥ��
		char c = uartReadByte();	
		uartWriteBlocking(&c,1);

	//���ֽ��շ�	
		Delay_Ms(period);			//��һ������ܵ����ݣ�����������󣬽��ջ�������
		int num = uartAvailable();	//��ȡ�ɶ��ֽ���
		if (num > 0 ){
            char buffer[1024];	
            uartRead(buffer , num);	//�����ݴӻ��������� buffer ��
			uartWriteBlocking(buffer , num); //�����ݷ��͸� PC
		}
	}

}

