# MyShell
Shell implementation in C  
### Features
- File redirection (>, 2>, >>)  
- Prompt change (prompt = <prompt msg>)  
- Echo command  
  - echo [<msg1> <msg2> ...]  
  - echo <var> (i.e. echo $var)  
  - echo $? - print status of last command  
- CD command (cd newpath)  
- Perform Last Command (!!)  
- Pipe (echo abc | head)   
- Variable setting ($var1 = dog)
- Read command (read var1)
- Scroll through past cmds and execute them (ArrowUp and ArrowDown, Enter)
- Support for if else statements  
    if date | grep Fri  
    then  
        echo "Yay! It is friday!"  
    else  
        echo "Back to work!"  
    fi  