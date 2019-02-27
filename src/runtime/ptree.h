#ifndef RUNTIME_PTREE_H_
#define RUNTIME_PTREE_H_

#ifndef TREE_SITTER_EXPORT
#define TREE_SITTER_EXPORT __declspec(dllexport)
#endif

#include "tree_sitter/runtime.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PTree {
  TSTree *tree;
  PNode *root_node;
  size_t length;
  const char *str;
};

PTree *ts_ptree_new(TSTree *tree, PNode *root);
TREE_SITTER_EXPORT PTree *ts_ptree_build(TSTree *tree, bool include_all);

#ifdef __cplusplus
}
#endif

#endif  // RUNTIME_PTREE_H_
