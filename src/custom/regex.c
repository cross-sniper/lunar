#include <regex.h>
#include <stdio.h>
#include <stdlib.h>

regex_t regex;
int reti;
char msgbuf[100];
static int match_regex(luna_State *L) {
    const char *input = lunaL_checkstring(L, 1);
    const char *pattern = lunaL_checkstring(L, 2);

    reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti) {
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        luna_pushnil(L);
        luna_pushstring(L, msgbuf);  // Push the error message
        regfree(&regex); // Free the memory allocated to the pattern buffer
        return 2;  // Return nil and the error message
    }

    reti = regexec(&regex, input, 0, NULL, 0);
    regfree(&regex); // Free the memory allocated to the pattern buffer

    if (!reti) {
        luna_pushboolean(L, 1); // Match
        return 1;
    } else if (reti == REG_NOMATCH) {
        luna_pushboolean(L, 0); // No match
        return 1;
    } else {
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        luna_pushnil(L);
        luna_pushstring(L, msgbuf);  // Push the error message
        return 2;  // Return nil and the error message
    }
}


