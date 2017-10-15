// first hello world sample

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h> // for tolower

#define MAX_FILE_SIZE 5000
#define MAX_WORDS 750
#define MAX_WORD_SIZE 34

// only expected non std characters in the file are .,;()

// Global variables
int num_words = 0;
int num_lines = 0;
FILE *fp;
char *fname;
char buf[MAX_FILE_SIZE]; // this is where all words go
int length_ticker[MAX_WORD_SIZE+1][MAX_WORDS]; // this one descriminates between different words of the same length, counts how many of each there are per length
int counter[MAX_WORD_SIZE+1]; // piles up all words of same length, counts DISTINCT words of each length
int total_counter[MAX_WORD_SIZE+1]; // counts total # of words of each length
char words[MAX_WORD_SIZE+1][MAX_WORDS][MAX_WORD_SIZE];
int sort = 0; // are we sorting by length
int print_words = 0; // are we printing words
int infile = 0;

// Prototypes
void lcase(char *); // converts a word to lowercase
void dump_words(void); // dumps the words array out to screen
void dump_counter(void); // displays counter array, for non 0 lengths only;
void read_multiline_file(FILE *fp, char *buf); // gets a validated, open handle to an open file and dumps all of it into buf, line by line
void tokenize(char *); // recieves a pointer to a null terminated string and separates into the words global array
void validate_arguments(int argc, char* argv[]);

//Implementation

void lcase(char *word) {
	int i;
	for (i=0; word[i]; i++) {
		word[i] = tolower(word[i]);
	}
}

void read_multiline_file(FILE *fp, char buf[]) {
	char t[MAX_FILE_SIZE]; // temporary to append to buf
	int chars_left = MAX_FILE_SIZE; // how many characters can we read without overstepping max size
	while(fgets(t, chars_left, fp)!=NULL) {
		printf("%s",t);
		strcat(buf,t);
		chars_left = chars_left - strlen(buf);
		num_lines++;
	}
	return;
}

void validate_arguments(int argc, char* argv[]) {
	printf("Word count by Arkadi\n");

	if (argc == 1) {
		printf("no arguments supplied.\n");
		exit(1);
	}

	// if reached here, some arguments exist
	// only expected arguments are --sort --print-words --infile

	for (int i=0; i<argc; i++) {
		if (strncmp(argv[i],"--print-words",13) == 0  && strlen(argv[i])==13) { printf ("--print-words flag detected\n"); print_words = 1;}
		if (strncmp(argv[i],"--sort",6) == 0 && strlen(argv[i])==6) { printf ("--sort flag detected\n"); sort = 1;}
		if (strncmp(argv[i],"--infile",8) == 0 && strlen(argv[i])==8) {
			printf ("--infile flag detected\n"); 
			infile = 1;
			if (i == argc-1) {
				// --infile is last argument, no file specified
				printf("not enough arguments, no file name specified. Exiting...\n");
				exit (1);
			} else {
				fname = argv[i+1];
				// printf("file name specified: %s\n", fname);
			}
		}
			
	}

	if (!infile) {
		printf ("no input file specified.\n");
		exit(1);
	} else {
		// test input file
		printf("File name to open: %s\nAttempting file open...\n", fname);
		fp = fopen(fname, "r");
		if (fp) {
			printf("File opened successfully: %s\nProcessing...\n",fname);
			// found good file, dump all contents into buf	
			read_multiline_file(fp,buf);
			
			fclose(fp);
		} else {
			printf("Counldn\'t open %s\nExiting...\n", fname);
			exit(1);
		}
	}
	return;
}

void dump_words() {

	printf("Initiate word dump. Total words: %d, Total lines: %d\n", num_words, num_lines);

	int i,j;
	for (i=0; i<num_words; i++) {
		for (j=0; j<counter[i]; j++) {
			printf("length %d. Word: %s. Repeats: %02d \n", i , words[i][j],length_ticker[i][j]);	
		}
		
	}
	return;
}

void dump_counter() {
	int i;
	printf("Printing counter array: Distinct words of each length\n");
	for (i=1; i<MAX_WORD_SIZE+1; i++) {
		if (counter[i] >0) {
			printf("Counter[%02d]: %02d\n",i,counter[i]);
		}
	}

	printf("Printing counter array: TOTAL number of words of each length\n");
	for (i=1; i<MAX_WORD_SIZE+1; i++) {
		if (total_counter[i] >0) {
			printf("Counter[%02d]: %02d\n",i,total_counter[i]);
		}
	}
	return;
}

void tokenize(char *buf) {
	// buf contains null terminated string
	char *t;
	char delim[]=" .,;()\n"; // assuming that newline characters are just delimiters

	// for 1.2, we need to account for sorting the array
	// for 1.3 we need to record information re each word (converted to lcase) and its incidence

	t = strtok(buf,delim);
	int i;
	while (t && num_words < MAX_WORDS) {
		lcase(t);
		// have we seen this word yet? if so, find it and aggregate, if not, add it
		i=0;
		while (i<=counter[strlen(t)] && strncmp(t,words[strlen(t)][i],strlen(t))!=0) {
			i++;
		}
		if (strncmp(t,words[strlen(t)][i],strlen(t))==0) {
			// found same word, just increase individual len
			length_ticker[strlen(t)][i]++;	
		} else {
			// didnt find same word, got to add a new value
			strncpy(words[strlen(t)][counter[strlen(t)]],t,MAX_WORD_SIZE);
			length_ticker[strlen(t)][counter[strlen(t)]]++;
			counter[strlen(t)]++; // distinct words of each length
		}

		total_counter[strlen(t)]++; // distinct words of each length
		
		num_words++;
		t = strtok(NULL,delim);
	}
	return;
}

// Main
int main(int argc, char *argv[]) {

	// int i; 

	validate_arguments(argc,argv); 
	// if got this far, all contents of file are now in buf
	tokenize(buf);
	if (sort) {

	}
	dump_words();
	dump_counter();

	return 0;
}

