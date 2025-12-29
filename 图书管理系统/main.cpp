#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include "book.h"
#include "reader.h"
#include "manageBorrowrecord.h"
#include "manager.h"
#include "manageTime.h"
#include "manageBook.h"
#include<graphics.h>//引用图形库头文件
#include <conio.h>
using namespace std;

extern int main_gui();//声明主界面函数 
//全局变量
Manager manager;
ManageBook bookmanager;
ManageBorrowRecord recordMgr;

//辅助：把时间字符串中的非数字字符移除，返回仅数字的字符串
static string normalize_digits(const string& s) {
	string out;
	out.reserve(s.size());
	for (char c : s) if (c >= '0' && c <= '9') out.push_back(c);
	return out;
}

//格式化 time_t 为连续数字时间 YYYYMMDDHHMMSS
static string format_time_continuous(time_t t) {
	char buf[32] = { 0 };
	struct tm tm1;
#if defined(_WIN32) || defined(_WIN64)
	localtime_s(&tm1, &t);
#else
	localtime_r(&t, &tm1);
#endif
	sprintf(buf, "%04d%02d%02d%02d%02d%02d",
		tm1.tm_year + 1900, tm1.tm_mon + 1, tm1.tm_mday,
		tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
	return string(buf);
}
//转化时间格式使其更易读，输入任意包含至少14个数字的字符串，输出格式 YYYY.MM.DD.HH.MM.SS
static string pretty_format_digits(const string& s) {
	string d = normalize_digits(s);
	if (d.size() < 14) return s; // 长度不够，返回原串
	return d.substr(0, 4) + "." + d.substr(4, 2) + "." + d.substr(6, 2) + "." +
		d.substr(8, 2) + "." + d.substr(10, 2) + "." + d.substr(12, 2);
}

void lendbook()
{
	string readerName, bookName;
	int readerTemp, bookTemp;
	string borrowTime, shouldReturnTime;
	int shouldreturnDays;
	cout << "请输入读者名称" << endl;
	cin >> readerName;
	readerTemp = manager.findReader(readerName);
	if (readerTemp == -1)
	{
		cout << "读者不存在" << endl;
		return;
	}
	cout << "请输入书籍名称" << endl;
	cin >> bookName;
	bookTemp = bookmanager.findBook(bookName);
	if (bookTemp == -1)
	{
		cout << "书籍不存在或者已经没有库存" << endl;
		return;
	}
	cout << "请输入借书时间（格式 YYYYMMDDHHMMSS，可留空自动生成）" << endl;
	cin >> borrowTime;
	cout << "请输入应还书时间（格式 YYYYMMDDHHMMSS，可留空自动生成）" << endl;
	cin >> shouldReturnTime;
	bookmanager.chgBookIsborrowed(bookTemp, true);
	manager.chgReaderIsborrowing(readerTemp, true);
	manager.chgReaderCurrentBorrownum(readerTemp, manager.getCurrentBorrowNum() - 1);
	//若用户留空，使用 ManageTime 生成
	ManageTime timeUtil;
	if (borrowTime.empty()) {
		time_t now = time(nullptr);
		char buf[32] = { 0 };
		timeUtil.TimeToString(now, buf); // 注意 TimeToString 输出 "YYYY.MM.DD"
		borrowTime = string(buf);
	}
	if (shouldReturnTime.empty()) {
		time_t t2 = time(nullptr) + (7 * 24 * 3600);
		char buf[32] = { 0 };
		timeUtil.TimeToString(t2, buf);
		shouldReturnTime = string(buf);
	}
	string nb = normalize_digits(borrowTime);
	string ns = normalize_digits(shouldReturnTime);
	int borrowDays = timeUtil.calculateDaysDifference(nb, ns); // 使用字符串接口（非 const）
	shouldreturnDays = (borrowDays < 0 ? 0 : borrowDays);

	//创建借阅记录并保存到 recordMgr（borrowID 使用当前记录数+1）
	int borrowID = static_cast<int>(recordMgr.allRecords().size()) + 1;
	BorrowRecord rec(manager.getreader(readerTemp).getID(), bookmanager.getBook(bookTemp).getID(), borrowTime, shouldReturnTime, string(""), 0, false, false, 0.0, borrowID);
	rec.setBprrowID(borrowID);
	recordMgr.addBorrowRecord(rec);
	//把借的书写回到 Reader 中，便于 returnBook 查找
	manager.setReaderBook(readerTemp, bookmanager.getBook(bookTemp));
	cout << "借书成功，借期 (天): " << shouldreturnDays << endl;
}

void returnBook()
{
	cout << "请输入您的真实姓名" << endl;
	string realname;
	cin >> realname;
	int readerIdx = manager.findReader(realname);
	if (readerIdx == -1)
	{
		cout << "读者不存在" << endl;
		return;
	}
	Reader reader = manager.getreader(readerIdx);
	Book book = reader.getBook(readerIdx);
	int booksubscript = bookmanager.findBook(book.getBookname());
	if (booksubscript == -1)
	{
		cout << "并未找到此书" << endl;
		return;
	}
	// 在 recordMgr 中查找该读者的未归还记录（匹配 bookID 且 returnTime 为空）
	int userID = reader.getID();
	vector<int> recIndices = recordMgr.findRecordsByUser(userID);
	int targetIdx = -1;
	for (int idx : recIndices) {
		const BorrowRecord& r = recordMgr.allRecords()[idx];
		if (r.getBookID() == book.getID() && r.getReturnTime().empty()) {
			targetIdx = idx;
			break;
		}
	}
	if (targetIdx == -1) {
		cout << "未找到该读者对应的未归还借阅记录" << endl;
		// 仍尝试恢复状态
		bookmanager.chgBookIsborrowed(booksubscript, false);
		manager.chgReaderIsborrowing(readerIdx, false);
		return;
	}

	// 获取借阅记录的 borrowID
	const BorrowRecord& recBefore = recordMgr.allRecords()[targetIdx];
	int borrowID = recBefore.getBorrowID();

	// 记录实际归还时间（连续数字格式）
	time_t now = time(nullptr);
	string actualReturn = format_time_continuous(now);

	// 使用 recordMgr.returnBook 来处理恢复库存与设置归还时间，并获取罚款（该函数会写 returnTime）
	double fine = recordMgr.returnBook(borrowID, bookmanager.allBooks());
	if (fine < 0) {
		cout << "还书处理失败（记录可能已归还或不存在）" << endl;
		return;
	}


	// 为了比较，我们再次读取记录（已被 recordMgr.returnBook 更新）
	const BorrowRecord& recAfter = recordMgr.allRecords()[targetIdx];
	// 将记录中的应还时间和实际归还时间规范化为纯数字，然后计算天数差
	ManageTime timeUtil;
	string shouldStr = normalize_digits(recAfter.getShouldReturnTime());
	string returnStr = normalize_digits(recAfter.getReturnTime());
	int overdueDays = 0;
	if (!shouldStr.empty() && !returnStr.empty()) {
		overdueDays = timeUtil.calculateDaysDifference(shouldStr, returnStr);
		if (overdueDays < 0) overdueDays = 0;
	}

	//美观输出日期（YYYY.MM.DD.HH.MM.SS）
	cout << "实际归还时间: " << pretty_format_digits(returnStr) << endl;
	cout << "应还时间: " << pretty_format_digits(shouldStr) << endl;
	cout << "超期天数: " << overdueDays << endl;
	cout << "罚款: " << (overdueDays * 8) << " 元" << endl;

	// 输入评价（1-5星）
	cout << "请为这本书评分（1-5星）：" << endl;
	int rating = 0;
	cin >> rating;
	if (rating < 1 || rating > 5) {
		cout << "评价必须在1-5星之间，已设置为默认值5星" << endl;
		rating = 5;
	}
	recordMgr.setRecordRating(borrowID, rating);
	cout << "评价已保存：" << rating << " 星" << endl;

	//更新读者内部借书信息（还书后清空）
	manager.clearReaderBook(readerIdx);

	//更新界面/状态
	bookmanager.chgBookIsborrowed(booksubscript, false);
	manager.chgReaderIsborrowing(readerIdx, false);
}

void createReader()
{
	int code;
	string name;
	string username;
	string passord;
	Reader reader;
	cout << "请输入读者编号" << endl;
	cin >> code;
	if (manager.findReader(code) != -1)
	{
		cout << "读者编号已存在，请重新创建" << endl;
		return;
	}
	cout << "请输入读者真实姓名" << endl;
	cin >> name;
	cout << "请输入给自己取的用户名" << endl;
	cin >> username;
	if (manager.findReader(username) != -1)
	{
		cout << "用户名已存在，请重新创建" << endl;
		return;
	}
	cout << "请输入你的账号密码" << endl;
	cin >> passord;
	reader.setID(code);
	reader.setRealName(name);
	reader.setPassord(passord);
	manager.addReader(reader);
}

void createBook()
{
	Book book;
	int code;
	string name;
	cout << "请输入书号" << endl;
	cin >> code;
	if (bookmanager.findBook(code) != -1)
	{
		cout << "书号已存在，请重新创建" << endl;
		return;
	}
	cout << "请输入书名" << endl;
	cin >> name;
	if (bookmanager.findBook(name) != -1)
	{
		book.setTotal(book.getTotal() + 1);
	}
	book.setID(code);
	book.setBookname(name);
	bookmanager.addBook(book);
}

void delBook()
{
	cout << "请输入书名" << endl;
	string bookname;
	cin >> bookname;
	int booksubscript = bookmanager.findBook(bookname);
	if (booksubscript == -1)
	{
		cout << "没有这本书" << endl;
		return;
	}
	bookmanager.delBook(booksubscript);
	cout << "删除成功" << endl;
}

void editBook()
{
	cout << "请输入书名" << endl;
	string bookname;
	cin >> bookname;
	int booksubscript = bookmanager.findBook(bookname);
	if (booksubscript == -1)
	{
		cout << "没有这本书" << endl;
		return;
	}
	int newbookcode;
	string newbookname;
	string category;
	string newauthor;
	string newintroduction;
	cout << "==========请选择你要修改的选项==========" << endl;
	cout << "1.修改书名" << endl;
	cout << "2.修改书号" << endl;
	cout << "3.修改作者" << endl;
	cout << "4.修改类别" << endl;
	cout << "5.修改简介" << endl;
	cout << "0.退出修改" << endl;
	cout << "====================================" << endl;
	int choice;
	cin >> choice;
	switch (choice)
	{
	case 1:
		cout << "请输入新书名" << endl;
		cin >> newbookname;
		bookmanager.chgBookname(booksubscript, newbookname);
		break;
	case 2:
		cout << "请输入新书号" << endl;
		cin >> newbookcode;
		bookmanager.chgBookID(booksubscript, newbookcode);
		break;
	case 3:
		cout << "请输入新作者" << endl;
		cin >> newauthor;
		bookmanager.chgBookAuthor(booksubscript, newauthor);
		break;
	case 4:
		cout << "请输入新类别" << endl;
		cin >> category;
		bookmanager.chgBookCategory(booksubscript, category);
		break;
	case 5:
		cout << "请输入新简介" << endl;
		cin >> newintroduction;
		bookmanager.chgBookIntroduction(booksubscript, newintroduction);
		break;
	case 0:
		return;
	}
}

void findBook()
{
	cout << "1.按书名查找" << endl << "2.按书号查找" << "其他退出" << endl;
	int command;
	cin >> command;
	int booksubscript = -1;
	if (command == 1)
	{
		cout << "请输入书名" << endl;
		string bookname;
		cin >> bookname;
		booksubscript = bookmanager.findBook(bookname);
	}
	else if (command == 2)
	{
		cout << "请输入书号" << endl;
		int bookcode;
		cin >> bookcode;
		booksubscript = bookmanager.findBook(bookcode);
	}
	else
	{
		return;
	}
	if (booksubscript == -1)
	{
		cout << "没有这本书" << endl;
		return;
	}
	Book book = bookmanager.getBook(booksubscript);
	book.displayBook();
}

void listBook() {
	bookmanager.showAllBooks();
}

void manageBook() {
	cout << "==========图书管理系统==========" << endl;
	cout << "1.增加图书" << endl;
	cout << "2.删除图书" << endl;
	cout << "3.更改图书" << endl;
	cout << "4.查找图书" << endl;
	cout << "5.显示图书" << endl;
	cout << "0.退出图书管理系统" << endl;
	cout << "==============================" << endl;
	int command;
	cout << "请输入指令" << endl;
	cin >> command;
	switch (command)
	{
	case 1:
		createBook();
		break;
	case 2:
		delBook();
		break;
	case 3:
		editBook();
		break;
	case 4:
		findBook();
		break;
	case 5:
		listBook();
	defult:
		return;
	}
}

void createReaderA() {//创建账号
	int code;
	string realname;
	string username;
	string passord;
	string contact;
	cout << "请输入读者ID" << endl;
	cin >> code;
	if (manager.findReader(code) != -1)
	{
		cout << "读者ID已存在，请重新创建" << endl;
		return;
	}
	cout << "请输入读者姓名" << endl;
	cin >> realname;
	cout << "请输入给自己取的用户名" << endl;
	cin >> username;
	if (manager.findReader(username) != -1)
	{
		cout << "用户名已存在，请重新创建" << endl;
		return;
	}
	cout << "请输入你的账号密码" << endl;
	cin >> passord;
	cout << "请输入联系方式" << endl;
	cin >> contact;
	Reader reader;
	reader.setID(code);
	reader.setRealName(realname);
	reader.setUsername(username);
	reader.setPassord(passord);
	reader.setContact(contact);
	manager.addReader(reader);
}


void delReader() {
	//删除账号
	cout << "请输入读者ID" << endl;
	int code;
	cin >> code;
	int readersubscript = manager.findReader(code);
	if (readersubscript == -1)
	{
		cout << "没有这位读者" << endl;
		return;
	}
	manager.delReader(readersubscript);
	cout << "删除成功" << endl;
}

void editReader() {
	string username;
	string realname;
	int code;
	string passord;
	string contact;
	int maxBorrownum;
	cout << "请输入读者ID" << endl;
	cin >> code;
	int readersubscript = manager.findReader(code);
	if (readersubscript == -1)
	{
		cout << "没有这位读者" << endl;
		return;
	}
	cout << "==========请选择你要修改的选项==========" << endl;
	cout << "1.修改读者姓名" << endl;
	cout << "2.修改读者用户名" << endl;
	cout << "3.修改读者密码" << endl;
	cout << "4.修改读者联系方式" << endl;
	cout << "5.修改读者最大借书量" << endl;
	cout << "0.退出修改" << endl;
	cout << "====================================" << endl;
	int choice;
	cin >> choice;
	switch (choice)
	{
	case 1:
		cout << "请输入新读者姓名" << endl;
		cin >> realname;
		manager.chgReaderRealname(readersubscript, realname);
		break;
	case 2:
		cout << "请输入新读者用户名" << endl;
		cin >> username;
		manager.chgReaderUsername(readersubscript, username);
		break;
	case 3:
		cout << "请输入新读者密码" << endl;
		cin >> passord;
		manager.chgReaderPassord(readersubscript, passord);
		break;
	case 4:
		cout << "请输入新读者联系方式" << endl;
		cin >> contact;
		manager.chgReaderContact(readersubscript, contact);
		break;
	case 5:
		cout << "请输入新读者最大借书量" << endl;
		cin >> maxBorrownum;
		manager.chgReaderMaxBorrownum(readersubscript, maxBorrownum);
		break;
	case 0:
		return;
	}
}

void findReader() {
	cout << "1.按用户名查找" << endl << "2.按读者ID查找" << "其他退出" << endl;
	int command;
	cin >> command;
	int readersubscript = -1;
	if (command == 1)
	{
		cout << "请输入用户名" << endl;
		string username;
		cin >> username;
		readersubscript = manager.findReader(username);
	}
	else if (command == 2)
	{
		cout << "请输入读者ID" << endl;
		int readercode;
		cin >> readercode;
		readersubscript = manager.findReader(readercode);
	}
	else
	{
		return;
	}
	if (readersubscript == -1)
	{
		cout << "没有这位读者" << endl;
		return;
	}
	Reader reader = manager.getreader(readersubscript);
	reader.displayReader();
}

void listReader() {
	manager.displayAllReaderInfo();
}

void manageReader() {
	cout << "==========读者管理系统==========" << endl;
	cout << "1.增加读者" << endl;
	cout << "2.删除读者" << endl;
	cout << "3.更改读者" << endl;
	cout << "4.查找读者" << endl;
	cout << "0.退出读者管理系统" << endl;
	cout << "==============================" << endl;
	int command;
	cout << "请输入指令" << endl;
	cin >> command;
	switch (command)
	{
	case 1:
		createReaderA();
		break;
	case 2:
		delReader();
		break;
	case 3:
		editReader();
		break;
	case 4:
		findReader();
		break;
	case 5:
		listReader();
		break;
	default:
		return;
	}
}

void manageFile() {
	cout << "========== 文件管理系统 ==========" << endl;
	cout << "1.文件存储" << endl;
	cout << "2.文件读取" << endl;
	cout << "=================================" << endl;
	int command;
	cin >> command;
	if (command == 1)
	{
		manager.save("readers.db");
		bookmanager.save("books.db");
		cout << "已存储在books.db和readers.db中" << endl;
		return;
	}
	else if (command == 2)
	{
		manager.load("readers.db");
		bookmanager.load("books.db");
		cout << "文件已读取" << endl;
		bookmanager.showAllBooks();
		cout << endl;
		manager.displayAllReaderInfo();
		return;
	}
	else
	{
		cout << "文件未操作" << endl;
		return;
	}
}

int main()
{
	// 直接启动图形界面
	return main_gui();
}