#define AVR		1
//#define STM32	1

#define DIO_ENCODE_CHANNEL(port, pin)     ( ((port & 0x0F) << 4) | (pin & 0x0F) )
#define DIO_GET_PORT(channelId)           ( ((channelId) >> 4) & 0x0F )
#define DIO_GET_PIN(channelId)            ( (channelId) & 0x0F )

#define WRITE_HIGH(reg, pin)	do{ reg |= (1<<pin); }while(0)

#define CHANNEL_DIR_INPUT  0x00
#define CHANNEL_DIR_OUTPUT 0x01
#define CHANNEL_LEVEL_LOW  0x00
#define CHANNEL_LEVEL_HIGH 0x01

#define PORT_A	0
#define PORT_B	1
#define PORT_C	2
#define PORT_D	3

//PLATFORM
#if defined(AVR)
	#define F_CPU	16000000UL
	#include <stdint.h>
	#include <util/delay.h>
	
	typedef uint8_t RegType;
	
	#define GPIO_NUMBERS 4
	#define ADDR_PORT_A	(RegType*)0x20
	#define ADDR_PORT_B	(RegType*)0x23
	#define ADDR_PORT_C	(RegType*)0x26
	#define ADDR_PORT_D	(RegType*)0x29
	
	typedef struct s_ChannelInitTypeDef{
		uint8_t id;
		uint8_t direction;
	}ChannelInitTypeDef;
	
	RegType* regBaseAddrs[GPIO_NUMBERS] = { ADDR_PORT_A, ADDR_PORT_B, ADDR_PORT_C, ADDR_PORT_D };
		
	#define CHANNEL_SIZE 2
	#define RX_LED DIO_ENCODE_CHANNEL(PORT_D, 0)
	#define TX_LED DIO_ENCODE_CHANNEL(PORT_D, 1)
	
#elif defined(STM32) //to test 32bit environment
	typedef struct{
		volatile uint8_t _res;
		volatile uint8_t pin;
		volatile uint8_t ddr;
		volatile uint8_t port;
	}GpioTypeDef;
	
	typedef GpioTypeDef RegType;
	
	#define GPIO_NUMBERS 3
	#define ADDR_PORT_A	(RegType*)0x00000020
	#define ADDR_PORT_B	(RegType*)0x00000024
	#define ADDR_PORT_C	(RegType*)0x00000028
	
	RegType* regBaseAddrs[GPIO_NUMBERS] = { ADDR_PORT_A, ADDR_PORT_B, ADDR_PORT_C };
		
	#define CHANNEL_SIZE 2
	#define RX_LED DIO_ENCODE_CHANNEL(PORT_C, 0)
	#define TX_LED DIO_ENCODE_CHANNEL(PORT_C, 1)
#endif 


ChannelInitTypeDef channels[CHANNEL_SIZE] = {
	{RX_LED, CHANNEL_DIR_OUTPUT},
	{TX_LED, CHANNEL_DIR_OUTPUT}
};

#if defined(AVR)
	#define DIR_REG_OFFSET	1
	#define PORT_REG_OFFSET	2
#elif defined(STM32)
	#define DIR_REG_OFFSET	2
	#define PORT_REG_OFFSET	3
#endif 

#if defined(AVR)

void InitChannels(ChannelInitTypeDef* arr, uint8_t size){
	uint8_t i;
	for(i=0; i<size; i++){
		uint8_t pin = DIO_GET_PIN(arr[i].id);
		uint8_t port_id = DIO_GET_PORT(arr[i].id);
		
		volatile RegType* dirReg = (volatile RegType*) (regBaseAddrs[port_id] + DIR_REG_OFFSET);
		
		if(arr[i].direction) 
			*dirReg |= (1<<pin);
		else 
			*dirReg &= ~(1<<pin);		
	}
}

static inline void channel_write(uint8_t id, uint8_t level){
	uint8_t pin = DIO_GET_PIN(id);
	uint8_t port_id = DIO_GET_PORT(id);
	volatile RegType* portReg = (volatile RegType*) (regBaseAddrs[port_id] + PORT_REG_OFFSET);
	
	if(level)
		*portReg |= (1<<pin);
	else
		*portReg &= ~(1<<pin);

}

#define DELAY	500

#elif defined(STM32)

void InitChannels(ChannelInitTypeDef* arr, uint8_t size){
	uint8_t i;
	for(i=0; i<size; i++){
		uint8_t pin = DIO_GET_PIN(arr[i].id);
		uint8_t port_id = DIO_GET_PORT(arr[i].id);
		
		if(arr[i].direction)
		regBaseAddrs[port_id]->ddr |= (1<<pin);
		else
		regBaseAddrs[port_id]->ddr &= ~(1<<pin);
	}
}

static inline void channel_write(uint8_t id, uint8_t level){
	uint8_t pin = DIO_GET_PIN(id);
	uint8_t port_id = DIO_GET_PORT(id);
	RegType* portReg = regBaseAddrs[port_id] + PORT_REG_OFFSET;
	
	if(level)
	regBaseAddrs[port_id]->port |= (1<<pin);
	else
	regBaseAddrs[port_id]->port &= ~(1<<pin);
}

#define DELAY	1000

#endif


void mcu_delay_ms(uint16_t _delay){
	while(_delay--){
		_delay_ms(1);
	}
}


int main(void){
	InitChannels(channels, CHANNEL_SIZE);

	while(1){
		channel_write(RX_LED, CHANNEL_LEVEL_HIGH);
		channel_write(TX_LED, CHANNEL_LEVEL_LOW);
		mcu_delay_ms(DELAY);
		
		channel_write(RX_LED, CHANNEL_LEVEL_LOW);
		channel_write(TX_LED, CHANNEL_LEVEL_HIGH);
		mcu_delay_ms(DELAY);
	
	}
	
}

