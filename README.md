<h3 align="center">MRThread</h3>

-------

<p align="center"> MRThread is a userland Multi-Threading library
    <br> 
</p>

## 📝 Table of Contents

- [About](#about)
- [Usage](#usage)
- [Testing](#testing)
- [Acknowledgments](#acknowledgements)

</br>

## &#x1F537; About <a name = "about"></a>

This project is done as a part of Operating Systems course. It aims at the implementation of multithreading library using system calls in C language.

### Prerequisites

1. GNU GCC Compiler

## &#x1F537; Usage <a name = "usage"></a>

Follow the given steps to use the library

1. Include mrthread.h file in your program
2. Compile your code with appropriate implementation
```sh
$ cc -I one-one yourcode.c
$ cc -I many-one yourcode.c
```

## &#x1F537; Testing the library <a name = "testing"></a>

A script file `test.sh` will perform automated testing of the library for all implemented thread functions and applications.

For testing, run the following command in a new terminal window

```sh
$ bash test.sh
```

It will take few seconds to complete all the tests

##### You can also add your own test cases

## &#x1F537; Acknowledgements <a name = "acknowledgements"></a>

- [pthreads](https://man7.org/linux/man-pages/man7/pthreads.7.html)
