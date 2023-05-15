#include "easybasic.h"

int main(int argc, char *argv[]) {
    easybasic b;
    
    int loaded = false;    
    if (argc > 1) {
        b.load(argv[1]);       
        loaded = true;
	b.run(1);
    }    
    return 0;
}
