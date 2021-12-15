/*******************************************************
This program was created by the
CodeWizardAVR V3.12 Advanced
Automatic Program Generator
© Copyright 1998-2014 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : PHM-1
Version : 
Date    : 10.07.2021
Author  : 
Company : 
Comments: 


Chip type               : ATtiny13
AVR Core Clock frequency: 9,600000 MHz
Memory model            : Tiny
External RAM size       : 0
Data Stack size         : 16
*******************************************************/

#include <tiny13.h>

#include <delay.h>

#define DEBUG 0
#define SETUP 0

#define SOUND PORTB.2 // PORTB.0 in design
#define METER PORTB.1
#define POT 2 // ADC PORTB.4
#define AC 3 // ADC PORTB.3

#if DEBUG
#define BAUND (1000000/4800-2)
#define UART PORTB.1 // .0
#define CR uart_byte(13)
#define MINUS uart_byte('-')

void uart_byte(char Byte) {
  //#asm("wdr")
  //#asm("cli")
  UART=0; delay_us(BAUND);
  UART=Byte&0b00000001; Byte=Byte>>1; delay_us(BAUND);
  UART=Byte&0b00000001; Byte=Byte>>1; delay_us(BAUND);
  UART=Byte&0b00000001; Byte=Byte>>1; delay_us(BAUND);
  UART=Byte&0b00000001; Byte=Byte>>1; delay_us(BAUND);
  UART=Byte&0b00000001; Byte=Byte>>1; delay_us(BAUND);
  UART=Byte&0b00000001; Byte=Byte>>1; delay_us(BAUND);
  UART=Byte&0b00000001; Byte=Byte>>1; delay_us(BAUND);
  UART=Byte&0b00000001; delay_us(BAUND);
  UART=1; delay_us(BAUND);
  //#asm("sei")
}

void uart_dec_i(unsigned int code)
{
  unsigned char a,b,c,d,e;
  a=code/10000; uart_byte(a+48);  
  b=(code/1000)%10; uart_byte(b+48);
  //uart_byte('.');
  c=(code/100)%10; uart_byte(c+48);
  d=(code/10)%10; uart_byte(d+48);
  e=(code)%10; uart_byte(e+48);
} 

void uart_dec_c(unsigned char code)
{
  unsigned char a,b,c;
  a=code/100; uart_byte(a+'0');
  b=(code/10)%10; uart_byte(b+'0');
  c=code%10; uart_byte(c+'0');
} 

void uart_hex_c(unsigned char C)
{
  unsigned char a;
  a=C>>4;  if (a>9) a+=7; uart_byte(a+48);  
  a=C&15;  if (a>9) a+=7; uart_byte(a+48);
} 

void uart_hex_i(unsigned int C)
{
  unsigned char a;
  a=(C>>12)&15;  if (a>9) a+=7; uart_byte(a+48);  
  a=(C>>8)&15;   if (a>9) a+=7; uart_byte(a+48);
  a=(C>>4)&15;   if (a>9) a+=7; uart_byte(a+48);  
  a=C&15;        if (a>9) a+=7; uart_byte(a+48);
}

#endif //DEBUG

// Declare your global variables here
unsigned int sound_it, sound_set;
unsigned char meter_it, meter_set, meter_max, sound_on;
#define METER_GOLD_MAX 220

// Timer 0 output compare A interrupt service routine
interrupt [TIM0_COMPA] void timer0_compa_isr(void)
{
#if SETUP==0
  meter_it++; 
  if(meter_it >= METER_GOLD_MAX) {
    meter_it = 0;
  }    
  if(meter_it < meter_set) {
    METER = 1;
  } else {
    METER = 0;
  }

  if (sound_on) {
    sound_it++;
    if(sound_it >= sound_set) {
      sound_it = 0;
      SOUND ^= 1;   
    }
  } else {
    SOUND=0;
  }                 
#endif
}

#define GOLD_VOLTAGE_MEM 2
void EEPROM_write(unsigned char addr, unsigned char data)
{
  while(EECR & (1<<EEPE));
  EEARL = addr;
  EEDR = data;
  EECR |= (1<<EEMPE);
  EECR |= (1<<EEPE);
}
unsigned char EEPROM_read(unsigned char addr)
{
  while(EECR & (1<<EEPE));
  EEAR = addr;
  EECR |= (1<<EERE);
  return EEDR;
}

// Bandgap Voltage Reference: On
#define ADC_VREF_TYPE ((1<<REFS0) | (0<<ADLAR))

#define REF_INTERNAL 1
#define REF_EXTERNAL 0
// Read the AD conversion result
unsigned int read_adc(unsigned char adc_input, unsigned char ref)
{
  if(ref) {
    ADMUX=adc_input | ADC_VREF_TYPE;  
  } else {
    ADMUX = adc_input;
  }                                 
  
  // Delay needed for the stabilization of the ADC input voltage
  delay_us(10);
  // Start the AD conversion
  ADCSRA|=(1<<ADSC);
  // Wait for the AD conversion to complete
  while ((ADCSRA & (1<<ADIF))==0);
  ADCSRA|=(1<<ADIF);
  return ADCW;
}

unsigned char _rand(void) {
  unsigned char rnd = 0, i;
  for(i = 0; i < 8; i++) { 
    rnd <<=1;
    rnd |= (read_adc(POT, REF_INTERNAL) & 1);
  }                       

  return rnd;
}

inline unsigned char rand(void) {
#if DEBUG == 2
  unsigned char r = (_rand() ^ _rand()) ^ _rand();
  uart_byte(r);                          
  return r;
#else
  return (_rand() ^ _rand()) ^ _rand(); 
#endif
}

inline void setup(void) {
  // Declare your local variables here

  // Crystal Oscillator division factor: 1
  #pragma optsize-
  CLKPR=(1<<CLKPCE);
  CLKPR=(0<<CLKPCE) | (0<<CLKPS3) | (0<<CLKPS2) | (0<<CLKPS1) | (0<<CLKPS0);
  #ifdef _OPTIMIZE_SIZE_
  #pragma optsize+
  #endif

  // Input/Output Ports initialization
  // Port B initialization
  // Function: Bit5=In Bit4=In Bit3=Out Bit2=Out Bit1=Out Bit0=Out 
  DDRB=(0<<DDB5) | (0<<DDB4) | (0<<DDB3) | (1<<DDB2) | (1<<DDB1) | (1<<DDB0);
  // State: Bit5=P Bit4=T Bit3=0 Bit2=T Bit1=0 Bit0=0 
  PORTB=(1<<PORTB5) | (0<<PORTB4) | (0<<PORTB3) | (0<<PORTB2) | (0<<PORTB1) | (1<<PORTB0);

  // Timer/Counter 0 initialization
  // Clock source: System Clock
  // Clock value: 9600,000 kHz
  // Mode: CTC top=OCR0A
  // OC0A output: Disconnected
  // OC0B output: Disconnected
  // Timer Period: 10 us
  TCCR0A=(0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (1<<WGM01) | (0<<WGM00);
  TCCR0B=(0<<WGM02) | (0<<CS02) | (0<<CS01) | (1<<CS00);
  TCNT0=0x00;
  OCR0B=0x00;
  OCR0A=(0x60/1)-1; // 0x5F;

  // Timer/Counter 0 Interrupt(s) initialization
  TIMSK0=(0<<OCIE0B) | (1<<OCIE0A) | (0<<TOIE0);

  // External Interrupt(s) initialization
  // INT0: Off
  // Interrupt on any change on pins PCINT0-5: Off
  GIMSK=(0<<INT0) | (0<<PCIE);
  MCUCR=(0<<ISC01) | (0<<ISC00);

  // Analog Comparator initialization
  // Analog Comparator: Off
  // The Analog Comparator's positive input is connected to the AIN0 pin
  // The Analog Comparator's negative input is connected to the AIN1 pin
  // Digital input buffer on AIN0: On
  // Digital input buffer on AIN1: On
  DIDR0=(0<<AIN0D) | (0<<AIN1D);
  ACSR=(1<<ACD) | (0<<ACBG) | (0<<ACO) | (0<<ACI) | (0<<ACIE) | (0<<ACIS1) | (0<<ACIS0);

  // ADC initialization
  // ADC Clock frequency: 150,000 kHz
  // ADC Bandgap Voltage Reference: On
  // ADC Auto Trigger Source: Free Running
  // Digital input buffers on ADC0: On, ADC1: On, ADC2: On, ADC3: On
  DIDR0|=(0<<ADC0D) | (0<<ADC2D) | (0<<ADC3D) | (0<<ADC1D);
  ADCSRB=(0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0);
  ADMUX=ADC_VREF_TYPE;
  ADCSRA=(1<<ADEN) | (0<<ADSC) | (1<<ADATE) | (0<<ADIF) | (0<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (0<<ADPS0);

}

void main(void)
{ 
  unsigned char i, m;
  unsigned int vgold, d;
  unsigned long vac, mm;  
  
  #asm("cli");
  setup();
  sound_on=0;

#if SETUP  ////////////////////////////

  delay_ms(1000);
  
  d = 0;                 
  for(i = 0; i < 64; i++) {
    d += read_adc(AC, REF_INTERNAL);
  }
  //m = d >> 8;       
  #if DEBUG
   CR; delay_ms(5);
   CR; delay_ms(5);
   CR; delay_ms(5);
  #endif // DEBUG
  if (PINB.4==0) {
    EEPROM_write(GOLD_VOLTAGE_MEM, d);
    EEPROM_write(GOLD_VOLTAGE_MEM + 1, d >> 8);  
    #if DEBUG
     CR; uart_byte('w'); uart_hex_i(d); CR; 
    #endif // DEBUG
  }         
  

 #if DEBUG
  delay_ms(1000);
  /*vgold = EEPROM_read(GOLD_VOLTAGE_MEM + 1);
  vgold <<= 8;    
  vgold |= EEPROM_read(GOLD_VOLTAGE_MEM); 
  uart_byte('r'); uart_hex_i(vgold); CR; 
  uart_hex_i(d); CR;
  */
  CR;
  for (i=0; i<64; i++) {
    if ( (i%16)==0 ) CR;
    m=EEPROM_read(i);
    uart_hex_c(m); uart_byte(' ');
  }
  CR;  
 #endif // DEBUG        
  
/*  m = m * 2 / 5;
 #if DEBUG == 10
  uart_dec_c(m); CR; CR;           
  while(1) {   
    uart_byte('V');
    uart_byte(' ');
    uart_byte('=');
    uart_byte(' ');
    #asm("cli");
    d = 0;                 
    for(i = 0; i < 64; i++) {
      d += read_adc(AC, REF_INTERNAL);
    }
    //i = (d >> 8);
    uart_dec_i(d);
    //uart_dec_c(i / m);
    //uart_byte('.');
    //uart_byte('0' + (((i * 10) / m) % 10));
    CR;  
    #asm("cli");
    
    delay_ms(500);
  }    
 #endif // DEBUG 
*/
  while(1) {
    METER = 1;
    delay_ms(5000);
    METER = 0;
    delay_ms(2000);
  }
  
#else // SETUP  //////////////////////////

  sound_set = 2500;
  #if DEBUG
   CR; delay_ms(5);
   CR; delay_ms(5);
   CR; delay_ms(5);
  #endif
  vac = 0; // замер напряжения батареи                
  for(i = 0; i < 64; i++) {
    vac += read_adc(AC, REF_INTERNAL);
  }       
  #if DEBUG
   uart_hex_i(vac); CR;
  #endif

  //vgold = 30707; //TODO: read from eeprom  
  vgold = EEPROM_read(GOLD_VOLTAGE_MEM + 1);
  vgold <<= 8;    
  vgold |= EEPROM_read(GOLD_VOLTAGE_MEM); 
  #if DEBUG
   uart_hex_i(vgold); CR; delay_ms(1000);
  #endif
  
  #asm("sei");
  sound_on=1;
  // индикация разряженной батареи
  #if DEBUG==0
  if (vgold>vac) {
    while (1) {
      sound_set = 62; delay_ms(200);
      sound_set = 125; delay_ms(200);
    }
  }  
  #endif // DEBUG==0
           
//-  vac *= 10000;
//-  vac /= vgold;
                      
  //uart_dec_i(vac); CR;
  
  // расчет максимального значения ШИМ для текущего напряжения питания
//-  meter_max = ((unsigned long)METER_GOLD_MAX *10000 / vac);
  mm = vgold; 
  mm *= METER_GOLD_MAX;
  mm /= vac;
  meter_max = mm;
  //uart_dec_i(meter_max); CR;
           
  //meter_max = (unsigned long)METER_GOLD_MAX * a
  //meter_max /= b; 
     
  // Global enable interrupts
  while(1) {
    sound_on=1;  
#if DEBUG==0
    d = 0;                 
    for(i = 0; i < 64; i++) {
      d += read_adc(POT, REF_EXTERNAL);
    }  
    d >>= 8;
    m = d;       
#else // DEBUG==0
    while(1);
#endif // DEBUG==0
    
#if DEBUG==0
    if(m < 35) {
      // random
      for(i = 0; i < 50; i++) {
        m = rand();          
        if(m > METER_GOLD_MAX) {
          m = METER_GOLD_MAX;
        }
        meter_set = (unsigned int)m * meter_max / METER_GOLD_MAX;
        sound_set = 1200 - m * 5;
        delay_ms(50);
      }
      //uart_dec_c(m); CR;
    } else {
#endif // DEBUG==0
      // fixed         
      m -= 35;
#if DEBUG
      uart_hex_i(m); MINUS;
      uart_hex_i(meter_max); MINUS;
      i=m;
#else // DEBUG
      for(i = 0; i < m; i++) {
#endif // DEBUG
        meter_set = (unsigned int)i * meter_max / METER_GOLD_MAX;
#if DEBUG
        uart_hex_i(meter_set); CR;
#else // DEBUG
        sound_set = 172 - (i / 2); //1082
        delay_ms(2);
      }          
    }
    delay_ms(4000);
    sound_on=0;
#endif // DEBUG
    delay_ms(1000); 
  }    

#endif // SETUP
}
