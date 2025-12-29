#pragma once
#include<iostream>
#include<string>
#include<vector>
#include"book.h"
#include"manageTime.h"
#include"borrowRecord.h"
#include"user.h"
#include"manager.h"
#include"reader.h"
#include<fstream>
#include<ctime>
#include<climits>
#define fineperday 8.0
using namespace std;

class ManageBorrowRecord : public BorrowRecord
{
private:
	vector<BorrowRecord> br;
	//辅助：检查用户是否存在（在 readers 列表中并且 idExisting 为 true）
	bool isReaderExisting(int userID, const vector<Reader>& readers) const {
		for (size_t i = 0; i < readers.size(); ++i) {
			if (readers[i].getID() == userID) {
				return readers[i].getIdExisting();
			}
		}
		return false;
	}
	// 辅助：对 '|' 和 '\' 做转义，写文件用
	static string escape_field(const string& s) {
		string out;
		out.reserve(s.size());
		for (char c : s) {
			if (c == '|' || c == '\\') out.push_back('\\');
			out.push_back(c);
		}
		return out;
	}
	// 取消转义
	static string unescape_field(const string& s) {
		string out;
		out.reserve(s.size());
		for (size_t i = 0; i < s.size(); ++i) {
			if (s[i] == '\\' && i + 1 < s.size()) out.push_back(s[++i]);
			else out.push_back(s[i]);
		}
		return out;
	}
	// 按转义规则切分一行，返回字段数组
	static vector<string> split_escaped_line(const string& line) {
		vector<string> fields;
		string cur;
		for (size_t i = 0; i < line.size(); ++i) {
			char c = line[i];
			if (c == '\\' && i + 1 < line.size()) {
				cur.push_back(line[++i]);
			}
			else if (c == '|') {
				fields.push_back(cur);
				cur.clear();
			}
			else {
				cur.push_back(c);
			}
		}
		fields.push_back(cur);
		return fields;
	}
public:
	ManageBorrowRecord() {}//默认构造函数
	~ManageBorrowRecord() {}//析构函数

	//1.添加借阅记录（不改变图书库存，仅记录）
	void addBorrowRecord(const BorrowRecord& newbr) {
		br.push_back(newbr);
	}

	//2.查找借阅记录，通过借阅ID查找索引
	int findBorrowRecord(int borrowID) const {
		for (int i = 0; i < static_cast<int>(br.size()); i++) {
			if (br[i].getBorrowID() == borrowID) {
				return i;
			}
		}
		return -1;//找不到返回-1
	}

	//3.借书操作
	bool borrowBook(int userID, string name, string aut, int bookID, vector<Book>& books, const vector<Reader>& readers, int borrowID = 0) {
		//先检查读者是否存在
		if (!isReaderExisting(userID, readers)) return false;
		//查找书籍,通过书名和作者名双重保险
		int bi = -1;
		for (int i = 0; i < static_cast<int>(books.size()); ++i) {
			if (books[i].getBookname() == name && books[i].getAuthor() == aut) { bi = i; break; }
		}
		if (bi == -1) return false; //未找到图书
		//检查库存
		Book& b = books[bi];
		if (b.getLeftnum() <= 0) return false; //无库存
		// 减少库存
		b.setLeftnum(b.getLeftnum() - 1);

		//生成借阅时间和应还时间
		time_t now = time(nullptr);//借阅时间
		char szNow[32] = { 0 };
		TimeToString(now, szNow);
		string borrowTime = szNow;
		time_t t2 = now;
		string shouldReturn = AddMonth(t2);

		//创建借阅记录
		if (borrowID == 0) borrowID = static_cast<int>(br.size()) + 1;
		BorrowRecord record(userID, bookID, borrowTime, shouldReturn, string(""), 0, false, false, 0.0, borrowID);
		record.setBprrowID(borrowID);
		addBorrowRecord(record);
		return true;
	}

	//4.还书操作：
	double returnBook(int borrowID, vector<Book>& books) {
		int idx = findBorrowRecord(borrowID);
		if (idx == -1) return -1.0;
		BorrowRecord& rec = br[idx];
		//如果已经有 returnTime，视为已归还，返回 -1.0 表示错误/已处理
		if (!rec.getReturnTime().empty()) return -1.0;

		//记录当前归还时间
		time_t now = time(nullptr);
		char szNow[32] = { 0 };
		BorrowRecord::TimeToString(now, szNow);
		string returnTime = szNow;
		rec.setReturnTime(returnTime);

		//计算总借阅天数
		string borrowTimeStr = rec.getBorrowTime();
		string returnTimeStr = rec.getReturnTime();
		int totalDays = BorrowRecord::calculateDaysDifference(borrowTimeStr, returnTimeStr);
		if (totalDays < 0) totalDays = 0;

		//计算超期天数：shouldReturn 与 returnTime 比较
		string shouldReturnStr = rec.getShouldReturnTime();
		int overdueDays = BorrowRecord::calculateDaysDifference(shouldReturnStr, returnTimeStr);
		if (overdueDays < 0) overdueDays = 0;

		//罚款：每超一天 8 元
		double fine = overdueDays * fineperday;

		// 恢复图书库存（找到图书并 leftnum++）
		for (int i = 0; i < static_cast<int>(books.size()); ++i) {
			if (books[i].getID() == rec.getBookID()) {
				books[i].setLeftnum(books[i].getLeftnum() + 1);
				break;
			}
		}
		//返回返款数
		return fine;
	}

	//5.按用户ID查询该用户所有借阅记录的索引
	vector<int> findRecordsByUser(int userID) const {
		vector<int> res;
		for (int i = 0; i < static_cast<int>(br.size()); ++i) {
			if (br[i].getUserID() == userID) res.push_back(i);
		}
		return res;
	}

	//6.列出当前所有逾期未还的借阅记录（以当前日期为准）
	vector<BorrowRecord> listOverdueRecords() const {
		vector<BorrowRecord> res;
		time_t now = time(nullptr);
		char szNow[32] = { 0 };
		ManageBorrowRecord* self = const_cast<ManageBorrowRecord*>(this);
		self->TimeToString(now, szNow);
		string today = szNow;

		for (const BorrowRecord& r : br) {
			if (r.getReturnTime().empty()) {
				string should = r.getShouldReturnTime();
				int diff = self->calculateDaysDifference(should, today);
				if (diff > 0) res.push_back(r);
			}
		}
		return res;
	}

	//7.输出某条记录
	void displayRecord(const BorrowRecord& r) const {
		cout << "BorrowID: " << r.getBorrowID()
			<< " UserID: " << r.getUserID()
			<< " BookID: " << r.getBookID()
			<< " BorrowTime: " << r.getBorrowTime()
			<< " ShouldReturn: " << r.getShouldReturnTime()
			<< " ReturnTime: " << r.getReturnTime()
			<< endl;
	}

	//8.访问br
	const vector<BorrowRecord>& allRecords() const { return br; }
	vector<BorrowRecord>& allRecords() { return br; } // 非const版本用于修改

	//10.设置借阅记录的评价
	bool setRecordRating(int borrowID, int rating) {
		int idx = findBorrowRecord(borrowID);
		if (idx == -1) return false;
		if (rating < 1 || rating > 5) return false;
		br[idx].setRating(rating);
		return true;
	}

	//9.数据化管理
//9.数据化管理（文本格式，支持转义）
	void save(string filename)
	{
		ofstream ofs(filename.c_str(), ios::out | ios::trunc);
		if (!ofs.is_open()) {
			cout << "Error opening file for write: " << filename << endl;
			return;
		}
		for (const BorrowRecord& r : br) {
			ofs << r.getUserID() << '|' << r.getBookID() << '|' << r.getBorrowID() << '|'
				<< escape_field(r.getBorrowTime()) << '|' << escape_field(r.getShouldReturnTime()) << '|'
				<< escape_field(r.getReturnTime()) << '|' << r.getContinue() << '|' << r.getRating()
				<< '\n';
		}
		ofs.close();
	}

	void load(string filename)
	{
		ifstream ifs(filename.c_str(), ios::in);
		if (!ifs.is_open()) {
			// 文件不存在或打开失败：不报错，保持 br 不变
			return;
		}
		br.clear();
		string line;
		while (std::getline(ifs, line)) {
			auto fields = split_escaped_line(line);
			// 期望 8 个字段（新增评价字段）
			if (fields.size() < 7) continue;
			try {
				int userID = stoi(fields[0]);
				int bookID = stoi(fields[1]);
				int borrowID = stoi(fields[2]);
				string borrowTime = unescape_field(fields[3]);
				string shouldReturn = unescape_field(fields[4]);
				string returnTime = unescape_field(fields[5]);
				int cont = stoi(fields[6]);
				int rating = (fields.size() >= 8) ? stoi(fields[7]) : 0; // 兼容旧数据，默认为0

				BorrowRecord rec(userID, bookID, borrowTime, shouldReturn, returnTime, cont, false, false, 0.0, borrowID, rating);
				rec.setBprrowID(borrowID);
				br.push_back(rec);
			}
			catch (...) {
				continue;
			}
		}
		ifs.close();
	}
};