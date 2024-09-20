#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

bool isSameFileCheck(FILE* f1, FILE* f2) {
  struct stat stat1, stat2;
  int fd1 = fileno(f1), fd2 = fileno(f2);

  // Get file metadata
  if (fstat(fd1, &stat1) != 0 || fstat(fd2, &stat2) != 0) {
    cout << "fstat failed" << endl;
    return false;
  }

  // Files are the same if their device and inode number are the same
  return (stat1.st_dev == stat2.st_dev && stat1.st_ino == stat2.st_ino);
}

void acceptInput(vector<string>& contents) {
  string currLine;
  while (true) {
    getline(cin, currLine);

    if (currLine.empty()) break;
    contents.push_back(currLine + '\n');
  }
}

void parseInput(FILE*& file, vector<string>& processed) {
  if (!file) return;

  char* line = nullptr;
  size_t lineLen = 0;

  while (getline(&line, &lineLen, file) != -1) {
    processed.push_back(string(line));
  }
  free(line);
}

int main(int argc, char* argv[]) {
  if (argc > 3) {
    cerr << "usage: reverse <input> <output>" << endl;
    exit(1);
  }

  vector<string> processedInput;

  FILE* in = nullptr;
  FILE* out = stdout;

  // If user provides no arguments, we accept input text from stdin and we print
  // the output to stdout
  if (argc == 1) {
    acceptInput(processedInput);
  } else {
    char* inputFile = argv[1];

    if (argc == 3) {
      char* outputFile = argv[2];

      if (!(out = fopen(outputFile, "w"))) {
        cerr << "error: cannot open file '" << outputFile << "'" << endl;
        exit(1);
      }
    }

    // Attempt to open both files and report errors if applicable
    if (!(in = fopen(inputFile, "r"))) {
      cerr << "reverse: cannot open file '" << inputFile << "'" << endl;
      exit(1);
    }

    if (isSameFileCheck(in, out)) {
      cerr << "reverse: input and output file must differ" << endl;
      exit(1);
    }
  }
  // Parse the input, routine will differ if it's a file or CLI
  parseInput(in, processedInput);
  fclose(in);

  // Use int64s here to acount for long filesizes
  int64_t lines = processedInput.size();
  for (int64_t i = lines - 1; i > -1; --i) {
    fprintf(out, (processedInput[i]).c_str());
  }
  fclose(out);

  return 0;
}
