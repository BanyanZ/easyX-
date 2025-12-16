#pragma once
#include<iostream>
#include<string>
#include"manageTime.h"
using namespace std;

class BorrowRecord : public ManageTime
{
public:
	int userID_;//用户ID
	int bookID_;//图书ID
	int borrowID_;//借阅ID
	string borrowTime_;//借书时间
	string shouldReturn_;//应还时间
	string returnTime_;//实际归还时间
	int continue_;//剩余借阅次数

public:
	BorrowRecord() {}//默认构造
	BorrowRecord(int userID, int bookID, string borrowTime, string shouldReturn, string returnTime, int continueNum, bool isReturned = false, bool isOverdue = false, double fineAmount = 0.0, int borrowID = 0)
		:userID_(userID), bookID_(bookID), borrowTime_(borrowTime), shouldReturn_(shouldReturn), borrowID_(borrowID),
		returnTime_(returnTime), continue_(continueNum) {
	}//有参构造

//返回值
	int getUserID()const { return userID_; }
	int getBookID()const { return bookID_; }
	string getBorrowTime()const { return borrowTime_; }
	string getShouldReturnTime()const { return shouldReturn_; }
	string getReturnTime()const { return returnTime_; }
	int getContinue()const { return continue_; }
	int getBorrowID()const { return borrowID_; }

	//设置值
	void setUserID(int userID) { userID_ = userID; }
	void setBookID(int bookID) { bookID_ = bookID; }
	void setBorrowTime(const string& borrowTime) { borrowTime_ = borrowTime; }
	void setShouldReturnTime(const string& shouldReturn) { shouldReturn_ = shouldReturn; }
	void setReturnTime(const string& returnTime) { returnTime_ = returnTime; }
	void setContinue(int continueNum) { continue_ = continueNum; }
	void setBprrowID(int borrowID) { borrowID_ = borrowID; }
};