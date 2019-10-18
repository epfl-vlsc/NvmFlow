#include <iostream>

using namespace std;

void f(int* c){
    *c = 5;
}

int main(){
    int* a = new int;
    int *b = new int;
    
    *a = 4;
    *b = 6;

    f(a);

    cout << a << b;

    return 0;
}