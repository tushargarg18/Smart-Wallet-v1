#include "stm32f10x.h"
void uart1_init(void);
void gpio_init(void);
void lcd_init(void);
void lcd_commnad_enable_to_write(void);
void lcd_command(char);
void lcd_data_enable_to_write(void);
void lcd_data(char);


#define grocries 14
#define clothing 15
#define dairy 16
#define electricity 17
#define internet 18
#define transport 19
#define others 20


char group1[12] = { 0x00, 0xAA, 0x09, 0x30, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x0A};
char group2[12] = { 0x00, 0xAA, 0x09, 0x30, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x0A};

uint8_t buffer[10] = {0};// buffer to store response of voice module
unsigned int char_count = 0, rec_done = 0, digit_count = 5, amount = 0, state = 0 ,send = 0; // number of bytes recived counter, receive complete flag, no. of digit commands recived counter
//initialise uart1 init for transmit and recieve
void uart1_init()
{
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN; // clock to usart1 enable and i/o port a enable
	
	//PIN CONFIGURATION : pA9- TX IS SET TO ALTERNATEPUSH PULL AND 50MHZ
	GPIOA->CRH |= GPIO_CRH_MODE9 | GPIO_CRH_CNF9_1;
	GPIOA->CRH &= ~(GPIO_CRH_CNF9_0);
	
	//PIN 10 IS THE RX - INPUT FLOATING (THIS IS DEFAULT VALUE)
	
	USART1->BRR = 0X1D4F; //FOR 72 MHZ ON APB1 BUS FOR 9600 BAUDRATE
	
	//---------- RECEIVE ENABLE| TRANSMIT ENABLE| USART ENABLE
	USART1->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_UE;
	
	//enable rxne and txe interrupt
	USART1->CR1 |= USART_CR1_RXNEIE;
	//USART1->CR1 |= USART_CR1_RXNEIE | USART_CR1_TXEIE;
	//NTERRUOT FOR USART FROM NVIC SIDE
	NVIC_EnableIRQ(USART1_IRQn);
}
	
void gpio_init()
{
	//initialise the clock for port c
	RCC->APB2ENR |= (1<<4);	
	
	//configuration pin3 of c port
	GPIOC->CRH |= (  (1<<20) | (1<<21)  ); //output mode maximum speed 50mhz
	GPIOC->CRH &= ~((1<<22) | (1<<23)); //general purpose output
	
	//gpioc15 as input floating type
	GPIOC->CRH &= ~(  (1<<30) | (1<<31)  );
	GPIOC->CRH |=  (1<<28);
	GPIOC->CRH &=  ~(1<<29);
}


void lcd_init()
{
	//..pin0..pin1..pin2..pin4..pin5..pin6..pin7--output
	GPIOA->CRL = 0x22220222; //output mode maximum speed 2mhz--general purpose output
	lcd_command(0x28);
	lcd_command(0x02);
	lcd_command(0x0E);
	lcd_command(0x80);
}

void lcd_commnad_enable_to_write()
{
 GPIOA->BSRR &= ~(1<<0);
 GPIOA->BSRR |= (1<<(0+16)); //RS = 0
 
 GPIOA->BSRR &= ~(1<<1);
	GPIOA->BSRR |= (1<<(1+16)); //RW=0
 //GPIOA->BSRR &= ~(1<<1);
 GPIOA->BSRR |= (1<<(2)); //EN1
 GPIOA->BSRR &= ~(1<<2);
	GPIOA->BSRR |= (1<<(2+16)); //EN0
 //GPIOA->BSRR &= ~(1<<2);
 for(int q = 0; q<5000; q++);
}

void lcd_data_enable_to_write()
{
 GPIOA->BSRR |= (1<<0); //RS = 0
 
 GPIOA->BSRR &= ~(1<<1);
 GPIOA->BSRR |= (1<<(1+16)); //RW=0

 GPIOA->BSRR |= (1<<(2)); //EN1
 GPIOA->BSRR &= ~(1<<2);
 GPIOA->BSRR |= (1<<(2+16)); //EN0
	
 for(int q = 0; q<5000; q++);
}

void lcd_command(char x)
{
	char tempc = x & 0xf0;
	GPIOA->BSRR = 0x00;
	GPIOA->BSRR |= (15<<20);
	GPIOA->BSRR |= (tempc << 4);
	
	//PORTA=(x&0xf0);  //upper 4 bit
	lcd_commnad_enable_to_write();
	//PORTA=((x<<4)&0xf0); //lower 4 bit
	tempc = x & 0x0f;
	GPIOA->BSRR |= (tempc << 8); 
	lcd_commnad_enable_to_write();
}

void lcd_data(char x)
{
	char tempc = x & 0xf0;
	//GPIOA->BSRR = 0x00;
	GPIOA->BSRR |= (15<<20);
	GPIOA->BSRR |= (tempc << 4);
	
	//PORTA=(x&0xf0);  //upper 4 bit
	lcd_data_enable_to_write();
	//PORTA=((x<<4)&0xf0); //lower 4 bit
	tempc = x & 0x0f;
	GPIOA->BSRR |= (tempc << 8); 
	lcd_data_enable_to_write();
}
int	main()
{
	uart1_init();
	gpio_init();
	lcd_init();
	while(1)
	{
		lcd_command(0x80);
		
		lcd_data('a');
		for(int q=0; q<5000000; q++);
	}
	
	/*while(1)
	{
		amount = 0;
		GPIOC->BSRR |=(1<<13); //set pin 13 high
		
		
		//initialisation button
		if(GPIOC->IDR & (1<<15))
			{
				state = 1;
				GPIOC->BSRR = (1<<(13+16)); //set pin13 low
				int i = 0;
				while(i<=11)
				{				
					USART1->DR = group1[i++];
					while(!(USART1->SR & USART_SR_TC));
				}
				for(int j =0; j<1000000; j++);
				GPIOC->BSRR =(1<<13); //set pin 13 high
			}
		
			
		//state 1 coding	
		while(state == 1)
		{
			while(!(USART1->SR & USART_SR_TC));
			int temp = 0;
			if(rec_done == 1)
				{
					rec_done = 0; // clear the flag
					temp = buffer[5];
					switch(digit_count)
					{
						case 1: 
							{
								GPIOC->BSRR = (1<<(13+16)); //set pin13 low
								amount = amount + temp;	
								int div = 1000;
								int temp_amt = amount;
								for(int q= 0; q<= 10 ; q++)
								{
									USART1->DR = buffer[q];
									while(!(USART1->SR & USART_SR_TC));
								}
								while(div)
								{
									int temp_num = temp_amt /div;
									temp_amt = temp_amt - temp_num *div;
									char transmit = 48 + temp_num;
									USART1->DR = transmit;
									while(!(USART1->SR & USART_SR_TC));
									div = div/10;
								}
								state = 2;
								break;
							}
						case 2:
							{
								GPIOC->BSRR = (1<<(13+16)); //set pin13 low
								amount = amount + temp *10;
								break;
							}
						case 3: 
							{
								GPIOC->BSRR = (1<<(13+16)); //set pin13 low
								amount = amount + temp *100;
								break;
							}
						case 4: 
							{
								GPIOC->BSRR = (1<<(13+16)); //set pin13 low
								amount = amount + temp *1000;
								break;
							}
							default: break;
						}
						GPIOC->BSRR =(1<<13); //set pin 13 high
					}
				}

				
				
		//state 2 coding			
		while(state == 2)
		{
				if(GPIOC->IDR & (1<<15))
				{
					GPIOC->BSRR = (1<<(13+16)); //set pin13 low
					int i = 1;
					while(i<=11)
					{				
						USART1->DR = group2[i++];
						while(!(USART1->SR & USART_SR_TC));
					}
					for(int j =0; j<1000000; j++);
					GPIOC->BSRR =(1<<13); //set pin 13 high
				}
			if(rec_done == 1)
			{
				rec_done = 0;
				unsigned int rec_cmd = 0;
				rec_cmd = buffer[5];
				char asci_cmd =  (char) (rec_cmd + 66);
				USART1->DR = asci_cmd;
				while(!(USART1->SR & USART_SR_TC));
			}	
		}
	}
	*/
}

void USART1_IRQHandler()
{
	if(USART1->SR & USART_SR_RXNE)
		{
			buffer[char_count++] = USART1->DR;	
		}
	if(char_count == 9) //receive complete for state 1
		{
			//if(state == 1)
			//{			
				char_count = 0;
				--digit_count;
				rec_done = 1; 	// setting flag
				if(digit_count == 0) 
					{	
						digit_count = 5;
					}
				return;
			//}	
			//if(state == 2)
			//{
				//char_count = 0;
				//rec_done = 1;
				//return;
			//}
		}
	}
