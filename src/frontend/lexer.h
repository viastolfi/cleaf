/*
------------------------------------------------------------------------------
LEXER LIB FOR C - ASTOLFI Vincent 2025

/!\ Warning

This lib is greatly inspired by the stb_c_lexer lib 
This is just a try from me to understand it and rewrite it only using what I understand of it 
This is clear that some part of it are exactly the same as it is in the stb_c_lexer lib
There is still no copy past from it at all. Otherwise it would make no sence to rewrite it

*/

#ifndef LEXER_H
#define LEXER_H

#ifdef LEXER_LIB_IMPLEMENTATION
#ifndef LEXER_LIB_DEFINITION

// We need Y and N for later #if usage so the lib can be used in multiple context
#if defined(Y) || defined(N)
#error "You can only use this lexer lib if Y and N are not defined yet"
#endif

// BASIC INT IN DECIMAL FORM (ex: 12)
#define LEXER_LIB_DECIMAL_INTS  Y  // "0|[1-9][0-9]*"   LEXER_token_intlit
#define LEXER_LIB_INCREMENTS    Y  // "++"              LEXER_token_plusplus
                                   // "--"              LEXER_token_minusminus
#define LEXER_LIB_ARITH         Y  // "+="              LEXER_token_pluseq
                                   // "-="              LEXER_token_minusminus
                                   // "*="              LEXER_token_muleq
                                   // "%="              LEXER_token_modeq    
                                   // "/="              LEXER_token_diveq
#define LEXER_LIB_SIMPLE_ARROW  Y  // "->"              LEXER_token_sarrow
#define LEXER_LIB_COMPARISON    Y  // "=="              LEXER_token_eq
                                   // ">="              LEXER_token_gteq
                                   // "<="              LEXER_token_lseq
                                   // "!="              LEXER_token_neq
#define LEXER_LIB_JS_COMPARISON Y  // "==="             LEXER_token_js_eq
                                   // "!=="             LEXER_token_js_neq
#define LEXER_LIB_DOUBLE_ARROW  Y  // "=>"              LEXER_token_darrow
#define LEXER_LIB_LOGICAL       Y  // "||"              LEXER_token_or
                                   // "&&"              LEXER_token_and
#define LEXER_LIB_SQ_STRINGS    N  // single quotes delimited strings LEXER_token_sqstring
#define LEXER_LIB_DQ_STRINGS    Y  // doubles quotes delimited string LEXER_token_dqstring
#define LEXER_LIB_LIT_CHARS     Y  // single quotes delimited char with escape LEXER_token_charlit
#define LEXER_LIB_IDENTIFIERS   Y  // "[_a-zA-Z][_a-zA-Z0-9]*" LEXER_token_id
#define LEXER_LIB_SL_COMMENTS   Y  // single line comments starting with '//' 
#define LEXER_LIB_ML_COMMENTS   Y  // multiple line comments like '/*' ... '*/' 

// TODO: add all other possible token

#define LEXER_LIB_DEFINITION

#endif // LEXER_LIB_IMPLEMENTATION
#endif // LEXER_LIB_DEFINITION

typedef struct 
{
  const char* input_stream;
  const char* eof;

  // actual point where we are at in the parsing
  const char* parse_point;

  // String storage for better memory management
  /* Why do we use a string storage :
   * Using a string storage, we make sure that the lib doesn't do any memory management
   * The libs keeps lightweight and doing so we ensure that it doesn't create any memory leak
   * Any memory management is done beforehand by the user
   */
  char* string_storage;
  int string_storage_len;

  // Token variables
  long token;
  long int_value;

  char* string_value;
  int string_len;
} lexer_t;

// We start at 256 since this is the end of the ASCII table
// Doing this, we can store single char only using their ASCII value
// Hence reducing the amount of needed token in this enum
enum 
{
  LEXER_token_eof = 256,
  LEXER_token_parse_error,
  LEXER_token_id,
  LEXER_token_intlit,
  LEXER_token_plusplus,
  LEXER_token_pluseq,
  LEXER_token_minusminus,
  LEXER_token_sarrow, 
  LEXER_token_minuseq,
  LEXER_token_eq,
  LEXER_token_js_eq,
  LEXER_token_darrow,
  LEXER_token_gteq,
  LEXER_token_lseq,
  LEXER_token_neq,
  LEXER_token_js_neq,
  LEXER_token_muleq,
  LEXER_token_modeq,
  LEXER_token_diveq,
  LEXER_token_or,
  LEXER_token_and,
  LEXER_token_sqstring,
  LEXER_token_dqstring,
  LEXER_token_charlit
};

// So we can #if on each token definition
#define Y(x) 1
#define N(x) 0

#if LEXER_LIB_DECIMAL_INTS(x)
#define LEXER_decimal_ints
#endif // LEXER_LIB_DECIMAL_INTS

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern int lexer_get_token(lexer_t* l);
extern void lexer_init_lexer(lexer_t* l, const char* input_stream, const char* end_input_stream, char* string_storage, int string_storage_len);

#ifdef __cplusplus
  }
#endif // __cplusplus

#ifdef LEXER_LIB_IMPLEMENTATION

#ifndef LEXER_STDLIB
#define LEXER_STDLIB
#include <stdlib.h>
#endif // LEXER_STDLIB

// Disable the 'config' usage of Y and N
// Now, Y expand to its content and N to nothing
// This avoid compiler error
#undef Y
#define Y(a) a
#undef N
#define N(a)

static int lexer_is_white(char c) 
{
  return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f'; 
}

static int lexer_eof(lexer_t* l)
{
  l->token = LEXER_token_eof;
  return 0;
}

// Helper function for creating a token and updating the lexer
static int lexer_create_token(lexer_t* l, int token, const char* end)
{
  l->token = token;

  l->parse_point = end + 1;
  return 1;
}

static int lexer_parse_char(const char* p, const char **q)
{
  if (*p == '\\') {
    *q = p+2; 
    switch (p[1]) {
      case '\\': return '\\';
      case '\'': return '\'';
      case '"': return '"';
      case 't': return '\t';
      case 'f': return '\f';
      case 'n': return '\n';
      case 'r': return '\r';
      case '0': return '\0'; 
    }
  }

  *q = p+1;
  return (unsigned char) *p;
}

static int lexer_parse_string(lexer_t* l, const char* p, int type) 
{
  //const char* start = p;
  // We store the actual char pointed at 'p' and advance the 'p' pointer once
  // Example : "abc", delim = '"' and p points to a
  char delim = *p++;

  char* out = l->string_storage;
  // To ensure we don't overflow
  char* outend = l->string_storage + l->string_storage_len;

  while (*p != delim) {
    int n;
    if (*p == '\\') {
      const char* q;
      n = lexer_parse_char(p, &q);
      p = q;
      if (n < 0) 
        return lexer_create_token(l, LEXER_token_parse_error, p); 
    } else {
      n = (unsigned char) *p++;
    }

    if (out + 1 > outend) {
      // We might need to add 'start' to the created token for better debugging
      return lexer_create_token(l, LEXER_token_parse_error, p);
    }

    *out++ = (char) n;
  }
  *out = 0;
  l->string_value = l->string_storage;
  l->string_len = (int) (out - l->string_storage);
  return lexer_create_token(l, type, p);
}

void lexer_init_lexer(lexer_t* l, const char* input_stream, const char* end_input_stream, char* string_storage, int string_storage_len) 
{
  l->input_stream = input_stream;
  l->eof = end_input_stream;
  l->parse_point = input_stream;

  l->string_storage = string_storage;
  l->string_storage_len = string_storage_len;
}

int lexer_get_token(lexer_t* l) 
{
  const char* p = l->parse_point;

  for (;;) {
    while (p != l->eof && lexer_is_white(*p)) {
      ++p;
    }

    LEXER_LIB_SL_COMMENTS(
      if (p != l->eof && p[0] == '/' && p[1] == '/') {
        while (p != l->eof && *p != '\n' && *p != '\r')
          ++p;
        continue;
      }
    )

    LEXER_LIB_ML_COMMENTS(
      if (p != l->eof && p[0] == '/' && p[1] == '*') {
        p += 2;
        while (p != l->eof && (p[0] != '*' || p[1] != '/'))
          ++p;
        if (p == l->eof) 
          return lexer_create_token(l, LEXER_token_parse_error, p-1); 
        p += 2;
        continue;
      }
    )
    break;
  }

  if (p == l->eof)
    return lexer_eof(l);

  switch (*p) {
    default:
      if (   (*p > 'a' && *p < 'z')
          || (*p > 'A' && *p < 'Z')
          || *p == '_') {
        int n = 0;
        l->string_value = l->string_storage;
        l->string_len = n;
        do {
          if (n + 1 >= l->string_storage_len)
            return lexer_create_token(l, LEXER_token_parse_error, p+n);
          l->string_value[n] = p[n];
          ++n;
        } while (   (p[n] >= 'a' && p[n] <= 'z')
                 || (p[n] >= 'A' && p[n] <= 'Z')
                 || (p[n] >= '0' && p[n] <= '9') // allow digits in middle of identifier
                 || p[n] == '_');
        l->string_value[n] = 0;
        // We use p+n-1 because we '\0' terminated the string and we don't count that in the parsing point
        return lexer_create_token(l, LEXER_token_id, p+n-1);
      }

    single_char:
      return lexer_create_token(l, *p, p);

    case '+':
      if (p+1 != l->eof) {
        LEXER_LIB_INCREMENTS(if (p[1] == '+') return lexer_create_token(l, LEXER_token_plusplus, p+1);)
        LEXER_LIB_ARITH(if (p[1] == '=') return lexer_create_token(l, LEXER_token_pluseq, p+1);)
      }
      goto single_char;
    case '-':
      if (p + 1 != l->eof) {
        LEXER_LIB_INCREMENTS( if (p[1] == '-') return lexer_create_token(l, LEXER_token_minusminus, p+1);)
        LEXER_LIB_ARITH( if (p[1] == '=') return lexer_create_token(l, LEXER_token_minuseq, p+1);)
        LEXER_LIB_SIMPLE_ARROW( if (p[1] == '>') return lexer_create_token(l, LEXER_token_sarrow, p+1);)
      }
      goto single_char;
    case '=':
      if (p + 1 != l->eof) {
        if (p[1] == '=') {
          if (p + 2 != l->eof) {
            LEXER_LIB_JS_COMPARISON( if (p[2] == '=') return lexer_create_token(l, LEXER_token_js_eq, p+2);)
          }
          LEXER_LIB_COMPARISON(return lexer_create_token(l, LEXER_token_eq, p+1);)
        }
        if (p[1] == '>') {
          LEXER_LIB_DOUBLE_ARROW(return lexer_create_token(l, LEXER_token_darrow, p+1);) 
        }
      }
      goto single_char;
    case '<':
        LEXER_LIB_COMPARISON( if (p+1 != l->eof && p[1] == '=') return lexer_create_token(l, LEXER_token_lseq, p+1);)
      goto single_char;
    case '>':
        LEXER_LIB_COMPARISON( if (p+1 != l->eof && p[1] == '=') return lexer_create_token(l, LEXER_token_gteq, p+1);)
        goto single_char;
    case '!':
        if (p + 1 != l->eof) {
          LEXER_LIB_JS_COMPARISON( if (p+2 != l->eof && p[1] == '=' && p[2] == '=') return lexer_create_token(l, LEXER_token_js_neq, p+2);)
          LEXER_LIB_COMPARISON( if (p[1] == '=') return lexer_create_token(l, LEXER_token_neq, p+1);)
        }
        goto single_char;
    case '*':
        LEXER_LIB_ARITH( if (p+1 != l->eof && p[1] == '=') return lexer_create_token(l, LEXER_token_muleq, p+1);)
        goto single_char;
    case '%':
        LEXER_LIB_ARITH( if (p+1 != l->eof && p[1] == '=') return lexer_create_token(l, LEXER_token_modeq, p+1);)
        goto single_char;
    case '/':
        LEXER_LIB_ARITH( if (p+1 != l->eof && p[1] == '=') return lexer_create_token(l, LEXER_token_diveq, p+1);)
        goto single_char;
    case '|':
        LEXER_LIB_LOGICAL( if (p+1 != l->eof && p[1] == '|') return lexer_create_token(l, LEXER_token_or, p+1);)
        goto single_char;
    case '&':
        LEXER_LIB_LOGICAL( if (p+1 != l->eof && p[1] == '&') return lexer_create_token(l, LEXER_token_and, p+1);)
        goto single_char;
    case '\'':
        LEXER_LIB_SQ_STRINGS(return lexer_parse_string(l, p, LEXER_token_sqstring);)
        LEXER_LIB_LIT_CHARS(
        {
          l->int_value = lexer_parse_char(p+1, &p);
          // case the char is not an ASCII character
          if (l->int_value < 0) 
            return lexer_create_token(l, LEXER_token_parse_error, p);
          // case the sq 'string' is more than one chat long
          if (p == l->eof || *p != '\'')
            return lexer_create_token(l, LEXER_token_parse_error, p);
          return lexer_create_token(l, LEXER_token_charlit, p);
        })
        goto single_char;
    case '"':
        LEXER_LIB_DQ_STRINGS(return lexer_parse_string(l, p, LEXER_token_dqstring);)
        goto single_char;
    case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
    #ifdef LEXER_decimal_ints
    {
      const char* q = p;
      #ifdef LEXER_STDLIB
      /* strtol -> string to long converter
       * params: 
       * * ptr of the string we want to convert
       * * ptr that will indicate the end of the long (ie: '12345Toto q -> 'T')
       * * type of the number (ie: 10 decimal number)
       * return the converted long value
       */
      l->int_value = strtol((const char *) p, (char **) &q, 10);  
      #else
      // TODO: implement case of no stdlib
      return 0;
      #endif // LEXER_STDLIB
      return lexer_create_token(l, LEXER_token_intlit, q-1);
    }
    // TODO: add some suffixe parsing (ie: 42U)
    #endif // LEXER_decimal_ints
  }

  return 0;
}

static void lexer_print_token(lexer_t *l) 
{
  switch (l->token)
  {
    case LEXER_token_eof: printf("EOF"); break;
    case LEXER_token_intlit: printf("#%ld", l->int_value); break;
    case LEXER_token_plusplus: printf("++"); break;
    case LEXER_token_pluseq: printf("+="); break;
    case LEXER_token_minusminus: printf("--"); break;
    case LEXER_token_minuseq : printf("-="); break;
    case LEXER_token_sarrow: printf("->"); break;
    case LEXER_token_eq: printf("=="); break;
    case LEXER_token_js_eq: printf("==="); break;
    case LEXER_token_darrow: printf("=>"); break;
    case LEXER_token_gteq: printf(">="); break;
    case LEXER_token_lseq: printf("<="); break;
    case LEXER_token_neq: printf("!="); break;
    case LEXER_token_js_neq: printf("!=="); break;
    case LEXER_token_muleq: printf("*="); break;
    case LEXER_token_modeq: printf("%%="); break;
    case LEXER_token_diveq: printf("/="); break;
    case LEXER_token_or : printf("||"); break;
    case LEXER_token_and : printf("&&"); break;
    case LEXER_token_sqstring: printf("'%s'", l->string_value); break;
    case LEXER_token_dqstring: printf("\"%s\"", l->string_value); break;
    case LEXER_token_charlit: printf("'%c'", (unsigned char) l->int_value); break;
    case LEXER_token_id: printf("_%s", l->string_value); break;
    default:
      if (l->token >= 0 && l->token < 256)
        printf("%c", (int) l->token);
      else {
        printf("<<<UNKNOWN TOKEN %ld >>>\n", l->token);
      }
      break;
  }
}

#endif // LEXER_LIB_IMPLEMENTATION
#endif // LEXER_H
