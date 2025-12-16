#pragma once
#include<iostream>
#include<cstring>
#include<string>
using namespace std;

class User
{
public:
	int m_nID; //对应ID
	string username_;//用户账号
	string passord_;//用户密码
	int role_;//用户角色，1是管理员，2是普通用户
	string realname_;//用户真实姓名
	string contact_;//联系方式
	int maxBorrownum_;//最大借书数量
	int currentBorrownum_;//当前借书数量
public:
	User() :m_nID(-1), username_(""), passord_(""), role_(2), realname_(""), contact_(""),
		maxBorrownum_(5), currentBorrownum_(0) {
	}//默认构造函数
	User(string username, int userID, string passord, int role, string realName, string contact, int maxBorrownum, int currentBorrownum) :username_(username),
		m_nID(userID), passord_(passord), role_(role), realname_(realName), contact_(contact),
		maxBorrownum_(maxBorrownum), currentBorrownum_(currentBorrownum) {
	}//有参构造

	//设置初始值
	void setUsername(string username) { username_ = username; }
	void setPassord(string passord) { passord_ = passord; }
	void setRole(int role) { role_ = role; }
	void setRealName(string realName) { realname_ = realName; }
	void setContact(string contact) { contact_ = contact; }
	void setMaxBorrowNum(int maxNum) { maxBorrownum_ = maxNum; }
	void setCurrentBorrowNum(int currentNum) { currentBorrownum_ = currentNum; }
	void setID(int id) { m_nID = id; }
	//返回值(添加const)
	string getUsername() const { return username_; }
	string getPassord() const { return passord_; }
	int getRole() const { return role_; }
	string getRealName() const { return realname_; }
	string getContact() const { return contact_; }
	int getMaxBorrowNum() const { return maxBorrownum_; }
	int getCurrentBorrowNum() const { return currentBorrownum_; }
	int getID() const { return m_nID; }

	//增加/减少当前借书数量
	void incrementBorrowNum() { if (currentBorrownum_ < maxBorrownum_) currentBorrownum_++; }
	void decrementBorrowNum() { if (currentBorrownum_ > 0) currentBorrownum_--; }

	//显示用户信息
	void displayUser() const {
		cout << "\n========== 用户信息 ==========" << endl;
		cout << "ID: " << m_nID << endl;
		cout << "用户名: " << username_ << endl;
		cout << "真实姓名: " << realname_ << endl;
		cout << "联系方式: " << contact_ << endl;
		cout << "最大借书量: " << maxBorrownum_ << endl;
		cout << "当前借书量: " << currentBorrownum_ << endl;
		cout << "角色: " << (role_ == 1 ? "管理员" : "普通用户") << endl;
		cout << "==============================" << endl;
	}
};