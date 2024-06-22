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


#include <stdint.h>


#ifndef TFFN_VA_START
    #include <stdarg.h>
    #define TFFN_VA_START va_start
#endif

#ifndef TFFN_VA_END
    #include <stdarg.h>
    #define TFFN_VA_END va_end
#endif

#ifndef TFFN_ASSERT
    #include <assert.h>
    #define TFFN_ASSERT assert
#endif

#ifndef TFFN_MALLOC
    #include <stdlib.h>
    #define TFFN_MALLOC malloc
#endif

#ifndef TFFN_REALLOC
    #include <stdlib.h>
    #define TFFN_REALLOC realloc
#endif

#ifndef TFFN_CALLOC
    #include <stdlib.h>
    #define TFFN_CALLOC calloc
#endif

#ifndef TFFN_FREE
    #include <stdlib.h>
    #define TFFN_FREE free
#endif


///////////////////////

typedef struct _TFFNStrBuilder {
    char* buffer;     // not NULL terminated
    size_t count;     // how many letters are there in the buffer?
    size_t capacity;  // maximum amount of letters that can fit into buffer
} TFFNStrBuilder;

TFFNStrBuilder* tffn_sb_new(size_t);
TFFNStrBuilder* tffn_sb_new_from(const char*, size_t);
void tffn_sb_append(TFFNStrBuilder*, const char*, size_t);
void tffn_sb_clear(TFFNStrBuilder*);
void tffn_sb_free(TFFNStrBuilder*);
char* tffn_sb_to_str(TFFNStrBuilder*);

///////////////////////

typedef struct _TFFNStep {
    void (*dynamic_step)(TFFNStrBuilder*); // function to run
    const char* static_step; // already existing string to replace
} TFFNStep;

TFFNStep* tffn_step_new_dynamic(void (*dynamicStep)(TFFNStrBuilder*));
TFFNStep* tffn_step_new_static(const char* staticStep);

///////////////////////

typedef enum _TFFNErrorType {
    TFFN_ERR_NONE,
    TFFN_ERR_DANGLING_CLOSE_BRACKET,
    TFFN_ERR_NESTING_BRACKETS,
    TFFN_ERR_DANGLING_IGNORE_TOKEN,
    TFFN_ERR_UNCLOSED_BRACKET,
    TFFN_ERR_IGNORE_TOKEN_INSIDE_BRACKET,
    TFFN_ERR_UNDEFINED_ACTION,
    TFFN_ERR_ACTION_TEXT_ALREADY_EXISTS
} TFFNErrorType;

#define TFFN_ERR_TO_STR(et, ...) tffn_err_to_str_internal(et, ##__VA_ARGS__)

const char* tffn_err_to_str_internal(TFFNErrorType, ...);

///////////////////////

typedef struct _TFFNEntry {
    char* key; // must be NULL terminated!
    size_t key_length;
    void* object;
    struct _TFFNEntry* next;
} TFFNEntry;

typedef struct _TFFNHashTable {
    uint32_t table_size;
    TFFNEntry** entries;
} TFFNHashTable;

TFFNHashTable* tffn_htable_new(uint32_t);
void tffn_htable_free(TFFNHashTable*);
int tffn_htable_insert(TFFNHashTable*, const char*, void*);
void* tffn_htable_lookup(TFFNHashTable*, const char*);
void* tffn_htable_delete(TFFNHashTable*, const char*);

///////////////////////


#endif // TFFN_H

// ------------------------------------------------------------ //


#ifdef TFFN_IMPLEMENTATION

#ifdef __cplusplus
extern "C" {  // prevents name mangling of functions when used in C++
#endif


/////////////////// StrBuilder Struct ///////////////////


TFFNStrBuilder* tffn_sb_new(size_t initial_capacity) {
    TFFN_ASSERT(initial_capacity > 0);
    TFFNStrBuilder* sb = (TFFNStrBuilder*) TFFN_MALLOC(sizeof(TFFNStrBuilder));
    TFFN_ASSERT(sb != NULL && "Couldn't allocate memory");

    sb->count = 0;
    sb->capacity = initial_capacity;
    sb->buffer = (char*) TFFN_MALLOC(sb->capacity * sizeof(char));
    TFFN_ASSERT(sb->buffer != NULL && "Couldn't allocate memory");

    return sb;
}


TFFNStrBuilder* tffn_sb_new_from(const char* buffer, size_t char_count) {
    TFFN_ASSERT(char_count > 0);
    TFFNStrBuilder* sb = tffn_sb_new(char_count * 2);
    tffn_sb_append(sb, buffer, char_count);
    return sb;
}


void tffn_sb_append(TFFNStrBuilder* sb, const char* buffer, size_t char_count) {
    if (buffer == NULL || char_count <= 0) return; // nothing to append

    // The given string doesnt fit into the current buffer, so increase the buffer capacity
    while(sb->count + char_count > sb->capacity) {
        sb->capacity *= 2;
    }

    sb->buffer = TFFN_REALLOC(sb->buffer, sb->capacity * sizeof(char));
    TFFN_ASSERT(sb->buffer != NULL && "Couldn't allocate memory");

    memcpy(sb->buffer + sb->count, buffer, char_count * sizeof(char));
    sb->count += char_count;
}


void tffn_sb_clear(TFFNStrBuilder* sb) {
    sb->count = 0;
}


void tffn_sb_free(TFFNStrBuilder* sb) {
    sb->buffer[sb->count-1] = '\0';
    TFFN_FREE(sb->buffer);
    TFFN_FREE(sb);
}


char* tffn_sb_to_str(TFFNStrBuilder* sb) {
    char* res = (char*) TFFN_MALLOC((sb->count+1) * sizeof(char));
    for (size_t i = 0; i < sb->count; i++) {
        res[i] = sb->buffer[i];
    }
    res[sb->count] = '\0';
    return res;
}


/////////////////// Step Struct ///////////////////


TFFNStep* tffn_step_new_dynamic(void (*dyn_func)(TFFNStrBuilder*)) {
    TFFNStep* step = (TFFNStep*) TFFN_MALLOC(sizeof(TFFNStep));
    TFFN_ASSERT(step != NULL);
    step->dynamic_step = dyn_func;
    step->static_step = NULL;
    return step;
}


TFFNStep* tffn_step_new_static(const char* static_str) {
    TFFNStep* step = (TFFNStep*) TFFN_MALLOC(sizeof(TFFNStep));
    TFFN_ASSERT(step != NULL);
    step->dynamic_step = NULL;
    step->static_step = static_str;
    return step;
}


/////////////////// Error Enum ///////////////////


const char* tffn_err_to_str_internal(TFFNErrorType et, ...) {
    if(et == TFFN_ERR_NONE) return NULL;

    static char buffer[256];
    va_list args;
    TFFN_VA_START(args, et);

    if(et == TFFN_ERR_DANGLING_CLOSE_BRACKET) 
        snprintf(buffer, sizeof(buffer), "INVALID FORMAT: you forgot to open a bracket\n");
    else if(et == TFFN_ERR_NESTING_BRACKETS) 
        snprintf(buffer, sizeof(buffer), "INVALID FORMAT: nesting brackets are prohibited in TFFN\n");
    else if(et == TFFN_ERR_DANGLING_IGNORE_TOKEN) 
        snprintf(buffer, sizeof(buffer), "INVALID FORMAT: format string cant end with '!'\n");
    else if(et == TFFN_ERR_UNCLOSED_BRACKET) 
        snprintf(buffer, sizeof(buffer), "INVALID FORMAT: you forgot to close a bracket\n");
    else if(et == TFFN_ERR_IGNORE_TOKEN_INSIDE_BRACKET) 
        snprintf(buffer, sizeof(buffer), "INVALID FORMAT: '!' token cant be used inside brackets\n");
    else if(et == TFFN_ERR_UNDEFINED_ACTION) 
        vsnprintf(buffer, sizeof(buffer), "INVALID FORMAT: '%s' action was never defined to the parser\n", args);
    else if(et == TFFN_ERR_ACTION_TEXT_ALREADY_EXISTS) 
        vsnprintf(buffer, sizeof(buffer), "An action with '%s' name already exists!\n", args);
    else buffer[0] = '\0'; // Empty string for no error

    TFFN_VA_END(args);
    return buffer;
}


/////////////////// HashTable Struct ///////////////////


static size_t tffn_htable_str_to_index(TFFNHashTable* ht, const char* str) {
    uint64_t hash = 0;

    size_t str_length = strlen(str);
    for (int i = 0; i < str_length; i++) {
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
    size_t index = hash % ht->table_size;
    return index;
}


TFFNHashTable* tffn_htable_new(uint32_t size) {
    TFFNHashTable* ht = (TFFNHashTable*) TFFN_MALLOC(sizeof(TFFNHashTable));
    ht->table_size = size;
    ht->entries = (TFFNEntry**) TFFN_CALLOC(sizeof(TFFNEntry*), ht->table_size);
    return ht;
}


void tffn_htable_free(TFFNHashTable* ht) {
    if (ht == NULL) return;

    for (size_t i = 0; i < ht->table_size; i++) {
        TFFNEntry* temp = ht->entries[i];
        while (temp != NULL) {
            TFFNEntry* next = temp->next;
            TFFN_FREE(temp->key);
            TFFN_FREE(temp);
            temp = next;
        }
    }

    TFFN_FREE(ht->entries);
    TFFN_FREE(ht);
}


int tffn_htable_insert(TFFNHashTable* ht, const char* key, void* object) {
    TFFN_ASSERT(ht != NULL);
    TFFN_ASSERT(key != NULL);
    
    // Do nothing if the object being inserted is NULL
    if(object == NULL) return 0;

    // Do nothing if the key already exists
    size_t str_length = strlen(key);
    if(tffn_htable_lookup(ht, key) != NULL) return 0;

    // Create new entry
    TFFNEntry* entry = (TFFNEntry*) TFFN_MALLOC(sizeof(TFFNEntry));
    entry->object = object;
    entry->key = (char*) TFFN_MALLOC(str_length+1);
    for(int i = 0; i < str_length; i++) {
        entry->key[i] = key[i];
    }
    entry->key[str_length] = '\0';
    entry->key_length = str_length;

    // Insert new entry
    size_t index = tffn_htable_str_to_index(ht, key);
    entry->next = ht->entries[index];
    ht->entries[index] = entry;
    return 1;
}


static int tffn_util_strequal(const char* str1, size_t len1, const char* str2, size_t len2) {
    if(len1 != len2) return 0;
    
    for(int i = 0; i < len1; i++) {
        if(str1[i] != str2[i]) return 0;
    }

    return 1;
}


void* tffn_htable_lookup(TFFNHashTable* ht, const char* key) {
    TFFN_ASSERT(ht != NULL);
    TFFN_ASSERT(key != NULL);

    size_t str_length = strlen(key);
    size_t index = tffn_htable_str_to_index(ht, key);
    TFFNEntry* temp = ht->entries[index];
    while(temp != NULL) {
        if(tffn_util_strequal(temp->key, temp->key_length, key, str_length)) break;
        temp = temp->next;
    }

    if(temp == NULL) return NULL;
    return temp->object;
}


void* tffn_htable_delete(TFFNHashTable* ht, const char* key) {
    TFFN_ASSERT(ht != NULL);
    TFFN_ASSERT(key != NULL);

    size_t str_length = strlen(key);
    size_t index = tffn_htable_str_to_index(ht, key);
    TFFNEntry* temp = ht->entries[index];
    TFFNEntry* prev = NULL;
    while(temp != NULL) {
        if(tffn_util_strequal(temp->key, temp->key_length, key, str_length)) break;
        prev = temp;
        temp = temp->next;
    }

    if(temp == NULL) return NULL;

    if(prev == NULL) ht->entries[index] = temp->next;
    else prev->next = temp->next;
    
    void* object = temp->object;
    TFFN_FREE(temp);
    return object;
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
