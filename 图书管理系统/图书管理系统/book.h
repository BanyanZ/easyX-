#pragma once
#include<iostream>
#include<cstring>
#include<string>//支持string长文本
using namespace std;

class Book
{
public:
	string bookname_;//书名
	double price_;//价格
	string author_;//作者
	string ISBN_;//ISBN码
	int ID_;//书籍ID
	string press_;//出版社
	int total_;//图书总数
	int leftnum_;//剩余图书数
	string introduction_;//简介
	string date_;//入库日期
	string category_;//图书分类
	int publishYear_;//出版年份
	bool idBorrowed_ = false;//是否被借出
	bool isExisting_ = true;//是否存在
public:
	Book() :bookname_(""), price_(0), author_(""), ISBN_(""), ID_(0), press_(""),
		total_(0), leftnum_(0), introduction_(""), date_(), category_(""), publishYear_(0) {
	}//默认构造函数
	Book(const string& bookname, double price, const string& author, const string& ISBN, int ID, const string& press, int total, int leftnum, const string& introduction, const string& date,
		const string& category, bool isBorrowed) :bookname_(bookname), price_(price), author_(author), ISBN_(ISBN), ID_(ID), press_(press), total_(total), leftnum_(leftnum), introduction_(introduction), date_(date)
		, category_(category), idBorrowed_(isBorrowed) {
	}//有参构造函数
//以下函数的作用是设置书籍的基本信息
	void setBookname(const string& bookname) { bookname_ = bookname; }
	void setPrice(double price) { price_ = price; }
	void setAuthor(const string& author) { author_ = author; }
	void setISBN(const string& ISBN) { ISBN_ = ISBN; }
	void setID(int ID) { ID_ = ID; }
	void setPress(const string& press) { press_ = press; }
	void setTotal(int total) { total_ = total; }
	void setLeftnum(int leftnum) { leftnum_ = leftnum; }
	void setIntroduction(const string& introduction) { introduction_ = introduction; }
	void setDate(string date) { date_ = date; }
	void setCategory(const string& category) { category_ = category; }
	void setPublishYear(int year) { publishYear_ = year; }
	void setIsborrowed(const bool ib) { idBorrowed_ = ib; }
	void setIsexisting(const bool ex) { isExisting_ = ex; }

	//以下是返回值
	string getBookname() const { return bookname_; }
	double getPrice() const { return price_; }
	string getAuthor() const { return author_; }
	string getISBN() const { return ISBN_; }
	int getID() const { return ID_; }
	string getPress() const { return press_; }
	int getTotal() const { return total_; }
	int getLeftnum() const { return leftnum_; }
	string getIntroduction() const { return introduction_; }
	string getDate() const { return date_; }
	string getCategory() const { return category_; }
	int getPublishYear() const { return publishYear_; }
	bool getIsborrowed()const { return idBorrowed_; }
	bool getIsexisting()const { return isExisting_; }

	//显示图书信息
	void displayBook() const {
		cout << "\n========== 图书信息 ==========" << endl;
		cout << "图书ID: " << ID_ << endl;
		cout << "图书名: " << bookname_ << endl;
		cout << "图书价格: " << price_ << endl;
		cout << "图书作者: " << author_ << endl;
		cout << "图书ISBN码: " << ISBN_ << endl;
		cout << "图书出版社: " << press_ << endl;
		cout << "图书总数: " << total_ << endl;
		if (leftnum_ == 0) {
			cout << "当前没有这本书啦" << endl;
		}
		else {
			cout << "图书剩余数量：" << leftnum_ << endl;
		}
		cout << "图书简介：" << introduction_ << endl;
		cout << "入库日期：" << date_ << endl;
		cout << "该图书属于：" << category_ << "类" << endl;
		cout << "==============================" << endl;
	}
};