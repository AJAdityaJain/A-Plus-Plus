
# The A++ Language

## What is A++?

> A++ is a blazingly fast ***COMPILED*** language 😎. It converts its code directly into assembly for FASM. 
> It is ***NOT*** transpiled into C or C++ like other languages because that is lame 😒.
> - [x] Blazing speeds
> - [x] Simple syntax
> - [x] Pseudo-dynamically typed
> - [x] Made with love
> - [x] No dependencies
> - [x] 16-byte Stack aligned
> ### Why is this even a thing?
> <a href='https://en.wikipedia.org/wiki/A%2B_(programming_language)'>A and A+</a> existed 40 years ago, but became obsolete with the arrival of C and C++
> Then I was bored on a Tuesday and decided to start this journey.


## Requirements
> None 😉

> [!IMPORTANT]
> Currenty only ready for Intel x64. Rest is uncharted territory.

## Install
> Just download the latest release and run the installer.


## Syntax
> The syntax is very mildly Pythonic
> None of that indentation nonsense though, we love our {CURLY BRACES}
> 
### Example
 - Variable definition and assignment
```python 
func main(){
    f = 25; #Defining variables

    y = 2.2; #Float
    y *= f; #Integers auto casts to float
    
    z = cast(int, f * y); #Manual cast down float to int
    z %= 50; #Augmented assignment
    
    write(z, '\n' ,y,'\n');
    
    @pi = 3.14; #Defining constants
write(pi);
}
```

 - If-else
 ```python
func main(){    
    write('Enter your age:');
    age = -1; #Default value
    read(age);

    if(age >= 18){
        write('You can drive');
    }
    else
        write('Can\'t drive');
    
    write('\nEnd');
}

```

- Loops
```python

func main(){
    x = 0;
    while(x < 10){
        write("Value of x is ", x, "\n");
        x += 1;
    }
}

```
```python
func main(){    

    start_inclusive = 2;
    end_exclusive = 12;
    
    #loop from start_inclusive to end_exclusive - 1
    loop start_inclusive:end_exclusive with iter{
        write(iter,'\n');
    }

    #pool keword for reverse loop from 9 to 0
    pool 9:-1 with j{
        write(j,'\n');
    }
}
```