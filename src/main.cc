#include "raw_stack.hpp"

int main() {
    auto stack = stack::raw_stack::get_instance();
    stack->init();
    stack->run();
    stack->wait();
}