///////////////////////////////////////////////////////////////////////////////
// Name        	: HolaTokens.c
// Author      	: M.A.Tucker
// Date      	: 09MAY17
//
// Description 	:
// A C program that takes as input a sufficiently large text document
// (several are available online for testing; e.g. via Project Gutenberg),
// and produces as output (via stdout) an alphabetical listing of each unique word in the document
// (case insensitive and space separated, though be careful to consider hyphenated words),
// along with the lines from the input document that the word appears on.
// Each unique word (and the list of lines that it appears on) should be on a separate line in the output.
//
// Tool Chain: Eclipse, Cygwin, uthash hash macros (https://troydhanson.github.io/uthash)
//
// Time Complexity: O(n)
// - get each line in input stream
// - each token in line is hashed
// - added to the hash table if appropriate
// - line number summary is updated if appropriate
//
// Space Complexity: O(n)
// - sizeof(results struct) * # of unique tokens
//
//
// Limitations:
// - hyphenated words are treated as 2 distinct words
// - words containing apostrophes are considered a single word
// - the summary of lines containing a token will be truncated at LONGEST_LINE_SUMMARY_LEN
// - words longer than the longest word in major dictionaries (LONGEST_WORD_LEN 45) will be rejected
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// contains macros for convenient hash operations - https://troydhanson.github.io/uthash
#include "uthash.h"

#define TRUE  (1==1)
#define FALSE (!TRUE)
#define DEBUG FALSE
#define ERROR FALSE
///////////////////////////////////////////////////////////////////////////////
// hash table helpers
#define LONGEST_WORD_LEN 45
#define LONGEST_LINE_SUMMARY_LEN 16535

struct result_struct {
    int key;                    	// key - hash of token
    char word[LONGEST_WORD_LEN];			// longest word in major dictionary
    char line_summary[LONGEST_LINE_SUMMARY_LEN];	// summary of lines containing word
    UT_hash_handle hh;         	// makes this structure hashable */
};

struct result_struct *results = NULL;

void add_result(int key, char *name, char *line_number) {
    struct result_struct *s;

    // reject outrageously long words
    if (strlen(name) > sizeof(s->word)) {
    	if (ERROR) printf("\nOops! rejecting word %s length (%d) exceeds capacity(%d)...\n",name,strlen(name),sizeof(s->word));
    	return;
    }
    // check if exists
    HASH_FIND_INT(results, &key, s);  // key already in the hash?
    if (DEBUG) {
		if (s == NULL) printf("\nfind_result: <%d> NOT found for <%s>...", key, name);
		else printf("\nfind_result: <%d> found  for <%s>...", key, name);
    }
    // if no entry found, create an entry
    if (s==NULL) {
    	if (DEBUG) printf("\nadd_result new entry at <%d>", key);
    	s = (struct result_struct*)malloc(sizeof(struct result_struct));
    	s->key = key;
    	HASH_ADD_INT( results, key, s );  /* id: name of key field */
    }
    // set word
    strcpy(s->word, name);
    // if line summary length will not be exceeded
    int len = strlen(s->line_summary);
    if (DEBUG) printf("\nline summary(%d) %s", len, s->line_summary);
    if (len < (sizeof(s->line_summary) - 16)) {
//        if (len < 4080) {
		// if line_summary doesn't already contain the incoming line number, cat the new line number
		if(strstr(s->line_summary, line_number) == NULL) {
			strcat(s->line_summary, line_number);
			if (DEBUG)  {
				int len = strlen(s->line_summary);
				printf("\nline summary(%d) %s", len, s->line_summary);
			}
		}
    }
    else {
    	if (ERROR) printf("\nOops! line length (%d) for word %s exceeds capacity...\n", len, name);
    }

}

struct result_struct *find_result(int key) {
    struct result_struct *s;

    HASH_FIND_INT( results, &key, s );  /* s: output pointer */
    if (DEBUG) {
		if (s == NULL) printf("\nfind_result: %d NOT found...", key);
		else printf("\nfind_result: %d found...", key);
    }
    return s;
}

int hash (const char* word)
{
    unsigned int hash = 0;
    for (int i = 0 ; word[i] != '\0' ; i++)
    {
        hash = 31*hash + word[i];
    }
    return hash;
}

void print_results() {
    struct result_struct *s;

    for(s=results; s != NULL; s=s->hh.next) {
    	if (DEBUG) printf("key %d: name %s %s\n", s->key, s->word, s->line_summary);
        printf("%s%s\n", s->word, s->line_summary);
    }
}

int name_sort(struct result_struct *a, struct result_struct *b) {
    return strcmp(a->word,b->word);
}

void sort_by_name() {
    HASH_SORT(results, name_sort);
}

///////////////////////////////////////////////////////////////////////////////
// main - token scanner
int main() {
	char delimitors[] = " ";
	int read_byte_actual = 0;
	int read_byte_request = 16;
	char *test_line;
	int line_number = 0;

	char *token;
	char *saveptr = NULL;

	int key = 1;

	if (DEBUG) puts("!!!Hola Tokens begins...");

	// while not EOF
	while (read_byte_actual != -1) {
		// getline
		++line_number;
		read_byte_actual = getline(&test_line, &read_byte_request, stdin);
		if (DEBUG) printf("\nactual: %d...", read_byte_actual);
		if (read_byte_actual > 0) {
			// scan incoming line
			for (char *p = test_line; *p; p++) {
				// clear any non-alpha chars (except apostrophes)
//				if (!isalpha(*p) && *p != '\'' && *p != '-') *p = ' ';
				if (!isalpha(*p) && *p != '\'') *p = ' ';
				// convert alpha chars to lower case
				else if (isalpha(*p)) *p = tolower(*p);
			}
			if (DEBUG) printf(test_line);

		   // get first token
		   token = strtok_r(test_line, delimitors, &saveptr);

		   // for each token in line
		   while( token != NULL )
		   {
			   // hash token to derive key
			   key = hash(token);
			   char line_number_text[1024];
			   sprintf(line_number_text, " %d", line_number);
			   // add to hash table
			   if (DEBUG) printf( "\n\nadding hash key %d with token <%s> on line %s...", key, token, line_number_text);
			   add_result(key, token, line_number_text);

			   // extract next token
			   token = strtok_r(NULL, delimitors, &saveptr);
		   }


		}

	}
	if (DEBUG) {
		unsigned int num_users;
		num_users = HASH_COUNT(results);
		printf("there are %u results\n", num_users);
		// unsorted
		// print_results();
	}
	// sorted by name
	sort_by_name();
	print_results();

	if (DEBUG) printf("!!!Hola Tokens ends...");

	return EXIT_SUCCESS;
}
