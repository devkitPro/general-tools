/*---------------------------------------------------------------------------------

bin2s: convert a binary file to a gcc asm module
for gfx/foo.bin it'll write foo_bin (an array of char)
foo_bin_end, and foo_bin_len (an unsigned int)
for 4bit.chr it'll write _4bit_chr, _4bit_chr_end, and
_4bit_chr_len


Copyright 2003 - 2005 Damian Yerrick
Copyright 2004 - 2017 Dave (WinterMute) Murphy

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

---------------------------------------------------------------------------------*/



/*
.align
.global SomeLabel_len
.int 1234
.global SomeLabel
.byte blah,blah,blah,blah...
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>

/*---------------------------------------------------------------------------------
Print the closest valid C identifier to a given word.
---------------------------------------------------------------------------------*/
void strnident(FILE *fout, const char *src, int apple_llvm ) {
//---------------------------------------------------------------------------------
char got_first = 0;

while(*src != 0) {

	int s = *src++;

	/* prepend _ for apple-llvm mode or initial digit  */
	if((isdigit(s) || apple_llvm) && !got_first)
	fputc('_', fout);  /* stick a '_' before an initial digit */

	/* convert only out-of-range characters */
		if(!isalpha(s) && !isdigit(s) && (s != '_')) {
			if(s == '-' || s == '.' || s == '/') s = '_';
			else
				s = 0;
		}

		if(s) {
			fputc(s, fout);
			got_first = 1;
		}
	}
}

//---------------------------------------------------------------------------------
void showhelp(char *name) {
//---------------------------------------------------------------------------------
	fprintf( stderr, "%s - convert binary files to assembly language\n", name);

	fprintf( stderr, "usage: %s [option ...] [binary files ...]\n", name);

	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -h, --help        show this help\n");
	fprintf(stderr, "  -a, --alignment   set parameter for .align\n");
	fprintf(stderr, "  --apple-llvm      output for apple assembler\n");

}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	FILE *fin;
	size_t filelen;
	int linelen;
	int arg;
	int alignment = 4;
	static int apple_llvm = 0;

	if(argc < 2) {
		showhelp(argv[0]);
		return -1;
	}

	int c;

	while (1) {
		static struct option long_options[] = {
			{"apple-llvm", no_argument, &apple_llvm, 1},
			{"alignment",  no_argument, 0, 'a'},
			{"help",       no_argument, 0, 'h'},
			{0, 0, 0, 0}
		};

		int option_index = 0;

		c = getopt_long (argc, argv, "a:h",
			long_options, &option_index);
		if (c == -1)
			break;

		switch(c) {
			case 'h':
			showhelp(argv[0]);
			return 0;
			case 'a':
			alignment = strtoul(optarg,0,0);
			break;
			case '?':
			if (optopt == 'a')
				fprintf (stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint (optopt))
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf (stderr, "Unknown option character `\\x%x'.\n",optopt);
			return 1;
		}
	}

	for(arg = optind; arg < argc; arg++) {

		fin = fopen(argv[arg], "rb");

		if(!fin) {
			fputs("bin2s: could not open ", stderr);
			perror(argv[arg]);
			return 1;
		}

		fseek(fin, 0, SEEK_END);
		filelen = ftell(fin);
		rewind(fin);

		if(filelen == 0) {
			fclose(fin);
			fprintf(stderr, "bin2s: warning: skipping empty file %s\n", argv[arg]);
			continue;
		}

		char *ptr = argv[arg];
		char chr;
		char *filename = NULL;

		while ( (chr=*ptr) ) {

			if ( chr == '\\' || chr == '/') {

				filename = ptr;
			}

			ptr++;
		}

		if ( NULL != filename ) { 
			filename++;
		} else {
			filename = argv[arg];
		}

	/*---------------------------------------------------------------------------------
		Generate the prolog for each included file.  It has two purposes:
		
		1. provide length info, and
		2. align to user defined boundary, default is 32bit 

	---------------------------------------------------------------------------------*/
		fprintf( stdout, "/* Generated by BIN2S - please don't edit directly */\n");

		if (apple_llvm) {
			fprintf( stdout, "\t.const_data\n");
		} else {
			fprintf( stdout,"\t.section .rodata\n");
		}

		fprintf( stdout, "\t.balign %d\n"
			"\t.global ", alignment);
		strnident(stdout, filename, apple_llvm);
		fputs("_size\n", stdout);
		fputs("\t.global ", stdout);
		strnident(stdout, filename, apple_llvm);
		fputs("\n", stdout);
		strnident(stdout, filename, apple_llvm);
		fputs(":\n\t.byte ", stdout);

		linelen = 0;

		int count = filelen;
		
		while(count > 0) {
			unsigned char c = fgetc(fin);
			
			printf("%3u", (unsigned int)c);
			count--;
			
			/* don't put a comma after the last item */
			if(count) {

				/* break after every 16th number */
				if(++linelen >= 16) {
					linelen = 0;
					fputs("\n\t.byte ", stdout);
				} else {
					fputc(',', stdout);
				}
			}
		}

		fputs("\n\n\t.global ", stdout);
		strnident(stdout, filename, apple_llvm);
		fputs("_end\n", stdout);
		strnident(stdout, filename, apple_llvm);
		fputs("_end:\n\n", stdout);
		fputs("\t.balign 4\n",stdout);
		strnident(stdout, filename, apple_llvm);
		fprintf( stdout, "_size: .int %lu\n", (unsigned long)filelen);

		fclose(fin);
	}

	return 0;
}

