// first hello world sample

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h> // for tolower

#define MAX_FILE_SIZE 5000
#define MAX_WORDS 750
#define MAX_WORD_SIZE 34
#define PRINT_TEXT 1 // allows to print entire text file as it is read for debugging. 1 for printing, 0 for no printing


// Global variables
typedef struct {
	// holds one cell of a word and how many times it is repeated
	char word[MAX_WORD_SIZE];
	int repeats; // not required by assignment, added as expansion
} WORD_AND_REPEATS;

typedef struct {
	int len; // the length of words in this block
	int total_word_count; // how many total words of this length there are
	int distinct_word_count; // how many distinct words of this length there are
	WORD_AND_REPEATS word_list[MAX_WORDS]; // list of all distinct words of similar length and how many repeats per word
}	data;

int num_words = 0;
int num_lines = 0;
FILE *fp;
char *fname;
char buf[MAX_FILE_SIZE]; // this is where all words go
data words[MAX_WORD_SIZE];
int sort = 0; // are we sorting by length
int print_words = 0; // are we printing words
int infile = 0;



// Prototypes
void init_counters(data [MAX_WORD_SIZE+1]); // inits global counters with proper values
	// for total_counter, [i][0] corresponds to i, and is needed to preserve the index as it gets sorted
void lcase(char *); // converts a word to lowercase
void dump_words(void); // dumps the words array out to screen
void dump_counter(void); // displays counter array, for non 0 lengths only;
void read_multiline_file(FILE *fp, char *buf); // gets a validated, open handle to an open file and dumps all of it into buf, line by line
void tokenize(char *); // recieves a pointer to a null terminated string and separates into the words global array
void validate_arguments(int argc, char* argv[]);
void sort_by_incidence(data [MAX_WORD_SIZE]); // sorts words array by descending total_word_count
void lexicographical_word_sort(data [MAX_WORD_SIZE]); // sorts words array (per length) lexicographically in each "column" of word length

//Implementation

void init_counters(data words[]) {
	// 0 out the data holding structure
	int i,j;
	for (i=0; i<MAX_WORD_SIZE; i++) {
		words[i].len = i+1;
		words[i].total_word_count = 0;
		words[i].distinct_word_count = 0;
		for (j=0; j<MAX_WORDS; j++) {
			// words[i].word_list[j].word = "\0";
			strncpy(words[i].word_list[j].word,"\0",1);
			words[i].word_list[j].repeats = 0;
		}
	}
	return;
}

void lcase(char *word) {
	int i;
	for (i=0; word[i]; i++) {
		word[i] = tolower(word[i]);
	}
}

void read_multiline_file(FILE *fp, char buf[]) {
	char t[MAX_FILE_SIZE]; // temporary to append to buf
	int chars_left = MAX_FILE_SIZE; // how many characters can we read without overstepping max size
	// print entire file here
	if (PRINT_TEXT) { printf("---------------------- FILE START --------------------------------\n"); }
	while(fgets(t, chars_left, fp)!=NULL) {
		if (PRINT_TEXT) {printf("%s",t);}
		strcat(buf,t);
		chars_left = chars_left - strlen(buf);
		num_lines++;
	}
	if (PRINT_TEXT) {printf("---------------------- FILE END --------------------------------\n");}
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
		if (strncmp(argv[i],"--print-words",13) == 0  && strlen(argv[i])==13) { 
			// printf ("--print-words flag detected\n");
			print_words = 1;
		}
		if (strncmp(argv[i],"--sort",6) == 0 && strlen(argv[i])==6) { 
			// printf ("--sort flag detected\n");
			sort = 1;
		}
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
	for (i=0; i<MAX_WORD_SIZE; i++) {
		if (words[i].total_word_count > 0) {
			printf("Count[%02d]: Total words: %02d. Distinct words: %02d\n",words[i].len, words[i].total_word_count, words[i].distinct_word_count );
			
			if (print_words) {
				printf("Word list: ");
				for (j=0; j<words[i].distinct_word_count; j++) {
					printf("%s: %02d | ", words[i].word_list[j].word, words[i].word_list[j].repeats);
				}
				printf("\n");		
			}
			
		}
			
	}
	return;
}


void tokenize(char *buf) {
	// buf contains null terminated string
	char *t;
	// only expected non std characters in the file are .,;()
	char delim[]=" .,;()\n"; // assuming that newline characters are just delimiters

	// for 1.2, we need to account for sorting the array
	// for 1.3 we need to record information re each word (converted to lcase) and its incidence

	t = strtok(buf,delim);
	int j;
	data *temp_word_record;

	while (t && num_words < MAX_WORDS) {
		lcase(t);
		// have we seen this word yet? if so, find it and aggregate, if not, add it
		j=0;
		temp_word_record = &words[strlen(t)-1]; // len 1 is stored in words[0]; // trying to work with pointers
		while (j<(*temp_word_record).distinct_word_count && strncmp(t,(*temp_word_record).word_list[j].word,strlen(t))!=0) {
			j++;
		}
		//at this point j is at first index that either is out of use or is at word that needs to be incremented
		if (strncmp(t,(*temp_word_record).word_list[j].word,strlen(t))!=0) {
			// word not detected in list, add it
			strncpy((*temp_word_record).word_list[j].word,t,MAX_WORD_SIZE);
			(*temp_word_record).distinct_word_count++;
		}
		// for either options, either if the word is new, or if its an existing word, increase total number of words and individual repeats
		(*temp_word_record).total_word_count++;
		(*temp_word_record).word_list[j].repeats++;
		
		num_words++;
		t = strtok(NULL,delim);
	}
	return;
}

/* qsort struct comparision function (by total_word_count) */ 
int compare_by_total_word_count(const void *a, const void *b) 
{ 
	// FROM http://anyexample.com/programming/c/qsort__sorting_array_of_strings__integers_and_structs.xml
    data *ia = (data *)a;
    data *ib = (data *)b;
    return (int)(ib->total_word_count - ia->total_word_count);
	/* returns negative if b > a 
	and positive if a > b. */ 
 
} 

void sort_by_total_word_count_descending(data words[MAX_WORD_SIZE]) {
	// order can be ASCENDING or DESCENDING
	qsort(words, MAX_WORD_SIZE, sizeof(data),compare_by_total_word_count);
	return;
}

int compare_by_word(const void *a, const void *b) 
{ 
	// FROM http://anyexample.com/programming/c/qsort__sorting_array_of_strings__integers_and_structs.xml
    WORD_AND_REPEATS *ia = (WORD_AND_REPEATS *)a;
    WORD_AND_REPEATS *ib = (WORD_AND_REPEATS *)b;
    return strncmp(ia->word, ib->word,MAX_WORD_SIZE);
	/* strcmp functions works exactly as expected from
	comparison function */ 
} 

void lexicographical_word_sort(data words[MAX_WORD_SIZE]) {
	int i;
	for (i=0; i<MAX_WORD_SIZE; i++) {
		if (words[i].distinct_word_count) {
			// if have any words of this length, sort them
			qsort(words[i].word_list, words[i].distinct_word_count, sizeof(WORD_AND_REPEATS), compare_by_word);
		}
	}
	return;
	
}

// Main
int main(int argc, char *argv[]) {

	init_counters(words); 

	validate_arguments(argc,argv); 
	// if got this far, all contents of file are now in buf
	tokenize(buf);
	if (sort) {
		// sort the words array/struct by total_word_count ASCENDING/DESCENDING
		printf("--sort flag detected. Sorting descending by total_word_count...\n");
		sort_by_total_word_count_descending(words);
	}

	if (print_words) {
		/*sort and print individual words of each length
		if this flag isnt enabled, theres' no reason to sort individual words by alphanumeric*/
		printf("--print-words flag detected. Sorting words lexicographically before printing...\n");
		lexicographical_word_sort(words);
	}
	dump_words();
	return 0;
}

