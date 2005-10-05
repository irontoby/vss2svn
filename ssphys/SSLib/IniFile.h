#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <functional>

class CIniFile
{
public:
	struct IniRecord
	{
    IniRecord ()
    {
    }
    IniRecord (std::string comments, char commented, std::string section, std::string key, std::string value)
      : Comments (comments), Commented (commented), Section (section), Key (key), Value (value)
    {
    }
    std::string Comments;
		char Commented;
		std::string Section;
		std::string Key;
		std::string Value;
	};

	enum CommentChar
	{
		Pound = '#',
		SemiColon = ';'
	};

	CIniFile(void);
	virtual ~CIniFile(void);

	static bool AddSection(std::string SectionName, std::string FileName);
	static bool CommentRecord(CommentChar cc, std::string KeyName,std::string SectionName,std::string FileName);
	static bool CommentSection(char CommentChar, std::string SectionName, std::string FileName);
	static std::string Content(std::string FileName);
	static bool Create(std::string FileName);
	static bool DeleteRecord(std::string KeyName, std::string SectionName, std::string FileName);
	static bool DeleteSection(std::string SectionName, std::string FileName);
	static std::vector<IniRecord> GetRecord(std::string KeyName, std::string SectionName, std::string FileName);
	static std::vector<IniRecord> GetSection(std::string SectionName, std::string FileName);
	static std::vector<std::string> GetSectionNames(std::string FileName);
	static std::string GetValue(std::string KeyName, std::string SectionName, std::string FileName);
	static bool RecordExists(std::string KeyName, std::string SectionName, std::string FileName);
	static bool RenameSection(std::string OldSectionName, std::string NewSectionName, std::string FileName);
	static bool SectionExists(std::string SectionName, std::string FileName);
	static bool SetRecordComments(std::string Comments, std::string KeyName, std::string SectionName, std::string FileName);
	static bool SetSectionComments(std::string Comments, std::string SectionName, std::string FileName);
	static bool SetValue(std::string KeyName, std::string Value, std::string SectionName, std::string FileName);
	static bool Sort(std::string FileName, bool Descending);
	static bool UnCommentRecord(std::string KeyName,std::string SectionName,std::string FileName);
	static bool UnCommentSection(std::string SectionName, std::string FileName);

private:
	static std::vector<IniRecord> GetSections(std::string FileName);
	static bool Load(std::string FileName, std::vector<IniRecord>& content);	
	static bool Save(std::string FileName, std::vector<IniRecord>& content);

	struct RecordSectionIs : std::unary_function<IniRecord, bool>
	{
		std::string section_;

		RecordSectionIs(const std::string& section): section_(section){}

		bool operator()( const IniRecord& rec ) const
		{
			return rec.Section == section_;
		}
	};

	struct RecordSectionKeyIs : std::unary_function<IniRecord, bool>
	{
		std::string section_;
		std::string key_;

		RecordSectionKeyIs(const std::string& section, const std::string& key): section_(section),key_(key){}

		bool operator()( const IniRecord& rec ) const
		{
			return ((rec.Section == section_)&&(rec.Key == key_));
		}
	};

	struct AscendingSectionSort
	{
		bool operator()(IniRecord& Start, IniRecord& End)
		{
			return Start.Section < End.Section;
		}
	};

	struct DescendingSectionSort
	{
		bool operator()(IniRecord& Start, IniRecord& End)
		{
			return Start.Section > End.Section;
		}
	};

	struct AscendingRecordSort
	{
		bool operator()(IniRecord& Start, IniRecord& End)
		{
			return Start.Key < End.Key;
		}
	};

	struct DescendingRecordSort
	{
		bool operator()(IniRecord& Start, IniRecord& End)
		{
			return Start.Key > End.Key;
		}
	};
};