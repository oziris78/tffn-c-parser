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


#define expect_equal(exp, act) expect_equal_inner((exp), (act), __FILE__, __LINE__)
#define check_err(exp, et) expect_equal((exp), TFFN_ERR_TO_STR(et))
#define check_err_with_param(exp, et, str) expect_equal((exp), TFFN_ERR_TO_STR(et, str))


void expect_equal_inner(const char* exp, const char* act, const char* file, int line) {
    if(exp == NULL && act == NULL) return;
    if(strcmp(exp, act) == 0) return;
    printf("\n---------------------------\n");
    printf("TEST FAILS!\nPlace: '%s:%d'\nExpected: '%s'\nReceived: '%s'\n", file, line, exp, act);
    printf("---------------------------\n");
    exit(78);
}


// ------------------------------------------------------------ //

void string_builder_tests();
void step_struct_tests();
void error_type_tests();


int main() {
    string_builder_tests();
    step_struct_tests();
    error_type_tests();

    printf("ALL TESTS PASSES!!!!\n");
    return 0;
}


// ------------------------------------------------------------ //



// For dynamic step testing
void empty_func(TFFNStrBuilder *sb) {}


void step_struct_tests() {
    // DYNAMIC
    {
        TFFNStep *step = tffn_step_new_dynamic(empty_func);
        assert(step != NULL);
        assert(step->dynamic_step == empty_func);
        assert(step->static_step == NULL);
        TFFN_FREE(step);
    }
    // STATIC
    {
        const char *static_str = "Static Step";
        TFFNStep *step = tffn_step_new_static(static_str);
        assert(step != NULL);
        assert(step->dynamic_step == NULL);
        assert(strcmp(step->static_step, static_str) == 0);
        TFFN_FREE(step);
    }
}


void error_type_tests() {
    check_err(
        "INVALID FORMAT: you forgot to open a bracket\n",
        TFFN_ERR_DANGLING_CLOSE_BRACKET
    );

    check_err(
        "INVALID FORMAT: nesting brackets are prohibited in TFFN\n",
        TFFN_ERR_NESTING_BRACKETS
    );

    check_err(
        "INVALID FORMAT: format string cant end with '!'\n", 
        TFFN_ERR_DANGLING_IGNORE_TOKEN
    );

    check_err(
        "INVALID FORMAT: you forgot to close a bracket\n", 
        TFFN_ERR_UNCLOSED_BRACKET
    );
    
    check_err(
        "INVALID FORMAT: '!' token cant be used inside brackets\n", 
        TFFN_ERR_IGNORE_TOKEN_INSIDE_BRACKET
    );

    check_err_with_param(
        "INVALID FORMAT: 'actionnnn' action was never defined to the parser\n",
        TFFN_ERR_UNDEFINED_ACTION, "actionnnn"
    );

    check_err_with_param(
        "An action with 'daksjjakdsjdkas' name already exists!\n",
        TFFN_ERR_ACTION_TEXT_ALREADY_EXISTS, "daksjjakdsjdkas"
    );

    check_err(NULL, TFFN_ERR_NONE);
}


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

