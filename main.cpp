#include <iostream>
#include <string>
#include <fstream>   // 文件读写
#include <sstream>   // 字符串分割
enum class Role {
	Student,
	Teacher,
	Admin
};

std::string roleToString(Role role) {
	if (role == Role::Student)return "学生";
	if (role == Role::Teacher)return "教师";
	if (role == Role::Admin)return "管理员";
	return "未知";
}
//抽象基类
class User {
protected:
	std::string username_;
	Role role_;
public:
	User(std::string username,Role role)
		: username_(username),role_(role){ }
	virtual ~User() = default;

	// 纯虚函数！这个函数就是刚才让 Student 报错的那个“参考标准”
	virtual void showMenu() const = 0;

	// 只读窗口
	std::string getUsername() const { return username_; }
	Role getRole() const { return role_; }
};

//学生类
class Student : public User {
public:
	Student(std::string username) : User(username, Role::Student) {}
	void showMenu()const override {
		std::cout << "\n===== 学生菜单 =====\n";
		std::cout << "1. 查看个人信息\n";
		std::cout << "2. 查询成绩与排名\n";
		std::cout << "3. 导出个人成绩报表\n";
		std::cout << "0. 退出登录\n";
		std::cout << "====================\n";
	}

};
// 教师类
class Teacher : public User {
public:
	Teacher(std::string username) : User(username, Role::Teacher) {}
	void showMenu() const override {
		std::cout << "\n===== 教师菜单 =====\n";
		std::cout << "1. 管理班级学生信息\n";
		std::cout << "2. 录入/修改成绩\n";
		std::cout << "3. 查看班级排名与统计\n";
		std::cout << "0. 退出登录\n";
		std::cout << "====================\n";
	}
};
// 管理员类
class Admin : public User {
public:
	Admin(std::string username) : User(username, Role::Admin) {}
	void showMenu() const override {
		std::cout << "\n===== 超级管理员菜单 =====\n";
		std::cout << "1. 全局账号管理\n";
		std::cout << "2. 系统权限管控\n";
		std::cout << "3. 全量数据管理\n";
		std::cout << "0. 退出登录\n";
		std::cout << "=========================\n";
	}
};
int main() {
	// ========== 测试：读取 users.txt ==========
	std::ifstream file("users.txt"); // 打开文件
	if (!file.is_open()) {
		std::cout << "错误：找不到 users.txt 文件！请检查文件位置。\n";
		return 0;
	}

	std::string line;
	std::cout << "===== 成功读取到的用户数据 =====\n";
	while (std::getline(file, line)) { // 逐行读取
		std::cout << "读取到: " << line << "\n";
	}
	file.close();
	std::cout << "===============================\n";
	// ==========================================
	std::string inputUser;
	std::cout << "请输入用户名 (student / teacher / admin): ";
	std::cin >> inputUser;
	User* currentUser = nullptr;
	// 根据输入，创建对应的子类对象，交给父类指针保管
	if (inputUser == "student") {
		currentUser = new Student("张三");
	}
	else if (inputUser == "teacher") {
		currentUser = new Teacher("李老师");
	}
	else if (inputUser == "admin") {
		currentUser = new Admin("超级管理员");
	}
	else {
		std::cout << "账号不存在！\n";
		return 0;
	}
	std::cout << "欢迎回来，" << currentUser->getUsername()
		<< " [" << roleToString(currentUser->getRole()) << "]\n";
	int choice = -1;
	while (choice != 0) {
		currentUser->showMenu(); // 多态调用：不同角色显示不同菜单
		std::cout << "请选择操作: ";
		std::cin >> choice;
		// 根据角色分流，执行不同的业务
		if (currentUser->getRole() == Role::Student) {
			switch (choice) {
			case 1: std::cout << "【学生】查看个人信息\n"; break;
			case 2: std::cout << "【学生】查询成绩与排名\n"; break;
			case 3: std::cout << "【学生】导出个人成绩报表\n"; break;
			case 0: std::cout << "退出登录...\n"; break;
			default: std::cout << "无效选项！\n"; break;
			}
		}
		else if (currentUser->getRole() == Role::Teacher) {
			switch (choice) {
			case 1: std::cout << "【教师】管理班级学生信息\n"; break;
			case 2: std::cout << "【教师】录入/修改成绩\n"; break;
			case 3: std::cout << "【教师】查看班级排名与统计\n"; break;
			case 0: std::cout << "退出登录...\n"; break;
			default: std::cout << "无效选项！\n"; break;
			}
		}
		else if (currentUser->getRole() == Role::Admin) {
			switch (choice) {
			case 1: std::cout << "【管理员】全局账号管理\n"; break;
			case 2: std::cout << "【管理员】系统权限管控\n"; break;
			case 3: std::cout << "【管理员】全量数据管理\n"; break;
			case 0: std::cout << "退出登录...\n"; break;
			default: std::cout << "无效选项！\n"; break;
			}
		}
	}
	std::cout << "感谢使用，再见！\n";
	delete currentUser; // 安全释放内存（虚析构的功劳）
	return 0;
}
