#include <algorithm>
#include <vector>
#include <iostream>

using namespace std;
// формат матрицы [][] стодбец строка

typedef double *Pdouble;// создали указатель на дабл
class DoubleVector{
  private:
    int sz;
  public:
    int size(){ return sz; };
    Pdouble *data;
    DoubleVector(int size){
      sz = size;
      this->data = new Pdouble[size];
      for(int i = 0; i < size; i++){
        this->data[i] = new double[size];
      }
     }
    Pdouble& operator [](int i){
      return this->data[i];
    }
};

std::ostream& operator<<(std::ostream& os, DoubleVector& obj){// создали оператор <<
  for(int i = 0; i < obj.size(); i++){
    for(int j = 0; j < obj.size(); j++){
      os << obj.data[i][j] << " ";
    }
    os << endl;
  }
  return os;
}

std::istream& operator >> (std::istream& is, DoubleVector& obj){// создали поток ввода
  for(int i = 0; i < obj.size(); i++){
    for(int j = 0; j < obj.size(); j++){
      is >> obj.data[i][j];
    }
  }
  return is;
}

void normalize_row(DoubleVector& matrix, int from, double k){
    for (int j = 0; j < obj.size(); j++){
      matrix[from][j] /= k;
    }
}
void normalize_col(DoubleVector& matrix, int from, double k){
    for (int i = from + 1; i < matrix.size(); i++) {
        k = matrix[i][from];
        for(int j = from; j < matrix.size(); j++){
            cout << matrix[from][j] << endl;
            double m = k * matrix[from][j];
            // cout << "m " << m  << " el " << obj[i][j] << endl;
            matrix[i][j] = matrix[i][j] - m;
        }
    }
}
int det(DoubleVector& matrix){
    int res = 0;
    int degree = 1;
    int k_res = 1;
    if(matrix.size() == 1) {
        return matrix[0][0];
    }
    if (matrix.size() == 2) {
      return matrix[0][0]*matrix[1][1] - matrix[0][1]*matrix[1][0];
    }
    if (matrix.size() > 2) {// попробовать привести матрицу к треугольному виду
        for(int i = 0; i < matrix.size() - 1; i++){
            double k = matrix[i][i];
            k_res *= k;
            normalize_row(matrix, i, k);
            normalize_col(matrix, i, matrix[i + 1][i]);
            cout << matrix << endl;
        }
    }
    double mut = 1.0;
    for (int i = 0; i < matrix.size(); i++) {
        mut *= matrix[i][i];
    }
    return mut * k_res;

}



int main(){
    int n;
    cin >> n;
    DoubleVector matrix(n);
    cin >> matrix;
    cout << det(matrix);
    return 0;
}

