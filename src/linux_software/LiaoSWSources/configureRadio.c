#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#define _BSD_SOURCE

#define RADIO_TUNER_FAKE_ADC_PINC_OFFSET 0
#define RADIO_TUNER_TUNER_PINC_OFFSET 1
#define RADIO_TUNER_CONTROL_REG_OFFSET 2
#define RADIO_TUNER_TIMER_REG_OFFSET 3
#define RADIO_PERIPH_ADDRESS 0x43c00000

#define FIFO_ADDRESS 0x43c10000
#define FIFO_RESET_OFFSET 6
#define FIFO_OCC_OFFSET 7
#define FIFO_READ_DATA_OFFSET 8
#define FIFO_READ_LENGTH_OFFSET 9

// the below code uses a device called /dev/mem to get a pointer to a physical
// address.  We will use this pointer to read/write the custom peripheral
volatile unsigned int * get_a_pointer(unsigned int phys_addr)
{

        int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
        void *map_base = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, phys_addr);
        volatile unsigned int *radio_base = (volatile unsigned int *)map_base;
        return (radio_base);
}


void radioTuner_tuneRadio(volatile unsigned int *ptrToRadio, float tune_frequency)
{
        float pinc = (-1.0*tune_frequency)*(float)(1<<27)/125.0e6;
        *(ptrToRadio+RADIO_TUNER_TUNER_PINC_OFFSET)=(int)pinc;
}

void radioTuner_setAdcFreq(volatile unsigned int* ptrToRadio, float freq)
{
        float pinc = freq*(float)(1<<27)/125.0e6;
        *(ptrToRadio+RADIO_TUNER_FAKE_ADC_PINC_OFFSET) = (int)pinc;
}

void radioTuner_setControlReg(volatile unsigned int* ptrToRadio, int cntrlSet)
{
        *(ptrToRadio+RADIO_TUNER_CONTROL_REG_OFFSET) = (int)cntrlSet;
}

void setTune(int tuneFreq)
{
    volatile unsigned int *my_periph = get_a_pointer(RADIO_PERIPH_ADDRESS);

    //printf("Tuning Radio\n\r");
    radioTuner_tuneRadio(my_periph, tuneFreq);

} 

void setADC(int adcFreq){
        volatile unsigned int *my_periph = get_a_pointer(RADIO_PERIPH_ADDRESS);
        //printf("Setting ADC\r\n");
        radioTuner_setAdcFreq(my_periph, adcFreq);
}

void enableStream(){
        volatile unsigned int *my_periph = get_a_pointer(RADIO_PERIPH_ADDRESS);
        //printf("Enabling UDP stream\r\n");
        radioTuner_setControlReg(my_periph, 0x8);
}

void disableStream(){
        volatile unsigned int *my_periph = get_a_pointer(RADIO_PERIPH_ADDRESS);
        //printf("Disabling UDP stream\r\n");
        radioTuner_setControlReg(my_periph, 0x0);

}
