#include "stdafx.h"

int reduce(int argc, char *argv[])
{
  int size, rank;

  MPI_Init(&argc, &argv);

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  const int itemsPerProcess = 10;
  const int count = size * itemsPerProcess;
  int *data = new int[count];

  if (rank == 0)
  {
    for (size_t i = 0; i < count; i++)
    {
      data[i] = rand() % 10;
    }
  }

  int *localData = new int[itemsPerProcess];
  MPI_Scatter(data, itemsPerProcess, MPI_INT,
    localData, itemsPerProcess, MPI_INT, 0, MPI_COMM_WORLD);

  int localSum = 0;
  for (size_t i = 0; i < itemsPerProcess; i++)
  {
    localSum += localData[i];
  }

  delete[] localData;

  int globalSum;
  MPI_Reduce(&localSum, &globalSum, 1, MPI_INT, MPI_SUM, 0,
    MPI_COMM_WORLD);

  if (rank == 0)
  {
    cout << "Total sum = " << globalSum << endl;
    delete[] data;
  }

  MPI_Finalize();


  return 0;
}

int broadcast(int argc, char* argv[])
{
  int size, rank;

  MPI_Init(&argc, &argv);

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int value = 42;
  MPI_Bcast(&value, 1, MPI_INT, 0, MPI_COMM_WORLD);

  cout << "Rank " << rank << " received from 0 the value " << value
    << endl;

  MPI_Barrier(MPI_COMM_WORLD);

  cout << "Rank " << rank << " is done working" << endl;

  MPI_Finalize();

  return 0;
}

int main(int argc, char* argv[])
{
  random_device rd{};
  srand(rd());
  //return broadcast(argc, argv);
  return reduce(argc, argv);
}