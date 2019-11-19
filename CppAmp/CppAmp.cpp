#include "stdafx.h"
#include "Timer.h"
#include <numeric>
using concurrency::array; // disambiguate with STL

const int length = 3;

accelerator_view setup()
{
  // get all accelerators and list them
  vector<accelerator> all = accelerator::get_all();
  for (accelerator& a : all)
  {
    wcout << a.description;
    cout << " has " << a.dedicated_memory / 1e6 << "GB dedicated memory,";
    if (a.supports_cpu_shared_memory)
      cout << " supports CPU shared memory, "; // can be accessed by both gpu and cpu
    if (a.supports_double_precision)
      cout << " supports double precision, ";

    cout << endl;

    // could set as default
    //bool success = accelerator::set_default(a.device_path);
  }

  // gpu accelerator CPPAMP_DEFAULT_ACCELERATOR
  accelerator gpu(accelerator::default_accelerator);
  // virtual device abstraction, handle to the accelerator
  // qos concept: create a view for each of the threads
  return gpu.default_view;

  // single-threaded emulator
  //accelerator::direct3d_ref

  // fallback cpu multi-core solution that uses SSE
  //accelerator::direct3d_warp

  // cpu accelerator used for staging arrays

  // now show the view parameter
}

void add_in_amp(accelerator_view acc_view)
{
  int ah[] = { 1, 2, 3 };
  int bh[] = { 5, 7, 9 };
  int sumh[length];

  // represents an N-dimensional view over data held in another container
  array_view<const int, 1> ad(length, ah);
  array_view<const int, 1> bd(length, bh);
  array_view<int, 1> sumd(length, sumh);
  sumd.discard_data(); // we don't need this data on the device

  parallel_for_each(acc_view, sumd.extent, [=](index<1> idx) restrict(amp) {
    // cannot do a cout here
    // need to enable GPU debugging for this to work!
    int a = ad[idx];
    int b = bd[idx];
    sumd[idx] = a + b;
    // press F10 and all the values are assigned

    // debug->windows->parallel stacks, parallel watch
  });

  // without this, we get junk
  sumd.synchronize();

  for (size_t i = 0; i < length; i++)
  {
    cout << sumh[i] << "\t";
  }
  cout << endl;
}

void add_in_cpp()
{
  // h = host, d = device
  int ah[] = { 1, 2, 3 };
  int bh[] = { 5, 7, 9 };
  int sumh[length];

  for (size_t i = 0; i < length; i++)
  {
    sumh[i] = ah[i] + bh[i];
  }

  for (size_t i = 0; i < length; i++)
  {
    cout << sumh[i] << "\t";
  }
  cout << endl;

  // now try this in debug
}

//void consume_library()
//{
//  using concurrency::array;
//
//  const int length = 1024*1024;
//  
//  array<float> data(length);
//  array_view<float, 1> view(data);
//  amp_stl_algorithms::iota(begin(view), end(view), 1.0f);
//
//  Timer timer(true);
//  amp_stl_algorithms::iota(begin(view), end(view), 1.0f);
//  auto last = amp_stl_algorithms::remove_if(begin(view), end(view),
//    [=](const float& v) restrict(amp) { return int(v) % 2 == 1; });
//  // interesting note: sinf will not work below (restrict!), need <amp_math>
//  /*amp_stl_algorithms::transform(begin(view), end(view), begin(view), [=](float f) restrict(amp)
//  {
//    return precise_math::sin(f);
//  });*/
//  float total = amp_stl_algorithms::reduce(begin(view), last, 0.0f);
//  auto elapsed = timer.Elapsed().length();
//  cout << setprecision(0) << fixed << total << " on GPU in " << elapsed << "msec."<< endl;
//
//  vector<float> v(length);
//  timer.Reset();
//  std::iota(begin(v), end(v), 1.0f);
//  auto l = std::remove_if(begin(v), end(v), [=](const float& z) { return int(z) % 2 == 1; });
//  total = accumulate(begin(v), l, 0.0f, [=](float f1, float f2) { return f1 + f2; });
//  elapsed = timer.Elapsed().length();
//  cout << setprecision(0) << fixed << total << " on CPU in " << elapsed << "msec." << endl;
//}

string print_matrix(const float* mtx, const int dim)
{
  ostringstream oss;
  oss << "\n";
  for (int i = 0; i < dim; ++i)
  {
    oss << "[";
    for (int j = 0; j < dim; ++j)
    {
      oss << mtx[i*dim + j];
      if (j + 1 < dim)
        oss << ",\t";
    }
    oss << "\t]\n";
  }
  return oss.str();
}

void native_multiply(float* a, float* b, float* c, const int dim)
{
  array_view<float, 2> av(dim, dim, a);
  array_view<float, 2> bv(dim, dim, b);
  array_view<float, 2> cv(dim, dim, c);
  cv.discard_data();

  parallel_for_each(cv.extent, [=](index<2> idx) restrict(amp)
  {
    auto r = idx[0];
    auto c = idx[1];

    auto sum = 0.f;

    for (int i = 0; i < dim; i++)
    {
      sum += av(r, i)*bv(i, c);
    }
    cv[idx] = sum;
  });
  cv.synchronize();
}

template<int ts>
void tiled_multiply(float* a, float* b, float* c, const int dim)
{
  array_view<float, 2> av(dim, dim, a);
  array_view<float, 2> bv(dim, dim, b);
  array_view<float, 2> cv(dim, dim, c);
  cv.discard_data();

  parallel_for_each(cv.extent.tile<ts,ts>(), 
    [=](tiled_index<ts,ts> idx) restrict(amp)
  {
    tile_static float al[ts][ts];
    tile_static float bl[ts][ts];

    int rl = idx.local[0];
    int cl = idx.local[1];
    int rg = idx.global[0];
    int cg = idx.global[1];

    auto sum = 0.f;

    for (int i = 0; i < dim; i += ts)
    {
      al[rl][cl] = av(rg, cl + i);
      bl[rl][cl] = bv(rl + i, cg);
      idx.barrier.wait();

      for (int j = 0; j < ts; ++j)
      {
        sum += al[rl][j] * bl[j][cl];
      }
      idx.barrier.wait();
    }

    cv[idx.global] = sum;
  });
  cv.synchronize();
}

void matrix_multiplication()
{
  const int dim = 4;
  float a[dim*dim];
  float b[dim*dim];
  float c[dim*dim];

  for (int i = 0; i < dim; i++)
  {
    for (size_t j = 0; j < dim; j++)
    {
      a[i*dim + j] = i*dim + j;
      b[i*dim + j] = j*dim + i;
    }
  }

  //native_multiply(a, b, c, dim);
  tiled_multiply<2>(a, b, c, dim);

  cout << "The product of " << print_matrix(a, dim) << " and " <<
    print_matrix(b, dim) << " is " << print_matrix(c, dim) << endl;
}

int main(int argc, char* argv[])
{
  //auto acc_view = setup();
  //add_in_cpp();
  //add_in_amp(acc_view);
  //consume_library();
  matrix_multiplication();

  getchar();
  return 0;
}

