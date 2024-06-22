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
    const char* letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int len = strlen(letters);
    char* word = (char*)malloc(len + 1);

    for (int i = 0; i < len; ++i) {
        int randomIndex = rand() % len;
        word[i] = letters[randomIndex];
    }
    word[len] = '\0';

    return word;
}



// ------------------------------------------------------------ //

void string_builder_tests();
void step_struct_tests();
void error_type_tests();
void htable_tests();


int main() {
    srand(time(NULL));

    string_builder_tests();
    step_struct_tests();
    error_type_tests();
    htable_tests();

    printf("ALL TESTS PASSES!!!!\n");
    return 0;
}


// ------------------------------------------------------------ //


void htable_tests() {
    for(uint32_t size = 1; size < 2000; size++) {
        TFFNHashTable* ht = tffn_htable_new(size);

        // Insertion
        char* word1 = gen_rand_word();
        char* word2 = gen_rand_word();
        char* word3 = gen_rand_word();
        char* word4 = gen_rand_word();

        expect_equal_int(1, tffn_htable_insert(ht, word1, (void*)45));
        expect_equal_int(1, tffn_htable_insert(ht, word2, (void*)7878));
        expect_equal_int(1, tffn_htable_insert(ht, word3, (void*)3));

        // Lookup
        expect_equal_int(45, tffn_htable_lookup(ht, word1));
        expect_equal_int(7878, tffn_htable_lookup(ht, word2));
        expect_equal_int(3, tffn_htable_lookup(ht, word3));
        expect_null(tffn_htable_lookup(ht, word4));

        // Deletion
        expect_equal_int(7878, tffn_htable_delete(ht, word2));
        expect_null(tffn_htable_lookup(ht, word2));

        // Freeing
        tffn_htable_free(ht);
    }
}



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
    expect_equal_str(
        "INVALID FORMAT: you forgot to open a bracket\n",
        TFFN_ERR_TO_STR(TFFN_ERR_DANGLING_CLOSE_BRACKET)
    );

    expect_equal_str(
        "INVALID FORMAT: nesting brackets are prohibited in TFFN\n",
        TFFN_ERR_TO_STR(TFFN_ERR_NESTING_BRACKETS)
    );

    expect_equal_str(
        "INVALID FORMAT: format string cant end with '!'\n", 
        TFFN_ERR_TO_STR(TFFN_ERR_DANGLING_IGNORE_TOKEN)
    );

    expect_equal_str(
        "INVALID FORMAT: you forgot to close a bracket\n", 
        TFFN_ERR_TO_STR(TFFN_ERR_UNCLOSED_BRACKET)
    );
    
    expect_equal_str(
        "INVALID FORMAT: '!' token cant be used inside brackets\n", 
        TFFN_ERR_TO_STR(TFFN_ERR_IGNORE_TOKEN_INSIDE_BRACKET)
    );

    expect_equal_str(
        "INVALID FORMAT: 'actionnnn' action was never defined to the parser\n",
        TFFN_ERR_TO_STR(TFFN_ERR_UNDEFINED_ACTION, "actionnnn")
    );

    expect_equal_str(
        "An action with 'daksjjakdsjdkas' name already exists!\n",
        TFFN_ERR_TO_STR(TFFN_ERR_ACTION_TEXT_ALREADY_EXISTS, "daksjjakdsjdkas")
    );

    expect_null(TFFN_ERR_TO_STR(TFFN_ERR_NONE));
}


void string_builder_tests() {
    for (size_t cap = 1; cap < 2000; cap *= 2) {
        TFFNStrBuilder* sb = tffn_sb_new(cap);
        tffn_sb_append(sb, "Hello world!", 12);
        char* str = tffn_sb_to_str(sb);
        expect_equal_str("Hello world!", str);
        free(str);
        tffn_sb_free(sb);
    }
    
    for (size_t cap = 1; cap < 2000; cap *= 2) {
        TFFNStrBuilder* sb = tffn_sb_new(cap);
        tffn_sb_append(sb, "Hello world!", 12);
        tffn_sb_append(sb, "Hello world!", 12);
        tffn_sb_append(sb, "Hello world!", 12);
        char* str = tffn_sb_to_str(sb);
        expect_equal_str("Hello world!Hello world!Hello world!", str);
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
        expect_equal_str("Hello world!Hello world!", str);
        free(str);
        tffn_sb_free(sb);
    }

    {
        TFFNStrBuilder sb = *tffn_sb_new_from("Hello", 5);
        tffn_sb_append(&sb, " world!", 7);
        char* str = tffn_sb_to_str(&sb);
        expect_equal_str("Hello world!", str);
        free(str);
    }
}

