cmake_minimum_required(VERSION 2.8)
project(ulcer)

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -std=c89 -lm")

set(SOURCE_FILES
        src/alloc.c
        src/array.c
        src/cstring.c
        src/environment.c
        src/error.c
        src/evaluator.c
        src/executor.c
        src/expression.c
        src/hash_table.c
        src/hashfn.c
        src/heap.c
        src/lexer.c
        src/parser.c
        src/module.c
        src/native.c
        src/source_code.c
        src/statement.c
        src/token.c
        src/main.c
        src/libfile.c
        src/libheap.c
        src/libmath.c
        src/libnative.c
        src/libruntime.c
        src/libstr.c
        src/list.c
)

add_executable(ulcer ${SOURCE_FILES})