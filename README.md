# Parallel Lempel-Ziv compression
Parallel lzw compression using BSP

## Build instructions

Just use 
```
>make all
```
to build the project. The output file will be in the bin directory. 

## Usage
```
> parallel_lzw [path] [window size] [lookahead] [n_cores (P)]";
```
Example: 
```
bin/parallel_lzw testfiles/yabbadabadoo.test 10 10 2

Name: testfiles/yabbadabadoo.test
Window length: 10
Look ahead length: 10
Number of processors: 2
File size: 22 bytes

(0,0,y)
(0,0,a)
(0,0,b)
(1,1,a)
(0,0,d)
(5,2,a)
(4,1,o)
(1,1,o)
(3,3,o)
(7,4,o)
1: (1,10,'o')
0: (0,0,'y')
0: (0,0,'a')
0: (0,0,'b')
0: (1,1,'a')
0: (0,0,'d')
0: (5,2,'a')
0: (4,1,'o')
```
