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

char* inSearchPath(char* executable) {
  cout << "jere" << endl;
  char** editable = paths.data();
  for (char* &path : paths) {
    snprintf(path, paths.size(), "%s/%s", path, executable);
    if (access(path, X_OK) == 0) {
      cout << "path: " << path << endl;
      return path;
    }
  }
  cout << "here" << endl;
  return nullptr;
}

void handleStatus(int*& status) {
  if (WIFEXITED(status)) {
    cout << "Child exited with status: " << WEXITSTATUS(status) << endl;
  } else if (WIFSIGNALED(status)) {
    cout << "Child terminated by signal: " << WTERMSIG(status) << endl;
  }
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

void executeCommand(char*& command) {
  /* Parse the command into its constituent arguments with strsep */
  vector<char*> args;
  char* currArg;

  while ((currArg = strsep(&command, " ")) != NULL) {
    args.push_back(currArg);
  }

  /* Handle built-in commands with parent process */
  if (strcmp(args[0], "exit") == 0) {
    exit(0);
  } else if (strcmp(args[0], "cd") == 0) {
    if (chdir(args[1]) == -1) printErrorMessage();
  } else if (strcmp(args[0], "path") == 0) {
    for (size_t i = 1; i < args.size(); ++i) {
      paths.push_back(args[i]);
    }
    args.push_back(nullptr);
  } else {  // If not a built-in command, we fork the process and run on a child
    pid_t pid = fork();

    if (pid > 0) {  // Parent process waits for the child, processes exit status
      int* status;
      waitpid(pid, status, 0);
      cout << "waiting done" << endl;
      handleStatus(status);
    } else if (pid == 0) {  // Child process calls execv to execute command
      /* TODO: comment these two lines out after testing */
      cout << "Parsed input: " << args[0] << endl;
      for (size_t i = 1; i < args.size(); ++i) cout << " " << args[i];

      /* Check to see if the command is executable in any of the search paths */
      cout << "here" << endl;
      char* path = inSearchPath(args[0]);
      path += '/';
      cout << "Path: " << path << endl;

      if (!path) {
        printErrorMessage();
      } else {
        /* Execute the command in the CLI with execv */
        strcat(path, args[0]);
        args[0] = path;

        cout << "here" << endl;
        int rc = execv(path, args.data());
        if (rc == -1) {
          printErrorMessage();
        }
        cout << "executed: " << path << " "
             << args.data() << endl;
      }
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
    }
  }
  return 0;
}