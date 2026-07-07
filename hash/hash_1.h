#pragma once
#include<iostream>
using namespace std;

template<class T>
class Hash_table {
private:
    // Закрытая structcell для внутреннего хранения данных и ключей.
    struct cell {
        T data;
        int key;
        cell *next;

        cell(const T &cell_data, int cell_key, cell *cell_next = 0)
            : data(cell_data), key(cell_key), next(cell_next) {
        }
    };

    // Внутренний массив таблицы. Каждый элемент указывает на (возможно пустой) список ячеек.
    cell **table;
    unsigned int table_size;
    // T_notfound - специальное значение типа T, передаваемое конструктору пользователем. Когда find(int)//возвращает данное значение, это означает, что искомый ключ в таблице отсутствует.
    T T_notfound;
    // Hash(int) вычисляет значение простейшей подстановки. Она может подойти, а может и неподойтидля //вашей задачи. Функции подстановки должны быть просты (и быстры), насколько возможно, отвечая,тем
    // не менее, основному своему назначению.
    unsigned int hash(int key) {
        return unsigned(key % table_size);
    }

    // Find_cell( ) возвращает указатель на ячейку, содержащую key. Возвращает нуль, если ключ не найден.
    cell *find_cell(int key) {
        unsigned int slot = hash(key);
        for (cell *cp = table[slot]; cp != 0; cp = cp->next) {
            if (cp->key == key) return cp;
        }
        return nullptr;
    }

    // Присваивание и копирование не поддерживается.
    Hash_table(const Hash_table &) = delete;

    Hash_table &operator=(const Hash_table &) = delete;

public:
    // Конструктор выделяет память под таблицу и инициализирует T_notfound.
    Hash_table(unsigned int size, const T &notfound)
        : table_size(size), T_notfound(notfound) {
        table = new cell *[table_size];
        for (int i = 0; i < size; table[i++] = 0);
    }

    // Деструктор удаляет все ячейки, затем удаляет саму таблицу.
    ~Hash_table() {
        for (int i = 0; i < table_size; i++) {
            cell *cp, *cp_next;
            if ((cp = table[i]) == 0) continue;
            else cp_next = cp->next;
            // Table[i] is non-erapty.
            while (true) {
                delete cp;
                if (cp_next == 0) break;
                cp = cp_next;
                cp_next = cp_next->next;
            }
        }
        delete[]table;
    }

    // Ввести элемент в таблицу, если только его ключ не обнаружен в таблице. Вызов find__cell(int)удобен
    // и облегчает сопровождение, но дублирует вызов hash(int). Если добавление элементов //должно быть
    //высокоэффективным, поиск ключа должен быть закодирован здесь "вручную". //Элементы вставляются в
    //начало списка, чтобы избежать прохода по нему. Если почему-то //важно, чтобы они присоединялись в
    //конец, потребуется чуть более сложная схема поддержания хвостового указателя, работающая быстро.
    //Возвращает true, если элемент успешно размещен, false в противном случае.
    bool add(const T &item, int key) {
        if (find_cell(key) != 0) return false;
        unsigned int slot = hash(key);
        if ((table[slot] == 0)) //?????????????????????
            table[slot] = new cell(item, key);
        else {
            table[slot] = new cell(item, key, table[slot]);
        }
        return true;
    }

    // Удалить пункт таблицы с ключом key. Для удаления нужна переменная - указатель на предыдущую //ячейку, так как список односвязный.
    void remove(int key) {
        unsigned int slot = hash(key);
        cell *cp_prev = table[slot];
        if (cp_prev == nullptr) return;
        // Специальный случай для первого элемента,
        if (cp_prev->key == key) {
            table[slot] = cp_prev->next;
            delete cp_prev;
            return;
        }
        for (cell *cp = cp_prev->next;
             cp != nullptr;
             cp_prev = cp_prev->next, cp = cp->next) {
            if (cp->key == key) {
                cp_prev->next = cp->next;
                delete cp;
                return;
            }
        }
    }

    // Возвращает элемент, соответствующий key. Если этого элемента нет в таблице, возвратить спе-
    //циальное значение T_notfound, переданный пользователем.
    const T &find(int key) {
        cell *cp = find_cell(key);
        return (cp ? cp->data : T_notfound);
    }

    // Функция print( ) для отладки. Пригодна только для небольших таблиц.
    void print() {
        cout << endl;
        cout << "slot #" << endl << "------" << endl;
        for (unsigned int index = 0; index < table_size; ++index) {
            // Print a row of the table,
            cout << index << ":" << '\n';
            for (cell *cp = table[index]; cp != 0; cp = cp->next) {
                cout << '[' << cp->data << ',' << cp->key << "]" << endl;
            }
            cout << endl;
        }
        cout << endl;
    }
};
