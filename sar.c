#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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
second is file permissions
the very last is file data checksum. (Checksum of the data itself, not the archive.)

!! More lines can be added for more info as desired.



*/

int get_blocks(FILE *input_file, size_t start_at, uint64_t uint64_array[4])
{
    // Assumes file is open
    // Assumes it is at the correct place to start reading archive metadata.
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

char* read_metadata(uint64_t blocks[4], FILE* input_file)
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

void free_string_arr(char** array, int elements)
{
    // Frees arrays made by split().
    int i = 0;
    for (i = 0; elements > i; i++)
    {
      free(array[i]);
    }
    free(array);
}

char** split(char *input, short* element_ptr, const char* seperator)
{
  /*
    This function takes in a string, splits it based on seperator and returns as a string array (char**).
    !! The inputted string must be null terminated.4
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

int write_metadata(FILE* output_file, const char* filepath)
{
  // Writes file metadata of filepath to output_file
  // Assumes the current file pointer is the next archive file to write on, so it skips over first 4 sizeof(uint64_t) bytes
  // IMPORTANT NOTE Leaves an extra 512 bytes of space at the end of metadata, for writing data hash later on.
  uint64_t blocks[4];
  get_blocks(output_file, ftell(output_file), blocks);
  fseek(output_file, (4 * sizeof(uint64_t)), SEEK_CUR);
  blocks[0] = ftell(output_file); // Write start of metadata

  char* file_perms = read_file_perms(filepath);
  int elements;
  char** file_name_raw = split(filepath, &elements, "/");
  char* file_name = strdup(file_name_raw[elements]);

  free_string_arr(file_name_raw, elements);
  uint64_t metadata_size = sizeof(file_name) + sizeof(file_perms) + 512; // The +512 is for the trailing hash
  char* metadata = malloc(metadata_size);
  blocks[1] = metadata_size; // Write end of metadata block to archive metadata
  sprintf(metadata, "%s\n%s\n", file_name, file_perms);
  // Assume the file pointer is currently at the correct location to start writing metadata
  fwrite(metadata, metadata_size, 1, output_file);

  free(metadata);
  free(file_name);
  return 0;

}
int write_data()
{
  /*
  Takes in file_input, writes its data to archive. Also sets the required bytes and stuff
  */

  return 0;
}