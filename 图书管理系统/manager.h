#pragma once
#include<iostream>
#include"user.h"
#include"manageTime.h"
#include"book.h"
#include"reader.h"
#include<vector>
#include<string>
#include<fstream>
#include<sstream>
using namespace std;

class Manager :public User
{
public:
	vector<Reader> rd;//存储读者信息便于进行管理
	//将一本书设置为某读者的借阅书（保存到 Reader 内部）
	bool setReaderBook(int index, const Book& b) {
		if (index < 0 || static_cast<size_t>(index) >= rd.size()) return false;
		rd[index].setAllBooks(b);
		// 同时标记读者为正在借书
		rd[index].setIsBorrowing(true);
		return true;
	}
	//清空读者的借书信息
	bool clearReaderBook(int index) {
		if (index < 0 || static_cast<size_t>(index) >= rd.size()) return false;
		rd[index].setAllBooks(Book()); // 赋默认 Book 表示无借书
		rd[index].setIsBorrowing(false);
		return true;
	}
public:
	Manager() {}//默认构造函数
	~Manager() {}//析构函数

	//以下是对于读者的管理
	//1.添加读者
	void addReader(Reader newrd) {
		rd.push_back(newrd);
	}

	//2.找到读者，必须得有这个，要不你不知道更改哪个读者的信息，鉴于没有要求，这边直通过真实名字进行查找
	int findReader(string rn) {
		for (int i = 0; i < rd.size(); i++) {
			if (rd[i].getRealName() == rn) {
				return i;
			}
		}
		return -1;//找不到返回-1
	}

	int findReader(int rn) {
		for (int i = 0; i < rd.size(); i++) {
			if (rd[i].getID() == rn) {
				return i;
			}
		}
		return -1;//找不到返回-1
	}

	// 模糊查询读者，返回所有匹配的索引
	vector<int> fuzzyFindReader(const string& keyword) {
		vector<int> results;
		if (keyword.empty()) return results;

		for (int i = 0; i < (int)rd.size(); i++) {
			if (!rd[i].getIdExisting()) continue;

			string username = rd[i].getUsername();
			string realname = rd[i].getRealName();
			string contact = rd[i].getContact();
			string idStr = to_string(rd[i].getID());

			// 检查是否包含关键词（支持ID、用户名、真实姓名、联系方式）
			if (idStr.find(keyword) != string::npos ||
				username.find(keyword) != string::npos ||
				realname.find(keyword) != string::npos ||
				contact.find(keyword) != string::npos) {
				results.push_back(i);
			}
		}
		return results;
	}

	//3.读者信息修改,这里用的是重构函数
	void chgReaderID(int idx, int id) {
		rd[idx].setID(id);
	}
	void chgReaderUsername(int idx, string um) {
		rd[idx].setUsername(um);
	}
	void chgReaderPassord(int idx, string pa) {
		rd[idx].setPassord(pa);
	}
	void chgReaderRole(int idx, int ro) {
		rd[idx].setRole(ro);
	}
	void chgReaderRealname(int idx, string rn) {
		rd[idx].setRealName(rn);
	}
	void chgReaderContact(int idx, string con) {
		rd[idx].setContact(con);
	}
	void chgReaderMaxBorrownum(int idx, int max) {
		rd[idx].setMaxBorrowNum(max);
	}
	void chgReaderCurrentBorrownum(int idx, int curr) {
		rd[idx].setCurrentBorrowNum(curr);
	}
	void chgReaderIsborrowing(int idx, bool ib) {
		rd[idx].setIsBorrowing(ib);
	}
	void chgReaderIsecisting(int idx, int ie) {
		rd[idx].setIdExisting(ie);
	}

	//4.读者注销
	void delReader(int temp) {
		//如果读者当前借书数量大于0或标记为正在借书，则不允许注销
		if (rd[temp].getCurrentBorrowNum() > 0 || rd[temp].getIsBorrowing()) {
			cout << "该读者仍有未归还图书，无法注销账号。" << endl;
			return;
		}
		//没有借书，可以注销（标记为不存在）
		rd[temp].setIdExisting(false);
		rd.erase(rd.begin() + temp);
	}

	void save(string filename)
	{
		ofstream outfile(filename.c_str());
		if (!outfile.is_open())
		{
			cout << "Error opening file";
			return;
		}
		// CSV格式: ID,用户名,密码,角色,真实姓名,联系方式,最大借书量,当前借书量,是否借阅中,是否存在
		for (size_t i = 0; i < rd.size(); i++)
		{
			outfile << rd[i].getID() << ","
				<< rd[i].getUsername() << ","
				<< rd[i].getPassord() << ","
				<< rd[i].getRole() << ","
				<< rd[i].getRealName() << ","
				<< rd[i].getContact() << ","
				<< rd[i].getMaxBorrowNum() << ","
				<< rd[i].getCurrentBorrowNum() << ","
				<< rd[i].getIsBorrowing() << ","
				<< rd[i].getIdExisting() << endl;
		}
		outfile.close();
	}

	void load(string filename)
	{
		ifstream infile(filename.c_str());
		if (!infile.is_open())
		{
			return;
		}
		rd.clear();
		string line;
		while (getline(infile, line))
		{
			if (line.empty()) continue;
			Reader r;
			stringstream ss(line);
			string token;

			getline(ss, token, ','); r.setID(stoi(token));
			getline(ss, token, ','); r.setUsername(token);
			getline(ss, token, ','); r.setPassord(token);
			getline(ss, token, ','); r.setRole(stoi(token));
			getline(ss, token, ','); r.setRealName(token);
			getline(ss, token, ','); r.setContact(token);
			getline(ss, token, ','); r.setMaxBorrowNum(stoi(token));
			getline(ss, token, ','); r.setCurrentBorrowNum(stoi(token));
			getline(ss, token, ','); r.setIsBorrowing(stoi(token));
			getline(ss, token, ','); r.setIdExisting(stoi(token));
			rd.push_back(r);
		}
		infile.close();
	}

	//6.得到某个读者的所有信息
	Reader getreader(int temp) {
		return rd[temp];
	}

	//显示所有已录入的读者信息
	void displayAllReaderInfo() const {
		cout << "\n========== 所有读者信息 ==========" << endl;
		for (size_t i = 0; i < rd.size(); ++i) {
			rd[i].displayReader();
		}
		cout << "==================================" << endl;
	}
};