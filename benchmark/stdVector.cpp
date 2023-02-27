#include <benchmark/benchmark.h>
#include <vector>

std::vector<std::vector<std::string>> vectorOfVector()
{
    std::vector<std::vector<std::string>> myVector{};
    std::vector<std::string> myElem{};
    myVector.push_back(myElem);
    return myVector;
}

static void emptyVectorCreation(benchmark::State& state)
{
    // Code inside this loop is measured repeatedly
    for (auto _ : state)
    {
        auto var = vectorOfVector();
        // Make sure the variable is not optimized away by compiler
        benchmark::DoNotOptimize(var);
    }
}
// Register the function as a benchmark
BENCHMARK(emptyVectorCreation);

int main(int argc, char** argv)
{
    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
}