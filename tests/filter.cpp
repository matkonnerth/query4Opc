#include "Filter.h"
#include <iostream>

int main() {

  std::vector<UA_ReferenceDescription> refs;
  UA_ReferenceDescription ref;
  ref.typeDefinition = UA_EXPANDEDNODEID_NUMERIC(0, 58);
  ref.nodeId = UA_EXPANDEDNODEID_NUMERIC(1, 10000);
  for (auto i = 0; i < 100; ++i) {
    
    refs.emplace_back(ref);
  }

  Sink<Result> s{};
  UA_ReferenceDescription typeRef{};
  typeRef.nodeId.nodeId = UA_NODEID_NUMERIC(0, 58);
  TypeFilter<Result> typeFilter{std::vector<Result>{Result{UA_NODEID_NUMERIC(0, 58), typeRef}}};
  TypeFilter<Result> typeFilter2{std::vector<Result>{Result{UA_NODEID_NUMERIC(0, 58), typeRef}}};

  TakeAllFilter<Result> ta{};

  typeFilter.connect(&typeFilter2);
  typeFilter2.connect(&ta);
  ta.connect(&s);

  for (const auto &r : refs) {
    typeFilter.filter(Result{UA_NODEID_NUMERIC(0, 85), r});
  }

  std::cout << s.results().size() << "\n";
}