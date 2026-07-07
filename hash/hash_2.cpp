#include "hash_2.h" 
// Создать смешанную таблицу для целых с одной позицией
// и ключами типа char *; специальное значение равно -1.
// Число позиций увеличивается автоматически по мере
// добавления элементов в таблицу.
Hash_table<int>my_hash(1, -1);
int main( )
{
    my_hash.add(1, "a");
    my_hash.add(1, "b");
    my_hash.add(1, "c");
    my_hash.add(1, "d");
    my_hash.add(1, "e");
    my_hash.print();
    my_hash.add(1, "f");
    my_hash.print();
    my_hash.add(1, "g");
    my_hash.add(1, "h");
    my_hash.add(1, "i");
    my_hash.add(1, "j");
    my_hash.add(1, "к");
    my_hash.print() ;
    my_hash.add (1, "l");
    my_hash.add (1, "m");
    my_hash.add(1, "n");
    my_hash.add(1, "o");
    my_hash.add(1, "p");
    my_hash.add (1, "q"); 
    my_hash.add (1, "r"); 
    my_hash.add (1, "s"); 
    my_hash.add (1, "t"); 
    my_hash.add (1, "u"); 
    my_hash.add (1, "v"); 
    my_hash.add (1, "w"); 
    my_hash.add (1, "x"); 
    my_hash.add (1, "y"); 
    my_hash.add (1, "z"); 
    my_hash.print();
    return 0;
}
