#include "io.c"
#include "regex.c"
#include "server.cpp"
#include "raylib_wrapper.cpp"

static const lunaL_Reg base_funcs[] = {
  //custom functions start
  {"readfile", luna_readfile},
  {"writefile", luna_writefile},
  {"input", p_input},
  {"request", luna_curl_request},
  {"regex",match_regex},
  {"init_server",init_server},
  {"raylib_init", init_raylib},

  //custom functions end
  {"assert", lunaB_assert},
  {"collectgarbage", lunaB_collectgarbage},
  {"dofile", lunaB_dofile},
  {"error", lunaB_error},
  {"getmetatable", lunaB_getmetatable},
  {"ipairs", lunaB_ipairs},
  {"loadfile", lunaB_loadfile},
  {"load", lunaB_load},
  {"next", lunaB_next},
  {"pairs", lunaB_pairs},
  {"pcall", lunaB_pcall},
  {"print", lunaB_print},
  {"warn", lunaB_warn},
  {"rawequal", lunaB_rawequal},
  {"rawlen", lunaB_rawlen},
  {"rawget", lunaB_rawget},
  {"rawset", lunaB_rawset},
  {"select", lunaB_select},
  {"setmetatable", lunaB_setmetatable},
  {"tonumber", lunaB_tonumber},
  {"tostring", lunaB_tostring},
  {"type", lunaB_type},
  {"xpcall", lunaB_xpcall},
  /* placeholders */
  {LUNA_GNAME, NULL},
  {"_VERSION", NULL},
  {NULL, NULL}
};
