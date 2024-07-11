#include "stack.hpp"

int main() {
    auto stack = stack::stack::get_instance();
    stack->run();
}