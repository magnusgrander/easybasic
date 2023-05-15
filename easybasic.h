#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <cstdlib>
#include <iostream>

class easybasic
{

private:     
    //enum {false, true};
    typedef char* CHARP;
    int *gosub_stackp,gosub_stack[999],line[999],line_off[999], lim[999],linenum,i,inquote;
    long var[999];
    int *gosub_foop, gosub_foo[999], at[999];
    char buff[999],two[2];
     
  public:
    bool Isnr( char c );
    int findop(char op);
    int evalbinary(char op, int l1, int l2);
    long expr(int prec);
    void print_string(void);
    void print(void);
    void enterline(void);
    void load(char *fn);
    void run(int linenum);
    
    CHARP m[12*999],p,q,s,d;  // p used as expr ptr    
    FILE *f;


};






