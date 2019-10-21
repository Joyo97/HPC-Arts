#include <iostream>
#include <cstdio>
#include <chrono>
#include <vector>
using namespace std;

#include <omp.h>

void hello_openmp()
{
  omp_set_num_threads(8);

#pragma omp parallel
  {
#pragma omp critical
    cout << "Hello, OpenMP " << omp_get_thread_num()
      << "/" << omp_get_num_threads() << endl;
  }
}

void pfor()
{
  const int length = 1024 * 1024 * 64;
  float *a = new float[length],
    *b = new float[length],
    *c = new float[length],
    *result = new float[length];

#pragma omp parallel for
  for (int i = 0; i < length; i++)
  {
    result[i] = a[i] + b[i] * erff(c[i]);
  }

  delete[] a;
  delete[] b;
  delete[] c;
  delete[] result;
}

void sections()
{
#pragma omp sections
  {
#pragma omp section
    {
      for (int i = 0; i < 1000; i++)
      {
        cout << i;
      }
    }
#pragma omp section
    {
      for (int i = 0; i < 1000; i++)
      {
        cout << static_cast<char>('a' + (i % 26));
      }
    }
  }
}

void single_master()
{
#pragma omp parallel
  {
#pragma omp single nowait
    {
      int n;
      cin >> n;
      printf("gathering input: %d\n", omp_get_thread_num());
    }

    printf("in parallel on %d\n", omp_get_thread_num());

#pragma omp barrier

#pragma omp master
    printf("output on: %d\n", omp_get_thread_num());
  }
}

void sync()
{
  printf("\nATOMIC\n");

  int sum = 0;
#pragma omp parallel for num_threads(128)
  for (int i = 0; i < 100; i++)
  {
#pragma omp atomic
    ++sum;
  }

  cout << sum;

  printf("\nORDERED\n");

  vector<int> squares;
#pragma omp parallel for ordered
  for (int i = 0; i < 20; ++i)
  {
    printf("%d : %d\t", omp_get_thread_num(), i);
    int j = i*i;

#pragma omp ordered
    squares.push_back(j);
  }

  printf("\n");
  for (auto v : squares) printf("%d\t", v);
}

void data_sharing()
{
  int i = 10;

#pragma omp parallel for lastprivate(i)
  for (int a = 0; a < 10; a++)
  {
    printf("thread %d i = %d\n", omp_get_thread_num(), i);
    i = 1000 + omp_get_thread_num();
  }

  printf("%d\n", i);
}

int main(int argc, char* argv[])
{
  //hello_openmp();
  //pfor();
  //sections();
  //single_master();
  //sync();

  data_sharing();
  

  getchar();
	return 0;
}

