#ifndef KAPPOK_H
#define KAPPOK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h> // bool型のために追加
#include <math.h>    // round, roundf のために追加

// --- トークンタイプ ---
typedef enum {
    TOKEN_EOF = 0,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,          // 整数リテラル
    TOKEN_STRING,          // 文字列リテラル
    TOKEN_LPAREN,          // (
    TOKEN_RPAREN,          // )
    TOKEN_LBRACE,          // {
    TOKEN_RBRACE,          // }
    TOKEN_COMMA,           // ,
    TOKEN_ASSIGN,          // =
    
    // キーワード
    TOKEN_DEF,             // def
    TOKEN_RETURN,          // return
    TOKEN_INT,             // int
    TOKEN_STR,             // str
    TOKEN_DOUBLE,          // double
    TOKEN_BOOL,            // bool
    TOKEN_TRUE,            // True
    TOKEN_FALSE,           // False

    // 浮動小数点数リテラル
    TOKEN_FLOAT_LITERAL,   // 3.14など
    
    // 演算子
    TOKEN_PLUS,            // +
    TOKEN_MINUS,           // -
    TOKEN_ASTERISK,        // *
    TOKEN_SLASH,           // /

    TOKEN_UNKNOWN          // 不明なトークン
} TokenType;

// --- トークン構造体 ---
typedef struct Token {
    TokenType type;
    char *value;
    int line;
} Token;

// --- レクサー構造体 ---
typedef struct Lexer {
    char *source;
    int pos;
    int line;
} Lexer;

// --- ASTノードタイプ ---
typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION_DEFINITION,
    NODE_BLOCK,
    NODE_RETURN_STATEMENT,
    NODE_PRINT_STATEMENT,
    NODE_STRING_LITERAL,
    NODE_NUMBER_LITERAL,      // 整数リテラル
    NODE_FLOAT_LITERAL,       // 浮動小数点数リテラル
    NODE_FUNCTION_CALL,
    NODE_VAR_DECLARATION,
    NODE_ASSIGNMENT,
    NODE_IDENTIFIER_EXPR,
    // 算術演算
    NODE_ADD,
    NODE_SUBTRACT,
    NODE_MULTIPLY,
    NODE_DIVIDE
} ASTNodeType;

// --- ASTノード構造体 ---
struct ASTNode; // 前方宣言

typedef struct ASTNode {
    ASTNodeType type;
    int line;
    union {
        struct {
            struct ASTNode **statements;
            int num_statements;
            int capacity_statements;
        } program;
        struct {
            char *name;
            struct ASTNode **parameters; // 引数パース未実装
            int num_parameters;
            int capacity_parameters;
            struct ASTNode *body;
        } func_def;
        struct {
            struct ASTNode **statements;
            int num_statements;
            int capacity_statements;
        } block;
        struct {
            struct ASTNode *value;
        } return_stmt;
        struct {
            struct ASTNode **arguments;
            int num_arguments;
            int capacity_arguments;
        } print_stmt;
        struct {
            char *value;
        } string_literal;
        struct {
            long value; // 整数リテラル
        } number_literal;
        struct {
            double value; // 浮動小数点数リテラル
        } float_literal;
        struct {
            char *function_name;
            struct ASTNode **arguments;
            int num_arguments;
            int capacity_arguments;
        } func_call;
        struct {
            char *type_name; // "int", "str", "double", "bool"
            char *name;
            struct ASTNode *initializer;
        } var_decl;
        struct {
            char *name;
            struct ASTNode *value;
        } assignment;
        struct {
            char *name;
        } identifier_expr;
        // 算術演算子ノード
        struct {
            struct ASTNode *left;
            struct ASTNode *right;
        } binary_expr;
    } data;
} ASTNode;

// --- 値の型 ---
typedef enum {
    VALUE_TYPE_INT,
    VALUE_TYPE_STR,
    VALUE_TYPE_DOUBLE,
    VALUE_TYPE_BOOL,
    VALUE_TYPE_VOID,
    VALUE_TYPE_FUNCTION,
    VALUE_TYPE_UNKNOWN
} ValueType;

// --- 値構造体 ---
typedef struct Value {
    ValueType type;
    union {
        long int_value;
        char *str_value;
        double double_value;
        bool bool_value;
        struct {
            char *name;
            struct ASTNode *body;
            // TODO: parameters もここに追加する
        } func_ptr;
    } data;
} Value;

// --- シンボルテーブルのエントリ ---
typedef struct SymbolEntry {
    char *name;
    Value value;
    ValueType type; // シンボルの型を保存
} SymbolEntry;

// --- 環境 (シンボルテーブル) ---
typedef struct Environment {
    SymbolEntry *symbols;
    int num_symbols;
    int capacity_symbols;
    struct Environment *parent; // 親スコープ
} Environment;


// --- レクサー関数プロトタイプ ---
Lexer *lexer_create(char *source);
void lexer_destroy(Lexer *lexer);
Token *lexer_next_token(Lexer *lexer);
void token_destroy(Token *token);
void skip_whitespace(Lexer *lexer);
Token *read_string(Lexer *lexer);
Token *read_identifier(Lexer *lexer);
Token *read_number(Lexer *lexer);


// --- パーサー関数プロトタイプ ---
ASTNode *parse(Lexer *lexer);
ASTNode *create_ast_node(ASTNodeType type, int line);
void add_statement_to_program(ASTNode *program, ASTNode *statement);
void add_statement_to_block(ASTNode *block_node, ASTNode *statement);
void add_argument_to_print(ASTNode *print_node, ASTNode *argument);
void add_argument_to_function_call(ASTNode *func_call_node, ASTNode *argument);
ASTNode *parse_expression(Lexer *lexer);
ASTNode *parse_print_statement(Lexer *lexer);
ASTNode *parse_return_statement(Lexer *lexer);
ASTNode *parse_function_call(Lexer *lexer, char *function_name); // func_call.arguments が使えるように
ASTNode *parse_var_declaration(Lexer *lexer, char *type_name);
ASTNode *parse_assignment_or_call(Lexer *lexer, char *identifier_name);
ASTNode *parse_term(Lexer *lexer);
ASTNode *parse_factor(Lexer *lexer);


// --- AST解放関数プロトタイプ ---
void destroy_ast(ASTNode *node);

// --- インタプリタ関数プロトタイプ ---
void interpret_ast(ASTNode *program_node);
Value interpret_node(ASTNode *node, Environment *env);
Environment *create_environment(Environment *parent);
void destroy_environment(Environment *env);
void define_symbol(Environment *env, const char *name, Value value);
SymbolEntry *get_symbol(Environment *env, const char *name);
void free_value_data(Value value);
void print_value(Value val, int precision); // precision引数を追加
Value convert_value_to_double(Value val);

#endif // KAPPOK_H
