


# TFFN C Parser

<b>A C library for parsing format strings written in TFFN syntax.</b>

For documentation about the TFFN syntax you can look 
<a href="https://github.com/oziris78/tffn-java-parser#tffn-syntax">here</a>.

For documentation about this library, you can take a look at the 
huge comment section at the top of the tffn.h file.


<br>



# Basic Example

```c
#include <stdio.h>
#include <stdlib.h>

#define TFFN_IMPLEMENTATION
#include "tffn.h"

int main() {
    TFFNParser* parser = tffn_parser_new();
    tffn_parser_define_static_action(parser, "h", "Hello");
    tffn_parser_define_static_action(parser, "w", "World");

    char* str = tffn_parser_parse(parser, "[h] [w]!!");
    if(!tffn_parser_okay(parser)) exit(-1);
    printf(str); // prints "Hello World!"  (not "Hello World!!")

    free(str);
    tffn_parser_free(parser);
}
```

<br>

# Advanced Example

```c
#include <stdio.h>
#include <stdlib.h>

#define TFFN_IMPLEMENTATION
#include "tffn.h"

int f1 = 1, f2 = 1;

void dyn_func_inc_num(TFFNStrBuilder* sb) {
    int f3 = f1 + f2;
    f1 = f2;
    f2 = f3;

    char str[5];
    sprintf(str, "%d", f3);
    tffn_sb_append_nterm(sb, str);
}

int main() {
    TFFNParser* parser = tffn_parser_new();
    tffn_parser_define_dynamic_action(parser, "fib", dyn_func_inc_num);
    tffn_parser_define_static_action(parser, "text", "Here are the fibonacci numbers: ");

    char* str = tffn_parser_parse(parser, "[text] 1 1 [fib] [fib] [fib] [fib] [fib] ...");
    if(!tffn_parser_okay(parser)) {
        printf("A parsing error happened!\n");
        char* err_message = tffn_parser_err_msg(parser);
        printf("Error message: %s\n", err_message);
        free(err_message); // not really needed since we are going to exit(-1) but yeah
        exit(-1);
    }
    else {
        printf(str); // Here are the fibonacci numbers:  1 1 2 3 5 8 13 ...
        free(str);
    }

    tffn_parser_free(parser);
}
```


<br>

# Customizing TFFNParser

You can easily make this library use custom functions.

```c
void* my_custom_malloc(size_t);

#define TFFN_IMPLEMENTATION
#define TFFN_MALLOC my_custom_malloc
#include "tffn.h"

void* my_custom_malloc(size_t size_in_bytes) {
    void* memory = (void*) malloc(size_in_bytes);
    printf("My malloc just ran!\n");
    return memory;
}

int main() {
    // use the library the same way you would without a custom malloc
}
```


<br>


# Licensing

<a href="https://github.com/oziris78/tffn-c-parser">This library</a> is licensed under the terms of the Apache-2.0 license.

