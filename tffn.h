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


#ifndef TFFN_H
#define TFFN_H


#ifndef TFFN_ASSERT
    #define TFFN_ASSERT(x) assert((x))
#endif

#ifndef TFFN_MALLOC
    #define TFFN_MALLOC(x) malloc((x))
#endif

#ifndef TFFN_REALLOC
    #define TFFN_REALLOC(x, y) realloc((x), (y))
#endif

#ifndef TFFN_CALLOC
    #define TFFN_CALLOC(x, y) calloc((x), (y))
#endif

#ifndef TFFN_FREE
    #define TFFN_FREE(x) free((x))
#endif

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>



typedef struct _TFFNStrBuilder {
    char* buffer;     // not NULL terminated
    size_t count;     // how many letters are there in the buffer?
    size_t capacity;  // maximum amount of letters that can fit into buffer
} TFFNStrBuilder;

TFFNStrBuilder* tffn_sb_new(size_t);
void tffn_sb_append_sized(TFFNStrBuilder*, const char*, size_t);
void tffn_sb_append_nterm(TFFNStrBuilder*, const char*);
void tffn_sb_append_char(TFFNStrBuilder*, char);
void tffn_sb_clear(TFFNStrBuilder*);
void tffn_sb_free(TFFNStrBuilder*);
char* tffn_sb_to_str(TFFNStrBuilder*);

typedef struct _TFFNEntry {
    char* key; // must be NULL terminated!
    size_t key_length;
    void* object;
    struct _TFFNEntry* next;
} __TFFNEntry;

typedef struct _TFFNHashTable {
    uint32_t table_size;
    __TFFNEntry** entries;
} __TFFNHashTable;

typedef struct _TFFNFuncEntry {
    char* key; // must be NULL terminated!
    size_t key_length;
    void(*func)(TFFNStrBuilder*);
    struct _TFFNFuncEntry* next;
} __TFFNFuncEntry;

typedef struct _TFFNFuncHashTable {
    uint32_t table_size;
    __TFFNFuncEntry** entries;
} __TFFNFuncHashTable;

typedef struct _TFFNStep {
    void (*dynamic_step)(TFFNStrBuilder*); // function to run
    const char* static_step; // already existing string to replace
    struct _TFFNStep* next;
} __TFFNStep;


typedef struct _TFFNParser {
    __TFFNFuncHashTable* dynamic_actions;  // Objects are "void(*func)(TFFNStrBuilder*)"
    __TFFNHashTable* static_actions;       // Objects are "char*"
    __TFFNHashTable* format_cache;         // Objects are "__TFFNStep*"
    TFFNStrBuilder* sb_err;                // not NULL if an exception happened
    TFFNStrBuilder* sb_res;                // for speed
    TFFNStrBuilder* sb_part;               // for speed
    TFFNStrBuilder* sb_brack;              // for speed
} TFFNParser;

TFFNParser* tffn_parser_new();
bool tffn_parser_okay(TFFNParser*);
void tffn_parser_define_static_action(TFFNParser*, char*, char*);
void tffn_parser_define_dynamic_action(TFFNParser*, char*, void(*f)(TFFNStrBuilder*));
char* tffn_parser_parse(TFFNParser*, const char*);
char* tffn_parser_err_msg(TFFNParser*);
void tffn_parser_free(TFFNParser*);


#endif // TFFN_H

// ------------------------------------------------------------ //


#ifdef TFFN_IMPLEMENTATION

#ifdef __cplusplus
extern "C" {  // prevents name mangling of functions when used in C++
#endif


TFFNStrBuilder* tffn_sb_new(size_t initial_capacity) {
    TFFN_ASSERT(initial_capacity > 0);
    TFFNStrBuilder* sb = (TFFNStrBuilder*) TFFN_MALLOC(sizeof(TFFNStrBuilder));
    TFFN_ASSERT(sb != NULL && "Couldn't allocate memory");

    sb->count = 0;
    sb->capacity = initial_capacity;
    sb->buffer = (char*) TFFN_CALLOC(sb->capacity, sizeof(char));
    TFFN_ASSERT(sb->buffer != NULL && "Couldn't allocate memory");

    return sb;
}


void tffn_sb_append_sized(TFFNStrBuilder* sb, const char* buffer, size_t char_count) {
    if (buffer == NULL || char_count <= 0) return; // nothing to append

    // The given string doesnt fit into the current buffer, so increase the buffer capacity
    int need_to_realloc = 0;
    while(sb->count + char_count > sb->capacity) {
        sb->capacity *= 2;
        need_to_realloc = 1;
    }

    if(need_to_realloc == 1) {
        sb->buffer = TFFN_REALLOC(sb->buffer, sb->capacity * sizeof(char));
        TFFN_ASSERT(sb->buffer != NULL && "Couldn't allocate memory");
    }

    memcpy(sb->buffer + sb->count, buffer, char_count * sizeof(char));
    sb->count += char_count;
}


void tffn_sb_append_char(TFFNStrBuilder* sb, char c) {
    if (sb == NULL) return; // Handle NULL pointer

    if (sb->count + 1 > sb->capacity) {
        sb->capacity *= 2;
        sb->buffer = TFFN_REALLOC(sb->buffer, sb->capacity * sizeof(char));
        TFFN_ASSERT(sb->buffer != NULL && "Couldn't allocate memory");
    }

    sb->buffer[sb->count] = c;
    sb->count++;
}


void tffn_sb_append_nterm(TFFNStrBuilder* sb, const char* str) {
    if(sb == NULL) return;
    tffn_sb_append_sized(sb, str, strlen(str));
}


void tffn_sb_clear(TFFNStrBuilder* sb) {
    if(sb != NULL) {
        sb->count = 0;
    }
}


void tffn_sb_free(TFFNStrBuilder* sb) {
    if (sb == NULL) return;

    if (sb->buffer != NULL) {
        TFFN_FREE(sb->buffer);
    }
    TFFN_FREE(sb);
}


char* tffn_sb_to_str(TFFNStrBuilder* sb) {
    if (sb == NULL) return NULL;

    char* res = (char*) TFFN_CALLOC((sb->count+1), sizeof(char));
    TFFN_ASSERT(res != NULL && "Couldn't allocate memory");
    for (size_t i = 0; i < sb->count; i++) {
        res[i] = sb->buffer[i];
    }
    res[sb->count] = '\0';
    return res;
}


static size_t __tffn_htable_str_to_index(uint32_t table_size, const char* str) {
    uint64_t hash = 0;

    size_t str_length = strlen(str);
    for (size_t i = 0; i < str_length; i++) {
        hash *= 17;
        hash += str[i];
    }

    // Apply the murmur hash algorithm onto the result, the following implementation was
    // heavily inspired by this public domain code which was written by Austin Appleby:
    // https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
    hash ^= hash >> 33;
    hash *= 0xFF51AFD7ED558CCDL;
    hash ^= hash >> 33;
    hash *= 0xC4CEB9FE1A85EC53L;
    hash ^= hash >> 33;

    // Compute the index which is in range [0, ht->table_size)
    size_t index = hash % table_size;
    return index;
}


static void (*__tffn_fhtable_lookup(__TFFNFuncHashTable* ht, const char* key))(TFFNStrBuilder*) {
    TFFN_ASSERT(ht != NULL);
    TFFN_ASSERT(key != NULL);

    size_t index = __tffn_htable_str_to_index(ht->table_size, key);
    __TFFNFuncEntry* temp = ht->entries[index];
    while(temp != NULL) {
        if(strcmp(temp->key, key) == 0) break;
        temp = temp->next;
    }

    if(temp == NULL) return NULL;
    return temp->func;
}


static void* __tffn_htable_lookup(__TFFNHashTable* ht, const char* key) {
    TFFN_ASSERT(ht != NULL);
    TFFN_ASSERT(key != NULL);

    size_t index = __tffn_htable_str_to_index(ht->table_size, key);
    __TFFNEntry* temp = ht->entries[index];
    while(temp != NULL) {
        if(strcmp(temp->key, key) == 0) break;
        temp = temp->next;
    }

    if(temp == NULL) return NULL;
    return temp->object;
}


static void __tffn_htable_insert(__TFFNHashTable* ht, const char* key, void* object) {
    TFFN_ASSERT(ht != NULL);
    TFFN_ASSERT(key != NULL);
    
    // Do nothing if the object being inserted is NULL
    if(object == NULL) return;

    // Do nothing if the key already exists
    size_t str_length = strlen(key);
    if(__tffn_htable_lookup(ht, key) != NULL) return;

    // Create new entry
    __TFFNEntry* entry = (__TFFNEntry*) TFFN_MALLOC(sizeof(__TFFNEntry));
    TFFN_ASSERT(entry != NULL && "Couldn't allocate memory");
    entry->object = object;
    entry->key = (char*) TFFN_MALLOC(str_length+1);
    TFFN_ASSERT(entry->key != NULL && "Couldn't allocate memory");
    for(size_t i = 0; i < str_length; i++) {
        entry->key[i] = key[i];
    }
    entry->key[str_length] = '\0';
    entry->key_length = str_length;

    // Insert new entry
    size_t index = __tffn_htable_str_to_index(ht->table_size, key);
    entry->next = ht->entries[index];
    ht->entries[index] = entry;
}


static void __tffn_fhtable_insert(__TFFNFuncHashTable* ht, const char* key, void(*func)(TFFNStrBuilder*)) {
    TFFN_ASSERT(ht != NULL);
    TFFN_ASSERT(key != NULL);
    TFFN_ASSERT(func != NULL);

    // Do nothing if the key already exists
    size_t str_length = strlen(key);
    if(__tffn_fhtable_lookup(ht, key) != NULL) return;

    // Create new entry
    __TFFNFuncEntry* entry = (__TFFNFuncEntry*) TFFN_MALLOC(sizeof(__TFFNFuncEntry));
    TFFN_ASSERT(entry != NULL && "Couldn't allocate memory");
    entry->func = func;
    entry->key = (char*) TFFN_MALLOC(str_length + 1);
    TFFN_ASSERT(entry->key != NULL && "Couldn't allocate memory");
    for(size_t i = 0; i < str_length; i++) {
        entry->key[i] = key[i];
    }
    entry->key[str_length] = '\0';
    entry->key_length = str_length;

    // Insert new entry
    size_t index = __tffn_htable_str_to_index(ht->table_size, key);
    entry->next = ht->entries[index];
    ht->entries[index] = entry;
}


static int __tffn_parser_contains_act_text(TFFNParser* parser, char* act_text) {
    void(*dynamic_obj)(TFFNStrBuilder*);
    dynamic_obj = __tffn_fhtable_lookup(parser->dynamic_actions, act_text);

    void* static_obj = __tffn_htable_lookup(parser->static_actions, act_text);
    
    if(dynamic_obj != NULL || static_obj != NULL) {
        tffn_sb_clear(parser->sb_err);

        char buffer[512];
        sprintf(buffer, "An action with '%s' name already exists!", act_text);
        tffn_sb_append_nterm(parser->sb_err, buffer);
        return 1;
    }
    
    return 0;
}


static void __tffn_append_static_step(__TFFNStep** steps_head, char* static_str) {
    __TFFNStep* s = (__TFFNStep*) TFFN_MALLOC(sizeof(__TFFNStep));
    TFFN_ASSERT(s != NULL && "Couldn't allocate memory");
    s->dynamic_step = NULL;
    s->static_step = static_str;
    s->next = NULL;

    if(*steps_head == NULL) { // Set as first element
        *steps_head = s;
    }
    else { // Append to the end
        __TFFNStep* last = *steps_head;
        while(last->next != NULL) { last = last->next; }
        last->next = s;
    }
}


static void __tffn_append_dynamic_step(__TFFNStep** steps_head, void(*dynamic_act)(TFFNStrBuilder*)) {
    __TFFNStep* s = (__TFFNStep*) TFFN_MALLOC(sizeof(__TFFNStep));
    TFFN_ASSERT(s != NULL && "Couldn't allocate memory");
    s->dynamic_step = dynamic_act;
    s->static_step = NULL;
    s->next = NULL;

    if(*steps_head == NULL) { // Set as first element
        *steps_head = s;
    }
    else { // Append to the end
        __TFFNStep* last = *steps_head;
        while(last->next != NULL) { last = last->next; }
        last->next = s;
    }
}


static __TFFNStep* __tffn_parse_steps(TFFNParser* parser, const char* format) {
    tffn_sb_clear(parser->sb_part);
    tffn_sb_clear(parser->sb_brack);

    __TFFNStep* steps_head = NULL;
    int format_len = strlen(format);
    
    bool in_brack = false;

    int i = 0;
    while(i < format_len) {
        char c = format[i];

        switch (c) {
            case '[': {
                if(in_brack) {
                    tffn_sb_clear(parser->sb_err);
                    tffn_sb_append_nterm(
                        parser->sb_err, "INVALID FORMAT: nesting brackets are prohibited in TFFN"
                    );
                    return NULL;
                }

                in_brack = true;
                i++;
            } break;

            case ']': {
                if(!in_brack) {
                    tffn_sb_clear(parser->sb_err);
                    tffn_sb_append_nterm(
                        parser->sb_err, "INVALID FORMAT: you forgot to open a bracket"
                    );
                    return NULL;
                }

                in_brack = false;

                char* brack_content = tffn_sb_to_str(parser->sb_brack);
                tffn_sb_clear(parser->sb_brack);

                void* static_action = __tffn_htable_lookup(parser->static_actions, brack_content);
                
                void(*dynamic_action)(TFFNStrBuilder*);
                dynamic_action = __tffn_fhtable_lookup(parser->dynamic_actions, brack_content);

                if(static_action != NULL) {
                    char* step = (char*) static_action;
                    tffn_sb_append_nterm(parser->sb_part, step);
                }
                else if(dynamic_action != NULL) {
                    if(parser->sb_part->count > 0) {
                        char* static_str = tffn_sb_to_str(parser->sb_part);
                        __tffn_append_static_step(&steps_head, static_str);
                        tffn_sb_clear(parser->sb_part);
                    }
                    
                    __tffn_append_dynamic_step(&steps_head, dynamic_action);
                }
                else {
                    tffn_sb_clear(parser->sb_err);
                    char buffer[512];
                    sprintf(buffer, "INVALID FORMAT: '%s' action was never defined to the parser", brack_content);
                    tffn_sb_append_nterm(parser->sb_err, buffer);
                    TFFN_FREE(brack_content);
                    return NULL;
                }

                TFFN_FREE(brack_content);
                i++;
            } break;

            case '!': {
                if(in_brack) {
                    tffn_sb_clear(parser->sb_err);
                    tffn_sb_append_nterm(
                        parser->sb_err, "INVALID FORMAT: '!' token cant be used inside brackets"
                    );
                    return NULL;
                }
                
                if(i == format_len - 1) {
                    tffn_sb_clear(parser->sb_err);
                    tffn_sb_append_nterm(
                        parser->sb_err, "INVALID FORMAT: format string cant end with '!'"
                    );
                    return NULL;
                }

                tffn_sb_append_char(parser->sb_part, format[i+1]);
                i += 2;
            } break;

            default: {
                if(in_brack) {
                    tffn_sb_append_char(parser->sb_brack, c);
                }
                else {
                    tffn_sb_append_char(parser->sb_part, c);
                }

                i++;
            } break;
        }
    }

    // The format string ended but bracketText isnt empty so the last bracket was never closed
    if(parser->sb_brack->count != 0) {
        tffn_sb_clear(parser->sb_err);
        tffn_sb_append_nterm(
            parser->sb_err, "INVALID FORMAT: you forgot to close a bracket"
        );
        return NULL;
    }

    // Add the final static string part as a step
    if(parser->sb_part->count > 0) {
        char* static_str = tffn_sb_to_str(parser->sb_part);
        __tffn_append_static_step(&steps_head, static_str);
        tffn_sb_clear(parser->sb_part);
    }

    __tffn_htable_insert(parser->format_cache, format, (void*) steps_head);
    return steps_head;
}


char* tffn_parser_err_msg(TFFNParser* parser) {
    if (parser == NULL) return NULL;

    return tffn_sb_to_str(parser->sb_err);
}


void tffn_parser_free(TFFNParser* parser) {
    if(parser == NULL) return;

    // Free parser->format_cache
    if (parser->format_cache != NULL) {
        for (size_t i = 0; i < parser->format_cache->table_size; i++) {
            __TFFNEntry* temp = parser->format_cache->entries[i];
            while (temp != NULL) {
                __TFFNEntry* next = temp->next;
                TFFN_FREE(temp->key);
                TFFN_FREE(temp);
                temp = next;
            }
        }

        TFFN_FREE(parser->format_cache->entries);
        TFFN_FREE(parser->format_cache);
    }
    
    // Free parser->static_actions
    if (parser->static_actions != NULL) {
        for (size_t i = 0; i < parser->static_actions->table_size; i++) {
            __TFFNEntry* temp = parser->static_actions->entries[i];
            while (temp != NULL) {
                __TFFNEntry* next = temp->next;
                TFFN_FREE(temp->key);
                TFFN_FREE(temp);
                temp = next;
            }
        }

        TFFN_FREE(parser->static_actions->entries);
        TFFN_FREE(parser->static_actions);
    }

    // Free parser->dynamic_actions
    if (parser->dynamic_actions != NULL) {
        for (size_t i = 0; i < parser->dynamic_actions->table_size; i++) {
            __TFFNFuncEntry* temp = parser->dynamic_actions->entries[i];
            while (temp != NULL) {
                __TFFNFuncEntry* next = temp->next;
                TFFN_FREE(temp->key);
                TFFN_FREE(temp);
                temp = next;
            }
        }

        TFFN_FREE(parser->dynamic_actions->entries);
        TFFN_FREE(parser->dynamic_actions);
    }

    tffn_sb_free(parser->sb_brack);
    tffn_sb_free(parser->sb_part);
    tffn_sb_free(parser->sb_res);
    tffn_sb_free(parser->sb_err);
    TFFN_FREE(parser);
}


TFFNParser* tffn_parser_new() {
    TFFNParser* parser = (TFFNParser*) TFFN_MALLOC(sizeof(TFFNParser));
    TFFN_ASSERT(parser != NULL && "Couldn't allocate memory");
    
    // Init dynamic actions
    {
        const uint32_t TABLE_SIZE = 128;
        parser->dynamic_actions = (__TFFNFuncHashTable*) TFFN_MALLOC(sizeof(__TFFNFuncHashTable));
        TFFN_ASSERT(parser->dynamic_actions != NULL && "Couldn't allocate memory");

        parser->dynamic_actions->table_size = TABLE_SIZE;
        parser->dynamic_actions->entries = (__TFFNFuncEntry**) TFFN_CALLOC(sizeof(__TFFNFuncEntry*), TABLE_SIZE);
        TFFN_ASSERT(parser->dynamic_actions->entries != NULL && "Couldn't allocate memory");
    }

    // Init static actions
    {
        const uint32_t TABLE_SIZE = 128;
        parser->static_actions = (__TFFNHashTable*) TFFN_MALLOC(sizeof(__TFFNHashTable));
        TFFN_ASSERT(parser->static_actions != NULL && "Couldn't allocate memory");

        parser->static_actions->table_size = TABLE_SIZE;
        parser->static_actions->entries = (__TFFNEntry**) TFFN_CALLOC(sizeof(__TFFNEntry*), TABLE_SIZE);
        TFFN_ASSERT(parser->static_actions->entries != NULL && "Couldn't allocate memory");
    }

    // Init format cache
    {
        const uint32_t TABLE_SIZE = 128;
        parser->format_cache = (__TFFNHashTable*) TFFN_MALLOC(sizeof(__TFFNHashTable));
        TFFN_ASSERT(parser->format_cache != NULL && "Couldn't allocate memory");

        parser->format_cache->table_size = TABLE_SIZE;
        parser->format_cache->entries = (__TFFNEntry**) TFFN_CALLOC(sizeof(__TFFNEntry*), TABLE_SIZE);
        TFFN_ASSERT(parser->format_cache->entries != NULL && "Couldn't allocate memory");
    }

    parser->sb_brack = tffn_sb_new(64);
    parser->sb_part = tffn_sb_new(64);
    parser->sb_err = tffn_sb_new(64);
    parser->sb_res = tffn_sb_new(64);
    return parser;
}


bool tffn_parser_okay(TFFNParser* parser) {
    if (parser == NULL) return false; // parser is literally fucking NULL, do you think its okay?!?
    return parser->sb_err->count == 0; // does the err string exist?
}


void tffn_parser_define_static_action(TFFNParser* parser, char* act_text, char* static_act) {
    if (parser == NULL || static_act == NULL || act_text == NULL) return;
    if (act_text[0] == '\0') return;
    if (__tffn_parser_contains_act_text(parser, act_text)) return;

    tffn_sb_clear(parser->sb_err);
    __tffn_htable_insert(parser->static_actions, act_text, (void*)static_act);
}


void tffn_parser_define_dynamic_action(TFFNParser* parser, char* act_text, void(*dynamic_act)(TFFNStrBuilder*)) {
    if (parser == NULL || dynamic_act == NULL || act_text == NULL) return;
    if (act_text[0] == '\0') return;
    if (__tffn_parser_contains_act_text(parser, act_text)) return;
    
    tffn_sb_clear(parser->sb_err);
    __tffn_fhtable_insert(parser->dynamic_actions, act_text, dynamic_act);
}


char* tffn_parser_parse(TFFNParser* parser, const char* format) {
    TFFN_ASSERT(parser != NULL);
    TFFN_ASSERT(format != NULL);

    if(format[0] == '\0') {
        return ""; // format is empty string
    }

    void* steps_ojb = __tffn_htable_lookup(parser->format_cache, format);
    
    __TFFNStep* step = (__TFFNStep*) steps_ojb;
    if(step == NULL) {
        step = __tffn_parse_steps(parser, format);
        if(step == NULL) return NULL; // parsing error happened
    }

    tffn_sb_clear(parser->sb_res);

    while(step != NULL) {
        if(step->static_step != NULL) {
            tffn_sb_append_nterm(parser->sb_res, step->static_step);
        }
        else if(step->dynamic_step != NULL) { 
            step->dynamic_step(parser->sb_res);
        }
        else {
            TFFN_ASSERT(0 && "This line should have been unreachable!");
        }

        step = step->next;
    }

    char* result_str = tffn_sb_to_str(parser->sb_res);
    tffn_sb_clear(parser->sb_res);
    return result_str;
}



#ifdef __cplusplus
}  // closing the name mangling fix paranthesis for C++
#endif

#endif // TFFN_IMPLEMENTATION


//  -----------------------------------------------------------------------  //
//  --  This file is licensed under the terms of the Apache-2.0 license  --  //
//  --  You can find the full text of this license down below            --  //
//  -----------------------------------------------------------------------  //
//
//                                 Apache License
//                           Version 2.0, January 2004
//                        http://www.apache.org/licenses/
//
//   TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION
//
//   1. Definitions.
//
//      "License" shall mean the terms and conditions for use, reproduction,
//      and distribution as defined by Sections 1 through 9 of this document.
//
//      "Licensor" shall mean the copyright owner or entity authorized by
//      the copyright owner that is granting the License.
//
//      "Legal Entity" shall mean the union of the acting entity and all
//      other entities that control, are controlled by, or are under common
//      control with that entity. For the purposes of this definition,
//      "control" means (i) the power, direct or indirect, to cause the
//      direction or management of such entity, whether by contract or
//      otherwise, or (ii) ownership of fifty percent (50%) or more of the
//      outstanding shares, or (iii) beneficial ownership of such entity.
//
//      "You" (or "Your") shall mean an individual or Legal Entity
//      exercising permissions granted by this License.
//
//      "Source" form shall mean the preferred form for making modifications,
//      including but not limited to software source code, documentation
//      source, and configuration files.
//
//      "Object" form shall mean any form resulting from mechanical
//      transformation or translation of a Source form, including but
//      not limited to compiled object code, generated documentation,
//      and conversions to other media types.
//
//      "Work" shall mean the work of authorship, whether in Source or
//      Object form, made available under the License, as indicated by a
//      copyright notice that is included in or attached to the work
//      (an example is provided in the Appendix below).
//
//      "Derivative Works" shall mean any work, whether in Source or Object
//      form, that is based on (or derived from) the Work and for which the
//      editorial revisions, annotations, elaborations, or other modifications
//      represent, as a whole, an original work of authorship. For the purposes
//      of this License, Derivative Works shall not include works that remain
//      separable from, or merely link (or bind by name) to the interfaces of,
//      the Work and Derivative Works thereof.
//
//      "Contribution" shall mean any work of authorship, including
//      the original version of the Work and any modifications or additions
//      to that Work or Derivative Works thereof, that is intentionally
//      submitted to Licensor for inclusion in the Work by the copyright owner
//      or by an individual or Legal Entity authorized to submit on behalf of
//      the copyright owner. For the purposes of this definition, "submitted"
//      means any form of electronic, verbal, or written communication sent
//      to the Licensor or its representatives, including but not limited to
//      communication on electronic mailing lists, source code control systems,
//      and issue tracking systems that are managed by, or on behalf of, the
//      Licensor for the purpose of discussing and improving the Work, but
//      excluding communication that is conspicuously marked or otherwise
//      designated in writing by the copyright owner as "Not a Contribution."
//
//      "Contributor" shall mean Licensor and any individual or Legal Entity
//      on behalf of whom a Contribution has been received by Licensor and
//      subsequently incorporated within the Work.
//
//   2. Grant of Copyright License. Subject to the terms and conditions of
//      this License, each Contributor hereby grants to You a perpetual,
//      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
//      copyright license to reproduce, prepare Derivative Works of,
//      publicly display, publicly perform, sublicense, and distribute the
//      Work and such Derivative Works in Source or Object form.
//
//   3. Grant of Patent License. Subject to the terms and conditions of
//      this License, each Contributor hereby grants to You a perpetual,
//      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
//      (except as stated in this section) patent license to make, have made,
//      use, offer to sell, sell, import, and otherwise transfer the Work,
//      where such license applies only to those patent claims licensable
//      by such Contributor that are necessarily infringed by their
//      Contribution(s) alone or by combination of their Contribution(s)
//      with the Work to which such Contribution(s) was submitted. If You
//      institute patent litigation against any entity (including a
//      cross-claim or counterclaim in a lawsuit) alleging that the Work
//      or a Contribution incorporated within the Work constitutes direct
//      or contributory patent infringement, then any patent licenses
//      granted to You under this License for that Work shall terminate
//      as of the date such litigation is filed.
//
//   4. Redistribution. You may reproduce and distribute copies of the
//      Work or Derivative Works thereof in any medium, with or without
//      modifications, and in Source or Object form, provided that You
//      meet the following conditions:
//
//      (a) You must give any other recipients of the Work or
//          Derivative Works a copy of this License; and
//
//      (b) You must cause any modified files to carry prominent notices
//          stating that You changed the files; and
//
//      (c) You must retain, in the Source form of any Derivative Works
//          that You distribute, all copyright, patent, trademark, and
//          attribution notices from the Source form of the Work,
//          excluding those notices that do not pertain to any part of
//          the Derivative Works; and
//
//      (d) If the Work includes a "NOTICE" text file as part of its
//          distribution, then any Derivative Works that You distribute must
//          include a readable copy of the attribution notices contained
//          within such NOTICE file, excluding those notices that do not
//          pertain to any part of the Derivative Works, in at least one
//          of the following places: within a NOTICE text file distributed
//          as part of the Derivative Works; within the Source form or
//          documentation, if provided along with the Derivative Works; or,
//          within a display generated by the Derivative Works, if and
//          wherever such third-party notices normally appear. The contents
//          of the NOTICE file are for informational purposes only and
//          do not modify the License. You may add Your own attribution
//          notices within Derivative Works that You distribute, alongside
//          or as an addendum to the NOTICE text from the Work, provided
//          that such additional attribution notices cannot be construed
//          as modifying the License.
//
//      You may add Your own copyright statement to Your modifications and
//      may provide additional or different license terms and conditions
//      for use, reproduction, or distribution of Your modifications, or
//      for any such Derivative Works as a whole, provided Your use,
//      reproduction, and distribution of the Work otherwise complies with
//      the conditions stated in this License.
//
//   5. Submission of Contributions. Unless You explicitly state otherwise,
//      any Contribution intentionally submitted for inclusion in the Work
//      by You to the Licensor shall be under the terms and conditions of
//      this License, without any additional terms or conditions.
//      Notwithstanding the above, nothing herein shall supersede or modify
//      the terms of any separate license agreement you may have executed
//      with Licensor regarding such Contributions.
//
//   6. Trademarks. This License does not grant permission to use the trade
//      names, trademarks, service marks, or product names of the Licensor,
//      except as required for reasonable and customary use in describing the
//      origin of the Work and reproducing the content of the NOTICE file.
//
//   7. Disclaimer of Warranty. Unless required by applicable law or
//      agreed to in writing, Licensor provides the Work (and each
//      Contributor provides its Contributions) on an "AS IS" BASIS,
//      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
//      implied, including, without limitation, any warranties or conditions
//      of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A
//      PARTICULAR PURPOSE. You are solely responsible for determining the
//      appropriateness of using or redistributing the Work and assume any
//      risks associated with Your exercise of permissions under this License.
//
//   8. Limitation of Liability. In no event and under no legal theory,
//      whether in tort (including negligence), contract, or otherwise,
//      unless required by applicable law (such as deliberate and grossly
//      negligent acts) or agreed to in writing, shall any Contributor be
//      liable to You for damages, including any direct, indirect, special,
//      incidental, or consequential damages of any character arising as a
//      result of this License or out of the use or inability to use the
//      Work (including but not limited to damages for loss of goodwill,
//      work stoppage, computer failure or malfunction, or any and all
//      other commercial damages or losses), even if such Contributor
//      has been advised of the possibility of such damages.
//
//   9. Accepting Warranty or Additional Liability. While redistributing
//      the Work or Derivative Works thereof, You may choose to offer,
//      and charge a fee for, acceptance of support, warranty, indemnity,
//      or other liability obligations and/or rights consistent with this
//      License. However, in accepting such obligations, You may act only
//      on Your own behalf and on Your sole responsibility, not on behalf
//      of any other Contributor, and only if You agree to indemnify,
//      defend, and hold each Contributor harmless for any liability
//      incurred by, or claims asserted against, such Contributor by reason
//      of your accepting any such warranty or additional liability.
//
//   END OF TERMS AND CONDITIONS
//
//   APPENDIX: How to apply the Apache License to your work.
//
//      To apply the Apache License to your work, attach the following
//      boilerplate notice, with the fields enclosed by brackets "[]"
//      replaced with your own identifying information. (Don't include
//      the brackets!)  The text should be enclosed in the appropriate
//      comment syntax for the file format. We also recommend that a
//      file or class name and description of purpose be included on the
//      same "printed page" as the copyright notice for easier
//      identification within third-party archives.
//
//   Copyright [yyyy] [name of copyright owner]
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
