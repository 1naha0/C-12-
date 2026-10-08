#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <map>
#include <algorithm>

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

struct Score {
    std::string studentId;   // 学号
    std::string subject;     // 科目
    double value;            // 分数
    std::string term;        // 学期
};

// 抽象基类
class User {
protected:
    std::string username_;
    std::string password_;
    std::string name_;   
    Role role_;
    bool enabled_;

public:
    User(std::string username, std::string password, std::string name,
        Role role, bool enabled = true)
        : username_(username), password_(password), name_(name),
        role_(role), enabled_(enabled) {
    }

    virtual ~User() = default;
    virtual void showMenu() const = 0;

    std::string getUsername() const { return username_; }
    std::string getPassword() const { return password_; }
    std::string getName() const { return name_; }  
    Role getRole() const { return role_; }
    bool isEnabled() const { return enabled_; }

    void setPassword(const std::string& pwd) { password_ = pwd; }
    void setEnabled(bool e) { enabled_ = e; }
};

class Student : public User {
public:
    Student(std::string username, std::string password, std::string name,
        bool enabled = true)
        : User(username, password, name, Role::Student, enabled) {
    }
    void showMenu() const override {
        std::cout << "\n===== 学生菜单 =====\n";
        std::cout << "1. 查看个人信息\n";
        std::cout << "2. 查询成绩与排名\n";
        std::cout << "3. 导出个人成绩报表\n";
        std::cout << "0. 退出登录\n";
        std::cout << "====================\n";
    }
};

class Teacher : public User {
public:
    Teacher(std::string username, std::string password, std::string name,
        bool enabled = true)
        : User(username, password, name, Role::Teacher, enabled) {
    }
    void showMenu() const override {
        std::cout << "\n===== 教师菜单 =====\n";
        std::cout << "1. 管理学生信息\n";
        std::cout << "2. 录入/修改成绩\n";
        std::cout << "3. 查看班级统计\n";
        std::cout << "0. 退出\n";
        std::cout << "====================\n";
    }
};

class Admin : public User {
public:
    Admin(std::string username, std::string password, std::string name,
        bool enabled = true)
        : User(username, password, name, Role::Admin, enabled) {
    }
    void showMenu() const override {
        std::cout << "\n===== 管理员菜单 =====\n";
        std::cout << "1. 全局账号管理\n";
        std::cout << "2. 系统权限管控\n";
        std::cout << "3. 全量数据管理\n";
        std::cout << "0. 退出\n";
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
            bool enabled = (enabledStr == "1");

            if (roleStr == "student") {
                users.push_back(std::make_unique<Student>(username, password, name, enabled));
            }
            else if (roleStr == "teacher") {
                users.push_back(std::make_unique<Teacher>(username, password, name, enabled));
            }
            else if (roleStr == "admin") {
                users.push_back(std::make_unique<Admin>(username, password, name, enabled));
            }
        }
        file.close();
        return users;
    }

    //保存用户
    static void saveUsers(const std::string& filename,
        const std::vector<std::unique_ptr<User>>& users) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cout << "错误：无法写入 " << filename << "！\n";
            return;
        }
        for (const auto& u : users) {
            file << u->getUsername() << ","
                << u->getPassword() << ","
                << u->getName() << ","
                << roleToString(u->getRole()) << ","   
                << (u->isEnabled() ? "1" : "0") << "\n";
        }
        file.close();
        std::cout << "用户数据已保存。\n";
    }
    // 读取成绩文件
    static std::vector<Score> loadScores(const std::string& filename) {
        std::vector<Score> scores;
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "警告：找不到 " << filename << "，未加载任何成绩。\n";
            return scores;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string scoreStr;
            Score s;

            std::getline(ss, s.studentId, ',');
            std::getline(ss, s.subject, ',');
            std::getline(ss, scoreStr, ',');
            std::getline(ss, s.term, ',');

            s.value = std::stod(scoreStr);  // 字符串转 double
            scores.push_back(s);
        }
        file.close();
        return scores;
    }
    // 保存成绩到文件
    static void saveScores(const std::string& filename, const std::vector<Score>& scores) {
        std::ofstream file(filename); // 注意：这是 ofstream（输出流），会覆盖原文件
        if (!file.is_open()) {
            std::cout << "错误：无法写入 " << filename << "！\n";
            return;
        }
        for (const auto& s : scores) {
            file << s.studentId << "," << s.subject << ","
                << s.value << "," << s.term << "\n";
        }
        file.close();
        std::cout << "成绩数据已保存到 " << filename << "。\n";
    }
    static User* login(const std::vector<std::unique_ptr<User>>& users,
        const std::string& inputUsername,
        const std::string& inputPassword) {
        for (const auto& user : users) {
            if (user->getUsername() == inputUsername
                && user->getPassword() == inputPassword) {
                if (!user->isEnabled()) {
                    std::cout << "该账号已被禁用，请联系管理员！\n";
                    return nullptr;
                }
                return user.get();
            }
        }
        return nullptr;
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
    auto scores = DataManager::loadScores("scores.txt");
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

      
        if (choice == 0) {
            std::cout << "正在退出系统...\n";
            break;
        }

        if (currentUser->getRole() == Role::Student) {
            switch (choice) {
            case 1:
                std::cout << "\n===== 个人信息 =====\n";
                std::cout << "账号: " << currentUser->getUsername() << "\n";
                std::cout << "姓名: " << currentUser->getName() << "\n";
                std::cout << "角色: " << roleToString(currentUser->getRole()) << "\n";
                std::cout << "====================\n";
                break;
///////////////////////////////////////////////////////////////
            case 2: {
                std::cout << "\n===== 我的成绩 =====\n";
                double total = 0;
                int count = 0;

                // 1. 先打印自己的单科成绩
                for (const auto& s : scores) {
                    if (s.studentId == currentUser->getUsername()) {
                        std::cout << s.subject << ": " << s.value
                            << " (" << s.term << ")\n";
                        total += s.value;
                        count++;
                    }
                }

                if (count == 0) {
                    std::cout << "暂无成绩记录。\n";
                    break; // 没有成绩，直接退出这个分支
                }

                std::cout << "--------------------\n";
                std::cout << "总分: " << total << "\n";
                std::cout << "平均分: " << total / count << "\n";

                // ==========================================
                // 2. 核心新增：计算总分排名
                // ==========================================
                // 第一步：用 map 累加每个学生的总分
                std::map<std::string, double> totalScores;
                for (const auto& s : scores) {
                    totalScores[s.studentId] += s.value;
                }

                // 第二步：把 map 转成 vector，方便排序
                std::vector<std::pair<std::string, double>> rankList(
                    totalScores.begin(), totalScores.end()
                );

                // 第三步：用 std::sort 降序排序（分数高的排前面）
                std::sort(rankList.begin(), rankList.end(),
                    [](const std::pair<std::string, double>& a,
                        const std::pair<std::string, double>& b) {
                            return a.second > b.second; // 降序
                    });

                // 第四步：遍历排行榜，找到自己的名次
                std::cout << "====================\n";
                std::cout << "===== 班级排名 =====\n";
                for (int i = 0; i < rankList.size(); i++) {
                    if (rankList[i].first == currentUser->getUsername()) {
                        std::cout << "你的总分排名: 第 " << (i + 1)
                            << " 名 / 共 " << rankList.size() << " 人\n";
                        break; // 退出循环
                    }
                }

                std::cout << "====================\n";
                break;
            }
//////////////////////////////////////////////////////
            case 3:
                std::cout << "【导出报表】功能开发中...\n";
                break;
            default:
                std::cout << "无效选项！\n";
                break;
            }
        }
        ////////////////////老师
        else if (currentUser->getRole() == Role::Teacher) {
            switch (choice) {
            case 1:
                std::cout << "【教师】管理学生信息功能开发中...\n";
                break;

            case 2: {
                std::cout << "\n===== 录入/修改成绩 =====\n";
                std::string stuId, subject, term;
                double newScore;

                std::cout << "请输入学号: ";
                std::cin >> stuId;
                std::cout << "请输入科目: ";
                std::cin >> subject;
                std::cout << "请输入成绩 (0-100): ";
                std::cin >> newScore;

                // 1. 校验分数区间
                if (newScore < 0 || newScore > 100) {
                    std::cout << "错误：成绩必须在 0-100 之间！\n";
                    break;
                }

                // 2. 校验学号是否存在（且是学生）
                bool stuExists = false;
                for (const auto& u : users) {
                    if (u->getUsername() == stuId && u->getRole() == Role::Student) {
                        stuExists = true;
                        break;
                    }
                }
                if (!stuExists) {
                    std::cout << "错误：找不到该学生！\n";
                    break;
                }

                // 3. 查找并更新成绩
                bool found = false;
                for (auto& s : scores) {
                    if (s.studentId == stuId && s.subject == subject) {
                        s.value = newScore;
                        found = true;
                        std::cout << "该学生 " << subject << " 成绩已存在，已修改为 "
                            << newScore << " 分。\n";
                        break;
                    }
                }

                // 4. 如果没找到，新增一条记录
                if (!found) {
                    std::cout << "请输入学期 (如 2025-2026-1): ";
                    std::cin >> term;
                    scores.push_back({ stuId, subject, newScore, term });
                    std::cout << "已新增成绩记录。\n";
                }

                // 5. 保存回文件
                DataManager::saveScores("scores.txt", scores);
                break;
            }

            case 3:
                std::cout << "【教师】查看班级统计功能开发中...\n";
                break;

            case 0:
                std::cout << "退出登录...\n";
                break;

            default:
                std::cout << "无效选项！\n";
                break;
            }
        }
        /////////////////////master！
        else if (currentUser->getRole() == Role::Admin) {
            switch (choice) {
            case 1: {
                std::cout << "\n===== 全局账号管理 =====\n";
                std::cout << "1. 查看所有账号\n";
                std::cout << "2. 新增账号\n";
                std::cout << "3. 禁用/启用账号\n";
                std::cout << "4. 重置密码\n";
                std::cout << "请选择子操作: ";

                int subChoice;
                std::cin >> subChoice;

                if (subChoice == 1) {
                    // 查看所有账号
                    std::cout << "\n账号\t\t姓名\t\t角色\t状态\n";
                    std::cout << "------------------------------------\n";
                    for (const auto& u : users) {
                        std::cout << u->getUsername() << "\t\t"
                            << u->getName() << "\t\t"
                            << roleToString(u->getRole()) << "\t"
                            << (u->isEnabled() ? "启用" : "禁用") << "\n";
                    }
                }
                else if (subChoice == 2) {
                    // 新增账号
                    std::string newUser, newPwd, newName, roleStr;
                    std::cout << "请输入账号: "; std::cin >> newUser;
                    std::cout << "请输入密码: "; std::cin >> newPwd;
                    std::cout << "请输入姓名: "; std::cin >> newName;
                    std::cout << "请输入角色 (student/teacher): "; std::cin >> roleStr;

                    bool exists = false;
                    for (const auto& u : users) {
                        if (u->getUsername() == newUser) {
                            exists = true;
                            break;
                        }
                    }
                    if (exists) {
                        std::cout << "该账号已存在！\n";
                    }
                    else if (roleStr == "student") {
                        users.push_back(std::make_unique<Student>(newUser, newPwd, newName));
                        std::cout << "学生账号创建成功！\n";
                        DataManager::saveUsers("users.txt", users);
                    }
                    else if (roleStr == "teacher") {
                        users.push_back(std::make_unique<Teacher>(newUser, newPwd, newName));
                        std::cout << "教师账号创建成功！\n";
                        DataManager::saveUsers("users.txt", users);
                    }
                    else {
                        std::cout << "角色不合法！\n";
                    }
                }
                else if (subChoice == 3) {
                    // 禁用/启用账号
                    std::string target;
                    std::cout << "请输入要禁用/启用的账号: "; std::cin >> target;

                    bool found = false;
                    for (auto& u : users) {
                        if (u->getUsername() == target) {
                            u->setEnabled(!u->isEnabled());
                            std::cout << "账号 " << target << " 已"
                                << (u->isEnabled() ? "启用" : "禁用") << "。\n";
                            found = true;
                            break;
                        }
                    }
                    if (!found) std::cout << "找不到该账号！\n";
                    else DataManager::saveUsers("users.txt", users);
                }
                else if (subChoice == 4) {
                    // 重置密码
                    std::string target;
                    std::cout << "请输入要重置密码的账号: "; std::cin >> target;

                    bool found = false;
                    for (auto& u : users) {
                        if (u->getUsername() == target) {
                            u->setPassword("123456");
                            std::cout << "账号 " << target
                                << " 的密码已重置为 123456。\n";
                            found = true;
                            break;
                        }
                    }
                    if (!found) std::cout << "找不到该账号！\n";
                    else DataManager::saveUsers("users.txt", users);
                }
                break;
            }
            }
        }
    }

    std::cout << "感谢使用，再见！\n";
    return 0;
}
