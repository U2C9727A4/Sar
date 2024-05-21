#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Archive Stucture

the first 4 sizeof(uint64_t) bytes of archive is used for defining starting
and ending points of the archive metadata and data blocks.
IMPORTANT! The end of the data block should also be the starting of the next file inside the archive.
The first long is meta start, second is meta end.
third is data start, fourth is data end.


           Archive.sar
[&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&]
 |_||______||____________________________|
  |    |         Data block
  |    |
  |    Metadata block
  |
  |
  Archive metadata (The longs)


*/

/* Metadata structure

Metadata is a simple char*. The first line is file name,
second is file data checksum. (Checksum of the data itself, not the archive.)

!! More lines can be added for more info as desired.



*/

int get_blocks(FILE *input_file, size_t start_at, uint64_t uint64_array[4])
{
    // Assumes file is open
    // Returns meta_start at index 0, meta_end at index 1,
    // data_start at index 3 and data_end at index 4.
    // This function reads 4 uint64_t's from the input_file at start_at
    // and saves it in uint64_array as described above.
    // Restores file seek number, thus dosen't change the inital value of it.

    uint64_t before_seek = ftell(input_file);
    fseek(input_file, start_at, SEEK_SET);
    fread(uint64_array, sizeof(uint64_t), 4, input_file);
    fseek(input_file, before_seek, SEEK_SET);

    return 0;
}

char** split(char *input, short* element_ptr, const char* seperator)
{
  /*
    This function takes in a string, splits it based on seperator and returns as a string array (char**).
  
  */
  short iterations = 0;
  short elements = 0;
  char *input_string = strdup(input);
  char* token = strtok(input_string, seperator);
  char** result = malloc(sizeof(char*));
  
  while(token)
  {
      iterations++;
      elements++;
      
      result = realloc(result, sizeof(char*) * iterations);
      result[iterations - 1] = strdup(token);
      token = strtok(NULL, seperator);
      
      
  }
  *element_ptr = elements;

  // Null terminate resulting array
  result = realloc(result, sizeof(char*) * (iterations + 1) );
  result[iterations] = NULL;
  free(input_string);

  return result;
}