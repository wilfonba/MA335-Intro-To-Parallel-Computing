#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <time.h>
#include "arr2str.c"

int msg_len=10;
int seed=0;
const double N = 20.0; //generate random doubles between [0,N]

//Produce the standard result serially for comparison
double stddev_serial(int total_procs) {
  double mu, *x, sigma;
  int i,j;
  int len = total_procs*msg_len;

  x = malloc(msg_len*sizeof(double)*total_procs);

  //Truly a serial implementation - fakeout the parallel random data
  for(i=0; i<total_procs; i++) {
    srand(seed+i*1000);
    for (j=0;j<msg_len;j++) {
      x[i*msg_len+j]=((double)rand()/((double)RAND_MAX/N));
    }
    /*printf("Serial %d has message M:%s\n", i,
           dbl_arr2str(x+(i*msg_len),msg_len));*/
  }

  mu = 0.0;
  for(i = 0; i<len; i++) {
    mu += x[i];
  }
  mu/=len;

  sigma = 0.0;
  for(i = 0; i<len; i++) {
    sigma += (x[i]-mu)*(x[i]-mu);
  }

  sigma = (1.0/(len-1.0))*sigma;
  sigma = sqrt(sigma);

  printf("Serial Mean: %lf\n", mu);  
  printf("Serial Std Dev: %lf\n", sigma);

  free(x);
}

int main(int argc, char** argv) {
  int total_procs;
  int rank;
  int i;
  double *M;
  double mu=0.0, sigma=0.0;

  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD,&total_procs);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  srand(seed+rank*1000); //Consistent seed for predictable results

  M = malloc(msg_len*sizeof(double));

  for (i=0;i<msg_len;i++) {
    M[i]=((double)rand()/((double)RAND_MAX/N));
  }

  printf("Rank %d has message M:%s\n",rank,dbl_arr2str(M,msg_len));
  fflush(stdout);
  MPI_Barrier(MPI_COMM_WORLD);

  /* Do calculations here */
  if (rank > 0) {
    double totalSum = 0;
    MPI_Reduce(M,NULL,msg_len,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&mu,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
    //printf("Rank %d receiving mean: %lf \n",rank,mu);
    int jjj;
    double mySigmaSum = 0;
    for (jjj = 0;jjj < msg_len;jjj++) {
      mySigmaSum += pow(M[jjj] - mu,2);
    }
    //printf("Rank %d with mySigmaSum = %lf\n",rank,mySigmaSum);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Reduce(&mySigmaSum,NULL,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
  }
  else {
    double *A = malloc(msg_len*sizeof(double));
    MPI_Reduce(M,A,msg_len,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
    int iii=0;
    double sum = 0;
    for(iii = 0;iii < msg_len;iii++) {
      sum += A[iii];
    }
    mu = sum/(msg_len*total_procs);
    //printf("Rank 0 sending mean %lf \n",mu);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&mu,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
    double mySigmaSum = 0;
    for (iii = 0;iii < msg_len;iii++) {
      mySigmaSum += pow(M[iii] - mu,2);
    }
    double sigmaSum;
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Reduce(&mySigmaSum,&sigmaSum,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
    //printf("Sigma Sum = %lf \n",sigmaSum);
    sigma = sqrt((1.0/(msg_len*total_procs - 1))*sigmaSum);
  }

  fflush(stdout);
  MPI_Barrier(MPI_COMM_WORLD);

  if(rank==0) {
    printf("Mean: %lf\n", mu);
    printf("Std Dev: %lf\n", sigma);
    stddev_serial(total_procs);
  }

  MPI_Finalize();
}
