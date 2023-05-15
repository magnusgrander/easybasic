#include "easybasic.h"

bool easybasic::Isnr( char c )
{
   static const char alpha[] = "1234567890.";
   return (strchr( alpha, c ) != NULL );
}

int easybasic::findop(char op) {
    switch (op) {
        case '=': return 1;
        case '#': return 1; // <>, not equal
        case '<': return 2;
        case '>': return 2;
        case '$': return 2; // <=
        case '!': return 2; // >=
        case '+': return 3;
        case '-': return 3;
        case '*': return 4;
        case '/': return 4;
    }
    return -1;
}

int easybasic::evalbinary(char op, int l1, int l2) {
    switch (op) {
        case '=': return l1 == l2;
        case '#': return l1 != l2; // <>, not equal
        case '<': return l1 < l2;
        case '>': return l1 > l2;
        case '$': return l1 <= l2; // <=
        case '!': return l1 >= l2; // >=
        case '+': return l1 + l2;
        case '-': return l1 - l2;
        case '*': return l1 * l2;
        case '/': return l1 / l2;
    }
    return 0;
}

long easybasic::expr(int prec) {
    double o, this_prec;

    if (*p=='-') {
        p++;
        o = -expr(999);
    } else if (Isnr(*p)) {
        o = strtol(p,&p,10);    // can't use 0 for base, because of hex pbm.
    } else if (*p=='(') {
        ++p;
        o = expr(0);
        ++p;
    } else if (*p == '@') {   // @(exp)
        ++p;
        o = at[expr(999)];
    } else if (memcmp(p, "rnd", 3) == 0) {    // rnd(exp)
        p += 3;
        o = expr(999);
        o = rand() % int(o + 1);
    } else if (memcmp(p, "sin", 3) == 0) {    // sin(exp)
        p += 3;
        o = expr(999);
        o = sin(o) ;
     } else if (memcmp(p, "cos", 3) == 0) {    // cos(exp)
        p += 3;
        o = expr(999);
        o = cos(o) ;
     } else if (memcmp(p, "stp", 3) == 0) {    // just stupid(exp)
        p += 3;
        o = expr(999);
        o = 1000.12 ;                       
    } else if (memcmp(p, "abs", 3) == 0) {    // abs(exp)
        p += 3;
        o = abs(expr(999));
    } else if (memcmp(p, "asc", 3) == 0) {    // asc("x")
        p += 5;
        o = *p;
        p += 3;
    } else
        o = var[*p++];

    while ((this_prec = findop(*p)) > 0 && this_prec >= prec) {
        char op = *p++;
        o = evalbinary(op, o, expr(this_prec + 1));
    }
    return o;
}

void easybasic::print_string(void) {
    int width;
    ++p;
    width = strchr(p, '"') - p;
    printf("%*.*s", width, width, p);
    p += width + 1;
}

// 'print' [[#num ',' ] expr { ',' [#num ','] expr }] [','] {':' stmt} eol
// expr can also be a literal string
void easybasic::print(void) {
    int print_nl;

    print_nl = true;
    for (;;) {
        int width = 0;
        if (*p == ':' || *p == '\0')
            break;
        print_nl = true;
        if (*p == '#') {
            ++p;
            width = expr(0);
            if (*p == ',')
                ++p;
        }

        if (*p == '"')
            print_string();
        else
            printf("%*d", width, expr(0));

        if (*p == ',' || *p == ';') {
            ++p;
            print_nl = false;
        } else
            break;
    }
    if (print_nl)
        printf("\n");
}

void easybasic::enterline(void) {
    char *s = buff;
    while (*s && isspace(*s))
        s++;
    linenum=atoi(s);
    if (m[linenum])
        free(m[linenum]);
    if ((p=strstr(s, " ")) != NULL)
        strcpy(m[linenum]=malloc(strlen(p)),p+1);
    else
        m[linenum]=0;
}

void easybasic::load(char *fn) {
    f=fopen(fn,"r");
    while(fgets(buff,999,f))
        (*strstr(buff,"\n")=0,enterline());
    fclose(f);
}

void easybasic::run(int linenum) {
	m[11*999]="e";
    int offset = 0;
    gosub_stackp=gosub_stack;
    gosub_foop=gosub_foo;    
    for(i=0; i<999; var[i++]=0)
        ;
    // set s to point to first char of line
    while(linenum) {
        int if_cont;
        while((s=m[linenum]) == 0)
            linenum++;
        if (!strstr(s,"\"")) {
            while((p=strstr(s,"<>")) != 0) *p++='#',*p=' ';
            while((p=strstr(s,"<=")) != 0) *p++='$',*p=' ';
            while((p=strstr(s,">=")) != 0) *p++='!',*p=' ';
        }
        // remove extra spaces, line copied to buff
        d=buff;
        while((*two=*s) != '\0') {
            if(*s=='"')
                inquote++;
            if(inquote&1||!strstr(" \t",two))
                *d++=*s;
            s++;
        }
        inquote= *d = 0;
        s = buff;
    line_processed:
        p = (s += offset);
        offset = if_cont = 0;

        if(s[1] == '=') {        // assignment a=exp
            p=s+2;
            var[*s]=expr(0);
        } else if (s[0] == '@') { // assignment: @(exp)=exp
            int ndx;
            p = s + 1;
            ndx = expr(999);    // use high prec to force end at ')'
            ++p;
            at[ndx] = expr(0);
        } else
            switch(*s) {
                case'e':          // end
                case's':          // stop
                    linenum=-1;
                    break;
                case'r':          // rem and return
                    if (s[2]!='m') {
                        linenum=*--gosub_stackp;    // return
                        offset=*--gosub_foop;
                    }
                    break;
                case'i':          // input [constant_string,] var  and if
                    if (s[1]=='n') {     // input
                        int tmp;
                        char in_buff[20];
                        d = p = &s[5];
                        if (*p == '"') {
                            print_string();
                            d = ++p;            // skip ','
                        }
                        tmp = *d;
                        p = fgets(in_buff, sizeof(in_buff) - 2, stdin);
                        var[tmp] = Isnr(*p) ? expr(0) : *p;
                        p = ++d;
                    } else {                    // if
                        p=s+2;
                        if (expr(0)) {
                            --p;
                            if_cont = true;
                        } else
                            p = 0;
                    }
                    break;
                case'p':          // print string and expr
                    p = &s[5];
                    print();
                    break;
                case'g':          // goto, gosub
                    p=s+4;
                    if (s[2]=='s') {            // gosub
                        *gosub_stackp++=linenum;
                        p++;
                    }
                    linenum=expr(0)-1;
                    if (s[2] == 's')            // gosub
                        *gosub_foop++ = (*p == ':') ? p - buff + 1: 0;
                    p = 0;
                    break;
                case'f':          // for
                    *(q=strstr(s,"to"))=0;
                    p=s+5;
                    var[i=s[3]]=expr(0);
                    p=q+2;
                    lim[i]=expr(0);
                    line[i]=linenum;
                    line_off[i] = (*p == ':') ? p - buff + 1: 0;
                    break;
                case'n':          // next
                    d = s + 4;
                    p = d + 1;
                    if (++var[*d]<=lim[*d]) {
                        linenum=line[*d];
                        offset=line_off[*d];
                        p = 0;
                    }
		case'c':   // CLS
		  if (s[1]=='l') {
		  printf("cls");
		  } 		  
		  if (s[1]=='o') { //COL
		    if(s[3] =='x') { //COLX
		      printf("colx");
		    }
		    if(s[3] =='y') { //COLY
		      printf("coly");
		    }
		  }
		  break;
            }
        if (p && *p && (if_cont || *p == ':')) {
            s = ++p;
            goto line_processed;
        }
        if (!offset)
            linenum++;
    }
}


