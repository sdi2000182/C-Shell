# Custom Shell (mysh) - README

## Table of Contents
- [Introduction](#introduction)
- [Features](#features)
  - [I/O Redirection](#io-redirection)
  - [Appending to Existing Files](#appending-to-existing-files)
  - [Pipes](#pipes)
  - [Background Execution](#background-execution)
  - [Wildcards](#wildcards)
  - [Alias Management](#alias-management)
  - [Signal Handling](#signal-handling)
  - [Command History](#command-history)

## Introduction

Custom Shell (mysh) is a Unix-like shell ,it t is designed to be an interactive command interpreter that offers a wide range of capabilities.

## Features

### I/O Redirection

I/O redirection allows you to manipulate input and output streams for commands. Here are some examples:

- Redirect command output to a file:
  ```bash
  in-mysh-now:> myProgram > out.txt
  - Redirect command input from a file:
  ```bash
  in-mysh-now:> myProgram > input.txt
  -Use those features at the same time
  '''bash
  in-mysh-now:> sort < file1 > file2
   -Append to a file using input from another
  '''bash
  in-mysh-now:> cat file1 >> file2

### Pipes

Pipes are a fundamental feature of Custom Shell (mysh) that enable you to create powerful data pipelines by connecting multiple commands. With pipes, you can seamlessly pass the output of one command as the input to another, allowing for efficient data processing.

To use pipes, simply use the `|` symbol to chain commands together, like this:

```bash
in-mysh-now:> command1 | command2 | command3

### Background Execution (&)

Custom Shell (mysh) supports background execution, allowing you to run commands concurrently. You can use the `&` symbol to execute a command in the background, which means it runs independently while you continue to use the shell.

Here's how to use it:

```bash
in-mysh-now:> command1 &; command2 &;

#### Alias Management

```markdown
### Alias Management

Custom Shell (mysh) provides alias management, allowing you to create and use custom command aliases for frequently used commands. An alias is a shortcut for a longer command, making your workflow more efficient.

Here's how to create and use aliases:

- Create an alias:
  ```bash
  in-mysh-now:> createalias myhome "cd /home/users/smith";
-Use and alias:
'''bash
in-mysh-now:> myhome
-Destory an alias:
'''bash
in-mysh-now:> destroyalias myhome;

#### Wildcards

```markdown
### Wildcards

Wildcards are a powerful feature in Custom Shell (mysh) that allow you to specify a subset of files in the current directory based on patterns. They are particularly useful for selecting multiple files that match a specific naming convention.

Here's an example:

```bash
in-mysh-now:> ls file*.txt
in-mysh-now:> ls f*le?.txt


#### Command History

```markdown
### Command History

Custom Shell (mysh) maintains a command history, which is a record of the last 20 commands you entered. This feature allows you to easily recall and rerun previous commands without retyping them, saving you time and effort.

Here's how to navigate your command history:

- in-mysh-now:> myhistory
Displays the last 20 commands
- in-mysh-now:> myhistory 3
Executes the 3rd command from the myhistory list
