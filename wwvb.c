// https://github.com/apoluekt/raspberry-wwvb

#include <wiringPi.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

const int ledPin = 14;           // GPIO14, P1 pin 8  - LED is lit when carrier is transmitted at reduced strength
      int txPin  = 18;           // GPIO18, P1 pin 12 - to base of 2N3904 on TX antenna board

int bit;

void signaux(int sigtype) {
   digitalWrite(ledPin,LOW);     // turn off LED
   pwmWrite(txPin,0);            // turn off 60KHz carrier
	 printf (" - WWVB program terminated\n");
	 exit(1);
}

int is_leap_year(int year) {
   return year%4==0 && (year%100 != 0 || year%400==0);
}

// generate marker
void marker() {
  printf("%-4d Marker\n",bit++);
  digitalWrite(ledPin,HIGH);       // LED on  
  pwmWrite(txPin,2); delay(800);   // reduced strength carrier for 800ms
  digitalWrite(ledPin,LOW);        // LED off
  pwmWrite(txPin,80); delay(200);  // full strength carrier for 200ms
}

// generate "One"
void one() {
  printf("%-4d 1\n",bit++);
  digitalWrite(ledPin,HIGH);       // LED on 
  pwmWrite(txPin,2); delay(500);   // reduced strength carrier for 500ms
  digitalWrite(ledPin,LOW);        // LED off
  pwmWrite(txPin,80); delay(500);  // full strength carrier for 500ms
}

// generate "Zero"
void zero() {
  printf("%-4d 0\n",bit++);
  digitalWrite(ledPin,HIGH);       // LED on  
  pwmWrite(txPin,2); delay(200);   // reduced strength carrier for 200ms
  digitalWrite(ledPin,LOW);        // LED off
  pwmWrite(txPin,80); delay(800);  // full strength carrier for 800ms
}

int main (void) {
  int i;
   
  time_t current,tomorrow;
  struct tm *tm_struct;
  int hour;
  int minute;
  int year; 
  int day;
  int dst_today;
  int dst_tomorrow;
  FILE *logFile;
  char date_string[] = "01-01-1970 00:00:00";  

  signal(SIGINT,signaux);
  signal(SIGTERM,signaux);
  
  // Open the log  file in append mode
  logFile = fopen("/home/pi/wwvb/wwvb.log","a");

  // Get current time and format it
  time(&current);
  strftime(date_string,sizeof(date_string),"%m-%d-%Y %H:%M:%S",localtime(&current));

  // Write log message with timestamp
  fprintf(logFile,"[%s] wwvb started\n",date_string);

  // Close the log file
  fclose(logFile);  

  // Initialise wiringPi
  if (wiringPiSetupGpio() == -1) exit(1);  
  pinMode(ledPin,OUTPUT);        // set pin 2 as output
  digitalWrite(ledPin,LOW);      // turn off LED

  // Initialise PWM on pin 18
  pinMode(txPin,PWM_OUTPUT);
  pwmSetMode(PWM_MODE_MS);
  pwmSetClock(2);
  pwmSetRange(160);              // pwm Frequency in Hz = 19.2MHz/pwmClock/pwmRange = 19,200,000/2/160=60,000

  printf("\nWaiting for the start of the next minute...\n");  

  // transmit WWVB sequence 10 times
  for (i=0;i<10;i++) {
    bit=0;
 
    // wait for the next minute to start
    do {
       delay(10);
       current = time(NULL);
       tm_struct = localtime(&current);
    } while(tm_struct->tm_sec != 0);
  
    dst_today = tm_struct->tm_isdst;
    tomorrow = current+86400;
    tm_struct = localtime(&tomorrow);  
    dst_tomorrow = tm_struct->tm_isdst;

    tm_struct = gmtime(&current);  
    minute = tm_struct->tm_min;
    hour = tm_struct->tm_hour;
    day = tm_struct->tm_yday+1;
    year = tm_struct->tm_year-100; 
    
    printf("\n%2.2d:%2.2d UTC\n",hour,minute);

    // Transmit 60 bits of WWVB code
    // See https://en.wikipedia.org/wiki/WWVB

    marker();                                   // 0
    (((minute/10)>>2) & 1) ? one() : zero();    // 1     minute 40
    (((minute/10)>>1) & 1) ? one() : zero();    // 2     minute 20   
    (((minute/10)>>0) & 1) ? one() : zero();    // 3     minute 10
    zero();                                     // 4
    (((minute%10)>>3) & 1) ? one() : zero();    // 5     minute 8
    (((minute%10)>>2) & 1) ? one() : zero();    // 6     minute 3
    (((minute%10)>>1) & 1) ? one() : zero();    // 7     minute 2
    (((minute%10)>>0) & 1) ? one() : zero();    // 8     minute 1
    marker();                                   // 9
    zero();                                     // 10
    zero();                                     // 11
    (((hour/10)>>1) & 1) ? one() : zero();      // 12    hours 20
    (((hour/10)>>0) & 1) ? one() : zero();      // 13    hours 10
    zero();                                     // 14
    (((hour%10)>>3) & 1) ? one() : zero();      // 15    hours 8
    (((hour%10)>>2) & 1) ? one() : zero();      // 16    hours 4
    (((hour%10)>>1) & 1) ? one() : zero();      // 17    hours 2
    (((hour%10)>>0) & 1) ? one() : zero();      // 18    hours 1
    marker();                                   // 19
    zero();                                     // 20
    zero();                                     // 21
    (((day/100)>>1) & 1) ? one() : zero();      // 22    day 200
    (((day/100)>>0) & 1) ? one() : zero();      // 23    day 100
    zero();                                     // 24
    ((((day/10)%10)>>3) & 1) ? one() : zero();  // 25    day 80
    ((((day/10)%10)>>2) & 1) ? one() : zero();  // 26    day 40
    ((((day/10)%10)>>1) & 1) ? one() : zero();  // 27    day 20
    ((((day/10)%10)>>0) & 1) ? one() : zero();  // 28    day 10
    marker();                                   // 29
    (((day%10)>>3) & 1) ? one() : zero();       // 30    day 8
    (((day%10)>>2) & 1) ? one() : zero();       // 31    day 4
    (((day%10)>>1) & 1) ? one() : zero();       // 32    day 2
    (((day%10)>>0) & 1) ? one() : zero();       // 33    day 1
    zero();                                     // 34    
    zero();                                     // 35
    zero();                                     // 36    DUT1 sign   
    one();                                      // 37    DUT1 sign
    zero();                                     // 38    DUT1 sign
    marker();                                   // 39
    zero();                                     // 40    DUT1 0.8
    zero();                                     // 41    DUT1 0.4
    zero();                                     // 42    DUT1 0.2
    zero();                                     // 43    DUT1 0.1
    zero();                                     // 44
    (((year/10)>>3) & 1) ? one() : zero();      // 45    year 80
    (((year/10)>>2) & 1) ? one() : zero();      // 46    year 40
    (((year/10)>>1) & 1) ? one() : zero();      // 47    year 20
    (((year/10)>>0) & 1) ? one() : zero();      // 48    year 10
    marker();                                   // 49
    (((year%10)>>3) & 1) ? one() : zero();      // 50    year 8
    (((year%10)>>2) & 1) ? one() : zero();      // 51    year 4
    (((year%10)>>1) & 1) ? one() : zero();      // 52    year 2
    (((year%10)>>0) & 1) ? one() : zero();      // 53    year 1
    zero();                                     // 54
    is_leap_year(year) ? one() : zero();        // 55    leap year indicator
    zero();                                     // 56    leap second
    (dst_tomorrow == 1) ? one() : zero();       // 57    DST status
    (dst_today == 1) ? one() : zero();          // 58    DST status
    marker();                                   // 59
  }

  digitalWrite(ledPin,LOW);      // turn off LED
  pwmWrite(txPin,0);             // turn off 60KHz carrier
}
