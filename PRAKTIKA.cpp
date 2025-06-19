#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <filesystem>
#include <unistd.h>
#include <limits.h>

#define OBF_KEY 0x5A
#define OBF_CAST(x) static_cast<unsigned char>(x)

namespace NonsenseSpace {
    template<typename T>
    class JunkClass {
    public:
        JunkClass(T v) : val(v + 1 - 1) {}
        operator T() const { return val; }
    private:
        T val;
    };
}
#define JUNK_MACRO(x) ((x)+0-0)

std::string decode(const std::vector<unsigned char>& data, char key) {
    std::string result;
    for (auto c : data) result += c ^ key;
    return result;
}

std::string obfuscate_strings(const std::string& code) {
    std::string result;
    std::regex str_re("\"([^\"]*)\"");
    std::sregex_iterator it(code.begin(), code.end(), str_re), end;
    size_t last = 0;
    for (; it != end; ++it) {
        result += code.substr(last, it->position() - last);
        std::string s = (*it)[1];
        result += "decode({";
        for (size_t i = 0; i < s.size(); ++i) {
            result += std::to_string(OBF_CAST(s[i] ^ OBF_KEY));
            if (i + 1 < s.size()) result += ", ";
        }
        result += "}, OBF_KEY)";
        last = it->position() + it->length();
    }
    result += code.substr(last);
    return result;
}

std::string insert_junk_code() {
    return R"(
namespace NonsenseSpace {
    template<typename T>
    class JunkClass {
    public:
        JunkClass(T v) : val(v + 1 - 1) {}
        operator T() const { return val; }
    private:
        T val;
    };
}
#define JUNK_MACRO(x) ((x)+0-0)
)";
}

std::string wrap_main(const std::string& code) {
    std::regex main_re(R"(int\s+main\s*\()");
    std::string wrapped = std::regex_replace(code, main_re, "int real_main(");
    wrapped += R"(
int main() {
    for (int i = 0; i < 3; ++i) {
        NonsenseSpace::JunkClass<int> dummy(i);
        if (dummy == 2) break;
    }
    return real_main();
}
)";
    return wrapped;
}

std::string remove_comments(const std::string& code) {
    std::string no_single = std::regex_replace(code, std::regex("//.*"), "");
    std::string no_comments = std::regex_replace(
        no_single,
        std::regex("/\\*[\\s\\S]*?\\*/", std::regex::ECMAScript),
        ""
    );
    return no_comments;
}

int main() {
    std::string input_path;
    std::cout << "Введите путь к исходному .cpp файлу: ";
    std::getline(std::cin, input_path);

    std::ifstream fin(input_path);
    if (!fin) {
        std::cerr << "Ошибка открытия файла!" << std::endl;
        return 1;
    }
    std::stringstream buffer;
    buffer << fin.rdbuf();
    std::string code = buffer.str();
    fin.close();

    // Удаляем комментарии
    code = remove_comments(code);

    // Шифруем строки
    code = obfuscate_strings(code);

    // Вставляем макросы и функцию decode
    std::string macros = R"(
#define OBF_KEY 0x5A
#define OBF_CAST(x) static_cast<unsigned char>(x)
#include <vector>
#include <string>
std::string decode(const std::vector<unsigned char>& data, char key) {
    std::string result;
    for (auto c : data) result += c ^ key;
    return result;
}
)";
    code = macros + code;

    // Вставляем мусорный код
    code = insert_junk_code() + code;

    // Оборачиваем main
    code = wrap_main(code);

    // Генерируем имя файла
    int n = 1;
    std::string out_path;
    while (true) {
        out_path = "obf" + std::to_string(n) + ".cpp";
        std::ifstream test(out_path);
        if (!test) break;
        n++;
    }

    // Диагностика
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        std::cout << "Текущая рабочая директория: " << cwd << std::endl;
        std::cout << "Пытаюсь создать файл: " << out_path << std::endl;
    } else {
        std::cerr << "Не удалось получить рабочую директорию!" << std::endl;
    }

    std::ofstream fout(out_path);
    if (!fout) {
        std::cerr << "Ошибка создания файла! Проверь права на запись и путь." << std::endl;
        return 1;
    }
    fout << code;
    fout.close();

    std::cout << "Обфусцированный файл сохранён как " << out_path << std::endl;
    return 0;
}
