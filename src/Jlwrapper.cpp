#include "jlcxx/jlcxx.hpp"
#include "Alg.hpp"

static void max_indep_set(jlcxx::ArrayRef<int> _src, jlcxx::ArrayRef<int> _dst, jlcxx::ArrayRef<int> _sol)
{
    int nnodes = std::max(*std::max_element(std::begin(_src), std::end(_src)), *std::max_element(std::begin(_dst), std::end(_dst)));
    std::vector<uint32_t> src;
    std::vector<uint32_t> dst;
    for(int i = 0; i < (int) _src.size(); ++ i)
    {
      src.push_back((uint32_t)(_src[i] - 1));
      dst.push_back((uint32_t)(_dst[i] - 1));
    }
    bool check = true;
    Alg alg(src, dst, check);
    alg.run();
    // std::cout << "algorithm run without issue" << std::endl;
    const auto & sol = alg.getSolution();
    for(int i = 0; i < (int)sol.size(); ++ i) _sol[sol[i]] = 1;
    // std::cout << "obtained solution of size " << sol.size() << std::endl;
    return;
}
JLCXX_MODULE define_julia_module(jlcxx::Module& mod)
{
  mod.method("max_indep_set", &max_indep_set);
}