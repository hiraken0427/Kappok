#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kappok.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("使用方法: %s <ファイル名>\n", argv[0]);
        return 1;
    }
    
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("エラー: ファイル '%s' を開けません\n", argv[1]);
        return 1;
    }
    
    // ファイルサイズを取得
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // ファイル内容を読み込み
    char *source = malloc(file_size + 1);
    if (source == NULL) {
        printf("エラー: メモリ割り当てに失敗しました\n");
        fclose(file);
        return 1;
    }
    fread(source, 1, file_size, file);
    source[file_size] = '\0';
    fclose(file);
    
    // レクサーを作成
    Lexer *lexer = lexer_create(source);
    
    // ASTを構築
    ASTNode *program_node = parse(lexer);
    
    // ASTを解釈・実行
    if (program_node) { // AST構築が成功した場合のみ実行
        interpret_ast(program_node);
        // ASTを解放
        destroy_ast(program_node);
    }
    
    // レクサーを解放
    lexer_destroy(lexer);
    free(source); // ソースコードのメモリを解放
    printf("\n");
    return 0;
}
