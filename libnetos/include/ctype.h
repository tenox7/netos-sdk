#ifndef _CTYPE_H
#define _CTYPE_H
#define isdigit(c)  ((c) >= '0' && (c) <= '9')
#define isspace(c)  ((c)==' '||(c)=='\t'||(c)=='\n'||(c)=='\r'||(c)=='\f'||(c)=='\v')
#define isupper(c)  ((c) >= 'A' && (c) <= 'Z')
#define islower(c)  ((c) >= 'a' && (c) <= 'z')
#define isalpha(c)  (isupper(c) || islower(c))
#define isalnum(c)  (isalpha(c) || isdigit(c))
#define isprint(c)  ((c) >= ' ' && (c) < 0x7f)
#define toupper(c)  (islower(c) ? (c) - 'a' + 'A' : (c))
#define tolower(c)  (isupper(c) ? (c) - 'A' + 'a' : (c))
#endif
