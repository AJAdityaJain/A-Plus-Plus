
# The A++ Language

## What is A++?

> A++ is a blazingly fast ***COMPILED*** language 😎. It converts its code directly into assembly for FASM. 
> It is ***NOT*** transpiled into C or C++ like other languages because that is lame 😒.
> - [x] Blazing speeds
> - [x] Simple syntax
> - [x] Pseudo-dynamically typed
> - [x] Made with love
> - [x] No dependencies
> - [x] 64-bit Stack aligned
> ### Why is this even a thing?
> <a href='https://en.wikipedia.org/wiki/A%2B_(programming_language)'>A and A+</a> existed 40 years ago, but became obsolete with the arrival of C and C++
> Then I was bored on a Monday and decided to start this journey.


## Requirements
> None 😉

> [!IMPORTANT]
> Currenty only ready for Intel x64. Rest is uncharted territory.

## Install
> Just download the latest release and run the installer.


## Syntax
> The syntax is very similar to Python but with {CURLY BRACES}.
> None of that indentation nonsense.
> 
### Example
 - Variable definition and assignment
```python 
x = 12;    #Integers
y = 1.05;  #Floats
z = 1.05d; #Doubles
z = y+x;   #Dynamic casting and assignment
x = 0;
```

 - If-else
 ```python
if(x == 0){
    write("Value of x is 0\n");
}
else if (x > 0)
    write("Value of x is more than 0\n");
```

- Loops
```python
x = 0;
while(x < 10){
    write("Value of x is ", x, "\n");
    x += 1;
}
```
