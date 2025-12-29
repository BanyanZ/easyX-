#pragma once
#include<iostream>
#include"manageBook.h"
#include"book.h"
#include<vector>
#include<string>
#include<fstream>
#include<cstdint>
#include<sstream>
using namespace std;

class ManageBook :public Book
{
private:
	vector<Book> bkarray;

public:
	ManageBook() {}//默认构造
	~ManageBook() {}//析构函数

	//以下是一些功能的实现
	//1.添加图书
	void addBook(Book newbook) {
		bkarray.push_back(newbook);
	}

	//2.查找图书，通过重构函数实现，没找到就return -1
	int findBook(int an) {
		for (int i = 0; i < bkarray.size(); i++) {
			if (bkarray[i].getID() == an) {
				return i;
			}
		}
		return -1;
	}
	int findBook(string bn) {//根据书名，作者，类别，ISBN码（精确匹配）
		for (int i = 0; i < bkarray.size(); i++) {
			if (bkarray[i].getBookname() == bn || bkarray[i].getAuthor() == bn || bkarray[i].getCategory() == bn || bkarray[i].getISBN() == bn || bkarray[i].getDate() == bn) {
				return i;
			}
		}
		return -1;
	}

	// 模糊查询，返回所有匹配的索引
	vector<int> fuzzyFindBook(const string& keyword) {
		vector<int> results;
		if (keyword.empty()) return results;

		for (int i = 0; i < (int)bkarray.size(); i++) {
			if (!bkarray[i].getIsexisting()) continue;

			string bookname = bkarray[i].getBookname();
			string author = bkarray[i].getAuthor();
			string category = bkarray[i].getCategory();
			string isbn = bkarray[i].getISBN();
			string press = bkarray[i].getPress();

			// 转换为小写进行不区分大小写的匹配（简单处理，中文直接比较）
			// 检查是否包含关键词
			if (bookname.find(keyword) != string::npos ||
				author.find(keyword) != string::npos ||
				category.find(keyword) != string::npos ||
				isbn.find(keyword) != string::npos ||
				press.find(keyword) != string::npos) {
				results.push_back(i);
			}
		}
		return results;
	}

	//3.修改图书信息，根据上面查找到的书籍数字进行修改
	void chgBookID(int idx, int id) {
		bkarray[idx].setID(id);
	}
	void chgBookname(int idx, string um) {
		bkarray[idx].setBookname(um);
	}
	void chgBookAuthor(int idx, string pa) {
		bkarray[idx].setAuthor(pa);
	}
	void chgBookPrice(int idx, int ro) {
		bkarray[idx].setPrice(ro);
	}
	void chgBookCategory(int idx, string rn) {
		bkarray[idx].setCategory(rn);
	}
	void chgBookDate(int idx, string con) {
		bkarray[idx].setDate(con);
	}
	void chgBookIntroduction(int idx, string max) {
		bkarray[idx].setIntroduction(max);
	}
	void chgBookISBN(int idx, string curr) {
		bkarray[idx].setISBN(curr);
	}
	void chgBookIsborrowed(int idx, bool ib) {
		bkarray[idx].setIsborrowed(ib);
	}
	void chgBookPress(int idx, string ie) {
		bkarray[idx].setPress(ie);
	}
	void chgBookTotal(int idx, int to) {
		bkarray[idx].setTotal(to);
	}
	void chgBookLeftnum(int idx, int lef) {
		bkarray[idx].setLeftnum(lef);
	}

	//4.删除图书
	void delBook(int temp) {
		if (!bkarray[temp].getIsborrowed() || bkarray[temp].getLeftnum() == 0) {
			cout << "该书籍状态异常，无法删除" << endl;
		}
		bkarray.erase(bkarray.begin() + temp);
	}

	//5.数据化管理
	void save(string filename)
	{
		ofstream outfile(filename.c_str());
		if (!outfile.is_open())
		{
			cout << "Error opening file";
			return;
		}
		// CSV格式: ID,书名,价格,作者,ISBN,出版社,总数,剩余,简介,入库日期,分类,出版年份,是否借出,是否存在
		for (size_t i = 0; i < bkarray.size(); i++)
		{
			outfile << bkarray[i].getID() << ","
				<< bkarray[i].getBookname() << ","
				<< bkarray[i].getPrice() << ","
				<< bkarray[i].getAuthor() << ","
				<< bkarray[i].getISBN() << ","
				<< bkarray[i].getPress() << ","
				<< bkarray[i].getTotal() << ","
				<< bkarray[i].getLeftnum() << ","
				<< bkarray[i].getIntroduction() << ","
				<< bkarray[i].getDate() << ","
				<< bkarray[i].getCategory() << ","
				<< bkarray[i].getPublishYear() << ","
				<< bkarray[i].getIsborrowed() << ","
				<< bkarray[i].getIsexisting() << endl;
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
		bkarray.clear();
		string line;
		while (getline(infile, line))
		{
			if (line.empty()) continue;
			Book b;
			stringstream ss(line);
			string token;

			getline(ss, token, ','); b.setID(stoi(token));
			getline(ss, token, ','); b.setBookname(token);
			getline(ss, token, ','); b.setPrice(stod(token));
			getline(ss, token, ','); b.setAuthor(token);
			getline(ss, token, ','); b.setISBN(token);
			getline(ss, token, ','); b.setPress(token);
			getline(ss, token, ','); b.setTotal(stoi(token));
			getline(ss, token, ','); b.setLeftnum(stoi(token));
			getline(ss, token, ','); b.setIntroduction(token);
			getline(ss, token, ','); b.setDate(token);
			getline(ss, token, ','); b.setCategory(token);
			getline(ss, token, ','); b.setPublishYear(stoi(token));
			getline(ss, token, ','); b.setIsborrowed(stoi(token));
			getline(ss, token, ','); b.setIsexisting(stoi(token));
			bkarray.push_back(b);
		}
		infile.close();
	}

	//6.返回某本书的信息
	Book getBook(int idx) {
		return bkarray[idx];
	}

	//7.展示所有图书信息
	void showAllBooks() {
		cerr << "当前图书数量为：" << bkarray.size() << endl;
		cout << "所有的图书为" << endl;
		for (int i = 0; i < bkarray.size(); i++)
		{
			if (bkarray[i].getIsexisting())
				bkarray[i].displayBook();
		}
	}

	//8.辅助函数
	vector<Book>& allBooks() { return bkarray; }
};