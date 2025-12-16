#pragma once
#include<iostream>
#include"user.h"
#include"manageTime.h"
#include"book.h"
#include<vector>
#include<string>
using namespace std;

class Reader :public User
{
private:
	bool isBorrowing_;//是否正在借书
	bool idExisting_;//读者是否存在
	Book allBooks_;//读者借的所有书
public:
	Reader() :isBorrowing_(false), idExisting_(true) {}//默认构造函数
	~Reader() {}//析构函数

	//设置值
	void setIsBorrowing(bool isBorrowing) { isBorrowing_ = isBorrowing; }
	void setIdExisting(bool idExisting) { idExisting_ = idExisting; }
	void setAllBooks(Book allBooks) { allBooks_ = allBooks; }

	//返回值
	bool getIsBorrowing() const { return isBorrowing_; }
	bool getIdExisting() const { return idExisting_; }
	Book getAllBooks() const { return allBooks_; }

	//显示所借的所有图书（只展示书名）
	void displayAllBooks() const {
		cout << "借阅图书信息:" << endl;
		if (!isBorrowing_) {
			cout << " 目前没有借阅记录。" << endl;
			return;
		}
		cout << " 书名是 " << allBooks_.getBookname() << "， ID是 " << allBooks_.getID() << endl;
	}

	//显示读者信息
	void displayReader() const {
		cout << "\n========== 读者信息 ==========" << endl;
		displayUser(); // 调用基类User的显示函数
		cout << "是否正在借书: " << (isBorrowing_ ? "是" : "否") << endl;
		cout << "读者是否存在: " << (idExisting_ ? "存在" : "已删除") << endl;
		if (isBorrowing_) {
			cout << "借阅图书信息:" << endl;
			allBooks_.displayBook(); // 调用Book类的显示函数
		}
		cout << "==============================" << endl;
	}

	//显示读者借的所有书籍
	Book getBook(int temp) {
		return allBooks_;
	}


};