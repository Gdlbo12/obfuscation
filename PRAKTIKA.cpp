#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <algorithm>
#include <fstream>
#include <locale>
#include <filesystem>

// Функция для генерации случайных имен переменных
std::string generateRandomName(int length) {
    static const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, chars.size() - 1);
    
    std::string result;
    for (int i = 0; i < length; ++i) {
        result += chars[dis(gen)];
    }
    return result;
}

// Функция для шифрования строк с использованием XOR
std::string encryptString(const std::string& input, char key) {
    std::string result;
    for (char c : input) {
        result += c ^ key;
    }
    return result;
}

// Функция для переименования переменных в коде
std::string renameVariables(const std::string& code) {
    std::map<std::string, std::string> variableMap;
    std::string result = code;
    
    // Находим все объявления переменных и заменяем их
    size_t pos = 0;
    while ((pos = result.find("int ", pos)) != std::string::npos) {
        size_t end = result.find(";", pos);
        if (end != std::string::npos) {
            std::string varDecl = result.substr(pos, end - pos);
            size_t varStart = varDecl.find_last_of(" ");
            if (varStart != std::string::npos) {
                std::string oldName = varDecl.substr(varStart + 1);
                std::string newName = generateRandomName(8);
                variableMap[oldName] = newName;
            }
        }
        pos = end;
    }
    
    // Заменяем все вхождения старых имен переменных
    for (const auto& pair : variableMap) {
        size_t pos = 0;
        while ((pos = result.find(pair.first, pos)) != std::string::npos) {
            result.replace(pos, pair.first.length(), pair.second);
            pos += pair.second.length();
        }
    }
    
    return result;
}

// Функция для добавления мусорного кода
std::string addJunkCode() {

}

// Основная функция для обфускации кода
std::string obfuscateCode(const std::string& inputCode) {
    std::string obfuscatedCode = inputCode;
    
    // Применяем техники обфускации
    obfuscatedCode = renameVariables(obfuscatedCode);
    obfuscatedCode = addJunkCode(obfuscatedCode);
    
    return obfuscatedCode;
}

// Функция для чтения файла
std::string readFile() {

}

// Функция для записи в файл
void writeFile() {

}

// Функция для проверки и корректировки пути к выходному файлу
std::string prepareOutputPath(const std::string& inputPath, const std::string& outputPath) {
    std::filesystem::path input(inputPath);
    std::filesystem::path output(outputPath);
    
    // Если выходной путь - это директория
    if (std::filesystem::is_directory(output)) {
        // Создаем имя файла на основе входного файла
        std::string newFilename = "obfuscated_" + input.filename().string();
        return (output / newFilename).string();
    }
    
    // Если выходной путь не содержит расширение
    if (!output.has_extension()) {
        return output.string() + ".cpp";
    }
    
    return output.string();
}

int main() {
    setlocale(LC_ALL, "Russian");
    
    try {
        std::string inputFilename, outputFilename;
        
        std::cout << "Введите путь к исходному файлу: ";
        std::getline(std::cin, inputFilename);
        
        std::cout << "Введите путь для сохранения обфусцированного кода: ";
        std::getline(std::cin, outputFilename);
        
        // Подготавливаем путь к выходному файлу
        outputFilename = prepareOutputPath(inputFilename, outputFilename);
        
        // Читаем исходный код из файла
        std::string inputCode = readFile(inputFilename);
        
        // Обфусцируем код
        std::string obfuscatedCode = obfuscateCode(inputCode);
        
        // Сохраняем результат в файл
        writeFile(outputFilename, obfuscatedCode);
        
        std::cout << "Обфускация успешно завершена!\n";
        std::cout << "Результат сохранен в файл: " << outputFilename << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

