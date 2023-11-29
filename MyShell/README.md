# MyShell
Shell implementation in C  
## Disclaimer (Code Assumptions)
- Correct command syntax
- File redirection is only possible in commands without pipe (`|`)
- Commands are capped at 1023 characters (arbitrary buffer size)
- Max argument amount in each command is 10 (again due to arbitrary buffer size)
- Commands that change info in the main process cannot happen in pipes (rename dir, change prompt, etc..)
- Ctrl+D causes unexpected behavior
## Features
- File redirection (>, 2>, >>)  
- Prompt change (prompt = <prompt msg>)  
- Echo command  
  - echo [\<msg1\> \<msg2\> ...]  
  - echo <var> (i.e. echo $var)  
  - echo $? - print status of last command  
- CD command (cd newpath)  
- Perform Last Command (!!)  
- Pipe (echo abc | head)   
- Variable setting ($var1 = dog)
- Read command (read var1)
- Scroll through past cmds and execute them (ArrowUp and ArrowDown, Enter)
- Support for if else statements  
&nbsp;&nbsp;&nbsp;&nbsp;if date | grep Fri  
&nbsp;&nbsp;&nbsp;&nbsp;then  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;echo "Yay! It is friday!"  
&nbsp;&nbsp;&nbsp;&nbsp;else  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;echo "Back to work!"  
&nbsp;&nbsp;&nbsp;&nbsp;fi
## Usage

1. Compile program with GCC: ` $make`
2. Run myshell: `$ ./myshell`
3. To exit program (ctrl+C is overridden on purpose): `quit`
4. You can use one of the input files I've included: `./myshell < example_inputs/iftest
