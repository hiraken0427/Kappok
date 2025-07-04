#include "kappok.h"

Environment *create_environment(Environment *parent) {
    Environment *env = malloc(sizeof(Environment));
    if (env == NULL) {
        perror("Failed to allocate environment");
        exit(EXIT_FAILURE);
    }
    env->symbols = NULL;
    env->num_symbols = 0;
    env->capacity_symbols = 0;
    env->parent = parent;
    return env;
}

void destroy_environment(Environment *env) {
    if (env == NULL) {
        return;
    }
    for (int i = 0; i < env->num_symbols; i++) {
        free(env->symbols[i].name);
        // シンボルの値が動的に割り当てられたデータを持つ場合は解放
        free_value_data(env->symbols[i].value);
    }
    free(env->symbols);
    free(env);
}

// 動的に割り当てられたValueデータを解放する
void free_value_data(Value value) {
    if (value.type == VALUE_TYPE_STR && value.data.str_value != NULL) {
        free(value.data.str_value);
    }
    // double, bool, int, void, function は動的メモリを持たないため、ここではfreeしない
}

void define_symbol(Environment *env, const char *name, Value value) {
    // 既存のシンボルがあれば更新 (現状は同名変数宣言は許容しないが、代入時に使う)
    for (int i = 0; i < env->num_symbols; i++) {
        if (strcmp(env->symbols[i].name, name) == 0) {
            // 既存の値を解放してから新しい値をコピー
            free_value_data(env->symbols[i].value);
            env->symbols[i].value = value;
            env->symbols[i].type = value.type; // 型も更新
            return;
        }
    }

    // 新しいシンボルとして追加
    if (env->num_symbols >= env->capacity_symbols) {
        int new_capacity = (env->capacity_symbols == 0) ? 4 : env->capacity_symbols * 2;
        env->symbols = realloc(env->symbols, sizeof(SymbolEntry) * new_capacity);
        if (env->symbols == NULL) {
            perror("Failed to reallocate symbol table");
            exit(EXIT_FAILURE);
        }
        env->capacity_symbols = new_capacity;
    }

    env->symbols[env->num_symbols].name = strdup(name);
    if (env->symbols[env->num_symbols].name == NULL) {
        perror("Failed to duplicate symbol name");
        exit(EXIT_FAILURE);
    }
    env->symbols[env->num_symbols].value = value;
    env->symbols[env->num_symbols].type = value.type; // 型も保存
    env->num_symbols++;
}

SymbolEntry *get_symbol(Environment *env, const char *name) {
    Environment *current_env = env;
    while (current_env != NULL) {
        for (int i = 0; i < current_env->num_symbols; i++) {
            if (strcmp(current_env->symbols[i].name, name) == 0) {
                return &current_env->symbols[i];
            }
        }
        current_env = current_env->parent;
    }
    return NULL; // 見つからなかった
}

// なんでこんなこと始めちゃったんだろう
// 値を標準出力に表示する関数
void print_value(Value val, int precision) {
    switch (val.type) {
        case VALUE_TYPE_INT:
            printf("%ld", val.data.int_value);
            break;
        case VALUE_TYPE_STR:
            printf("%s", val.data.str_value);
            break;
        case VALUE_TYPE_DOUBLE: {
            char buffer[100]; // 十分な大きさのバッファ
            if (precision >= 0) {
                // 指定された精度でフォーマット
                sprintf(buffer, "%.*f", precision, val.data.double_value);
                
                // 不要な末尾のゼロと小数点以下が全てゼロの場合は小数点を削除
                // ただし、今回は round 関数からの結果は string 型として返されるため、
                // このロジックは print("元のpi", pi) のような直接 double を print する場合に適用される
                char *dot = strchr(buffer, '.');
                if (dot) {
                    char *end = buffer + strlen(buffer) - 1;
                    while (end > dot && *end == '0') {
                        *end = '\0'; // ゼロを削除
                        end--;
                    }
                    if (*end == '.') { // 小数点以下が全てゼロになった場合、小数点も削除
                        *end = '\0';
                    }
                }
                printf("%s", buffer);
            } else {
                // precision が指定されていない場合、デフォルトの浮動小数点数表示
                // これは %f で冗長な0が付くから%gをつかう
                sprintf(buffer, "%g", val.data.double_value);
                printf("%s", buffer);
            }
            break;
        }
        case VALUE_TYPE_BOOL:
            printf("%s", val.data.bool_value ? "True" : "False");
            break;
        case VALUE_TYPE_VOID:
            printf("void"); // 通常はprintされないが、デバッグ用
            break;
        case VALUE_TYPE_FUNCTION:
            printf("<function %s>", val.data.func_ptr.name);
            break;
        case VALUE_TYPE_UNKNOWN:
            printf("<unknown value type>");
            break;
    }
    // print文の最後に改行を出力するのはinterpret_nodeで行う
}

// 値をdouble型に変換するヘルパー関数
// int, bool から double への変換をサポート
Value convert_value_to_double(Value val) {
    Value new_val;
    new_val.type = VALUE_TYPE_DOUBLE;
    if (val.type == VALUE_TYPE_INT) {
        new_val.data.double_value = (double)val.data.int_value;
    } else if (val.type == VALUE_TYPE_BOOL) {
        new_val.data.double_value = val.data.bool_value ? 1.0 : 0.0;
    } else if (val.type == VALUE_TYPE_DOUBLE) {
        new_val.data.double_value = val.data.double_value;
    } else {
        fprintf(stderr, "型変換エラー: double型に変換できない型です。\n");
        exit(EXIT_FAILURE);
    }
    return new_val;
}

// ASTノードを解釈し、値を返す関数
Value interpret_node(ASTNode *node, Environment *env) {
    Value result;
    result.type = VALUE_TYPE_VOID; // デフォルト値

    if (node == NULL) {
        return result;
    }

    switch (node->type) {
        case NODE_PROGRAM:
        case NODE_BLOCK: {
            for (int i = 0; i < node->data.block.num_statements; i++) {
                result = interpret_node(node->data.block.statements[i], env);
                // 関数からのreturnを処理 (ただし、今はmain関数のみなので単純に継続)
                // return文での関数終了ロジックを正確に実装する
            }
            break;
        }
        case NODE_FUNCTION_DEFINITION: {
            // 関数をシンボルテーブルに登録
            Value func_val;
            func_val.type = VALUE_TYPE_FUNCTION;
            func_val.data.func_ptr.name = strdup(node->data.func_def.name);
            func_val.data.func_ptr.body = node->data.func_def.body;
            // parametersもfunc_ptrに設定する

            define_symbol(env, node->data.func_def.name, func_val);
            break;
        }
        case NODE_RETURN_STATEMENT: {
            // 戻り値の式を評価
            result = interpret_node(node->data.return_stmt.value, env);
            // 実際の関数の呼び出し元に値を返すメカニズムを実装
            break;
        }
        case NODE_PRINT_STATEMENT: {
            // print 文の引数を順に評価し、出力
            for (int i = 0; i < node->data.print_stmt.num_arguments; i++) {
                Value arg_val = interpret_node(node->data.print_stmt.arguments[i], env);
                
                // round関数からの結果が string 型として返されることを考慮
                if (arg_val.type == VALUE_TYPE_STR) {
                    printf("%s", arg_val.data.str_value);
                } else {
                    // print_value に -1 を渡すことで、デフォルトの表示を行う
                    print_value(arg_val, -1);
                }
                free_value_data(arg_val);
                // 最後の引数でない場合はスペースを出力（カンマの後のスペース）
                if (i < node->data.print_stmt.num_arguments - 1) {
                    printf(" ");
                }
            }
            printf("\n"); // print文の最後に改行を出力
            break;
        }
        case NODE_STRING_LITERAL: {
            result.type = VALUE_TYPE_STR;
            result.data.str_value = strdup(node->data.string_literal.value);
            break;
        }
        case NODE_NUMBER_LITERAL: {
            result.type = VALUE_TYPE_INT;
            result.data.int_value = node->data.number_literal.value;
            break;
        }
        case NODE_FLOAT_LITERAL: {
            result.type = VALUE_TYPE_DOUBLE;
            result.data.double_value = node->data.float_literal.value;
            break;
        }
        case NODE_FUNCTION_CALL: {
            const char *func_name = node->data.func_call.function_name;

            // 組み込み関数の処理
            if (strcmp(func_name, "round") == 0) {
                if (node->data.func_call.num_arguments != 2) {
                    fprintf(stderr, "実行時エラー (行 %d): 'round' 関数は2つの引数 (数値, 精度) を取ります。\n", node->line);
                    exit(EXIT_FAILURE);
                }
                Value num_val = interpret_node(node->data.func_call.arguments[0], env);
                Value precision_val = interpret_node(node->data.func_call.arguments[1], env);

                if ((num_val.type != VALUE_TYPE_INT && num_val.type != VALUE_TYPE_DOUBLE) || precision_val.type != VALUE_TYPE_INT) {
                    fprintf(stderr, "実行時エラー (行 %d): 'round' 関数の引数の型が不正です。round(数値, 整数) が期待されます。\n", node->line);
                    exit(EXIT_FAILURE);
                }

                double val_to_round;
                if (num_val.type == VALUE_TYPE_INT) {
                    val_to_round = (double)num_val.data.int_value;
                } else { // VALUE_TYPE_DOUBLE
                    val_to_round = num_val.data.double_value;
                }
                int precision = (int)precision_val.data.int_value;

                // 指定された精度で四捨五入
                double factor = pow(10, precision);
                double rounded_val = round(val_to_round * factor) / factor;
                
                // ここで直接文字列にフォーマットして返す。
                // round 関数は指定された精度で厳密に表示するため、末尾のゼロ削除は行わない。
                char buffer[100]; // 十分な大きさのバッファ
                sprintf(buffer, "%.*f", precision, rounded_val);

                result.type = VALUE_TYPE_STR;
                result.data.str_value = strdup(buffer); // 動的に割り当てられた文字列を返す
                free_value_data(num_val); // 元の数値のメモリを解放（文字列の場合のみ）
                free_value_data(precision_val);
                break;
            }

            // ユーザー定義関数の処理
            SymbolEntry *func_entry = get_symbol(env, func_name);
            if (func_entry == NULL || func_entry->type != VALUE_TYPE_FUNCTION) {
                fprintf(stderr, "実行時エラー (行 %d): 未定義の関数 '%s' を呼び出そうとしました。\n", node->line, func_name);
                exit(EXIT_FAILURE);
            }
            
            // 新しい関数スコープを作成
            Environment *func_env = create_environment(env);
            // 引数を func_env にバインドする
            
            // 関数本体のブロックを解釈
            result = interpret_node(func_entry->value.data.func_ptr.body, func_env);
            
            // 関数スコープを破棄
            destroy_environment(func_env);
            break;
        }
        case NODE_VAR_DECLARATION: { 
            const char *type_name = node->data.var_decl.type_name;
            const char *var_name = node->data.var_decl.name;
            ASTNode *initializer = node->data.var_decl.initializer;

            Value initial_value = interpret_node(initializer, env);

            if (strcmp(type_name, "int") == 0) {
                if (initial_value.type != VALUE_TYPE_INT && initial_value.type != VALUE_TYPE_BOOL) {
                    fprintf(stderr, "実行時エラー (行 %d): '%s' 型の変数 '%s' に互換性のない型の値を初期化しようとしました。\n", node->line, type_name, var_name);
                    exit(EXIT_FAILURE);
                }
                // bool (1/0) は int に変換可能
                if (initial_value.type == VALUE_TYPE_BOOL) {
                    initial_value.type = VALUE_TYPE_INT;
                    initial_value.data.int_value = initial_value.data.bool_value ? 1 : 0;
                }
                define_symbol(env, var_name, initial_value);
            } else if (strcmp(type_name, "str") == 0) {
                if (initial_value.type != VALUE_TYPE_STR) {
                    fprintf(stderr, "実行時エラー (行 %d): '%s' 型の変数 '%s' に互換性のない型の値を初期化しようとしました。\n", node->line, type_name, var_name);
                    exit(EXIT_FAILURE);
                }
                define_symbol(env, var_name, initial_value);
            } else if (strcmp(type_name, "double") == 0) {
                if (initial_value.type != VALUE_TYPE_DOUBLE && initial_value.type != VALUE_TYPE_INT && initial_value.type != VALUE_TYPE_BOOL) {
                    fprintf(stderr, "実行時エラー (行 %d): '%s' 型の変数 '%s' に互換性のない型の値を初期化しようとしました。\n", node->line, type_name, var_name);
                    exit(EXIT_FAILURE);
                }
                // int/bool から double への暗黙の変換を許可
                initial_value = convert_value_to_double(initial_value);
                define_symbol(env, var_name, initial_value);
            } else if (strcmp(type_name, "bool") == 0) {
                if (initial_value.type != VALUE_TYPE_BOOL && initial_value.type != VALUE_TYPE_INT) {
                    fprintf(stderr, "実行時エラー (行 %d): '%s' 型の変数 '%s' に互換性のない型の値を初期化しようとしました。\n", node->line, type_name, var_name);
                    exit(EXIT_FAILURE);
                }
                // int (1/0) から bool へ
                if (initial_value.type == VALUE_TYPE_INT) {
                    initial_value.type = VALUE_TYPE_BOOL;
                    initial_value.data.bool_value = (initial_value.data.int_value != 0); // 0はFalse, それ以外はTrue
                }
                define_symbol(env, var_name, initial_value);
            }
            else {
                fprintf(stderr, "実行時エラー (行 %d): 不明な型 '%s' です。\n", node->line, type_name);
                exit(EXIT_FAILURE);
            }
            break;
        }
        case NODE_ASSIGNMENT: { 
            const char *var_name = node->data.assignment.name;
            ASTNode *value_expr = node->data.assignment.value;

            SymbolEntry *entry = get_symbol(env, var_name);
            if (entry == NULL) {
                fprintf(stderr, "実行時エラー (行 %d): 未定義の変数 '%s' に代入しようとしました。\n", node->line, var_name);
                exit(EXIT_FAILURE);
            }

            Value new_value = interpret_node(value_expr, env);

            // 型チェックと代入
            if (entry->type == VALUE_TYPE_INT) {
                if (new_value.type == VALUE_TYPE_INT) {
                    entry->value.data.int_value = new_value.data.int_value;
                } else if (new_value.type == VALUE_TYPE_BOOL) { // boolからintへ
                    entry->value.data.int_value = new_value.data.bool_value ? 1 : 0;
                } else {
                    fprintf(stderr, "実行時エラー (行 %d): '%s' 変数に互換性のない型の値を代入しようとしました。\n", node->line, var_name);
                    exit(EXIT_FAILURE);
                }
            } else if (entry->type == VALUE_TYPE_STR) {
                if (new_value.type == VALUE_TYPE_STR) {
                    // 既存の文字列を解放してから新しい文字列をコピー
                    free_value_data(entry->value);
                    entry->value.data.str_value = new_value.data.str_value; // strdupされたものがそのまま来る
                } else {
                    fprintf(stderr, "実行時エラー (行 %d): '%s' 変数に互換性のない型の値を代入しようとしました。\n", node->line, var_name);
                    exit(EXIT_FAILURE);
                }
            } else if (entry->type == VALUE_TYPE_DOUBLE) {
                if (new_value.type == VALUE_TYPE_DOUBLE) {
                    entry->value.data.double_value = new_value.data.double_value;
                } else if (new_value.type == VALUE_TYPE_INT) { // intからdoubleへ
                    entry->value.data.double_value = (double)new_value.data.int_value;
                } else if (new_value.type == VALUE_TYPE_BOOL) { // boolからdoubleへ
                    entry->value.data.double_value = new_value.data.bool_value ? 1.0 : 0.0;
                } else {
                    fprintf(stderr, "実行時エラー (行 %d): '%s' 変数に互換性のない型の値を代入しようとしました。\n", node->line, var_name);
                    exit(EXIT_FAILURE);
                }
            } else if (entry->type == VALUE_TYPE_BOOL) {
                if (new_value.type == VALUE_TYPE_BOOL) {
                    entry->value.data.bool_value = new_value.data.bool_value;
                } else if (new_value.type == VALUE_TYPE_INT) { // int (1/0)からboolへ
                    entry->value.data.bool_value = (new_value.data.int_value != 0);
                } else {
                    fprintf(stderr, "実行時エラー (行 %d): '%s' 変数に互換性のない型の値を代入しようとしました。\n", node->line, var_name);
                    exit(EXIT_FAILURE);
                }
            }
            else {
                fprintf(stderr, "実行時エラー (行 %d): '%s' 変数への代入がサポートされていない型です。\n", node->line, var_name);
                exit(EXIT_FAILURE);
            }
            break;
        }
        case NODE_IDENTIFIER_EXPR: { 
            SymbolEntry *entry = get_symbol(env, node->data.identifier_expr.name);
            if (entry == NULL) {
                fprintf(stderr, "実行時エラー (行 %d): 未定義の識別子 '%s' です。\n", node->line, node->data.identifier_expr.name);
                exit(EXIT_FAILURE);
            }
            result = entry->value;
            if (result.type == VALUE_TYPE_STR && result.data.str_value != NULL) {
                result.data.str_value = strdup(result.data.str_value);
            }
            break;
        }
        case NODE_ADD:
        case NODE_SUBTRACT:
        case NODE_MULTIPLY:
        case NODE_DIVIDE: { 
            Value left_val = interpret_node(node->data.binary_expr.left, env);
            Value right_val = interpret_node(node->data.binary_expr.right, env);

            // 演算の型を決定: どちらかがdoubleなら結果もdouble、両方intなら結果もint
            bool use_double = (left_val.type == VALUE_TYPE_DOUBLE || right_val.type == VALUE_TYPE_DOUBLE);

            if (use_double) {
                Value d_left = convert_value_to_double(left_val);
                Value d_right = convert_value_to_double(right_val);
                result.type = VALUE_TYPE_DOUBLE;
                switch (node->type) {
                    case NODE_ADD:
                        result.data.double_value = d_left.data.double_value + d_right.data.double_value;
                        break;
                    case NODE_SUBTRACT:
                        result.data.double_value = d_left.data.double_value - d_right.data.double_value;
                        break;
                    case NODE_MULTIPLY:
                        result.data.double_value = d_left.data.double_value * d_right.data.double_value;
                        break;
                    case NODE_DIVIDE:
                        if (d_right.data.double_value == 0.0) {
                            fprintf(stderr, "実行時エラー (行 %d): 0による除算です。\n", node->line);
                            exit(EXIT_FAILURE);
                        }
                        result.data.double_value = d_left.data.double_value / d_right.data.double_value;
                        break;
                    default: break;
                }
            } else if (left_val.type == VALUE_TYPE_INT && right_val.type == VALUE_TYPE_INT) {
                result.type = VALUE_TYPE_INT;
                switch (node->type) {
                    case NODE_ADD:
                        result.data.int_value = left_val.data.int_value + right_val.data.int_value;
                        break;
                    case NODE_SUBTRACT:
                        result.data.int_value = left_val.data.int_value - right_val.data.int_value;
                        break;
                    case NODE_MULTIPLY:
                        result.data.int_value = left_val.data.int_value * right_val.data.int_value;
                        break;
                    case NODE_DIVIDE:
                        if (right_val.data.int_value == 0) {
                            fprintf(stderr, "実行時エラー (行 %d): 0による除算です。\n", node->line);
                            exit(EXIT_FAILURE);
                        }
                        result.data.int_value = left_val.data.int_value / right_val.data.int_value;
                        break;
                    default: break;
                }
            } else {
                fprintf(stderr, "実行時エラー (行 %d): 算術演算子に互換性のない型です。\n", node->line);
                exit(EXIT_FAILURE);
            }
            free_value_data(left_val); // 中間結果の文字列があれば解放
            free_value_data(right_val); // 中間結果の文字列があれば解放
            break;
        }
        default: 
            fprintf(stderr, "実行時エラー (行 %d): 未知のASTノードタイプ: %d\n", node->line, node->type);
            exit(EXIT_FAILURE);
    }
    return result;
}

// ASTを解釈するエントリポイント
void interpret_ast(ASTNode *program_node) {
    Environment *global_env = create_environment(NULL); // グローバルスコープ

    // プログラム内の全てのトップレベル文（関数定義など）を処理し、シンボルテーブルに登録
    // この段階では関数は「定義」されるだけで「実行」はされない
    if (program_node->type == NODE_PROGRAM) {
        for (int i = 0; i < program_node->data.program.num_statements; i++) {
            interpret_node(program_node->data.program.statements[i], global_env);
        }
    }

    // ここで 'main' 関数を検索し、存在すれば呼び出す
    SymbolEntry *main_func_entry = get_symbol(global_env, "main");
    if (main_func_entry != NULL && main_func_entry->type == VALUE_TYPE_FUNCTION) {
        // main 関数呼び出しのASTノードを仮想的に作成
        ASTNode *main_call_node = create_ast_node(NODE_FUNCTION_CALL, 0); // 行番号は適当
        main_call_node->data.func_call.function_name = strdup("main");
        // main関数は引数なしを想定
        main_call_node->data.func_call.arguments = NULL; 
        main_call_node->data.func_call.num_arguments = 0;
        main_call_node->data.func_call.capacity_arguments = 0;
        
        // main 関数を実行
        Value return_value = interpret_node(main_call_node, global_env);
        
        // main関数の戻り値が存在する場合は表示
        if (return_value.type != VALUE_TYPE_VOID) {
            print_value(return_value, -1); // mainの戻り値は通常精度で表示
            free_value_data(return_value); // 文字列の場合の解放
        }

        // 仮想ノードの解放
        free(main_call_node->data.func_call.function_name);
        free(main_call_node);
    } 

    destroy_environment(global_env);
}