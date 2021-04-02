#include<stdlib.h>
#include<stdio.h>
#include<mpi.h>


void getInput(int* N)
{
    char input[100];
    printf("Please enter an integer:");
    fflush(stdout);
    fgets(input,100,stdin);
    sscanf(input,"%d",N);
}


int main(int argc, char** argv)
{
    MPI_Init(&argc,&argv);
    int numProcs;
    int myRank;
    MPI_Comm_size(MPI_COMM_WORLD,&numProcs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myRank);
    int N = 1;
    int numb = 1;
    while (N >= 0 && numb >= 0)
    {
        if (myRank == 0)
        {
            getInput(&N);
            int temp;
            MPI_Status status;
            for (int iii = 1;iii < numProcs;iii++)
            {
                MPI_Send(&N,1,MPI_INT,iii,0,MPI_COMM_WORLD);
            }
            for (int iii = 1;iii < numProcs;iii++)
            {
                MPI_Recv(&temp,1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
            }
            fflush(stdout);
        }
        else
        {
            MPI_Status status;
            MPI_Recv(&numb,1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
            printf("My rank is %2d and I received the interger %d\n",myRank,numb);
            MPI_Send(&numb,1,MPI_INT,0,0,MPI_COMM_WORLD);
        }
    }
    MPI_Finalize();
    return 0;
}