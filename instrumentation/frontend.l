/*Turn of yywrap, necessary*/
%option noyywrap
%option yylineno

/* Currently only support Tofino P4_14 */
%s BMV2_S TOFINO_S

%{
#include <string>
#include <cstdio>
#include "include/ast_nodes.hpp" // Must be before tab.h
#include "include/ast_nodes_p4.hpp"
#include "frontend.tab.h"

%}


%%
%{
    // Copied verbatim at the very front of yylex()
    // Variables to be used defined in parser
    extern int with_tofino;
    extern int done_init;
    if (!done_init)
    {
        if(with_tofino) {
            BEGIN TOFINO_S;
            done_init=1;  // necessary
            return START_TOFINO;
        } else {
            BEGIN BMV2_S;
            done_init=1;
            return START_BMV2;            
        }
    }    
%}
    /*
    Files included in the program
    yytext is the variable name (containing characters) in lex compiler
    yylval comes from the union in .y file to set the token type
    The value would be what you putting in the union
    Returns a pointer to a null-terminated byte string, which is a duplicate of the string pointed to by input_argument
    */


"#".* { 
    /* Includes are just copied to output */
    // Currently assumes no p4r include
    yylval.sval = strdup(yytext);
    return INCLUDE; 
}

    /* Pragmas in the program */
"@pragma"[^\S\r\n]* { 
    /* For tofino compiler pragma only */
    yylval.sval = strdup(yytext);
    return PRAGMA;
}
    /**/
    /* "@pragma"[ \t]*[A-Za-z_][A-Za-z0-9_]* { /*
        /* For tofino compiler pragma only */
        /* yylval.sval = strdup(yytext); */
        /* return PRAGMA; */
    /* } */
    

"//".*                              { /* comments,do nothing  */ }
[/][*][^*]*[*]+([^*/][^*]*[*]+)*[/] { /* comments,do nothing  */ }
[ \t\r]+                            { /* whitespace*/ }
[\n]                                { /* newline */ }

    /* pragma args */
    /*
    "--""placement" { 
        yylval.sval = strdup(yytext);
        return PRAGMA_PLACEMENT;
    }
    "--""no-dead-code-elimination" {
        yylval.sval = strdup(yytext);
        return PRAGMA_NO_DEAD_CODE;    
    }
    "--""metadata-overlay" {
        yylval.sval = strdup(yytext);
        return PRAGMA_METADATA_OVERLAY;    
    }
    */

    /*  Missing words: attribute, attributes, bit, 
        blackbox_type, block, calculated_field, counter,
        default_action, direct, field_list,
        field_list_calculation, implementation, input,
        instance_count, in, int, latest, layout, length,
        mask, max_length, max_size, max_width, meter,
        method, min_size, min_width, optional,
        out, output_width, parse_error,
        parser_exception, payload, pre_color, primitive_action,
        result, saturating, selection_key, selection_mode,
        selection_type, signed, size, static, string, true,
        false, type, update, verify, varbit, EXPRESSION_LOCAL_VARIABLES,
        last, PARSER_VALUE_SET, BYTES, PACKETS, PACKETS_AND_BYTES, 
        SALU_MIN, SALU_MAX, algorithm
    */

    /*
        Missing symbols: shift_left, shift_right, divide, mod
    */

    /*
        Removed words: FIELD, 
    */

    /* Reserved characters */
"{"         {return L_BRACE;}
"}"         {return R_BRACE;}
"("         {return L_PAREN;}
")"         {return R_PAREN;}
";"         {return SEMICOLON;}
":"         {return COLON;}
","         {return COMMA;}
"."         {return PERIOD;}
"$"         {return DOLLAR;}
"["         {return L_BRACKET;}
"]"         {return R_BRACKET;}
"/"         {return SLASH;}
"+"         {return PLUS;}
">="        {return GTE;}
"<="        {return LTE;}
"<"        {return LT;}
">"         {return GT;}
"=="        {return EQU;}
"!="        {return NEQU;}
"||"        {return OR;}
"&&"        {return AND;}
"-"         {return MINUS;}

    /* Reserved words - variables */
    /*"malleable"  {return P4R_MALLEABLE;} */
"table"     {return TABLE;}
"value"     {return VALUE;}
"width"     {return WIDTH;}
"init"      {return INIT;}
"alts"      {return ALTS;}
"reads"     {return READS;}
"actions"   {return ACTIONS;}
"valid"     {return VALID;}
"apply"     {return APPLY;}
"if"        {return IF;}
"else"     {return ELSE;}
"default"   {return DEFAULT;}

    /* Reserved words - initialization */
    /*"init_block" {return P4R_INIT_BLOCK;} */

"register"  {return REGISTER;}
"parser"    {return PARSER;}
"control"   {return CONTROL;}
"extract"   {return EXTRACT;}
"next"      {return NEXT;}
"current"   {return CURRENT;}
"blackbox"   {return BLACKBOX;}

    /* Reserved words - P4 */
"header"        {return HEADER;}
"metadata"      {return METADATA;}
"set_metadata"    {return SET_METADATA;}
"header_type"   {return HEADER_TYPE;}
"fields"        {return FIELDS;}
"exact"         {return EXACT;}
"ternary"       {return TERNARY;}
"lpm"           {return LPM;}
"range"         {return RANGE;}
"action"        {return ACTION;}
"action_profile" {return ACTION_PROFILE;}
"action_selector" {return ACTION_SELECTOR;}
"dynamic_action_selection"  {return DYNAMIC_ACTION_SELECTION;}
    /* "algorithm"     {return ALGORITHM;} */
"return"        {return RETURNS;}
"select"        {return SELECT;}
"selection_key" {return SELECTION_KEY;}
"selection_mode" {return SELECTION_MODE;}
"selection_type" {return SELECTION_TYPE;}
"assert" {return ASSERT_KEY;}

    /* Parsed identifier word in P4 code. */
[A-Za-z_][A-Za-z0-9_]* {
    yylval.sval = strdup(yytext);
    return IDENTIFIER;  
}

    /* Integer */
[-]?[0-9]+ {
    yylval.sval = strdup(yytext);
    return INTEGER; 
}

    /* Hex */
0[xX][0-9a-fA-F_]+ {
    yylval.sval = strdup(yytext);
    return HEX;
}

[^{}\/()\[\]:;,\.$ \t\n]+ {
    yylval.sval = strdup(yytext);
    return STRING;
}

    /* \S+ {
        yylval.sval = strdup(yytext);
        return STR;
    } */

%%
