// Файл hash_2.h
// Шаблон простой смешанной таблицы с ключами типа char *.
// В конструкторе пользователь должен указать специальный объект типа T, который будет служить индика
//тором "не найдено", возвращаемым функцией find(char *), когда она пытается отыскать элемент, которого
//нет в таблице. Пользователь указывает также размер массива, используемого для реализации таблицы.
//Число пунктов таблицы не имеет предопределенного предела. // Add(const T &, char *) обнаруживает по
//повторяющиеся ключи и отказывается включать их в таблицу. Дублирование *данных* допускается.
// Т должен поддерживать копирование. Для печати необходима поддержка operator<<( ).
#pragma once
#include <string>
#include <iostream>
#include <cstring>
using namespace std;

template<class T>
class Hash_table {
private:
    // Закрытая структура struct cell для внутреннего хранения данных и ключей.
    struct cell {
        T data;
        const char *key;
        cell *next;

        cell(const T &cell_data, const char *cell_key, cell *cell_next = 0)
            : data(cell_data), key(cell_key), next(cell_next) {
        }
    };

    // Внутренний массив таблицы. Каждый элемент указывает на (возможно пустой) список ячеек.
    cell **table;
    unsigned int table_size;
    unsigned int n_entries;
    // T__notfound - специальное значение типа T, передаваемоеконструктору пользователем. Когда
    // find(char *)возвращает данное значение, это означает, что искомый ключ в таблице отсутствует.
    T T_notfound;
    // Hash(char *) вычисляет простую функцию подстановки.
    unsigned int hash(const char *key) {
        unsigned int return__value = 0;
        for (const char *cp = key; *cp != 0; ++cp) {
            return__value += *cp;
        }
        return unsigned(return__value % table_size);
    }

    // Find_cell( ) возвращает указатель на ячейку, содержащую key. Возвращает нуль, если ключ не найден.
    // Производит сравнение строк, а не указателей.
    cell *find_cell(const char *key) {
        unsigned int slot = hash(key);
        for (cell *cp = table[slot]; cp != 0; cp = cp->next) {
            if (strcmp(cp->key, key) == 0) return cp;
        }
        return nullptr;
    }

    // Присваивание и копирование не поддерживается.
    Hash_table(const Hash_table &);

    Hash_table &operator=(const Hash_table &);

    void destroy_table(cell **dead_table, unsigned int dead_table_size) {
        for (int i = 0; i < dead_table_size; i++) {
            cell *cp, *cp_next;
            if ((cp = dead_table[i]) == 0) continue;
            else cp_next = cp->next;
            // Dead_table[i] is non-empty,
            while (true) {
                delete cp;
                if (cp_next == 0) break;
                cp = cp_next;
                cp_next = cp_next->next;
            }
        }
        delete [] dead_table;
    }

    void expand_table() {
        // Сначала создать указатель на старую (существующую)  таблицу.
        cell **old_table = table;
        unsigned int old_table_size = table_size;
        // Организовать новую таблицу.
        table_size *= 2;
        n_entries = 0;
        table = new cell *[table_size];

        // Включить каждый из старых элементов в новую таблицу.
        // Обход, аналогичныйdestroy_table( ).
        for (int j = 0; j < old_table_size; j++) {
            cell *cp, *cp_next;
            if ((cp = old_table[j]) == 0) continue;
            else cp_next = cp->next;
            // Old_table[j] is non-empty,
            while (true) {
                add(cp->data, cp->key);
                if (cp_next == 0) break;
                cp = cp_next;
                cp_next = cp_next->next;
            }
        }
        // Наконец, уничтожить старую таблицу. В целях увеличения эффективности эти удаления можно
        // сделать "вручную" в предыдущем цикле,
        destroy_table(old_table, old_table_size);
    }

public:
    // Конструктор выделяет память под таблицу и инициализирует T_notfound.
    Hash_table(unsigned int size, const T &notfound)
        : table_size(size), T_notfound(notfound), n_entries(0) {
        table = new cell *[table_size];
        for (int i = 0; i < size; table[i++] = 0);
    }

    // Деструктор удаляет все ячейки, затем удаляет саму таблицу.
    ~Hash_table() { destroy_table(table, table_size); }

    // Ввести элемент в таблицу, если только его ключ не обнаружен в таблице. Вызов find__cell(int)
    // удобен и облегчает сопровождение, но дублирует  вызов hash(int).
    //Если добавление элементов должно быть высокоэффективным, поиск ключа должен быть зако-
    // дирован здесь "вручную". Элементы вставляются  в начало списка, чтобы избежать прохода по нему.
    // Если почему-то важно, чтобы они присоединялись в конец, потребуется чуть более сложная схема //поддержания  хвостового указателя, работающая достаточно быстро.
    // Возвращает true, если элемент успешно размещен,  false в противном случае.
    // Обратите внимание, что строковые ключи не копируются. Мы полагаем, что эти указатели на char        // будут  продолжать ссылаться на те же самые строки на протяжении всего времени жизни смешанной // таблицы.
    bool add(const T &item, const char *key) {
        if (find_cell(key) != 0) return false;
        // Проверить, ие превосходит ли средняя длина связанного списка пяти. Если превосходит, выделить // новую таблицу с удвоенным числом позиций и уничтожить старую,
        if ((n_entries / table_size) >= 5) expand_table();
        // Новая ячейка инициализируется адресом ключевой строки.  Если есть сомнения, что эти
        // указатели будут стабильны (т.е. будут продолжать указывать на ту же строку), сначала должна быть
        // сделана внутренняя копия. В этом случае деструктор ~cell( ) должен быть переписан, чтобы эти строки
        // удалялись.
        unsigned int slot = hash(key);
        if (table[slot] == 0)
            table[slot] = new cell(item, key);
        else {
            table[slot] = new cell(item, key, table[slot]);
        }
        ++n_entries;
        return true;
    }

    // Удалить пункт таблицы с ключом key. Для удаления  нужна переменная - указатель на предыдущую //ячейку,  так как список односвязный.
    void remove(const char *key) {
        unsigned int slot = hash(key);
        cell *cp_prev = table[slot];
        if (cp_prev == 0) return;
        // Special case for first item.
        if (strcmp(cp_prev->key, key) == 0) {
            table[slot] = cp_prev->next;
            delete cp_prev;
            --n_entries;
            return;
        }
        for (cell *cp = cp_prev->next;
             cp != 0;
             cp_prev = cp_prev->next, cp = cp->next) {
            if (strcmp(cp->key, key) == 0) {
                cp_prev->next = cp->next;
                delete cp;
                --n_entries;
                return;
            }
        }
    }

    // Возвращает элемент, соответствующий key. Если его  нет в таблице, возвратить специальное
    // значение T_notfound, переданное пользователем.
    const T &find(const char *key) {
        cell *cp = find_cell(key);
        return (cp ? cp->data : T_notfound);
    }

    // Функция print( ) для отладки. Пригодна только для  небольших таблиц.
    void print() {
        cout << endl;
        cout << "slot #" << endl << "-------------" << endl;
        for (unsigned int index = 0; index < table_size; ++index) {
            // Print a row of the table.
            cout << index << " ";
            for (cell *cp = table[index]; cp != 0; cp = cp->next) {
                cout << "[" << cp->data << " , " << cp->key << " ] ";
            }
            cout << endl;
        }
        cout << endl;
    }
};
