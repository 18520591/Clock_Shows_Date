#include <REGX52.H>

#define Set_key P3_4
#define Up_key	P3_5
#define Dw_key	P3_6
#define SDA P1_1	
#define SCL P1_0	
#define DS1307_ID 0xD0	
#define SEC 0x00
#define	MIN 0x01	
#define HOUR 0x02
#define DAY 0x04
#define MONTH 0x05
#define YEAR 0x06

/************KHAI BAO BIEN, MANG...******************************************* ********************/
unsigned char Array[10] = {0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90 }; 
unsigned char t_view, time, index, F_set, B_nhay, get_t ;
unsigned char day, month;
char year;
unsigned char HYear = 20;

/************KHAI BAO HAM, CHUONG TRINH CON*********************************************** ********/
void set_main(void);	 // Chuong trinh khoi tao main
void setup_timer(void);	 // Chuong trinh cai dat timer 1 va 0 
void delay(unsigned int time); // Delay
void Fix_time(void);	 // Kiem tra va hieu chinh gia tri cua gio,phut,giay
void Keypad(void);	 // Kiem tra xu ly phim nhan.	
void T1_ISR(void);	 //ngat timer 1 phuc vu nhay led
void T0_ISR(void); // Dung ngat timer 0 de quet led

/************CAC CHUONG TRINH CON*********************************************** ******************/
//I2C
/************************************************** *********************/

void I2C_start(void)
{
	SCL	= 1;	
	SCL = 0;
	SDA = 1;	
	SCL = 1;
	delay(2);
	SDA = 0; 
	delay(2);
	SCL = 0; 
	delay(2);
}

void I2C_stop(void)
{
	SCL = 1;	
	SCL = 0;
	SDA = 0;	
	delay(2);
	SCL = 1;	
	delay(2);
	SDA = 1;	
}

bit I2C_write(unsigned char dat)
{
	unsigned char i;	
	for( i=0;i<8; i++)
	{
		SDA = (dat & 0x80) ? 1:0;
		SCL = 1;
		SCL = 0;
		dat <<= 1;
	}
	SCL = 1;	
	delay(2);
	SCL = 0;
}

unsigned char I2C_read(void)
{
	bit rd_bit;	
	unsigned char i, dat;
	dat = 0x00;	
	for( i=0; i<8; i++)	 /* vong lap doc 1 byte du lieu */
	{
	delay(2);
	SCL = 1;	 /* dat SCL */
	delay(2); 
	rd_bit = SDA;	 /* giu de kiem tra xac nhan	*/
	dat = dat << 1;	
	dat = dat | rd_bit;	/* giu bit du lieu trong dat */
	SCL = 0;	 /* xoa SCL */
	}
return dat;
}
/************************************************** *********************/
//DS1307
/************************************************** *********************/

unsigned char DS1307_read(unsigned char addr) 
{
	unsigned int temp, ret;	
	I2C_start(); /* chay i2c bus */
	I2C_write(DS1307_ID); /* ket noi DS1307 */
	I2C_write(addr);	 /* yeu cau dia chi ram tren DS1307 */	
	I2C_start();	 /* chay i2c bus */
	I2C_write(DS1307_ID+1);	/* ket noi DS1307 de doc */
	ret = I2C_read();	 /* nhan du lieu */
	I2C_stop();
//*********************************************
	temp = ret;	 /*chuyen tu BCD -> HEX*/
	ret = (((ret/16)*10)+ (temp & 0x0f));	 /*cho Led 7seg*/
//*********************************************	
	return ret;	
}

void DS1307_Write(unsigned char addr,unsigned char dat)
{
	unsigned int temp;
//**********************************************	 /*HEX sang BCD*/
	temp = dat ;	 /*for Led 7seg*/
	dat = (((dat/10)*16)|(temp %10));
//**********************************************	
	I2C_start(); /* chay i2c bus */
	I2C_write(DS1307_ID); /* ket noi DS1307*/
	I2C_write(addr);	 /* yeu cau dia chi RAM tren DS1307 */	
	I2C_write(dat);	/* ket noi DS1307 de doc */
	I2C_stop();
}
/************************************************** *********************/

void delay(unsigned int time)
{	
	while(time--); 
}

void set_main(void)	 // Chuong trinh khoi tao main
{
	P1 = 0xFF;
	P0 = 0x00;
	P2 = 0x00;
	P3 = 0xFF;
}

void setup_timer(void)	// cai timer 0 va timer 1 
{
	TMOD = 0x11; // timer0 & timer1 set che do 1
	TH0 = 0xfc; 
	TL0 = 0x18;
	TH1 = 0x3c; 
	TL1 = 0xb0;
	ET1 = 1; 
	ET0 = 1;	
	EA = 1;
	TF0 = 0;
	TF1 = 0;	
	TR0 = 1;	
	TR1 = 1;
}
void Fix_time(void)	 // Kiem tra va hieu chinh gia tri cua gio,phut,giay
{
//Tang
	if( year == 100)
	{
	 	HYear++;
		year = 0;
	}
	if( month == 13)
	{
	 	month = 1;
		year++;
	}
	if((month == 1) || (month == 3) || (month == 5) || (month == 7) || (month == 8) || (month == 10))
	{
		if(day == 32)
		{
			day = 1;
			month++;
		}
	}
	else if ((month == 4) || (month == 6) || (month == 9) || (month == 11))
	{
	 	if(day == 31)
		{
			day = 1;
			month++;
		}	
	}
	else if (month == 12)
	{
		if(day == 32)
		{
			month = 1;
			day = 1;
			year++;
		}
	}
	else
	{
		if(((HYear*100+year)%400 == 0) || (((HYear*100+year)%4 == 0) && ((HYear*100+year)%100 !=0 ))) //kiem tra nam nhuan
		{
	 		if(day == 30)
			{
		 		day = 1;
				month++;
			}
		}
		else
		{
		 	if(day == 29)
			{
			 	day = 1;
				month++;
			}
		}
	}
//Giam
	if(year < 0)
	{
		HYear--;
		year = 99;
	}
	if(month == 0)
	{
		month = 12;
		year--;
	}
	if((month == 12) || (month == 10) || (month == 7) || (month == 5))
	{
	 	if(day == 0)
		{
			day = 30;
			month--;
		}
	}
	else if((month == 11) || (month == 9) || (month == 8) || (month == 6) || (month == 4) || (month == 2))
	{
	 	if(day == 0)
		{
		 	day = 31;
			month--;
		}
	}
	else if(month == 1)
	{
	 	if(day == 0)
		{
		 	day = 31;
			month = 12;
			year --;
		}
	}
	else 
	{
		if (((HYear*100+year)%400 == 0) || (((HYear*100+year)%4 == 0) && ((HYear*100+year)%100 != 0)))	//kiem tra nam nhuan
		{
			if(day == 0)
			{
		 		day = 29;
				month --;
			}
		}
		else
		{
		 	if(day == 0)
			{
			 	day = 28;
				month--;
			}
		}
	}
}
void Keypad(void)	 // Kiem tra phim nhan.	
{ 
	if(!Set_key)   //phim Set duoc nhan 
	{	 
		F_set++;	 // Bien F_set co gia tri tu 1->3
		if( F_set == 4) 
		{
			F_set = 0;
			DS1307_Write(0x06, year);
			DS1307_Write(0x05, month);
			DS1307_Write(0x04, day); 
		}
	}
	if( F_set == 1)
	{	 //Tang hoac giam ngay neu F_set = 1
		if(!Up_key) 
			day++;
		if(!Dw_key)	
			day--;
	}
	if( F_set == 2)
	{	 //Tang hoac giam thang neu F_set = 2
		if(!Up_key) 
			month++;
		if(!Dw_key) 
			month--;
	}
	if( F_set == 3)
	{	 //Tang hoac giam nam neu F_set = 2
		if(!Up_key) 
			year++;
		if(!Dw_key) 
			year--;
	}
	Fix_time();	 //kiem tra tran so 
	delay(20000);
}
void T1_ISR(void) interrupt 3 //ngat timer 1 chay dong ho
{ 
	TR1 = 0;
	TF1 = 0;
	TH1 = 0x3c; // nap lai gia tri cho thanh ghi 
	TL1 = 0xb0;
	time++;	
	if( time == 10)
	{
		time = 0; 
		B_nhay++; 
		get_t = 1;
		if( B_nhay == 2)
			B_nhay = 0;
	}
	TR1 = 1;
}	
/*************************************/
void T0_ISR(void) interrupt 1	 // Dung ngat timer 0 de quet led
{
	TR0 = 0;
	TF0 = 0;
	TH0 = 0xfc;//-1000/256; // Nap lai gia tri cho thanh ghi 
	TL0 = 0x18;//-1000%256;
	index++;
	if( index == 1)	
	{	
		if( F_set == 3 && B_nhay == 1)
		{
			index++;	
			index++;
			index++;
			index++;
		}
		else
		{ 
//year
			t_view = year; 
			P2 = 0xff;
			P0 = 0x80;	//10000000
			P2 = Array[t_view%10];	
		}
	}

	if( index == 2) 
	{ 
		P2 = 0xff;
		P0 = 0x40; //01000000
		P2 = Array[t_view/10]; 
	}

	if( index == 3)
	{
		P2 = 0xff;
		P0 = 0x20; //00100000
		P2 = Array[HYear%10];
	}
	if( index == 4)
	{
		P2 = 0xff;
		P0 = 0x10; //00010000
		P2 = Array[HYear/10];
	}
//month
	if( index == 5)
	{
		if( F_set == 2 && B_nhay == 1)
		{
			index++;	
			index++;
		}
		else
		{	
			t_view = month;
			P2 = 0xff;
			P0 = 0x08;//00001000
			P2 = Array[t_view%10];	
		}
	}

	if( index == 6)
	{
		P2 = 0xff;
		P0 = 0x04;//00000100
		P2 = Array[t_view/10];	
	} 
//day
	if( index == 7) 
	{
		if( F_set == 1 && B_nhay == 1)
			index=0;
		else
		{
			t_view = day;
			P2 = 0xff;
			P0 = 0x02;//00000010
			P2 = Array[t_view%10];
		} 
	}

	if( index == 8)
	{ 
		P2 = 0xff;
		P0 = 0x01;//00000001
		P2 = Array[t_view/10]; 
		index = 0;	 
	} 
	TR0 = 1;	
}

/************CHUONG TRINH CHINH********************************************* **********************/
void main()
{
	set_main();
	time = index = 0;
	day = 0;
	month = 0;
	year = 1;
	F_set = 0;
	get_t = 0;
	setup_timer();
	while(1)
	{ 
		Keypad();

		if( F_set == 0)
		{ 
			if( get_t == 1)
			{	
				get_t = 0;
				year = DS1307_read(YEAR);
				month = DS1307_read(MONTH);		
				day = DS1307_read(DAY);	
			}
		}
	}
}
