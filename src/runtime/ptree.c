#include "runtime/ptree.h"
#include "runtime/language.h"
#include "runtime/subtree.h"
#include "runtime/tree.h"
#include <stdbool.h>

const uint64_t PNODE_START = 0;
const uint64_t PNODE_END = 1;

const uint64_t PNODE_TOKEN_NODE_TYPE = 0;
const uint64_t PNODE_RULE_NODE_TYPE = 1;
const uint64_t PNODE_LIST_NODE_TYPE = 2;
const uint64_t PNODE_ERROR_NODE_TYPE = 3;

const uint64_t PNODE_ERROR_GENERAL = 0;
const uint64_t PNODE_ERROR_MISSING = 1;
const uint64_t PNODE_ERROR_UNEXPECTED = 2;

const uint64_t TT_BAD_TOKEN = 255;

#define PNODE_START_RULE_NODE(node, index, type)                              \
  (node[index] = (PNode)((((type)&0xFF) << 4) | (((PNODE_START)&0x1) << 3) | \
                   (((PNODE_RULE_NODE_TYPE)&0x7))))

#define PNODE_START_RULE_NODE_LENGTH(node, index, length) \
  (node[index] = (PNode)((((((uint64_t)length) & 0xFF) << 12) | (uint64_t)node[index])))

#define PNODE_END_RULE_NODE(node, index, type, length)                     \
  (node[index] = (PNode)(((((uint64_t)length)&0xFF) << 12) | (((type)&0xFF) << 4) | \
                   (((PNODE_END)&0x1) << 3) | (((PNODE_RULE_NODE_TYPE)&0x7))))

#define PNODE_TOKEN_NODE(node, index, input_start_index, type, length)         \
  (node[index] = (PNode)(((((uint64_t)length) & 0xFFFFFF) << 36) |      \
                   ((((uint64_t)input_start_index) & 0xFFFFFF) << 12) | \
                   (((type)&0xFF) << 3) | (((PNODE_TOKEN_NODE_TYPE)&0x7))))

#define PNODE_ERROR(node, index, input_start_index, type, length) \
  (node[index] =                                                       \
     (PNode)(((((uint64_t)length) & 0xFFFFFF) << 36) |                 \
             ((((uint64_t)input_start_index) & 0xFFFFFF) << 12) |      \
             (((type)&0xFF) << 3) | (((PNODE_ERROR_NODE_TYPE)&0x7))))

PTree* ts_ptree_new(TSTree* tree, PNode* root_node, size_t length) {
  PTree* result = ts_malloc(sizeof(PTree));
  result->tree = tree;
  result->root_node = root_node;
  result->length = length;
  result->str = NULL;
  return result;
}

void ts_ptree_delete(PTree* self) {
  ts_free(self->root_node);
  ts_free(self);
}

static void ts_ptree_build__(Subtree self, PNode* root_node, size_t *index, Length start_pos,
                               const TSLanguage* language, bool is_root,
                               bool include_all, TSSymbol alias_symbol,
                               bool alias_is_named, bool pre_count) {
  if (!self.ptr)
    return;

  bool visible = include_all || is_root || ts_subtree_missing(self) ||
                 (ts_subtree_visible(self) && ts_subtree_named(self)) ||
                 alias_is_named;

  uint32_t child_count = ts_subtree_child_count(self);
  size_t start_index = *index;
  bool is_token_node = false;
  bool is_rule_node = false;
  bool is_error_node_unexpected = false;
  bool is_error_node = ts_subtree_is_error(self);
  bool is_error_node_missing = ts_subtree_missing(self);
  TSSymbol symbol = alias_symbol ? alias_symbol : ts_subtree_symbol(self);

  if (symbol == ts_builtin_sym_end)
    return;

  if (visible) {
    is_error_node_unexpected = is_error_node && child_count == 0 && self.ptr->size.bytes > 0;
    if (is_error_node_unexpected) {
      // unexpected error... "(UNEXPECTED ...)"
      // cursor += ts_subtree__write_char_to_string(*writer, limit,
      // self.ptr->lookahead_char);
      is_error_node = true;
    } else if (is_error_node_missing) {
      // missing error... "(MISSING ...)"
      // cursor += snprintf(*writer, limit, "(MISSING");
      is_error_node = true;
    } else if (is_error_node) {
      // some other error node... "(ERROR)"
    } else {
      // we have a symbol to write - success...
      // const char* symbol_name = ts_language_symbol_name(language, symbol);
      // cursor += snprintf(*writer, limit, "(%s", symbol_name);

      // if it's a token symbol... then just write it as such: * no children?
      // otherwise we will write a start / end node
      is_rule_node = true;

      if (child_count == 0) {
        is_token_node = true;
        is_rule_node = false;
      }
    }
  }

  if (!pre_count) {
    if (is_rule_node) {
      PNODE_START_RULE_NODE(root_node, *index, symbol);
      (*index)++;
    } else if (is_token_node) {
      Length size = ts_subtree_size(self);
      Length padding = ts_subtree_padding(self);
      uint32_t start_offset = start_pos.bytes + padding.bytes;
      PNODE_TOKEN_NODE(root_node, *index, start_offset, symbol, size.bytes);
      (*index)++;
    } else if (is_error_node_unexpected) {
      // unexpected error... "(UNEXPECTED X)"
      Length size = ts_subtree_size(self);
      Length padding = ts_subtree_padding(self);
      uint32_t start_offset = start_pos.bytes + padding.bytes;
      uint32_t length = size.bytes;
      if (length == 0) length = 1;
      PNODE_ERROR(root_node, *index, start_offset, PNODE_ERROR_UNEXPECTED, length);
      (*index)++;
    } else if (is_error_node_missing) {
      // missing error... "(MISSING ...)"
      Length size = ts_subtree_size(self);
      Length padding = ts_subtree_padding(self);
      uint32_t start_offset = start_pos.bytes + padding.bytes;
      uint32_t length = size.bytes;
      if (length == 0) length = 1;
      PNODE_ERROR(root_node, *index, start_offset, PNODE_ERROR_MISSING, length);
      (*index)++;
    } else if (is_error_node) {
      // some other error node... "(ERROR)"
      Length size = ts_subtree_size(self);
      Length padding = ts_subtree_padding(self);
      uint32_t start_offset = start_pos.bytes + padding.bytes;
      uint32_t length = size.bytes;
      if (length == 0) length = 1;
      PNODE_ERROR(root_node, *index, start_offset, PNODE_ERROR_GENERAL, length);
      (*index)++;
    }
  } else {
    if (is_rule_node) {
      (*index)++;
    } else if (is_token_node) {
      (*index)++;
    } else if (is_error_node_unexpected) {
      (*index)++;
    } else if (is_error_node_missing) {
      (*index)++;
    } else if (is_error_node) {
      (*index)++;
    }
  }

  // process the children
  if (child_count) {
    const TSSymbol* alias_sequence = ts_language_alias_sequence(language, self.ptr->alias_sequence_id);
    Length pos = start_pos;
    uint32_t child_index = 0;
    for (uint32_t i = 0; i < self.ptr->child_count; i++) {
      Subtree child = self.ptr->children[i];
      if (ts_subtree_extra(child)) {
        // process the children
        ts_ptree_build__(child, root_node, index, pos, language, false, include_all, 0, false, pre_count);
        Length child_size = ts_subtree_size(child);
        Length child_padding = ts_subtree_padding(child);
        pos = length_add(length_add(pos, child_padding), child_size);
      } else {
        TSSymbol alias_symbol = alias_sequence ? alias_sequence[child_index] : 0;
        ts_ptree_build__(child, root_node, index, pos, language, false, include_all, alias_symbol, alias_symbol ? ts_language_symbol_metadata(language, alias_symbol).named : false, pre_count);
        Length child_size = ts_subtree_size(child);
        Length child_padding = ts_subtree_padding(child);
        pos = length_add(length_add(pos, child_padding), child_size);
        child_index++;
      }
    }
  }

  if (!pre_count) {
    if (is_rule_node) {
      uint32_t length = *index - start_index;
      PNODE_END_RULE_NODE(root_node, *index, symbol, length);
      PNODE_START_RULE_NODE_LENGTH(root_node, start_index, length);
      (*index)++;
    }
  } else {
    if (is_rule_node) {
      (*index)++;
    }
  }
}

PTree* ts_ptree_build_(TSTree* tree, bool include_all) {
  Subtree root = tree->root;
  // make an initial count of the required nodes
  size_t index = 0;
  ts_ptree_build__(root, NULL, &index, length_zero(), tree->language, true, include_all, 0, false, true);
  // allocate enough nodes required for the parse nodes array
  PNode* pnode = ts_calloc(index, sizeof(PNode));
  index = 0;
  // now build the nodes
  ts_ptree_build__(root, pnode, &index, length_zero(), tree->language, true, include_all, 0, false, false);
  // return a new ptree
  return ts_ptree_new(tree, pnode, index);
}

PTree* ts_ptree_build(TSTree* tree, bool include_all) {
  return ts_ptree_build_(tree, include_all);
}