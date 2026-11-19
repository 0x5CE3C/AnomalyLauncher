#pragma once
#include <stdexcept>
#include <iostream>
#include <exception>
#include <string>

// Класс пользовательского исключения
class ExceptionHandler : public std::exception {
public:
    ExceptionHandler(int id, const std::string& msg) : id_(id), message_(msg) {}

    int id() const noexcept { return id_; }
    const char* what() const noexcept override { return message_.c_str(); }
   
private:
    int id_;
    std::string message_;
};

enum EXCEPTID
{
    UNKNOWN_ERROR
};

// Функция для обработки пользовательских исключений
void handleException(const ExceptionHandler& e) {
    switch (e.id()) {
    case 1:
        std::cerr << "Error ID 1: " << e.what() << std::endl;
        break;
    case 2:
        std::cerr << "Error ID 2: " << e.what() << std::endl;
        break;
    case 3:
        std::cerr << "Error ID 3: " << e.what() << std::endl;
        break;    
    }
}

// Функция для обработки стандартных исключений
void handleStandardException(const std::exception& e) {
    if (const std::invalid_argument* arg = dynamic_cast<const std::invalid_argument*>(&e)) {
        std::cerr << "Неверный формат ID: " << arg->what() << std::endl;
    }
    else if (const std::out_of_range* oor = dynamic_cast<const std::out_of_range*>(&e)) {
        std::cerr << "Выход за пределы диапазона: " << oor->what() << std::endl;
    }
    else if (const std::runtime_error* re = dynamic_cast<const std::runtime_error*>(&e)) {
        std::cerr << "Ошибка выполнения: " << re->what() << std::endl;
    }
    else if (const std::logic_error* le = dynamic_cast<const std::logic_error*>(&e)) {
        std::cerr << "Логическая ошибка: " << le->what() << std::endl;
    }
    else if (const std::overflow_error* oe = dynamic_cast<const std::overflow_error*>(&e)) {
        std::cerr << "Ошибка переполнения: " << oe->what() << std::endl;
    }
    else if (const std::underflow_error* ue = dynamic_cast<const std::underflow_error*>(&e)) {
        std::cerr << "Ошибка недополнения: " << ue->what() << std::endl;
    }
    else {
        std::cerr << "Неожиданное исключение: " << e.what() << std::endl;
    }
}