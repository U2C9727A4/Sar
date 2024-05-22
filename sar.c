#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

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
    // Returns -1 if it hits EOF on the archive.

    uint64_t before_seek = ftell(input_file);
    fseek(input_file, start_at, SEEK_SET);
    if ((fread(uint64_array, sizeof(uint64_t), 4, input_file)) == 0)
    {
      return -1;
    }
    fseek(input_file, before_seek, SEEK_SET);

    return 0;
}

char** read_metadata(uint64_t blocks[4], FILE* input_file)
{
  /*
    WARNING! This function returns a heap-allocated string. If this function fails, it returns NULL and frees output.
    If it dosen't fail, the string it returns must be freed by the scope calling the function.
    This function reads metadata from the archive, returns a char* (Which is the entire metadata)
     And split() must be used to make sense of the data.
    */
  uint64_t oldSeek = ftell(input_file);
  fseek(input_file, blocks[0], SEEK_SET);
  char* output = malloc(blocks[1] - blocks[0]);
  if ((fread(output, (blocks[1] - blocks[0]), 1, input_file)) != 0)
  {
    free(output);
    fseek(input_file, oldSeek, SEEK_SET);
    return NULL;
  }
  fseek(input_file, oldSeek, SEEK_SET);
  return output;

}
char* read_file_perms(const char* file_path)
{
  /*
  This function reads file permission, and returns its numeric representation as a char*.
  Note: This function is designed to be directly writeable to archive metadata, and thus return is newline terminated.
  */
 struct stat file_stat;
 stat(file_path, &file_stat);
 char* permissions;
 sprintf(permissions, "%o\n", file_stat.st_mode & 0777);
 return permissions;
}



char** split(char *input, short* element_ptr, const char* seperator)
{
  /*
    This function takes in a string, splits it based on seperator and returns as a string array (char**).
    !! The inputted string must be null terminated.
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