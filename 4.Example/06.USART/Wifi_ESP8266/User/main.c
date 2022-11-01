/*******************************************************************************
* WiFi ģ��ӿڲ�������
* ���˿������� UART6 ���� WiFi ģ��ӿڣ����� ESP-01��ESP-01S WiFi ģ��
* ʹ��ʱע�� WiFi ���߳������
* ��������ʾʹ�� DMA ͨ�� UART6 �� WiFi ģ��ͨ��
* ������ uartWriteWiFi(), uartWriteWiFiStr() �Ƿ������ġ�
* ������Щ��������ʱ������һ�η�����δ��ɣ������ȴ���ֱ�ӷ���
*******************************************************************************/

#include "debug.h"



/* Global define */
#define RXBUF_SIZE 1024 // DMA buffer size
#define size(a)   (sizeof(a) / sizeof(*(a)))
/* Global Variable */
u8 TxBuffer[] = " ";
u8 RxBuffer[RXBUF_SIZE]={0};                                         


/*******************************************************************************
* Function Name  : USARTx_CFG
* Description    : Initializes the USART6 peripheral.
* ����	��	���ڳ�ʼ��
* Input          : None
* Return         : None
*******************************************************************************/
void USARTx_CFG(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	//����ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART6, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	/* USART6 TX-->C0  RX-->C1 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;           //RX����������
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;                    // ������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;     // ����λ 8
	USART_InitStructure.USART_StopBits = USART_StopBits_1;          // ֹͣλ 1
	USART_InitStructure.USART_Parity = USART_Parity_No;             // ��У��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // ��Ӳ������
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; //ʹ�� RX �� TX

	USART_Init(UART6, &USART_InitStructure);
	DMA_Cmd(DMA2_Channel7, ENABLE);                                  //�������� DMA
	USART_Cmd(UART6, ENABLE);                                        //����UART
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
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

	// TX DMA ��ʼ��
	DMA_DeInit(DMA2_Channel6);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&UART6->DATAR);        // DMA �����ַ����ָ���Ӧ������
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)TxBuffer;                   // DMA �ڴ��ַ��ָ���ͻ��������׵�ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                      // ���� : ���� ��Ϊ �յ㣬�� �ڴ� ->  ����
	DMA_InitStructure.DMA_BufferSize = 0;                                   // ��������С,��ҪDMA���͵����ݳ���,Ŀǰû�����ݿɷ�
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        // �����ַ����������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 // �ڴ��ַ����������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // ��������λ��8λ(Byte)
 	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         // �ڴ�����λ��8λ(Byte)
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           // ��ͨģʽ�������������ѭ������
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;                 // ���ȼ����
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                            // M2P,����M2M
	DMA_Init(DMA2_Channel6, &DMA_InitStructure);

	// RX DMA ��ʼ�������λ������Զ�����
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)RxBuffer;                   // ���ջ�����
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                      // ���� : ���� ��Ϊ Դ���� �ڴ� <- ����
	DMA_InitStructure.DMA_BufferSize = RXBUF_SIZE;                          // ����������Ϊ RXBUF_SIZE
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                         // ѭ��ģʽ�����ɻ��λ�����
	DMA_Init(DMA2_Channel7, &DMA_InitStructure);
}



/*******************************************************************************
* Function Name  :  uartWriteWiFi
* Description    :  send data to ESP8266 via UART6
* ����	��	�� WiFi ģ�鷢������
* Input          :  char * data          data to send	Ҫ���͵����ݵ��׵�ַ
*                   uint16_t num         number of data	���ݳ���
* Return         :  RESET                UART6 busy,failed to send	����ʧ��
*                   SET                  send success				���ͳɹ�
*******************************************************************************/
FlagStatus uartWriteWiFi(char * data , uint16_t num)
{
    //���ϴη���δ��ɣ�����
	if(DMA_GetCurrDataCounter(DMA2_Channel6) != 0){
		return RESET;
	}

    DMA_ClearFlag(DMA2_FLAG_TC8);
	DMA_Cmd(DMA2_Channel6, DISABLE );           // �� DMA �����
	DMA2_Channel6->MADDR = (uint32_t)data;      // ���ͻ�����Ϊ data
	DMA_SetCurrDataCounter(DMA2_Channel6,num);  // ���û���������
	DMA_Cmd(DMA2_Channel6, ENABLE);             // �� DMA
	return SET;
}

/*******************************************************************************
* Function Name  :  uartWriteWiFiStr
* Description    :  send string to ESP8266 via UART6	�� WiFi ģ�鷢���ַ���
* Input          :  char * str          string to send
* Return         :  RESET                UART busy,failed to send	����ʧ��
*                   SET                  send success				���ͳɹ�
*******************************************************************************/
FlagStatus uartWriteWiFiStr(char * str)
{
    uint16_t num = 0;
    while(str[num])num++;           // �����ַ�������
    return uartWriteWiFi(str,num);
}


/*******************************************************************************
* Function Name  :  uartReadWiFireceive
* Description    :  read some bytes from receive buffer �ӽ��ջ���������һ������
* Input          :  char * buffer        buffer to storage the data	������Ŷ������ݵĵ�ַ
*                   uint16_t num         number of data to read		Ҫ�����ֽ���
* Return         :  int                  number of bytes read		����ʵ�ʶ������ֽ���
*******************************************************************************/
uint16_t rxBufferReadPos = 0;       //���ջ�������ָ��
uint32_t uartReadWiFi(char * buffer , uint16_t num)
{
    uint16_t rxBufferEnd = RXBUF_SIZE - DMA_GetCurrDataCounter(DMA2_Channel7); //���� DMA ����β��λ��
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
            // ����������������
            rxBufferReadPos = 0;
        }
    }
    return i;
}

/*******************************************************************************
* Function Name  :  uartReadByteWiFi
* Description    :  read one byte from UART buffer	�ӽ��ջ��������� 1 �ֽ�����
* Input          :  None
* Return         :  char    read data				���ض���������(������Ҳ����0)
*******************************************************************************/
char uartReadByteWiFi()
{
    char ret;
    uint16_t rxBufferEnd = RXBUF_SIZE - DMA_GetCurrDataCounter(DMA2_Channel7);
    if (rxBufferReadPos == rxBufferEnd){
        // �����ݣ�����
        return 0;
    }
    ret = RxBuffer[rxBufferReadPos];
    rxBufferReadPos++;
    if(rxBufferReadPos >= RXBUF_SIZE){
        // ����������������
        rxBufferReadPos = 0;
    }
    return ret;
}
/*******************************************************************************
* Function Name  :  uartAvailableWiFi
* Description    :  get number of bytes Available to read from the UART buffer	��ȡ�������пɶ����ݵ�����
* Input          :  None
* Return         :  uint16_t    number of bytes Available to read				���ؿɶ���������
*******************************************************************************/
uint16_t uartAvailableWiFi()
{
    uint16_t rxBufferEnd = RXBUF_SIZE - DMA_GetCurrDataCounter(DMA2_Channel7);//���� DMA ����β��λ��
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
	printf("SystemClk:%d\r\n",SystemCoreClock);

	printf("8266 WiFi TEST\r\n");

	DMA_INIT();
	USARTx_CFG();                                                 /* USART INIT */
	USART_DMACmd(UART6,USART_DMAReq_Tx|USART_DMAReq_Rx,ENABLE);


    Delay_Ms(1000);
    // ��ѯ WiFi ģ���Ƿ���������
    uartWriteWiFi("AT\r\n",4);
    Delay_Ms(100);
    // ��ѯ WiFi ģ��汾��Ϣ
    while(uartWriteWiFi("AT+GMR\r\n",8)==RESET);
    Delay_Ms(100);
    // ��ѯģ��֧�ֵ� AT ���һЩģ�鲻֧�ִ�ָ��
    while(uartWriteWiFi("AT+CMD\r\n",8)==RESET);
    Delay_Ms(100);
    // ��Ϊ Station ģʽ
    while(uartWriteWiFiStr("AT+CWMODE=1\r\n")==RESET);
    Delay_Ms(100);
    // ����һ����Ϊ SSID������Ϊ PASSWORD �� WiFi ���磬
    while(uartWriteWiFiStr("AT+CWJAP=\"SSID\",\"PASSWORD\"\r\n")==RESET);
    Delay_Ms(100);

    //��ӡ֮ǰ�յ�����Ϣ
    int num = uartAvailableWiFi();
    if (num > 0 ){
        char buffer[1024]={"\0"};
        uartReadWiFi(buffer , num);
        printf("Revceived:\r\n%s",buffer);
    }

    //�ȴ������ظ�
    while(uartAvailableWiFi()==0);
    Delay_Ms(2000);
    num = uartAvailableWiFi();
    if (num > 0 ){
        char buffer[1024]={"\0"};
        uartReadWiFi(buffer , num);
        printf("Revceived:\r\n%s",buffer);
    }
    Delay_Ms(5000);
    //���ӷ�����
    while(uartWriteWiFiStr("AT+CIPSTART=\"TCP\",\"192.168.137.1\",80\r\n")==RESET);
    Delay_Ms(100);
    while(uartWriteWiFiStr("AT+CIPMODE=1\r\n")==RESET);
    Delay_Ms(100);
    while(uartWriteWiFiStr("AT+CIPSEND\r\n")==RESET);
    Delay_Ms(100);
    num = uartAvailableWiFi();
    if (num > 0 ){
        char buffer[1024]={"\0"};
        uartReadWiFi(buffer , num);
        printf("Revceived:\r\n%s",buffer);
    }

    while(1){}

}

