#ifndef _STDLIB_H
#define _STDLIB_H


#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define RAND_MAX 2147483647
/* TODO: Determine proper definition for this macro. */
#define MB_CUR_MAX

double atof(const char *str);
int atoi(const char *str);
long int atol(const char *str);
double strtod(const char *str, char **endptr);
long int strtol(const char *str, char **endptr, int base);
unsigned long int strtoul(const char *str, char **endptr, int base);

/* TODO: Implement these functions after sbrk is available.
void *calloc(size_t nitems, size_t size);
void free(void *ptr);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);
*/

/* TODO: Implement the following as time permits. 
void exit(int status);
int abs(int x);
long int labs(long int x);
int rand(void);
void srand(unsigned int seed);
*/

#endif
