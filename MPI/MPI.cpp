#include "stdafx.h"

int main_(int argc, char* argv[])
{
  int size, rank;

  MPI_Init(&argc, &argv);

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0)
  {
    int value = 42;
    for (size_t i = 1; i < size; i++)
    {
      cout << "Ready to send " << rank << "-->" << i << endl;
      MPI_Ssend(&value, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
      cout << "Data sent     " << rank << "-->" << i << endl;
    }
  }
  else {
    int value = -1;
    //MPI_Recv(&value, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    cout << rank << " received from 0 the value " << value << endl;
  }

  MPI_Finalize();

	return 0;
}

