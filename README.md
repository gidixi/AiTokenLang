# AiTokenLang

gcc main.c lexer.c parser.c ast.c codegen.c symbol_table.c -o compiler

./compiler test.atl


nasm -f elf64 output.asm 
ld -o program output.asm
./program
