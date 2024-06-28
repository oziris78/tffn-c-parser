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

void inc_g_num(TFFNStrBuilder* sb) {
    char str[10];
    sprintf(str, "%d", global_num);
    global_num++;

    tffn_sb_append_nterm(sb, str);
}

void parser_tests() {
    char* str = NULL;
    TFFNParser* parser = tffn_parser_new();
    tffn_parser_define_dynamic_action(parser, "inc", inc_g_num);
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

    parser_tests();
    string_builder_tests();
    // htable_tests();

    printf("ALL TESTS PASSES!!!!\n");
    return 0;
}


