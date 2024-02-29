// CONFIG
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)



#define _XTAL_FREQ 20000000
#define I2C_clock 100000
#define I2C_SSPADD_value(I2C_clock) (_XTAL_FREQ/(4*I2C_clock))-1


#include <xc.h>

void i2c_send(char address, char pointer);
void i2c_receive(char address, char *data, int data_size);
        
int main(void)
{
    char MSandLSbyte[2];
    //configuratuion
    INTCONbits.GIE=0; //disable all interruptions
    
    TRISCbits.TRISC2=0; //0-as output for an oscilloscope
    TRISCbits.TRISC3=1; //1-as input and as SCL
    TRISCbits.TRISC4=1; //as SDA
    
    SSPSTAT=0;
    SSPSTATbits.SMP=1; // Slew rate control disabled
    
    SSPCON &= 0b11111000; // clear last three bits
    SSPCONbits.SSPM3=1;   // for this: 0bxxxx1000 Master transmit i2c mode
    SSPCONbits.SSPEN=1;   //1  = Enables the serial port and configures the SDA a nd SCL pins as the source of the serial port pins
    
    SSPCON2=0;
    SSPCON2bits.ACKDT=0;  //0-ack will be transmitted
    
    SSPADD=I2C_SSPADD_value(I2C_clock);
    
    while(1)
    {
        
        //rise edge
        PORTCbits.RC2=0;
        PORTCbits.RC2=1;
    
        i2c_send(0x90, 0x00);
        i2c_receive(0x90, MSandLSbyte, sizeof(MSandLSbyte));
        
        //send a stop condition
        SSPCON2bits.PEN=1;
        while(SSPCON2bits.PEN){;} //wait
        PIR1bits.SSPIF=0;
        
        //fall edge
        PORTCbits.RC2=1;
        PORTCbits.RC2=0;
    
    }

    return 0;
}



        void i2c_send(char address, char pointer)
        {
            //write mode
            SSPCON2bits.RCEN=0;
            
            //send start condition
            SSPCON2bits.SEN=1;
            while(SSPCON2bits.SEN){;} //wait
            
        //send an address and a write bit
            PIR1bits.SSPIF=0;
            SSPBUF=address;
            
            //wait
            while(!PIR1bits.SSPIF){;}
            PIR1bits.SSPIF=0;
            
            //check an ack from a slave
            while(SSPCON2bits.ACKSTAT){;}
            
        //send a pointer 
            SSPBUF=pointer;
            
            //wait
            while(!PIR1bits.SSPIF){;}
            PIR1bits.SSPIF=0;
            
            //check an ack from a slave
            while(SSPCON2bits.ACKSTAT){;}      
        
        }
        
        void i2c_receive(char address, char *data, int data_size)
        {
            address |= 0b00000001; //0x91
            
            //write mode
            SSPCON2bits.RCEN=0;
            
            //send a re-start condition
            SSPCON2bits.RSEN=1;
            while(SSPCON2bits.RSEN){;} //wait
            
        //send an address and a read bit
            PIR1bits.SSPIF=0;
            SSPBUF=address;
            
            //wait
            while(!PIR1bits.SSPIF){;}
            PIR1bits.SSPIF=0;
            
            //check an ack from a slave
            while(SSPCON2bits.ACKSTAT){;}

            
            //read mode
        //read MSbyte
            SSPCON2bits.RCEN=1;
            
            //wait
            while(!PIR1bits.SSPIF){;}
            PIR1bits.SSPIF=0;
            
            //send an ack
            SSPCON2bits.ACKEN=1; //send an ACKDT for a slave
            
            //wait
            while(!PIR1bits.SSPIF){;}
            PIR1bits.SSPIF=0;
            
            //save MSbyte
            *(data+0)=SSPBUF;
            
        //read LSbyte
            SSPCON2bits.RCEN=1;
            
            //wait
            while(!PIR1bits.SSPIF){;}
            PIR1bits.SSPIF=0;
            
            //send an ack
            SSPCON2bits.ACKEN=1; //send an ACKDT for a slave
            
            //wait
            while(!PIR1bits.SSPIF){;}
            PIR1bits.SSPIF=0;
            
            //save MSbyte
            *(data+1)=SSPBUF; 
        
        
        }