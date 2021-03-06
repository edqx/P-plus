#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#include "def.h"

#define INCR_MEM(size) do { \
	if(key + (size) > keywords_size / sizeof(char*)) addSpaceForKeys(keywords, &keywords_size); \
} while(0)

#define INCR_MEM2(size) do { \
	if(pkey + (size) > pointers_size / sizeof(char*)) addSpaceForKeys(pointers, &pointers_size); \
} while(0)

bool inStr = false;
bool inStr2 = false;
bool escaping = false;
bool ignoring = false;

static void addSpaceForKeys(char ***keywords, size_t *keywords_size) {
	*keywords_size *= 2;
	
	char **res = realloc(*keywords, *keywords_size);
	if(res == NULL) {
		perror("ERROR");
		fprintf(stderr, "ID: %d\n", errno);
		exit(EXIT_FAILURE);
	} else {
		*keywords = res;
	}
}

static bool isSpecial(char c) {
	for(unsigned int i = 0; specials[i] != '\0'; i++) {
		if(c == specials[i]) return true;
	}
	
	return false;
}

void lex_parse(char *input, char ***keywords, char ***pointers) {
	INCR_MEM(1);
	(*keywords)[key] = input;
	key++;
	
	while(*input != '\0') {
		char *special;
		bool foundSpecial = false;
		
		if(ignoring) {
			*input = '\0';
			input++;
		}
		
		while((ignoring || inStr || inStr2 || *input != ' ') && *input != '\0') {
			if(ignoring) {
				if(*input == '*' && *(input + 1) == '/') {
					ignoring = false;
					input++;
					
					INCR_MEM(1);
					
					(*keywords)[key] = input + 1;
					key++;
				}
			} else if(!inStr && !inStr2 && isSpecial(*input)) {
				special = calloc(2, 1);
				special[0] = *input;
				foundSpecial = true;
				
				break;
			} else if(escaping) {
				escaping = false;
			} else if(!inStr2 && *input == '\'') {
				if(inStr) {
					inStr = false;
					break;
				} else {
					inStr = true;
				}
			} else if(!inStr && *input == '"') {
				if(inStr2) {
					inStr2 = false;
					break;
				} else {
					inStr2 = true;
				}
			} else if(*input == '\\') {
				escaping = true;
			}
			
			input++;
		}
		
		if(*input == ';' || *input == '{' || *input == '}') {
			*input = '\0';
			input++;
			
			while(*input == ' ') input++;
			if(*input == '/' && *(input + 1) == '/') while(*input != '\n') input++;
			
			if(*input == '/' && *(input + 1) == '*') {
				ignoring = true;
				input++;
				
				continue;
			} else {
				input--;
			}
		}
		
		if(*(input - 1) != '\0') {
			*input = '\0';
		}
		
		input++;
		
		if(foundSpecial) {
/*			if((special[0] == '>' || special[0] == '<' || special[0] == '=' || special[0] == '+' || special[0] == '-' || special[0] == '*' || special[0] == '/' || special[0] == '%') && (*keywords)[key - 1][strlen((*keywords)[key - 1]) - 1] != ')') {
				char *extra_special = malloc(2);
				extra_special[0] = '(';
				extra_special[1] = '\0';
				
				INCR_MEM(1);
				INCR_MEM2(1);
				
				(*keywords)[key] = (*keywords)[*key - 1];
				(*keywords)[key - 1] = extra_special;
				key++;
				
				(*pointers)[pkey] = extra_special;
				pkey++;
				// WIP
			} */
			
			INCR_MEM(1);
			INCR_MEM2(1);
			
			(*pointers)[pkey] = special; // This is used to mark where memory was allocated for 'special'
			pkey++;
			
			(*keywords)[key] = special;
			key++;
		}
		
		while(*input == ' ') input++;
		
		if(!isSpecial(*input)) {
			INCR_MEM(1);
			
			(*keywords)[key] = input;
			key++;
		}
	}
}