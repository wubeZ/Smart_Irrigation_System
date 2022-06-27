#include  <xc.h>
#include <stdio.h>
#define _XTAL_FREQ 8000000
#pragma config FOSC = INTIO67
 
#define LCD_DATA    PORTD               //LCD data port
#define en          PORTEbits.RE2      // enable signal
#define rs          PORTEbits.RE0     // register selectsignal
#define motor      PORTCbits.RC7     // Put the motor


void ADC_Init(){    
    ADCON1 = 0x00;
    // ADFM right; ACQT 2; ADCS FOSC/2; 
    ADCON2 = 0x88;
    // ADRESL 0; 
    ADRESL = 0x00;
    // ADRESH 0; 
    ADRESH = 0x00;
    // GO_nDONE stop; ADON enabled; CHS AN0; 
    ADCON0 = 0x01;
}

int ADC_Read(int channel){
    int digital;
    ADCON0 =(ADCON0 & 0b11000011)|((channel<<2) & 0b00111100);

    /*channel 0 is selected i.e.(CHS3CHS2CHS1CHS0=0000)& ADC is disabled*/
    ADCON0 |= ((1<<ADON)|(1<<GO));/*Enable ADC and start conversion*/

    /*wait for End of conversion i.e. Go/done'=0 conversion completed*/
    while(ADCON0bits.GO_nDONE==1);

    digital = (ADRESH*256) | (ADRESL);/*Combine 8-bit LSB and 2-bit MSB*/
    
    return(digital);
}

void lcd_lat(){
    en = 1;                       
    __delay_ms(10);                     
    en = 0;                      
}

void LCD_data(unsigned char data){//Function to write data to the LCD
    LCD_DATA = data; //Send data on LCD data bus
    rs = 1;    //RS = 1 since data to LCD
    lcd_lat();
}

void LCD_command(unsigned char cmd){ //Function to pass command to the LCD
    LCD_DATA = cmd;  //Send data on LCD data bus
    rs = 0;    //RS = 0 since command to LCD
    lcd_lat();
}

void LCD_write_string(const  char *str){
    while(*str != 0)
    {
        LCD_data(*str++);      // sending data on LCD byte by byte
        __delay_ms(1);
    }
}

void init_LCD(void){  // Function to initialise the LCD
    LCD_command(0x38);              //8-bit mode, 2 lines, 5x7 dots matrix
    LCD_command(0x0E);              //Cursor Blinking
    LCD_command(0x01);              //Clear LCD
    LCD_command(0x06);              //Increment Cursor
    LCD_command(0x80);              //Force cursor to beginning ( 1st line
}


void main( void ){ 
ANSELD = 0;
ANSELE = 0;
ANSELC = 0;

TRISD = 0x00;           //Configuring PORTD as output
TRISE = 0x00;         //Configuring PORTE as output
TRISC = 0x00;         //Configuring PORTC as output
TRISAbits.TRISA0 = 1;   // RA0 is input
TRISCbits.TRISC7 = 0;   // Buzzer control output


char msg1[] = "smart irrigation";
char msg2[] = " Dryness:";
char msg3[] = "MOTOR ON";
char msg4[] = "MOTOR OFF";
 
int result, moisture, ans;
char ADC_Array[5];

unsigned int a, b, c, d, e;
ADC_Init();    // Init ADC peripheral
init_LCD();    // Init LCD Module
 
LCD_write_string(msg1); // Display Message
__delay_ms(50);

while(1){
     
  LCD_command(0x01);      // clear LCD
  LCD_command (0x80);   // Goto first line, 0th place of LCD
  LCD_write_string(msg2); // Display Message
  
  result = ADC_Read(0);  // Get the moisture
  moisture = (float)result/10.23;
  ans = 100 - moisture;
  
  LCD_command(0x89);   //Goto 9th place on first line of LCD

  b = ans/100;
  c = ans%100;
  if(c != 0){
   d = ans/10;
   e = ans%10;
  }
  else{
   d = 0;
   e = 0;
  }
  
  ADC_Array[0]=(int)b+'0';
  ADC_Array[1]=(int)d+'0';
  ADC_Array[2]= (int)e+'0';
  ADC_Array[3] = '%';
  
  LCD_write_string(ADC_Array);
    
  __delay_ms(10);
   
  if ( result < 800) //if soil moisture is less than 300 then make relay i.e. motor ON
  {
   LCD_command (0xC0);   // Goto second line, 0th place of LCD
   LCD_write_string(msg3); // Display Message
   motor = 1;  // Relay ON to increase moisture in soil by motor run
  }
  else
  {
   LCD_command (0xC0);   // Goto second line, 0th place of LCD
   LCD_write_string(msg4); // Display Message
   motor = 0; 
 }
   __delay_ms(30);
 }
 
}
