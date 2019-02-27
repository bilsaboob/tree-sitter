#ifndef TREE_SITTER_RUNTIME_H_
#define TREE_SITTER_RUNTIME_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define TREE_SITTER_LANGUAGE_VERSION 9

#ifndef TREE_SITTER_EXPORT
  #define TREE_SITTER_EXPORT __declspec(dllexport)
#endif

typedef uint16_t TSSymbol;
typedef uint64_t PNode;
typedef struct TSLanguage TSLanguage;
typedef struct TSParser TSParser;
typedef struct TSTree TSTree;
typedef struct PTree PTree;

typedef enum {
  TSInputEncodingUTF8,
  TSInputEncodingUTF16,
} TSInputEncoding;

typedef enum {
  TSSymbolTypeRegular,
  TSSymbolTypeAnonymous,
  TSSymbolTypeAuxiliary,
} TSSymbolType;

typedef struct {
  uint32_t row;
  uint32_t column;
} TSPoint;

typedef struct {
  TSPoint start_point;
  TSPoint end_point;
  uint32_t start_byte;
  uint32_t end_byte;
} TSRange;

typedef struct {
  void *payload;
  const char *(*read)(void *payload, uint32_t byte_index, TSPoint position, uint32_t *bytes_read);
  TSInputEncoding encoding;
} TSInput;

typedef enum {
  TSLogTypeParse,
  TSLogTypeLex,
} TSLogType;

typedef struct {
  void *payload;
  void (*log)(void *payload, TSLogType, const char *);
} TSLogger;

typedef struct {
  uint32_t start_byte;
  uint32_t old_end_byte;
  uint32_t new_end_byte;
  TSPoint start_point;
  TSPoint old_end_point;
  TSPoint new_end_point;
} TSInputEdit;

typedef struct {
  uint32_t context[4];
  const void *id;
  const TSTree *tree;
} TSNode;

typedef struct {
  const void *tree;
  const void *id;
  uint32_t context[2];
} TSTreeCursor;

TREE_SITTER_EXPORT TSParser *ts_parser_new();
TREE_SITTER_EXPORT void ts_parser_delete(TSParser *);
TREE_SITTER_EXPORT const TSLanguage *ts_parser_language(const TSParser *);
TREE_SITTER_EXPORT bool ts_parser_set_language(TSParser *, const TSLanguage *);
TREE_SITTER_EXPORT TSLogger ts_parser_logger(const TSParser *);
TREE_SITTER_EXPORT void ts_parser_set_logger(TSParser *, TSLogger);
TREE_SITTER_EXPORT void ts_parser_print_dot_graphs(TSParser *, FILE *);
TREE_SITTER_EXPORT void ts_parser_halt_on_error(TSParser *, bool);
TREE_SITTER_EXPORT TSTree *ts_parser_parse(TSParser *, const TSTree *, TSInput);
TREE_SITTER_EXPORT TSTree *ts_parser_parse_string(TSParser *, const TSTree *, const char *, uint32_t);
TREE_SITTER_EXPORT PTree* ts_parser_parse_string_as_ptree(TSParser *, const TSTree *, const char *, uint32_t, bool);
TREE_SITTER_EXPORT TSTree *ts_parser_parse_string_encoding(TSParser *, const TSTree *, const char *, uint32_t, TSInputEncoding);
TREE_SITTER_EXPORT bool ts_parser_enabled(const TSParser *);
TREE_SITTER_EXPORT void ts_parser_set_enabled(TSParser *, bool);
TREE_SITTER_EXPORT size_t ts_parser_operation_limit(const TSParser *);
TREE_SITTER_EXPORT void ts_parser_set_operation_limit(TSParser *, size_t);
TREE_SITTER_EXPORT void ts_parser_reset(TSParser *);
TREE_SITTER_EXPORT void ts_parser_set_included_ranges(TSParser *, const TSRange *, uint32_t);
TREE_SITTER_EXPORT const TSRange *ts_parser_included_ranges(const TSParser *, uint32_t *);

TREE_SITTER_EXPORT TSTree *ts_tree_copy(const TSTree *);
TREE_SITTER_EXPORT void ts_tree_delete(TSTree *);
TREE_SITTER_EXPORT TSNode ts_tree_root_node(const TSTree *);
TREE_SITTER_EXPORT void ts_tree_edit(TSTree *, const TSInputEdit *);
TREE_SITTER_EXPORT TSRange *ts_tree_get_changed_ranges(const TSTree *, const TSTree *, uint32_t *);
TREE_SITTER_EXPORT void ts_tree_print_dot_graph(const TSTree *, FILE *);
TREE_SITTER_EXPORT const TSLanguage *ts_tree_language(const TSTree *);

TREE_SITTER_EXPORT void ts_ptree_delete(PTree *);

TREE_SITTER_EXPORT uint32_t ts_node_start_byte(TSNode);
TREE_SITTER_EXPORT TSPoint ts_node_start_point(TSNode);
TREE_SITTER_EXPORT uint32_t ts_node_end_byte(TSNode);
TREE_SITTER_EXPORT TSPoint ts_node_end_point(TSNode);
TREE_SITTER_EXPORT TSSymbol ts_node_symbol(TSNode);
TREE_SITTER_EXPORT const char *ts_node_type(TSNode);
TREE_SITTER_EXPORT char *ts_node_string(TSNode);
TREE_SITTER_EXPORT bool ts_node_eq(TSNode, TSNode);
TREE_SITTER_EXPORT bool ts_node_is_null(TSNode);
TREE_SITTER_EXPORT bool ts_node_is_named(TSNode);
TREE_SITTER_EXPORT bool ts_node_is_missing(TSNode);
TREE_SITTER_EXPORT bool ts_node_has_changes(TSNode);
TREE_SITTER_EXPORT bool ts_node_has_error(TSNode);
TREE_SITTER_EXPORT TSNode ts_node_parent(TSNode);
TREE_SITTER_EXPORT TSNode ts_node_child(TSNode, uint32_t);
TREE_SITTER_EXPORT TSNode ts_node_named_child(TSNode, uint32_t);
TREE_SITTER_EXPORT uint32_t ts_node_child_count(TSNode);
TREE_SITTER_EXPORT uint32_t ts_node_named_child_count(TSNode);
TREE_SITTER_EXPORT TSNode ts_node_next_sibling(TSNode);
TREE_SITTER_EXPORT TSNode ts_node_next_named_sibling(TSNode);
TREE_SITTER_EXPORT TSNode ts_node_prev_sibling(TSNode);
TREE_SITTER_EXPORT TSNode ts_node_prev_named_sibling(TSNode);
TREE_SITTER_EXPORT TSNode ts_node_first_child_for_byte(TSNode, uint32_t);
TREE_SITTER_EXPORT TSNode ts_node_first_named_child_for_byte(TSNode, uint32_t);
TREE_SITTER_EXPORT TSNode ts_node_descendant_for_byte_range(TSNode, uint32_t, uint32_t);
TREE_SITTER_EXPORT TSNode ts_node_named_descendant_for_byte_range(TSNode, uint32_t, uint32_t);
TREE_SITTER_EXPORT TSNode ts_node_descendant_for_point_range(TSNode, TSPoint, TSPoint);
TREE_SITTER_EXPORT TSNode ts_node_named_descendant_for_point_range(TSNode, TSPoint, TSPoint);
TREE_SITTER_EXPORT void ts_node_edit(TSNode *, const TSInputEdit *);

TREE_SITTER_EXPORT TSTreeCursor ts_tree_cursor_new(TSNode);
TREE_SITTER_EXPORT void ts_tree_cursor_delete(TSTreeCursor *);
TREE_SITTER_EXPORT void ts_tree_cursor_reset(TSTreeCursor *, TSNode);
TREE_SITTER_EXPORT TSNode ts_tree_cursor_current_node(const TSTreeCursor *);
TREE_SITTER_EXPORT bool ts_tree_cursor_goto_parent(TSTreeCursor *);
TREE_SITTER_EXPORT bool ts_tree_cursor_goto_next_sibling(TSTreeCursor *);
TREE_SITTER_EXPORT bool ts_tree_cursor_goto_first_child(TSTreeCursor *);
TREE_SITTER_EXPORT int64_t ts_tree_cursor_goto_first_child_for_byte(TSTreeCursor *, uint32_t);

TREE_SITTER_EXPORT uint32_t ts_language_symbol_count(const TSLanguage *);
TREE_SITTER_EXPORT const char *ts_language_symbol_name(const TSLanguage *, TSSymbol);
TREE_SITTER_EXPORT TSSymbol ts_language_symbol_for_name(const TSLanguage *, const char *);
TREE_SITTER_EXPORT TSSymbolType ts_language_symbol_type(const TSLanguage *, TSSymbol);
TREE_SITTER_EXPORT uint32_t ts_language_version(const TSLanguage *);

#ifdef __cplusplus
}
#endif

#endif  // TREE_SITTER_RUNTIME_H_
