/*
 * Send a UDP packet to server.c.
 */

 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <netdb.h>
 #include <stdio.h>
 #include <sys/mman.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <stdlib.h>
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


 // the below code uses a device called /dev/mem to get a pointer to a physical
// address.  We will use this pointer to read/write the custom peripheral
volatile unsigned int * get_a_pointer(unsigned int phys_addr)
{

        int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
        void *map_base = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, phys_addr);
        volatile unsigned int *radio_base = (volatile unsigned int *)map_base;
        return (radio_base);
}
 
 main(int argc, char *argv[])
 {
   int sock;
   struct sockaddr_in name;
   struct hostent *hp, *gethostbyname();
 
   if(argc != 2){
     fprintf(stderr, "Usage: targetIP\n");
     exit(1);
   }
 
   sock = socket(AF_INET, SOCK_DGRAM, 0);
   if(sock < 0){
     perror("socket");
     exit(1);
   }
 
   hp = gethostbyname(argv[1]);
   if(hp == 0){
     fprintf(stderr, "%s: unknown host\n", argv[1]);
     exit(1);
   }
   int numPack = argv[2];
   bcopy(hp->h_addr, &name.sin_addr, hp->h_length);
   name.sin_family = AF_INET;
   name.sin_port = htons(25344);
 
   // volatile unsigned int *my_periph = get_a_pointer(RADIO_PERIPH_ADDRESS);

   // *(my_periph+RADIO_TUNER_CONTROL_REG_OFFSET) = 0; // make sure radio isn't in reset

   //  radioTuner_tuneRadio(my_periph,30e6);
   // radioTuner_setAdcFreq(my_periph,31e6);

   int DUMMY[257];

   volatile unsigned int *my_fifo = get_a_pointer(FIFO_ADDRESS);
   *(my_fifo+FIFO_RESET_OFFSET) = 0xa5;
   *(my_fifo+FIFO_RESET_OFFSET) = 0x0;

   int i = 0;
   int num = 0;
   while (1) {
        for (int j = 0; j<256; j++) {
                int empty = 1;
                do{
			num = *(my_fifo+FIFO_OCC_OFFSET);
			if (num > 0) empty = 0;
                } while (empty == 1);
                int limit = *(my_fifo+FIFO_OCC_OFFSET);
                DUMMY[j+1] = *(my_fifo+FIFO_READ_DATA_OFFSET);
                int num = *(my_fifo+FIFO_READ_LENGTH_OFFSET);
        }
        
         DUMMY[0] = i++;
         if(sendto(sock, DUMMY, sizeof(DUMMY), 0, &name, sizeof(name)) < 0)
           perror("sendto");
   }
   exit(0);

 }
