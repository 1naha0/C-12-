#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>

// ==========================================
// 1. 数据模型层 (Model)
// ==========================================
enum class Role { Student, Teacher, Admin };

std::string roleToString(Role role) {
    if (role == Role::Student) return "学生";
    if (role == Role::Teacher) return "教师";
    if (role == Role::Admin) return "管理员";
    return "未知";
}

// 抽象基类
class User {
protected:
    std::string username_;
    std::string password_; // 新增密码
    Role role_;

public:
    User(std::string username, std::string password, Role role)
        : username_(username), password_(password), role_(role) {
    }
    virtual ~User() = default;

    virtual void showMenu() const = 0;
    std::string getUsername() const { return username_; }
    std::string getPassword() const { return password_; }
    Role getRole() const { return role_; }
};

class Student : public User {
public:
    Student(std::string username, std::string password)
        : User(username, password, Role::Student) {
    }
    void showMenu() const override {
        std::cout << "\n===== 学生菜单 =====\n";
        std::cout << "1. 查看个人信息\n2. 查询成绩与排名\n3. 导出成绩报表\n0. 退出\n";
        std::cout << "====================\n";
    }
};

class Teacher : public User {
public:
    Teacher(std::string username, std::string password)
        : User(username, password, Role::Teacher) {
    }
    void showMenu() const override {
        std::cout << "\n===== 教师菜单 =====\n";
        std::cout << "1. 管理学生信息\n2. 录入/修改成绩\n3. 查看班级统计\n0. 退出\n";
        std::cout << "====================\n";
    }
};

class Admin : public User {
public:
    Admin(std::string username, std::string password)
        : User(username, password, Role::Admin) {
    }
    void showMenu() const override {
        std::cout << "\n===== 管理员菜单 =====\n";
        std::cout << "1. 全局账号管理\n2. 系统权限管控\n3. 全量数据管理\n0. 退出\n";
        std::cout << "=========================\n";
    }
};

// ==========================================
// 2. 数据持久层 (Data Access)
// ==========================================
class DataManager {
public:
    // 核心功能：从 users.txt 读取所有用户，并动态创建出对应对象
    static std::vector<std::unique_ptr<User>> loadUsers(const std::string& filename) {
        std::vector<std::unique_ptr<User>> users;
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "警告：找不到 " << filename << "，未加载任何用户。\n";
            return users;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string username, password, name, roleStr, enabledStr;

            // 按逗号拆分：2025001,123456,张三,student,1
            std::getline(ss, username, ',');
            std::getline(ss, password, ',');
            std::getline(ss, name, ',');
            std::getline(ss, roleStr, ',');
            std::getline(ss, enabledStr, ',');

            // 根据角色字符串，动态创建对象（多态的核心！）
            if (roleStr == "student") {
                users.push_back(std::make_unique<Student>(username, password));
            }
            else if (roleStr == "teacher") {
                users.push_back(std::make_unique<Teacher>(username, password));
            }
            else if (roleStr == "admin") {
                users.push_back(std::make_unique<Admin>(username, password));
            }
        }
        file.close();
        return users;
    }
};

// ==========================================
// 3. 业务逻辑层 (Service)
// ==========================================
class AuthManager {
public:
    // 核心功能：验证账号密码，成功返回 User*，失败返回 nullptr
    static User* login(const std::vector<std::unique_ptr<User>>& users,
        const std::string& inputUsername,
        const std::string& inputPassword) {
        for (const auto& user : users) {
            if (user->getUsername() == inputUsername && user->getPassword() == inputPassword) {
                return user.get(); // 注意：这里返回的是原始指针，但不负责释放
            }
        }
        return nullptr;
    }
};

// ==========================================
// 4. 表现层 (UI) - 主函数
// ==========================================
int main() {
    // 1. 初始化：加载数据
    auto users = DataManager::loadUsers("users.txt");
    std::cout << "系统启动成功，共加载 " << users.size() << " 个用户。\n";

    // 2. 登录循环
    User* currentUser = nullptr;
    while (currentUser == nullptr) {
        std::string inputUser, inputPwd;
        std::cout << "\n=== 学生成绩管理系统 ===\n";
        std::cout << "账号: "; std::cin >> inputUser;
        std::cout << "密码: "; std::cin >> inputPwd;

        currentUser = AuthManager::login(users, inputUser, inputPwd);

        if (currentUser == nullptr) {
            std::cout << "账号或密码错误，请重试！\n";
        }
        else {
            std::cout << "登录成功！欢迎回来，" << currentUser->getUsername()
                << " [" << roleToString(currentUser->getRole()) << "]\n";
        }
    }

    // 3. 主菜单循环（多态调用）
    int choice = -1;
    while (choice != 0) {
        currentUser->showMenu();
        std::cout << "请选择操作: "; std::cin >> choice;

        // 这里暂时用占位提示，下一步我们会把真实功能填进去
        if (choice != 0) {
            std::cout << "【系统提示】功能 [" << choice << "] 正在开发中...\n";
        }
        else {
            std::cout << "正在退出系统...\n";
        }
    }

    std::cout << "感谢使用，再见！\n";
    return 0;
}
