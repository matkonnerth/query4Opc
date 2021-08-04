#pragma once

template <typename T> class Sink {
public:
  void emplace_back(T &&r) { res.emplace_back(r); }

  const auto &results() const { return res; }
  auto &results() { return res; }

private:
  std::vector<T> res;
};
