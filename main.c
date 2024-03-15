
#pragma config PWRTE=OFF
#pragma config WDTE=OFF
#pragma config BOREN=OFF
#pragma config CPD=OFF
#pragma config CP=OFF
#pragma config WRT=OFF
#pragma config LVP=OFF
#pragma config FOSC=HS

#include <xc.h>

#define _XTAL_FREQ 20000000
#define I2C_clock 100000
#define SSPADD_value(I2C_clock) (_XTAL_FREQ/(4*I2C_clock))-1


void i2c_send(unsigned char address, unsigned char pointer);
void i2c_receive(unsigned char address, unsigned char *data);

int main(void)
{
    unsigned char MSandLSbytes[2];
    
    //config
    SSPSTAT=0;
    SSPSTATbits.SMP=1; //disable slew rate control

    INTCONbits.TMR0IE=0; //disable a TMR0 interruptions
    INTCONbits.GIE=0; //disable all interrupts (not TMR0)
    PIE1bits.SSPIE=0; //disable SSP interruptions
    
    TRISCbits.TRISC2=0; //0-output for logic analyzer
    TRISCbits.TRISC3=1; //input for SCL
    TRISCbits.TRISC4=1; //input for SDA

    SSPCON &= 0b11111000; //clear the last three bits
    SSPCONbits.SSPM3=1; //I2C Master Mode    
    SSPCONbits.SSPEN=1; // enable SS port
    
    SSPCON2=0;
    SSPCON2bits.ACKDT=0; //0-ack will be transmitted
    
    SSPADD=SSPADD_value(I2C_clock);

    while(1)
    {
/*rise edge*/
        PORTCbits.RC2=0;
        PORTCbits.RC2=1;
        
        i2c_send(0x90, 0x00);
        i2c_receive(0x90, MSandLSbytes);
        
/*Stop*/        
        SSPCON2bits.PEN=1; //send a stop condition
        while(SSPCON2bits.PEN){;}
        
        //wait
            while(!PIR1bits.SSPIF){;}
            PIR1bits.SSPIF=0;
        
/*fall edge*/
        PORTCbits.RC2=1;
        PORTCbits.RC2=0;
    }
    
    return 0;
}


void i2c_send(unsigned char address, unsigned char pointer)
        {
/*write mode*/        
            SSPCON2bits.RCEN=0; //I2C writing mode
            
/*start and wait and wait*/            
            SSPCON2bits.SEN=1;
            while(SSPCON2bits.SEN){;}
            
            //wait
            while(!PIR1bits.SSPIF){;}
            PIR1bits.SSPIF=0;
            
/*send an address and a write bit and wait*/            
            SSPBUF=0x90; //send an address and a write bit
            
            //wait
            while(!PIR1bits.SSPIF){;}
            PIR1bits.SSPIF=0;
            
/*check an ack*/          
            while(SSPCON2bits.ACKSTAT){;}
            
/*send a temperature pointer and wait*/  
            SSPBUF=0x00; //send a temperature pointer
                        
            //wait
            while(!PIR1bits.SSPIF){;}
            PIR1bits.SSPIF=0;
            
/*check an ack*/            
            while(SSPCON2bits.ACKSTAT){;}        
        }
        

void i2c_receive(unsigned char address, unsigned char *data)
        {
/*write mode*/        
            SSPCON2bits.RCEN=0; //I2C writing mode
            
/*send a re-start condition and wait and wait*/   
            SSPCON2bits.RSEN=1;
            
            //wait
            while(!PIR1bits.SSPIF){;}
            PIR1bits.SSPIF=0;
            
/*prepair an address byte with a reading bit*/    
            address+=1;
            
/*send an address and a reading bit*/  
            SSPBUF=address; //send an address and a read bit
            
            //wait
            while(!PIR1bits.SSPIF){;}
            PIR1bits.SSPIF=0;
            
/*check an ack*/       
            while(SSPCON2bits.ACKSTAT){;}
            
/*start receive and wait and save a MSbyte variable*/  
            SSPCON2bits.RCEN=1; //start receiving
            
            //wait
            while(!PIR1bits.SSPIF){;}
            PIR1bits.SSPIF=0;
            
            /*send an ack*/
            SSPCON2bits.ACKEN=1; //send an ack sequence
            
            //wait
            while(!PIR1bits.SSPIF){;}
            PIR1bits.SSPIF=0;
            
            /*save MSbyte*/
            *data=SSPBUF;
            
/*start receive and wait and save a LSbyte variable*/    
            SSPCON2bits.RCEN=1; //start receiving
            
            //wait
            while(!PIR1bits.SSPIF){;}
            PIR1bits.SSPIF=0;
            
            /*send an ack*/
            SSPCON2bits.ACKEN=1; //send an ack sequence
 
            //wait
            while(!PIR1bits.SSPIF){;}
            PIR1bits.SSPIF=0;            
            
            /*save LSbyte*/
            *(data+1)=SSPBUF;
        }

