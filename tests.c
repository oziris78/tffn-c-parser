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

#define TFFN_IMPLEMENTATION
#include "tffn.h"


#define expect_equal(exp, act) expect_equal_inner(exp, act, __FILE__, __LINE__)

void expect_equal_inner(const char* exp, const char* act, const char* file, int line) {
    if(strcmp(exp, act) == 0) return;
    printf("\n---------------------------\n");
    printf("TEST FAILS!\nPlace: '%s:%d'\nExpected: '%s'\nReceived: '%s'\n", file, line, exp, act);
    printf("---------------------------\n");
    exit(78);
}


// ------------------------------------------------------------ //

void string_builder_tests();


int main() {
    string_builder_tests();

    printf("ALL TESTS PASSES!!!!\n");
    return 0;
}


// ------------------------------------------------------------ //


void string_builder_tests() {
    for (size_t cap = 1; cap < 2000; cap *= 2) {
        TFFNStrBuilder* sb = tffn_sb_new(cap);
        tffn_sb_append(sb, "Hello world!", 12);
        char* str = tffn_sb_to_str(sb);
        expect_equal("Hello world!", str);
        free(str);
        tffn_sb_free(sb);
    }
    
    for (size_t cap = 1; cap < 2000; cap *= 2) {
        TFFNStrBuilder* sb = tffn_sb_new(cap);
        tffn_sb_append(sb, "Hello world!", 12);
        tffn_sb_append(sb, "Hello world!", 12);
        tffn_sb_append(sb, "Hello world!", 12);
        char* str = tffn_sb_to_str(sb);
        expect_equal("Hello world!Hello world!Hello world!", str);
        free(str);
        tffn_sb_free(sb);
    }
    
    for (size_t cap = 1; cap < 2000; cap *= 2) {
        TFFNStrBuilder* sb = tffn_sb_new(cap);
        tffn_sb_append(sb, "Hello world!", 12);
        tffn_sb_clear(sb);
        tffn_sb_append(sb, "Hello world!", 12);
        tffn_sb_append(sb, "Hello world!", 12);
        char* str = tffn_sb_to_str(sb);
        expect_equal("Hello world!Hello world!", str);
        free(str);
        tffn_sb_free(sb);
    }

    {
        TFFNStrBuilder sb = *tffn_sb_new_from("Hello", 5);
        tffn_sb_append(&sb, " world!", 7);
        char* str = tffn_sb_to_str(&sb);
        expect_equal("Hello world!", str);
        free(str);
    }
}

