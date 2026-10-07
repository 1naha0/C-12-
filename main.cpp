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

public:
    User(std::string username, std::string password, std::string name, Role role)
        : username_(username), password_(password), name_(name), role_(role) {
    }

    virtual ~User() = default;
    virtual void showMenu() const = 0;

    std::string getUsername() const { return username_; }
    std::string getPassword() const { return password_; }
    std::string getName() const { return name_; }  
    Role getRole() const { return role_; }
};

class Student : public User {
public:
    Student(std::string username, std::string password, std::string name)
        : User(username, password, name, Role::Student) {
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
    Teacher(std::string username, std::string password, std::string name)
        : User(username, password, name, Role::Teacher) {
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
    Admin(std::string username, std::string password, std::string name)
        : User(username, password, name, Role::Admin) {
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
            if (roleStr == "student") {
                users.push_back(std::make_unique<Student>(username, password, name));
            }
            else if (roleStr == "teacher") {
                users.push_back(std::make_unique<Teacher>(username, password, name));
            }
            else if (roleStr == "admin") {
                users.push_back(std::make_unique<Admin>(username, password, name));
            }
        }
        file.close();
        return users;
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
        else if (currentUser->getRole() == Role::Teacher) {
            std::cout << "【教师】功能 [" << choice << "] 开发中...\n";
        }
        else if (currentUser->getRole() == Role::Admin) {
            std::cout << "【管理员】功能 [" << choice << "] 开发中...\n";
        }
    }

    std::cout << "感谢使用，再见！\n";
    return 0;
}
