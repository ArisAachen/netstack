#include "raw_stack.hpp"

int main() {
    auto stack = stack::raw_stack::get_instance();
    stack->run();
    stack->wait();
}