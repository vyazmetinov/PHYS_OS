#include "hash_1.h"
// Создать смешанную таблицу для строк с пятью позициями, используя пустую cтроку как cпециальное
//значение. Для ключей Hash_1.h использует целые.
Hash_table<char *> my_hash(5, " ");

int main() {
    // Ввести несколько значений.
    my_hash.add("abc", 99);
    my_hash.add("abc", 100); // Дублирование данных допустимо.
    my_hash.add("def", 100); // Повторяющийся ключ игнорируется.
    my_hash.add("ghi", 101);
    // Напечатать таблицу,
    my_hash.print();
    // Отыскать некоторые значения, идентифицированные ключами,
    cout << "my_hash.find(99): " << my_hash.find(99) << endl;
    cout << "my_hash.findA00):  " << my_hash.find(100) << endl;
    cout << "my_hash.findA01):  " << my_hash.find(101) << endl;
    cout << "my__hash.findF6): " << my_hash.find(66) << endl;
    // Удалить идентифицированные ключами элементы.
    my_hash.remove(99);
    my_hash.remove(100);
    my_hash.remove(101);
    my_hash.remove(5); // Ключигнорирован - нетвтаблице.
    // Ввести 25 идентичных значений, каждое со своим ключом,
    for (int i = 0; i < 25; ++i) {
        my_hash.add("xyz", i);
    }
    // Снова напечатать таблицу,
    my_hash.print();
    return 0;
}
