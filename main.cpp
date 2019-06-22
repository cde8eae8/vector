#include <iostream>
#include "vector.h"


struct A {
    A(int i) {
        id = i;
    }

    ~A() {
        id++;
    }

    void print() {
        std::cout << id << std::endl;
    }

    size_t id;
};

int main() {
    vector<A> v;
//    A& a = v[0];
//    A const& ac = v[0];
//    vector<A>::iterator ib = v.begin();
//    vector<A>::iterator ie = v.end();
    for (size_t i = 0; i < 10; ++i) {
        std::cout << "place for " << i << std::endl;
        v.push_back(A(i));
    }

    for (auto &item : v) {
        item.print();
    }

    return 0;
}