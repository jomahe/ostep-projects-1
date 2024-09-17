#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

using namespace std;

void compressContents(string& contents) {
  /* Consider first char. Count occurrences until reaching another char, push to
     output string.
   */

  size_t len = contents.size();
  if (len == 0) return;

  char currChar = contents[0];
  int currCount = 1;

  for (size_t i = 1; i < len; ++i) {
    if (contents[i] == currChar) {
      ++currCount;
    } else {
      fwrite(&currCount, sizeof(int), 1, stdout);
      fwrite(&currChar, sizeof(char), 1, stdout);

      currCount = 1;
      currChar = contents[i];
    }
  }
  fwrite(&currCount, sizeof(int), 1, stdout);
  fwrite(&currChar, sizeof(char), 1, stdout);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cout << "wzip: file1 [file2 ...]" << endl;
    return 1;
  }
  string concatenated;

  // Need to concatenate the contents of all input files
  for (size_t i = 1; i < argc; ++i) {
    FILE* file = fopen(argv[i], "r");

    if (!file) {
      cout << "wzip: cannot open file" << endl;
      return 1;
    }

    size_t bufferSize = 1024;
    char buffer[bufferSize];
    ssize_t bytesRead = fread(buffer, 1, bufferSize, file);

    do {
      concatenated.append(buffer, bytesRead);
      bytesRead = fread(buffer, 1, bufferSize, file);
    } while (bytesRead > 0);

    fclose(file);
  }
  // Finally compress the contents of the files concatenated
  compressContents(concatenated);

  return 0;
}
