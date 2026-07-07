#include <iostream>

using namespace std;

const int MAXSIZE = 100;

struct Stack {
    char data[MAXSIZE];
    int size;
};
typedef Stack *S;
// Вставка элемента
void Push ( Stack &S, char x )
{
    if ( S.size == MAXSIZE ) {
        printf ("Стек переполнен");
        return;
    }
    S.data[S.size] = x;
    S.size ++;
}

char Pop ( Stack &S )
{
    if ( S.size == 0 ) {
        printf ("Стек пуст");
        return char(255);
    }
    S.size --;
    return S.data[S.size];
}

char Get (Stack &S){
    return S.data[S.size];
}



int main(){
    char ch;
    Stack s;
    int res = 0;
    bool first = true;
    while(cin >> ch && ch != 'q'){
		if (ch == ' ') continue;
		if (ch == 'q') break;
        if (ch - '0' >= 0 && ch - '0' <= 9){
            Push(s, ch - '0');
            if (first){
				res = ch - '0';
				first = false;
			}
        }
        else{
            if(ch == '+'){
                res += Pop(s);
            }
			else if(ch == '-'){
				res -= Pop(s);
				}
			else if(ch == '/'){
				res /= Pop(s);
			}
			else if(ch == '*'){
				res *= Pop(s);
			}
    	}
	}
	cout << res << endl;
    return 0;
}