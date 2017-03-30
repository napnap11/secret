#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "papi.h"

#define NUM_EVENTS 2
#define THRESHOLD 1000000
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }

void Sort(int* array,int n)
{
  int c,d,t;
  for (c=1;c<=n-1;c++){
    d=c;
    while(d>0 && array[d]<array[d-1]){
      t = array[d];
      array[d] = array[d-1];
      array[d-1] = t;
      d--;
    }
  }
  printf("Sorted list in ascending order:\n");
  for(c=0;c<=n-1;c++){
    printf("%d\n",array[c]);
  }
}
void ReadFile(){
  FILE * pFile;
  long lSize;
  int * buffer;
  size_t result;

  pFile = fopen ( "data1d-100000" , "r" );
  if (pFile==NULL) {fputs ("File error",stderr); exit (1);}

  // obtain file size:
  fseek (pFile , 0 , SEEK_END);
  lSize = ftell (pFile);
  rewind (pFile);

  // allocate memory to contain the whole file:
  buffer = (int*) malloc (sizeof(char)*lSize);
  if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}

  // copy the file into the buffer:
  result = fread (buffer,4,lSize/4,pFile);
  if (result != lSize/4) {fputs ("Reading error",stderr); exit (3);}

  /* the whole file is now loaded in the memory buffer. */
  Sort(buffer,lSize/4);
  // terminate
  fclose (pFile);
  free (buffer);
}
int main(){/*Declaring and initializing the event set with the presets*/
    int Events[2] = {PAPI_TOT_INS, PAPI_TOT_CYC};
    /*The length of the events array should be no longer than the
      value returned by PAPI_num_counters.*/

    /*declaring place holder for no of hardware counters */
    int num_hwcntrs = 0;
    int retval, retval2;
    char errstring[PAPI_MAX_STR_LEN];
    /*This is going to store our list of results*/
    long long values[NUM_EVENTS];

    float real_time, proc_time,mflops;
    long long flpops;
    float ireal_time, iproc_time, imflops;
    long long iflpops;
    /***************************************************************************
     *  This part initializes the library and compares the version number of the*
     * header file, to the version of the library, if these don't match then it *
     * is likely that PAPI won't work correctly.If there is an error, retval    *
     * keeps track of the version number.                                       *
     ***************************************************************************/

    if((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT )
    {
        fprintf(stderr, "Error: %d %s\n",retval, errstring);
        exit(1);
    }

    /**************************************************************************
     * PAPI_num_counters returns the number of hardware counters the platform *
     * has or a negative number if there is an error                          *
     **************************************************************************/
    if ((num_hwcntrs = PAPI_num_counters()) < PAPI_OK)
    {
        printf("There are no counters available. \n");
        exit(1);
    }

    printf("There are %d counters in this system\n",num_hwcntrs);

    /**************************************************************************
     * PAPI_start_counters initializes the PAPI library (if necessary) and    *
     * starts counting the events named in the events array. This function    *
     * implicitly stops and initializes any counters running as a result of   *
     * a previous call to PAPI_start_counters.                                *
     **************************************************************************/

    if ( (retval = PAPI_start_counters(Events, NUM_EVENTS)) != PAPI_OK)
        ERROR_RETURN(retval);

    printf("\nCounter Started: \n");


    /**********************************************************************
     * PAPI_read_counters reads the counter values into values array      *
     **********************************************************************/

    if ( (retval=PAPI_read_counters(values, NUM_EVENTS)) != PAPI_OK)
        ERROR_RETURN(retval);

    printf("Read successfully\n");

    ReadFile();

    /******************* PAPI_stop_counters **********************************/
    if ((retval=PAPI_stop_counters(values, NUM_EVENTS)) != PAPI_OK)
        ERROR_RETURN(retval);

    printf("The total instruction executed for multiplication are %lld \n",
            values[0] );
    printf("The total cycles used are %lld \n", values[1] );

    printf("CPI = %lf \n", (double) values[1]/values[0] );

    /**** we compute flops ***/
    if((retval2=PAPI_flops(&ireal_time,&iproc_time,&iflpops,&imflops)) < PAPI_OK)
    {
        printf("Could not initialise PAPI_flops \n");
        printf("Your platform may not support floating point operation event.\n");
        printf("retval: %d\n", retval2);
        exit(1);
    }
    ReadFile();
    if((retval2=PAPI_flops(&real_time,&proc_time,&flpops,&mflops)) < PAPI_OK)
    {
        printf("retval: %d\n", retval2);
        exit(1);
    }

    printf("Real_time %f Proc_time: %f Total flpops: %lld MFLOPS: %f\n",
            real_time, proc_time,flpops,mflops);
    long long mips = values[0] / 1000000.0 / real_time;
    printf("MIPS: %lld\n", mips);

    const PAPI_hw_info_t *hwinfo = NULL;

    if((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT )
    {
        printf("Library initialization error! \n");
        exit(1);
    }

    /* Get hardware info*/
    if ((hwinfo = PAPI_get_hardware_info()) == NULL)
    {
        printf("PAPI_get_hardware_info error! \n");
        exit(1);
    }
    /* when there is an error, PAPI_get_hardware_info returns NULL */


    printf("%d CPU  at %f Mhz.\n",hwinfo->totalcpus,hwinfo->mhz);
    printf(" model string is %s \n", hwinfo->model_string);
    PAPI_shutdown();

    exit(0);
  return 0;
}
