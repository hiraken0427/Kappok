#include "kappok.h"

// ASTノードを作成するヘルパー関数
ASTNode *create_ast_node(ASTNodeType type, int line) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (node == NULL) {
        perror("Failed to allocate AST node");
        exit(EXIT_FAILURE);
    }
    node->type = type;
    node->line = line;
    // 必要に応じて共用体のメンバーを初期化
    switch (type) {
        case NODE_PROGRAM:
            node->data.program.statements = NULL;
            node->data.program.num_statements = 0;
            node->data.program.capacity_statements = 0;
            break;
        case NODE_FUNCTION_DEFINITION:
            node->data.func_def.name = NULL;
            node->data.func_def.parameters = NULL;
            node->data.func_def.num_parameters = 0;
            node->data.func_def.capacity_parameters = 0;
            node->data.func_def.body = NULL;
            break;
        case NODE_BLOCK:
            node->data.block.statements = NULL;
            node->data.block.num_statements = 0;
            node->data.block.capacity_statements = 0;
            break;
        case NODE_RETURN_STATEMENT:
            node->data.return_stmt.value = NULL;
            break;
        case NODE_PRINT_STATEMENT:
            node->data.print_stmt.arguments = NULL;
            node->data.print_stmt.num_arguments = 0;
            node->data.print_stmt.capacity_arguments = 0;
            break;
        case NODE_STRING_LITERAL:
            node->data.string_literal.value = NULL;
            break;
        case NODE_NUMBER_LITERAL:
            node->data.number_literal.value = 0;
            break;
        case NODE_FLOAT_LITERAL:
            node->data.float_literal.value = 0.0;
            break;
        case NODE_FUNCTION_CALL:
            node->data.func_call.function_name = NULL;
            node->data.func_call.arguments = NULL;
            node->data.func_call.num_arguments = 0;
            node->data.func_call.capacity_arguments = 0;
            break;
        case NODE_VAR_DECLARATION:
            node->data.var_decl.type_name = NULL;
            node->data.var_decl.name = NULL;
            node->data.var_decl.initializer = NULL;
            break;
        case NODE_ASSIGNMENT:
            node->data.assignment.name = NULL;
            node->data.assignment.value = NULL;
            break;
        case NODE_IDENTIFIER_EXPR:
            node->data.identifier_expr.name = NULL;
            break;
        // 算術演算子ノード
        case NODE_ADD:
        case NODE_SUBTRACT:
        case NODE_MULTIPLY:
        case NODE_DIVIDE:
            node->data.binary_expr.left = NULL;
            node->data.binary_expr.right = NULL;
            break;
        default:
            break;
    }
    return node;
}

// プログラムノードに文を追加するヘルパー関数
void add_statement_to_program(ASTNode *program, ASTNode *statement) {
    if (program->type != NODE_PROGRAM) {
        fprintf(stderr, "エラー: add_statement_to_programはNODE_PROGRAMノードにのみ適用できます。\n");
        return;
    }

    if (program->data.program.num_statements >= program->data.program.capacity_statements) {
        int new_capacity = (program->data.program.capacity_statements == 0) ? 4 : program->data.program.capacity_statements * 2;
        program->data.program.statements = realloc(program->data.program.statements, sizeof(ASTNode *) * new_capacity);
        if (program->data.program.statements == NULL) {
            perror("Failed to reallocate statements array for program");
            exit(EXIT_FAILURE);
        }
        program->data.program.capacity_statements = new_capacity;
    }
    program->data.program.statements[program->data.program.num_statements++] = statement;
}

// ブロックノードに文を追加するヘルパー関数
void add_statement_to_block(ASTNode *block_node, ASTNode *statement) {
    if (block_node->type != NODE_BLOCK) {
        fprintf(stderr, "エラー: add_statement_to_blockはNODE_BLOCKノードにのみ適用できます。\n");
        return;
    }

    if (block_node->data.block.num_statements >= block_node->data.block.capacity_statements) {
        int new_capacity = (block_node->data.block.capacity_statements == 0) ? 4 : block_node->data.block.capacity_statements * 2;
        block_node->data.block.statements = realloc(block_node->data.block.statements, sizeof(ASTNode *) * new_capacity);
        if (block_node->data.block.statements == NULL) {
            perror("Failed to reallocate statements array for block");
            exit(EXIT_FAILURE);
        }
        block_node->data.block.capacity_statements = new_capacity;
    }
    block_node->data.block.statements[block_node->data.block.num_statements++] = statement;
}

// print文ノードに引数を追加するヘルパー関数
void add_argument_to_print(ASTNode *print_node, ASTNode *argument) {
    if (print_node->type != NODE_PRINT_STATEMENT) {
        fprintf(stderr, "エラー: add_argument_to_printはNODE_PRINT_STATEMENTノードにのみ適用できます。\n");
        return;
    }

    if (print_node->data.print_stmt.num_arguments >= print_node->data.print_stmt.capacity_arguments) {
        int new_capacity = (print_node->data.print_stmt.capacity_arguments == 0) ? 4 : print_node->data.print_stmt.capacity_arguments * 2;
        print_node->data.print_stmt.arguments = realloc(print_node->data.print_stmt.arguments, sizeof(ASTNode *) * new_capacity);
        if (print_node->data.print_stmt.arguments == NULL) {
            perror("Failed to reallocate arguments array");
            exit(EXIT_FAILURE);
        }
        print_node->data.print_stmt.capacity_arguments = new_capacity;
    }
    print_node->data.print_stmt.arguments[print_node->data.print_stmt.num_arguments++] = argument;
}

// 関数呼び出しノードに引数を追加するヘルパー関数
void add_argument_to_function_call(ASTNode *func_call_node, ASTNode *argument) {
    if (func_call_node->type != NODE_FUNCTION_CALL) {
        fprintf(stderr, "エラー: add_argument_to_function_callはNODE_FUNCTION_CALLノードにのみ適用できます。\n");
        return;
    }
    if (func_call_node->data.func_call.num_arguments >= func_call_node->data.func_call.capacity_arguments) {
        int new_capacity = (func_call_node->data.func_call.capacity_arguments == 0) ? 4 : func_call_node->data.func_call.capacity_arguments * 2;
        func_call_node->data.func_call.arguments = realloc(func_call_node->data.func_call.arguments, sizeof(ASTNode *) * new_capacity);
        if (func_call_node->data.func_call.arguments == NULL) {
            perror("Failed to reallocate arguments array for function call");
            exit(EXIT_FAILURE);
        }
        func_call_node->data.func_call.capacity_arguments = new_capacity;
    }
    func_call_node->data.func_call.arguments[func_call_node->data.func_call.num_arguments++] = argument;
}


// 最も高い優先順位の式 (リテラル、識別子、括弧) をパースする関数
ASTNode *parse_factor(Lexer *lexer) {
    Token *token = lexer_next_token(lexer);
    ASTNode *node = NULL;

    if (token->type == TOKEN_NUMBER) {
        node = create_ast_node(NODE_NUMBER_LITERAL, token->line);
        node->data.number_literal.value = atol(token->value);
    } else if (token->type == TOKEN_FLOAT_LITERAL) {
        node = create_ast_node(NODE_FLOAT_LITERAL, token->line);
        node->data.float_literal.value = strtod(token->value, NULL);
    } else if (token->type == TOKEN_STRING) {
        node = create_ast_node(NODE_STRING_LITERAL, token->line);
        node->data.string_literal.value = strdup(token->value);
        if (node->data.string_literal.value == NULL) {
            perror("Failed to duplicate string for AST node");
            exit(EXIT_FAILURE);
        }
    } else if (token->type == TOKEN_TRUE) {
        node = create_ast_node(NODE_NUMBER_LITERAL, token->line); // bool値は数値として格納 (1)
        node->data.number_literal.value = 1;
    } else if (token->type == TOKEN_FALSE) {
        node = create_ast_node(NODE_NUMBER_LITERAL, token->line); // bool値は数値として格納 (0)
        node->data.number_literal.value = 0;
    } else if (token->type == TOKEN_IDENTIFIER) {
        // 識別子の後に '(' が続く場合は関数呼び出し
        int original_pos = lexer->pos;
        int original_line = lexer->line;
        Token *peek_token = lexer_next_token(lexer);

        lexer->pos = original_pos;
        lexer->line = original_line;

        if (peek_token->type == TOKEN_LPAREN) {
            // ここで peek_token を消費せず、parse_function_call に任せる
            node = parse_function_call(lexer, token->value); // 関数呼び出しとしてパース
            token_destroy(peek_token); // parse_function_call が消費しないのでここで解放
        } else {
            token_destroy(peek_token); // ピークしたトークンを解放
            node = create_ast_node(NODE_IDENTIFIER_EXPR, token->line);
            node->data.identifier_expr.name = strdup(token->value);
            if (node->data.identifier_expr.name == NULL) {
                perror("Failed to duplicate identifier name for AST node");
                exit(EXIT_FAILURE);
            }
        }
    } else if (token->type == TOKEN_LPAREN) {
        node = parse_expression(lexer); // 括弧内の式を再帰的にパース
        Token *rparen_token = lexer_next_token(lexer);
        if (rparen_token->type != TOKEN_RPAREN) {
            fprintf(stderr, "エラー (行 %d): 期待される ')' が見つかりません。見つかったのは '%s' です。\n", rparen_token->line, rparen_token->value);
            token_destroy(rparen_token);
            destroy_ast(node);
            return NULL;
        }
        token_destroy(rparen_token);
    } else {
        fprintf(stderr, "エラー (行 %d): 予期しないトークン '%s' です。式が期待されます。\n", token->line, token->value);
        node = NULL;
    }
    token_destroy(token);
    return node;
}

// 乗除算の式をパースする関数
// term: factor ( ( '*' | '/' ) factor )*
ASTNode *parse_term(Lexer *lexer) {
    ASTNode *node = parse_factor(lexer);
    if (node == NULL) return NULL;

    while (true) {
        int original_pos = lexer->pos;
        int original_line = lexer->line;
        Token *op_token = lexer_next_token(lexer);

        if (op_token->type == TOKEN_ASTERISK || op_token->type == TOKEN_SLASH) {
            ASTNode *binary_op_node = create_ast_node(
                (op_token->type == TOKEN_ASTERISK) ? NODE_MULTIPLY : NODE_DIVIDE,
                op_token->line
            );
            token_destroy(op_token);

            binary_op_node->data.binary_expr.left = node;
            binary_op_node->data.binary_expr.right = parse_factor(lexer);
            if (binary_op_node->data.binary_expr.right == NULL) {
                destroy_ast(binary_op_node);
                return NULL;
            }
            node = binary_op_node; // 新しいノードを現在のノードとする
        } else {
            lexer->pos = original_pos; // トークンを戻す
            lexer->line = original_line;
            token_destroy(op_token);
            break;
        }
    }
    return node;
}

// とりあえず今日はここまでにして寝る
// 加減算の式をパースする関数
// expression: term ( ( '+' | '-' ) term )*
ASTNode *parse_expression(Lexer *lexer) {
    ASTNode *node = parse_term(lexer);
    if (node == NULL) return NULL;

    while (true) {
        int original_pos = lexer->pos;
        int original_line = lexer->line;
        Token *op_token = lexer_next_token(lexer);

        if (op_token->type == TOKEN_PLUS || op_token->type == TOKEN_MINUS) {
            ASTNode *binary_op_node = create_ast_node(
                (op_token->type == TOKEN_PLUS) ? NODE_ADD : NODE_SUBTRACT,
                op_token->line
            );
            token_destroy(op_token);

            binary_op_node->data.binary_expr.left = node;
            binary_op_node->data.binary_expr.right = parse_term(lexer);
            if (binary_op_node->data.binary_expr.right == NULL) {
                destroy_ast(binary_op_node);
                return NULL;
            }
            node = binary_op_node; // 新しいノードを現在のノードとする
        } else {
            lexer->pos = original_pos; // トークンを戻す
            lexer->line = original_line;
            token_destroy(op_token);
            break;
        }
    }
    return node;
}


// print文をパースする関数
// print ( "Hello", 42, myVar )
ASTNode *parse_print_statement(Lexer *lexer) {
    int line = lexer->line;
    ASTNode *print_node = create_ast_node(NODE_PRINT_STATEMENT, line);
    
    Token *token = lexer_next_token(lexer); // '(' を読む
    if (token->type != TOKEN_LPAREN) {
        fprintf(stderr, "エラー (行 %d): 'print' の後に '(' が期待されますが '%s' が見つかりました。\n", line, token->value);
        token_destroy(token);
        destroy_ast(print_node);
        return NULL;
    }
    token_destroy(token);
    
    int expect_comma = 0;

    while (1) {
        // 現在のレクサーの状態を保存
        int original_pos = lexer->pos;
        int original_line = lexer->line;

        Token *peek_token = lexer_next_token(lexer); // ピーク
        
        // 状態を元に戻す
        lexer->pos = original_pos;
        lexer->line = original_line;
        
        if (peek_token->type == TOKEN_RPAREN) {
            token_destroy(peek_token); // ピークトークンを解放
            break; // ')' ならループを抜ける
        }

        token_destroy(peek_token); // ピークしたトークンを解放

        if (expect_comma) {
            token = lexer_next_token(lexer); // ',' を読む
            if (token->type != TOKEN_COMMA) {
                fprintf(stderr, "エラー (行 %d): 引数の間に ',' が期待されますが '%s' が見つかりました。\n", token->line, token->value);
                token_destroy(token);
                destroy_ast(print_node);
                return NULL;
            }
            token_destroy(token);
        }
        
        // 式をパースして引数として追加
        ASTNode *argument_expr = parse_expression(lexer); // parse_expression を呼び出す
        if (argument_expr == NULL) {
            destroy_ast(print_node);
            return NULL;
        }
        add_argument_to_print(print_node, argument_expr);
        expect_comma = 1;
    }
    
    token = lexer_next_token(lexer); // 閉じ ')' を読む
    token_destroy(token); // 既に上でピークして確認済みだが、ここで正式に消費

    return print_node;
}

// return文をパースする関数
// return 0
ASTNode *parse_return_statement(Lexer *lexer) {
    int line = lexer->line;
    ASTNode *return_node = create_ast_node(NODE_RETURN_STATEMENT, line);

    // 戻り値の式をパースする
    ASTNode *return_value_expr = parse_expression(lexer); // parse_expression を呼び出す
    if (return_value_expr == NULL) {
        destroy_ast(return_node);
        return NULL;
    }
    return_node->data.return_stmt.value = return_value_expr;

    return return_node;
}

// 関数呼び出しをパースする関数
// functionName(arg1, arg2)
ASTNode *parse_function_call(Lexer *lexer, char *function_name) {
    int line = lexer->line;
    ASTNode *func_call_node = create_ast_node(NODE_FUNCTION_CALL, line);
    func_call_node->data.func_call.function_name = strdup(function_name);
    if (func_call_node->data.func_call.function_name == NULL) {
        perror("Failed to duplicate function name for function call");
        exit(EXIT_FAILURE);
    }

    Token *token = lexer_next_token(lexer); // '(' を読む
    if (token->type != TOKEN_LPAREN) {
        fprintf(stderr, "エラー (行 %d): 関数呼び出しの後に '(' が期待されますが '%s' が見つかりました。\n", line, token->value);
        token_destroy(token);
        destroy_ast(func_call_node);
        return NULL;
    }
    token_destroy(token);

    // 引数のパースロジック
    int expect_comma = 0;
    while (1) {
        int original_pos = lexer->pos;
        int original_line = lexer->line;
        Token *peek_token = lexer_next_token(lexer);
        lexer->pos = original_pos;
        lexer->line = original_line;

        if (peek_token->type == TOKEN_RPAREN) {
            token_destroy(peek_token);
            break; // ')' ならループを抜ける
        }
        token_destroy(peek_token);

        if (expect_comma) {
            token = lexer_next_token(lexer); // ',' を読む
            if (token->type != TOKEN_COMMA) {
                fprintf(stderr, "エラー (行 %d): 関数引数の間に ',' が期待されますが '%s' が見つかりました。\n", token->line, token->value);
                token_destroy(token);
                destroy_ast(func_call_node);
                return NULL;
            }
            token_destroy(token);
        }
        
        ASTNode *argument_expr = parse_expression(lexer);
        if (argument_expr == NULL) {
            destroy_ast(func_call_node);
            return NULL;
        }
        add_argument_to_function_call(func_call_node, argument_expr);
        expect_comma = 1;
    }
    
    token = lexer_next_token(lexer); // 閉じ ')' を読む
    token_destroy(token);

    return func_call_node;
}

// 変数宣言をパースする関数
ASTNode *parse_var_declaration(Lexer *lexer, char *type_name) {
    int line = lexer->line;
    ASTNode *var_decl_node = create_ast_node(NODE_VAR_DECLARATION, line);
    var_decl_node->data.var_decl.type_name = strdup(type_name);
    if (var_decl_node->data.var_decl.type_name == NULL) {
        perror("Failed to duplicate type name for var declaration");
        exit(EXIT_FAILURE);
    }

    Token *token = lexer_next_token(lexer); // 変数名 (識別子) を読む
    if (token->type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "エラー (行 %d): 変数名が期待されますが '%s' が見つかりました。\n", line, token->value);
        token_destroy(token);
        destroy_ast(var_decl_node);
        return NULL;
    }
    var_decl_node->data.var_decl.name = strdup(token->value);
    if (var_decl_node->data.var_decl.name == NULL) {
        perror("Failed to duplicate var name for var declaration");
        exit(EXIT_FAILURE);
    }
    token_destroy(token);

    token = lexer_next_token(lexer); // '=' を読む
    if (token->type != TOKEN_ASSIGN) {
        fprintf(stderr, "エラー (行 %d): 変数宣言で '=' が期待されますが '%s' が見つかりました。\n", line, token->value);
        token_destroy(token);
        destroy_ast(var_decl_node);
        return NULL;
    }
    token_destroy(token);

    // 初期値の式をパースする
    ASTNode *initializer_expr = parse_expression(lexer);
    if (initializer_expr == NULL) {
        destroy_ast(var_decl_node);
        return NULL;
    }
    var_decl_node->data.var_decl.initializer = initializer_expr;

    return var_decl_node;
}

// 代入または関数呼び出しをパースする関数
ASTNode *parse_assignment_or_call(Lexer *lexer, char *identifier_name) {
    int current_line = lexer->line;

    // 次のトークンをピーク
    int original_pos = lexer->pos;
    int original_line = lexer->line;
    Token *peek_token = lexer_next_token(lexer); // ピークトークンを取得

    // レクサーの状態を元に戻す
    lexer->pos = original_pos;
    lexer->line = original_line;

    ASTNode *statement_node = NULL;
    if (peek_token->type == TOKEN_LPAREN) {
        // 関数呼び出しの場合
        // '(' は parse_function_call 内で消費される
        statement_node = parse_function_call(lexer, identifier_name);
    } else if (peek_token->type == TOKEN_ASSIGN) {
        // 代入の場合
        // '=' トークンを実際に消費
        Token *assign_token = lexer_next_token(lexer); // 正式に '=' を取得
        token_destroy(assign_token); // '=' トークンを解放

        ASTNode *assignment_node = create_ast_node(NODE_ASSIGNMENT, current_line);
        assignment_node->data.assignment.name = strdup(identifier_name);
        if (assignment_node->data.assignment.name == NULL) {
            perror("Failed to duplicate assignment target name");
            exit(EXIT_FAILURE);
        }

        // 代入する値の式をパース
        ASTNode *value_expr = parse_expression(lexer);
        if (value_expr == NULL) {
            destroy_ast(assignment_node);
            statement_node = NULL; // エラー時
        } else {
            assignment_node->data.assignment.value = value_expr;
            statement_node = assignment_node;
        }
    } else {
        fprintf(stderr, "エラー (行 %d): 識別子 '%s' の後に予期しないトークン '%s' です。代入または関数呼び出しが期待されます。\n", 
                peek_token->line, identifier_name, peek_token->value);
        statement_node = NULL; // エラー時
    }
    
    // ピークしたトークンは、この関数の役割が終わったらここで解放する。
    // 各分岐内で実際に消費されるトークンは別途解放される。
    token_destroy(peek_token);

    return statement_node;
}


// コードブロックをパースする関数
// { ... }
ASTNode *parse_block(Lexer *lexer) {
    int line = lexer->line;
    ASTNode *block_node = create_ast_node(NODE_BLOCK, line);

    Token *token = lexer_next_token(lexer); // '{' を読む
    if (token->type != TOKEN_LBRACE) {
        fprintf(stderr, "エラー (行 %d): '{' が期待されますが '%s' が見つかりました。\n", line, token->value);
        token_destroy(token);
        destroy_ast(block_node);
        return NULL;
    }
    token_destroy(token);

    while (1) {
        Token *current_token = lexer_next_token(lexer);
        if (current_token->type == TOKEN_RBRACE) {
            token_destroy(current_token);
            break; // ブロックの終わり
        }
        if (current_token->type == TOKEN_EOF) {
            fprintf(stderr, "エラー (行 %d): ブロックの終わりに '}' が期待されますが、ファイルの終わりに達しました。\n", current_token->line);
            token_destroy(current_token);
            destroy_ast(block_node);
            return NULL;
        }

        ASTNode *statement = NULL;

        if (current_token->type == TOKEN_IDENTIFIER) {
            if (strcmp(current_token->value, "print") == 0) {
                token_destroy(current_token);
                statement = parse_print_statement(lexer);
            } else { // 識別子の場合は、関数呼び出しまたは代入の可能性がある
                char *identifier_name = strdup(current_token->value);
                token_destroy(current_token);
                statement = parse_assignment_or_call(lexer, identifier_name);
                free(identifier_name); // strdupした名前は関数内でコピーされるのでここで解放
            }
        } else if (current_token->type == TOKEN_RETURN) {
            token_destroy(current_token);
            statement = parse_return_statement(lexer);
        } else if (current_token->type == TOKEN_INT || 
                   current_token->type == TOKEN_STR ||
                   current_token->type == TOKEN_DOUBLE ||
                   current_token->type == TOKEN_BOOL) {
            char *type_name = strdup(current_token->value);
            token_destroy(current_token);
            statement = parse_var_declaration(lexer, type_name);
            free(type_name); // strdupした名前は関数内でコピーされるのでここで解放
        }
        else {
            fprintf(stderr, "エラー (行 %d): ブロック内で不正な文です。'%s'\n", current_token->line, current_token->value);
            token_destroy(current_token);
            destroy_ast(block_node);
            return NULL;
        }
        
        if (statement == NULL) {
            destroy_ast(block_node);
            return NULL; // 文のパース中にエラーが発生
        }
        add_statement_to_block(block_node, statement);
    }
    return block_node;
}


// 関数定義をパースする関数
// def main() { ... }
ASTNode *parse_function_definition(Lexer *lexer) {
    int line = lexer->line;
    ASTNode *func_def_node = create_ast_node(NODE_FUNCTION_DEFINITION, line);

    Token *token = lexer_next_token(lexer); // 関数名 (識別子) を読む
    if (token->type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "エラー (行 %d): 関数名が期待されますが '%s' が見つかりました。\n", line, token->value);
        token_destroy(token);
        destroy_ast(func_def_node);
        return NULL;
    }
    func_def_node->data.func_def.name = strdup(token->value); // 関数名をコピー
    if (func_def_node->data.func_def.name == NULL) {
        perror("Failed to duplicate function name");
        exit(EXIT_FAILURE);
    }
    token_destroy(token);

    token = lexer_next_token(lexer); // '(' を読む
    if (token->type != TOKEN_LPAREN) {
        fprintf(stderr, "エラー (行 %d): 関数名の後に '(' が期待されますが '%s' が見つかりました。\n", line, token->value);
        token_destroy(token);
        destroy_ast(func_def_node);
        return NULL;
    }
    token_destroy(token);

    token = lexer_next_token(lexer); // ')' を読む (引数はまだサポートしないため)
    if (token->type != TOKEN_RPAREN) {
        fprintf(stderr, "エラー (行 %d): '(' の後に ')' が期待されますが '%s' が見つかりました。\n", line, token->value);
        token_destroy(token);
        destroy_ast(func_def_node);
        return NULL;
    }
    token_destroy(token);

    // 関数本体のブロックをパース
    ASTNode *body_block = parse_block(lexer);
    if (body_block == NULL) {
        destroy_ast(func_def_node);
        return NULL;
    }
    func_def_node->data.func_def.body = body_block;

    return func_def_node;
}

// プログラム全体をパースする関数
ASTNode *parse(Lexer *lexer) {
    ASTNode *program_node = create_ast_node(NODE_PROGRAM, 0);

    Token *token;
    while ((token = lexer_next_token(lexer))->type != TOKEN_EOF) {
        if (token->type == TOKEN_DEF) { // 'def' キーワードを見つけたら関数定義をパース
            token_destroy(token); // def トークンを解放
            ASTNode *func_def_stmt = parse_function_definition(lexer);
            if (func_def_stmt == NULL) {
                destroy_ast(program_node);
                return NULL;
            }
            add_statement_to_program(program_node, func_def_stmt);
        } else {
            fprintf(stderr, "エラー (行 %d): 不正なトークン '%s' です。関数定義が期待されます。\n", token->line, token->value);
            token_destroy(token);
            destroy_ast(program_node);
            return NULL;
        }
    }
    token_destroy(token);
    
    return program_node;
}

// ASTを解放する関数
void destroy_ast(ASTNode *node) {
    if (node == NULL) {
        return;
    }

    switch (node->type) {
        case NODE_PROGRAM:
            for (int i = 0; i < node->data.program.num_statements; i++) {
                destroy_ast(node->data.program.statements[i]);
            }
            if (node->data.program.statements) {
                free(node->data.program.statements);
            }
            break;
        case NODE_FUNCTION_DEFINITION:
            if (node->data.func_def.name) {
                free(node->data.func_def.name);
            }
            destroy_ast(node->data.func_def.body);
            break;
        case NODE_BLOCK:
            for (int i = 0; i < node->data.block.num_statements; i++) {
                destroy_ast(node->data.block.statements[i]);
            }
            if (node->data.block.statements) {
                free(node->data.block.statements);
            }
            break;
        case NODE_RETURN_STATEMENT:
            if (node->data.return_stmt.value) {
                destroy_ast(node->data.return_stmt.value);
            }
            break;
        case NODE_PRINT_STATEMENT:
            for (int i = 0; i < node->data.print_stmt.num_arguments; i++) {
                destroy_ast(node->data.print_stmt.arguments[i]);
            }
            if (node->data.print_stmt.arguments) {
                free(node->data.print_stmt.arguments);
            }
            break;
        case NODE_STRING_LITERAL:
            if (node->data.string_literal.value) {
                free(node->data.string_literal.value);
            }
            break;
        case NODE_NUMBER_LITERAL:
            break;
        case NODE_FLOAT_LITERAL:
            break;
        case NODE_FUNCTION_CALL:
            if (node->data.func_call.function_name) {
                free(node->data.func_call.function_name);
            }
            for (int i = 0; i < node->data.func_call.num_arguments; i++) {
                destroy_ast(node->data.func_call.arguments[i]);
            }
            if (node->data.func_call.arguments) {
                free(node->data.func_call.arguments);
            }
            break;
        case NODE_VAR_DECLARATION:
            if (node->data.var_decl.type_name) {
                free(node->data.var_decl.type_name);
            }
            if (node->data.var_decl.name) {
                free(node->data.var_decl.name);
            }
            if (node->data.var_decl.initializer) {
                destroy_ast(node->data.var_decl.initializer);
            }
            break;
        case NODE_ASSIGNMENT:
            if (node->data.assignment.name) {
                free(node->data.assignment.name);
            }
            if (node->data.assignment.value) {
                destroy_ast(node->data.assignment.value);
            }
            break;
        case NODE_IDENTIFIER_EXPR:
            if (node->data.identifier_expr.name) {
                free(node->data.identifier_expr.name);
            }
            break;
        case NODE_ADD:
        case NODE_SUBTRACT:
        case NODE_MULTIPLY:
        case NODE_DIVIDE:
            destroy_ast(node->data.binary_expr.left);
            destroy_ast(node->data.binary_expr.right);
            break;
    }
    free(node);
}