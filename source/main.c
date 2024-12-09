#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "fat16.h"
#include "commands.h"
#include "output.h"

/* Show usage help */
void usage(char *executable)
{
	fprintf(stdout, "Usage:\n");
	fprintf(stdout, "\t%s -h | --help for help\n", executable);
	fprintf(stdout, "\t%s ls <fat16-img> - List files from the FAT16 image\n", executable);
	fprintf(stdout, "\t%s cp <path> <dest> <fat16-img> - Copy files from the image path to local dest.\n", executable);
	fprintf(stdout, "\t%s mv <path> <dest> <fat16-img> - Move files from the path to the FAT16 path\n", executable);
	fprintf(stdout, "\t%s rm <path> <file> <fat16-img> - Remove files from the path to the FAT16 path\n", executable);
	fprintf(stdout, "\n");
	fprintf(stdout, "\tfat16-img needs to be a valid Fat16.\n\n");
}

int main(int argc, char **argv)
{
	////////////////////////
	/// Initialization ///

	setlocale(LC_ALL, getenv("LANG"));

	// Args <= 1: Invalid argument count
	if (argc <= 1)
		usage(argv[0]),
		exit(EXIT_FAILURE);

	// Args == 2: Help argument check
	if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
		usage(argv[0]),
		exit(EXIT_SUCCESS);

	// Args > 3: Operations with FAT16 image
	else if (argc >= 3 || argc >= 4)
	{
		// File opened in binary format for read/write (rb+)
		FILE *fp = fopen(argv[argc - 1], "rb+");

		if (!fp)
		{
			fprintf(stdout, "Could not open file %s\n", argv[argc - 1]);
			exit(1);
		}

		// Create and read BIOS parameter block
		struct fat_bpb bpb;
		rfat(fp, &bpb);
		char *command = argv[1];

		verbose(&bpb);

		////////////////////////
		/// Commands ///

		// List
		if (strcmp(command, "ls") == 0)
		{
			struct fat_dir *dirs = ls(fp, &bpb);
			show_files(dirs);
		}

		// Copy
		if (strcmp(command, "cp") == 0)
		{
			cp(fp, argv[2], argv[3], &bpb);
			fclose(fp);
		}

		// Move
		if (strcmp(command, "mv") == 0)
		{
			mv(fp, argv[2], argv[3], &bpb);
			fclose(fp);
		}

		// Remove
		if (strcmp(command, "rm") == 0)
		{
			rm(fp, argv[2], &bpb);
			fclose(fp);
		}

		// Cat (Concatenate)
		if (strcmp(command, "cat") == 0)
		{
			cat(fp, argv[2], &bpb);
			fclose(fp);
		}
	}

	return EXIT_SUCCESS;
}
