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

void play_tune(volatile unsigned int *ptrToRadio, float base_frequency)
{
	int i;
	float freqs[16] = {1760.0,1567.98,1396.91, 1318.51, 1174.66, 1318.51, 1396.91, 1567.98, 1760.0, 0, 1760.0, 0, 1760.0, 1975.53, 2093.0,0};
	float durations[16] = {1,1,1,1,1,1,1,1,.5,0.0001,.5,0.0001,1,1,2,0.0001};
	for (i=0;i<16;i++)
	{
		radioTuner_setAdcFreq(ptrToRadio,freqs[i]+base_frequency);
		usleep((int)(durations[i]*500000));
	}
}


void print_benchmark(volatile unsigned int *periph_base)
{
    // the below code does a little benchmark, reading from the peripheral a bunch 
    // of times, and seeing how many clocks it takes.  You can use this information
    // to get an idea of how fast you can generally read from an axi-lite slave device
    unsigned int start_time;
    unsigned int stop_time;
    start_time = *(periph_base+RADIO_TUNER_TIMER_REG_OFFSET);
    for (int i=0;i<2048;i++)
        stop_time = *(periph_base+RADIO_TUNER_TIMER_REG_OFFSET);
    printf("Elapsed time in clocks = %u\n",stop_time-start_time);
    float throughput=0; 
    // please insert your code here for calculate the actual throughput in Mbytes/second
    // how much data was transferred? How long did it take?
    unsigned int bytes_transferred = 2048; // change obviously
    float time_spent = (stop_time-start_time)/125000000.0f; // change obviously
    throughput = (bytes_transferred/1000000.0f)/time_spent;
    printf("You transferred %u bytes of data in %f seconds\n",bytes_transferred,time_spent);
    printf("Measured Transfer throughput = %f Mbytes/sec\n",throughput);
}

int main()
{

// first, get a pointer to the peripheral base address using /dev/mem and the function mmap
    volatile unsigned int *my_periph = get_a_pointer(RADIO_PERIPH_ADDRESS);	

    printf("\r\n\r\n\r\nLab 6 Kevin Liao - Custom Peripheral Demonstration\n\r");
    *(my_periph+RADIO_TUNER_CONTROL_REG_OFFSET) = 8; // make sure radio isn't in reset
    printf("Tuning Radio to 30MHz\n\r");
    radioTuner_tuneRadio(my_periph,30e6);
    printf("Playing Tune at near 30MHz\r\n");
    play_tune(my_periph,30e6);
    print_benchmark(my_periph);

    volatile unsigned int *my_fifo = get_a_pointer(FIFO_ADDRESS);    

    *(my_fifo+FIFO_RESET_OFFSET) = 0xa5;
    *(my_fifo+FIFO_RESET_OFFSET) = 0x0;

    printf("starting reading 480000 samples\n");
    for(int i = 0; i < 480000; i++){
	int empty = 1;
	do{
	    if (*(my_fifo+FIFO_OCC_OFFSET) != 0) empty = 0;
        } while (empty == 1);
	int limit = *(my_fifo+FIFO_OCC_OFFSET);
	int data = *(my_fifo+FIFO_READ_DATA_OFFSET);
        int num = *(my_fifo+FIFO_READ_LENGTH_OFFSET);
	
    }

    printf("finished reading 480000 samples\n");



    
    return 0;


}

