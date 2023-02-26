#include <benchmark/benchmark.h>
#include <iostream>
#include <functional>

static int myFreeFunction(int cnt)
{
    return ++cnt;
}



static void freeFunctionCall(benchmark::State& state)
{
    int cnt = 0;


    for (auto _ : state)
    {
        cnt = myFreeFunction(cnt);
    }



    std::cout << "cnt: " << cnt << "\n";
}

static void stdFunctionCall(benchmark::State& state)
{
    int cnt = 0;

    std::function<int(int)> f = myFreeFunction;


    for (auto _ : state)
    {
        cnt = f(cnt);
    }


    std::cout << "cnt: " << cnt << "\n";
}

BENCHMARK(freeFunctionCall);
BENCHMARK(stdFunctionCall);

int main(int argc, char** argv)
{
    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
}