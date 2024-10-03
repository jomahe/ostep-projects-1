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

vector<char*> paths;

inline void printErrorMessage() {
  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
}

void overwritePaths(vector<char*> &args) {
  args.erase(args.begin());
  paths = move(args);
  cout << "new size of paths: " << paths.size() << endl;
}

char* inSearchPath(char* executable) {
  char fullPath[1024];
  for (char* &path : paths) {
    snprintf(fullPath, sizeof(fullPath), "%s/%s", path, executable);
    if (access(fullPath, X_OK) == 0) {
      return strdup(fullPath);
    }
  }
  return nullptr;
}

void parseInput(FILE*& file, vector<char*>& commands) {
  if (!file) {
    cerr << "Invalid file!" << endl;
    exit(1);
  }

  char* line = nullptr;
  size_t lineLen = 0;

  while (getline(&line, &lineLen, file) != 0) {
    commands.push_back(line);
  }

  free(line);
}

void executeCommand(char* &command) {
  /* Parse the command into its constituent arguments with strsep */
  vector<char*> args;
  char* currArg;
  char* commandCopy = strdup(command);

  while ((currArg = strsep(&commandCopy, " ")) != NULL) {
    args.push_back(currArg);
  }
  free(commandCopy);

  /* Handle built-in commands with parent process */
  if (strcmp(args[0], "exit") == 0) {
    // Exit shouldn't be called with extra arguments
    if (args.size() == 1) {
      exit(0);
    }
    printErrorMessage();
  } else if (strcmp(args[0], "cd") == 0) {
    if (chdir(args[1]) == -1) printErrorMessage();
  } else if (strcmp(args[0], "path") == 0) {
    overwritePaths(args);
  } else {  // If not a built-in command, we fork the process and run on a child
    pid_t pid = fork();

    if (pid > 0) {  // Parent process waits for the child, processes exit status
      waitpid(pid, nullptr, 0);
    } else if (pid == 0) {  // Child process calls execv to execute command
      /* TODO: comment these two lines out after testing */
      cout << "Parsed input: " << args[0] << endl;
      for (size_t i = 1; i < args.size(); ++i) cout << " " << args[i];

      /* Check to see if the command is executable in any of the search paths */
      char* path = inSearchPath(args[0]);

      if (!path) {
        printErrorMessage();
      } else {
        /* Execute the command in the CLI with execv */

        int rc = execv(path, args.data());
        if (rc == -1) {
          printErrorMessage();
        }
      }
      exit(0);
    } else {
      printErrorMessage();
    }
  }
}

char* acceptInput() {
  string currLine;
  getline(cin, currLine);

  return strdup(currLine.c_str());
}

int main(int argc, char* argv[]) {
  paths.push_back("/bin");
  // We should be invoking the executable with either one or zero arguments
  if (argc > 2) {
    cerr << "Usage: ./wish <OPTIONAL FILENAME>" << endl;
    exit(1);

  } else if (argc == 2) {
    /* Read input from a batch file, execute the commands therein */
    FILE* in = nullptr;
    vector<char*> commands;

    if (!(in = fopen(argv[2], "r"))) {
      cerr << "wish: cannot open file: '" << argv[2] << "'" << endl;
      exit(1);
    }

    parseInput(in, commands);
    for (char*& command : commands) {
      executeCommand(command);
    }
  } else {
    /* Enter interactive mode; allow user to type in one command at a time */
    while (true) {
      cout << "wish> ";
      char* command = acceptInput();
      executeCommand(command);
      free(command);
    }
  }
  return 0;
}