#include "include/ft_mat.hpp"
#include <iostream>
#include <fstream>
// random
#include <cstdlib>
using namespace std;
#include <map>

int main()
{
    // ofstream file("test.txt");

    // ft::matrix<float> mat(4, 4);

    // for (int i = 0; i < 4; i++)
    //     for (int j = 0; j < 4; j++)
    //         mat[i][j] = (i * 4 + j) * std::rand()*0.00000000000000001;

    // file << mat;

    // file.close();

    // ifstream file2("test.txt");
    // cout << mat << endl;
    // ft::matrix<float> mat2(4, 4);
    // cout << "reading" << endl;
    // file2 >> mat2;

    // cout << mat << endl << endl;
    // cout << mat2 << endl;

    // cout << (mat2 == mat) << endl;
    // for (int i = 0; i < 4; i++)
    //     for (int j = 0; j < 4; j++)
    //          if (mat[i][j] != mat2[i][j])
    //             std::cout << mat[i][j] << " " << mat2[i][j] << " " << i << " " << j << std::endl;

    // test equal_range

    std::map<float, int> m = {{0, 1}, {1.0001, 2}, {1.2, 3}, {1.4, 4}, {2.45, 5}};

    auto range = m.equal_range(2.45);

    float t = 0;
    for (; t < 3.0f; t += 0.1f)
    {
        range = m.equal_range(t);
        if (range.first != m.begin())
            range.first--;
        if (range.second == m.end())
            range.second--;
        std::cout << range.first->second << std::endl;
        std::cout << range.second->second << std::endl;
        std::cout << "~~~~~~~~~" << std::endl;
    }
}