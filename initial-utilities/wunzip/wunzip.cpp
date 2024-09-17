#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <iostream>
#include <sstream>

using namespace std;

int getCount(char* countBytes) {
  int compressedCount = countBytes[3];
  for (int i = 2; i >= 0; --i) {
    compressedCount = (compressedCount << 8) | countBytes[i];
  }
  return compressedCount;
}

string unzipChunk(char* buffer) {
  // Contents should always be a string of length 0 (mod 5), since the pairs of
  // counts (four bytes) and chars (one byte) add to five. containing pairs of
  // counts and characters.

  int count = getCount(buffer);
  char c = buffer[4];

  return string(count, c);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cout << "wunzip: file1 [file2 ...]" << endl;
    return 1;
  }

  string uncompressed;
  // Concatenate the input of all compressed files into one, then output
  for (size_t i = 1; i < argc; ++i) {
    FILE* file = fopen(argv[i], "rb");

    if (!file) {
      cout << "wunzip: cannot open file" << endl;
      return 1;
    }

    size_t bufferSize = 5;
    char buffer[bufferSize];

    while (fread(buffer, 1, bufferSize, file)) {
      uncompressed += unzipChunk(buffer);
    }

    fclose(file);
  }
  cout << uncompressed;

  return 0;
}
