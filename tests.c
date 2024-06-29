// Copyright 2024 Oğuzhan Topaloğlu 
//  
// Licensed under the Apache License, Version 2.0 (the "License"); 
// you may not use this file except in compliance with the License. 
// You may obtain a copy of the License at 
//  
//     http://www.apache.org/licenses/LICENSE-2.0 
//  
// Unless required by applicable law or agreed to in writing, software 
// distributed under the License is distributed on an "AS IS" BASIS, 
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
// See the License for the specific language governing permissions and 
// limitations under the License.

// ------------------------------------------------------------ //


#include <stdio.h>
#include <string.h>
#include <time.h>

#define TFFN_IMPLEMENTATION
#include "tffn.h"


#define expect_equal_int(exp, act) expect_equal_int_inner((intmax_t)(exp), (intmax_t)(act), __FILE__, __LINE__)
#define expect_equal_str(exp, act) expect_equal_str_inner((const char*)(exp), (const char*)(act), __FILE__, __LINE__)
#define expect_null(act) expect_null_inner((void*)(act), __FILE__, __LINE__)
#define expect_not_null(act) expect_not_null_inner((void*)(act), __FILE__, __LINE__)
#define fail() fail_inner(__FILE__, __LINE__)


void fail_inner(const char* file, int line) {
    printf("\n---------------------------\n");
    printf("TEST FAILS!\nPlace: '%s:%d'\n", file, line);
    printf("---------------------------\n");
    exit(78);
}

void expect_not_null_inner(void* ptr, const char* file, int line) {
    if(ptr != NULL) return;
    printf("\n---------------------------\n");
    printf("TEST FAILS!\nPlace: '%s:%d'\nExpected NOT NULL but got NULL\n", file, line);
    printf("---------------------------\n");
    exit(78);
}

void expect_null_inner(void* ptr, const char* file, int line) {
    if(ptr == NULL) return;
    printf("\n---------------------------\n");
    printf("TEST FAILS!\nPlace: '%s:%d'\nExpected NULL but got: '%p'\n", file, line, ptr);
    printf("---------------------------\n");
    exit(78);
}

void expect_equal_int_inner(int exp, int act, const char* file, int line) {
    if(exp == act) return;
    printf("\n---------------------------\n");
    printf("TEST FAILS!\nPlace: '%s:%d'\nExpected: '%d'\nReceived: '%d'\n", file, line, exp, act);
    printf("---------------------------\n");
    exit(78);

}

void expect_equal_str_inner(const char* exp, const char* act, const char* file, int line) {
    if(exp == NULL && act == NULL) return;
    if(strcmp(exp, act) == 0) return;
    printf("\n---------------------------\n");
    printf("TEST FAILS!\nPlace: '%s:%d'\nExpected: '%s'\nReceived: '%s'\n", file, line, exp, act);
    printf("---------------------------\n");
    exit(78);
}

char* gen_rand_word() {
    static const char* letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int len = 5 + rand() % 20;
    char* word = (char*) malloc(len + 1);

    for (int i = 0; i < len; ++i) {
        int randomIndex = rand() % len;
        word[i] = letters[randomIndex];
    }
    word[len] = '\0';

    return word;
}



// ------------------------------------------------------------ //


void string_builder_tests() {
    for (size_t cap = 1; cap < 2000; cap *= 2) {
        TFFNStrBuilder* sb = tffn_sb_new(cap);
        tffn_sb_append_sized(sb, "Hello world!", 12);
        char* str = tffn_sb_to_str(sb);
        expect_equal_str("Hello world!", str);
        free(str);
        tffn_sb_free(sb);
    }
    
    for (size_t cap = 1; cap < 2000; cap *= 2) {
        TFFNStrBuilder* sb = tffn_sb_new(cap);
        tffn_sb_append_sized(sb, "Hello world!", 12);
        tffn_sb_append_sized(sb, "Hello world!", 12);
        tffn_sb_append_sized(sb, "Hello world!", 12);
        char* str = tffn_sb_to_str(sb);
        expect_equal_str("Hello world!Hello world!Hello world!", str);
        free(str);
        tffn_sb_free(sb);
    }
    
    for (size_t cap = 1; cap < 2000; cap *= 2) {
        TFFNStrBuilder* sb = tffn_sb_new(cap);
        tffn_sb_append_sized(sb, "Hello world!", 12);
        tffn_sb_clear(sb);
        tffn_sb_append_sized(sb, "Hello world!", 12);
        tffn_sb_append_sized(sb, "Hello world!", 12);
        char* str = tffn_sb_to_str(sb);
        expect_equal_str("Hello world!Hello world!", str);
        free(str);
        tffn_sb_free(sb);
    }
}


int global_num = 0;

void dyn_func_inc_num(TFFNStrBuilder* sb) {
    char str[30];
    sprintf(str, "%d", global_num++);
    tffn_sb_append_nterm(sb, str);
}
void dyn_func_mul_num(TFFNStrBuilder* sb) {
    char str[30];
    sprintf(str, "Check out my counter: %d", global_num);
    global_num *= 2;
    tffn_sb_append_nterm(sb, str); 
}
void dyn_func_dynamic(TFFNStrBuilder* sb) { tffn_sb_append_nterm(sb, "Dynamic Part"); }
void dyn_func_greet(TFFNStrBuilder* sb) { tffn_sb_append_nterm(sb, "Hello, Dynamic World!"); }
void dyn_func_this(TFFNStrBuilder* sb) { tffn_sb_append_nterm(sb, "this will be"); }
void dyn_func_dup(TFFNStrBuilder* sb) { tffn_sb_append_nterm(sb, "Dynamic duplicate"); }


void parser_valid_tests() {
    TFFNParser* parser = NULL;
    
    // Static
    tffn_parser_free(parser); // only frees if not NULL
    parser = tffn_parser_new();
    tffn_parser_define_static_action(parser, "hello", "hello world");
    tffn_parser_define_static_action(parser, "author", "oziris78");

    expect_equal_str("hello world from oziris78", tffn_parser_parse(parser, "[hello] from [author]"));

    // Static & Dynamic
    tffn_parser_free(parser);
    parser = tffn_parser_new();
    tffn_parser_define_static_action(parser, "static", "Static Part");
    tffn_parser_define_dynamic_action(parser, "dynamic", dyn_func_dynamic);
    tffn_parser_define_dynamic_action(parser, "greet", dyn_func_greet);

    expect_equal_str("Static Part Dynamic Part", tffn_parser_parse(parser, "[static] [dynamic]"));
    expect_equal_str("Hello, Dynamic World!", tffn_parser_parse(parser, "[greet]"));

    // Complex Dynamic
    tffn_parser_free(parser);
    parser = tffn_parser_new();
    global_num = 1;
    tffn_parser_define_dynamic_action(parser, "num", dyn_func_mul_num);

    expect_equal_str("Hey! Check out my counter: 1", tffn_parser_parse(parser, "Hey!! [num]"));
    expect_equal_str("Hey! Check out my counter: 2", tffn_parser_parse(parser, "Hey!! [num]"));
    expect_equal_str("Hey! Check out my counter: 4", tffn_parser_parse(parser, "Hey!! [num]"));
    expect_equal_str("Hey! Check out my counter: 8", tffn_parser_parse(parser, "Hey!! [num]"));
    expect_equal_str("Hey! Check out my counter: 16", tffn_parser_parse(parser, "Hey!! [num]"));
    expect_equal_str("Hey! Check out my counter: 32", tffn_parser_parse(parser, "Hey!! [num]"));

    // Escaping
    tffn_parser_free(parser);
    parser = tffn_parser_new();
    tffn_parser_define_static_action(parser, "test", "in brackets!");
    tffn_parser_define_dynamic_action(parser, "this", dyn_func_this);

    expect_equal_str("this will be [in brackets!]", tffn_parser_parse(parser, "[this] ![[test]!]"));
    expect_equal_str("wow!!!", tffn_parser_parse(parser, "wow!!!!!!"));
    expect_equal_str("!!!!", tffn_parser_parse(parser, "!!!!!!!!"));

    // Multiple Dynamic in one statement
    tffn_parser_free(parser);
    parser = tffn_parser_new();
    global_num = 1;
    tffn_parser_define_dynamic_action(parser, "inc", dyn_func_inc_num);

    expect_equal_str("1 2 3", tffn_parser_parse(parser, "[inc] [inc] [inc]"));

    // Parsing an empty format should return an empty string
    tffn_parser_free(parser);
    parser = tffn_parser_new();
    expect_equal_str("", tffn_parser_parse(parser, ""));
}


void parser_invalid_tests() {
    TFFNParser* parser = tffn_parser_new();

    tffn_parser_define_static_action(parser, "nested", "test345");
    tffn_parser_define_static_action(parser, "brackets", "testing123");

    expect_null(tffn_parser_parse(parser, "]"));
    expect_null(tffn_parser_parse(parser, "abc]"));
    expect_null(tffn_parser_parse(parser, "[]]"));
    expect_null(tffn_parser_parse(parser, "!!]"));
    expect_null(tffn_parser_parse(parser, "]!!"));
    expect_null(tffn_parser_parse(parser, "x]"));
    expect_null(tffn_parser_parse(parser, "]x"));

    expect_null(tffn_parser_parse(parser, "[nested[brackets]]"));
    expect_null(tffn_parser_parse(parser, "[nes[brackets]ted]"));
    expect_null(tffn_parser_parse(parser, "[[brackets]]"));

    expect_null(tffn_parser_parse(parser, "Hello World!"));
    expect_null(tffn_parser_parse(parser, "Hello!! World!"));

    expect_null(tffn_parser_parse(parser, "[unclosed"));
    expect_null(tffn_parser_parse(parser, "[nested][unclosed"));

    expect_null(tffn_parser_parse(parser, "[ignore!token]"));
    expect_null(tffn_parser_parse(parser, "[ignore!!token]"));
    expect_null(tffn_parser_parse(parser, "[!!token]"));
    expect_null(tffn_parser_parse(parser, "[token!!]"));
    expect_null(tffn_parser_parse(parser, "[!!]"));
    expect_null(tffn_parser_parse(parser, "[!]"));

    expect_null(tffn_parser_parse(parser, "[]"));
    expect_null(tffn_parser_parse(parser, "[nester]"));
    expect_null(tffn_parser_parse(parser, "[undefined]"));

    tffn_parser_define_static_action(parser, "nested", "Static duplicate");
    expect_equal_str("An action with 'nested' name already exists!", tffn_parser_err_msg(parser));
        
    tffn_parser_define_dynamic_action(parser, "nested", dyn_func_dup);
    expect_equal_str("An action with 'nested' name already exists!", tffn_parser_err_msg(parser));
}


void parser_edge_case_tests() {
    TFFNStrBuilder* sb_temp = tffn_sb_new(2000000);
    TFFNParser* parser = NULL;

    // EXTREMELY LONG ACTION NAME
    tffn_parser_free(parser);
    parser = tffn_parser_new();
    
    tffn_sb_clear(sb_temp);
    for (int i = 0; i < 100000; i++) 
        tffn_sb_append_char(sb_temp, 'a');
    char* long_act_name = tffn_sb_to_str(sb_temp);
    tffn_parser_define_static_action(parser, long_act_name, "Long Action Name");
    
    tffn_sb_clear(sb_temp);
    tffn_sb_append_char(sb_temp, '[');
    tffn_sb_append_nterm(sb_temp, long_act_name);
    tffn_sb_append_char(sb_temp, ']');
    expect_equal_str("Long Action Name", tffn_parser_parse(parser, tffn_sb_to_str(sb_temp)));

    // EXTREMELY LONG ACTION CONTENT
    tffn_parser_free(parser);
    parser = tffn_parser_new();

    tffn_sb_clear(sb_temp);
    for (int i = 0; i < 100000; i++) 
        tffn_sb_append_char(sb_temp, 'b');
    char* long_act_content = tffn_sb_to_str(sb_temp);
    tffn_parser_define_static_action(parser, "longContent", long_act_content);
    expect_equal_str(long_act_content, tffn_parser_parse(parser, "[longContent]"));

    // MULTIPLE CONSECUTIVE BRACKETS
    tffn_parser_free(parser);
    parser = tffn_parser_new();
    tffn_parser_define_static_action(parser, "action1", "First");
    tffn_parser_define_static_action(parser, "action2", "Second");
    expect_equal_str("FirstSecond", tffn_parser_parse(parser, "[action1][action2]"));
}


void parser_tests() {
    char* str = NULL;
    TFFNParser* parser = tffn_parser_new();
    tffn_parser_define_dynamic_action(parser, "inc", dyn_func_inc_num);
    tffn_parser_define_static_action(parser, "author", "oziris78");
    tffn_parser_define_static_action(parser, "hello", "Hello world!");

    str = tffn_parser_parse(parser, "[hello]");
    if(!tffn_parser_okay(parser)) fail();
    expect_equal_str("Hello world!", str);
    free(str);
    
    str = tffn_parser_parse(parser, "[hello] [author] [inc]");
    if(!tffn_parser_okay(parser)) fail();
    expect_equal_str("Hello world! oziris78 0", str);
    free(str);
    
    global_num = 5;
    str = tffn_parser_parse(parser, "[inc] [inc][inc][inc]");
    if(!tffn_parser_okay(parser)) fail();
    expect_equal_str("5 678", str);
    free(str);

    tffn_parser_free(parser);
}



int main() {
    srand(time(NULL));
    printf("Running tests...\n");

    string_builder_tests();
    parser_tests();
    parser_valid_tests();
    parser_invalid_tests();
    parser_edge_case_tests();

    printf("ALL TESTS PASSES!!!!\n");
    return 0;
}


