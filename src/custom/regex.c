#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lauxlib.h>

static int match_regex(luna_State *L) {
    const char *input = lunaL_checkstring(L, 1);
    const char *pattern = lunaL_checkstring(L, 2);

    regex_t regex;
    int ret;

    // Compile the regular expression
    if ((ret = regcomp(&regex, pattern, REG_EXTENDED)) != 0) {
        char error_message[100];
        regerror(ret, &regex, error_message, sizeof(error_message));
        fprintf(stderr, "Regex compilation error: %s\n", error_message);
        regfree(&regex);
        luna_pushnil(L);
        luna_pushstring(L, error_message);
        return 2;  // Return nil and error message
    }

    // Execute the regular expression
    regmatch_t matches[10];  // Adjust the size based on your expected number of capturing groups
    if ((ret = regexec(&regex, input, sizeof(matches) / sizeof(matches[0]), matches, 0)) == 0) {
        // Match found
        for (int i = 1; i < sizeof(matches) / sizeof(matches[0]); i++) {
            if (matches[i].rm_so == -1) {
                break;  // No more capturing groups
            }
            size_t start = matches[i].rm_so;
            size_t end = matches[i].rm_eo;
            luna_pushlstring(L, input + start, end - start);
        }
        regfree(&regex);
        return luna_gettop(L) - 2;  // Return the number of captured groups
    } else if (ret == REG_NOMATCH) {
        // No match found
        regfree(&regex);
        luna_pushnil(L);
        return 1;  // Return nil
    } else {
        // Other error
        char error_message[100];
        regerror(ret, &regex, error_message, sizeof(error_message));
        fprintf(stderr, "Regex execution error: %s\n", error_message);
        regfree(&regex);
        luna_pushnil(L);
        luna_pushstring(L, error_message);
        return 2;  // Return nil and error message
    }
}