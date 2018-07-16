#include <iostream>
#include <stdio.h>
#include <string.h>
#include "AloMovesInfo.h"
#include "dict.h"

static void test4path(const AloMovesInfoCache &cache)
{
	const char* paths[] = {
		"/", 
		"/series/flexibility-intelligence"
	};
	for(size_t i=0; i<sizeof(paths)/sizeof(paths[0]); ++i)
	{
		AloMovesResult result;
		std::string path(paths[i]);
		std::cout<<std::endl<<"query according to path \""<<path<<"\" :"<<std::endl;
		cache.queryWithPath(result, path);
		if(result.items().empty()){
			std::cout<<"no result for path \""<<path<<"\""<<std::endl;
		}else{
			std::ostringstream s;
			cache.serialize(s, result);
			std::cout<<s.str()<<std::endl;
		}
	}
}

static void  test4keyword(const AloMovesInfoCache &cache)
{
	const char* words[] = {
		"flexibility", "intelligence"
	};
	std::string encoded;
	std::vector<std::string> key_words;
	for(size_t i=0; i<sizeof(words)/sizeof(words[0]); ++i)
	{
		key_words.push_back(std::string(words[i]));
		if(i>0)
			encoded += std::string("+");
		encoded += std::string(words[i]);
	}

	AloMovesResult result;
	std::cout<<std::endl<<"query according to key-word \""<<encoded<<"\" :"<<std::endl;
	cache.queryWithKeyWords(result, key_words);
	if(result.items().empty()){
		std::cout<<"no result for key-word \""<<encoded<<"\""<<std::endl;
	}else{
		std::ostringstream s;
		cache.serialize(s, result);
		std::cout<<s.str()<<std::endl;
	}
}

struct StringSplitter
{
	char _ch;

	StringSplitter(char ch) : _ch(ch)
	{
	}

	std::vector<std::string> operator()(const std::string &s) const
	{
		std::vector<std::string> res;
		size_t start = 0, end  = 0;
	
		end = s.find_first_of(_ch, start);
		while(end!=std::string::npos){
			if(end>=start)
				res.push_back(s.substr(start, end-start));
			start = end+1;
			end = s.find_first_of(_ch, start);
		}
		res.push_back(s.substr(start));

		return res;
	}
};

struct StringJoiner
{
	char _ch;

	StringJoiner(char ch) : _ch(ch)
	{
	}

	std::string operator()(const std::vector<std::string> &l) const
	{
		std::string res;
		for(size_t i=0; i<l.size(); ++i)
			res += l[i]+_ch;
		if(res.length()>0)
			res.erase(res.size()-1);
		return res;
	}
};

struct StringStripper
{
	char _ch;

	StringStripper(char ch) : _ch(ch)
	{
	}

	std::string operator()(const std::string &s)
	{
		size_t start = s.find_first_not_of(_ch);
		if(start==std::string::npos)
			return std::string("");
		size_t end = s.find_last_not_of(_ch);
		if(end==std::string::npos)
			return std::string("");
		return s.substr(start, end-start+1);
	}
};

int main(int argc, char* argv[])
{
	--argc; ++argv;
	if(argc<=0)
		return -1;
	const char *file = argv[0];

	try{
	cppbind::JsonBind<AloMovesInfo> jsonBind;
	std::ifstream ifs(file, std::ifstream::in);
	boost::shared_ptr<AloMovesInfo> json = jsonBind.decode(ifs);
	ifs.close();

	AloMovesInfoCache cache(json);

	/*
	{
		cppbind::JsonBind<AloMovesInfo> jsonBind;
		std::stringstream s;
		jsonBind.encode(*json, &s);

		std::ofstream of("abc.txt");
		of<<s.str();
		of.close();
	}
	*/

	//test4path(cache);
	//test4keyword(cache);


	while(true)
	{
		char line_buf[1024];
		memset(line_buf, 0, sizeof(line_buf));
		::fgets(line_buf, sizeof(line_buf), stdin);

		std::vector<std::string> l = StringSplitter(' ')(line_buf);
		if(l.size()<=0)
			continue;
		std::string cmd = l.front();
		std::vector<std::string> args = l;
		args.erase(args.begin());

		if(args.size()>0)
		{
			std::string last = args.back();
			last = StringStripper('\n')(last);
			last = StringStripper('\r')(last);
			args.pop_back();
			args.push_back(last);
		}

		if(cmd=="I")
		{
			std::vector<std::string> paths = args;
			if(paths.size()>0)
			{
				std::string path = paths.front();
				AloMovesResult result;
				cache.queryWithPath(result, path);
				if(result.items().empty()){
					std::cout<<"no result for path \""<<path<<"\""<<std::endl;
				}else{
					std::ostringstream s;
					cache.serialize(s, result);
					std::cout<<s.str()<<std::endl;
				}
			}
		}
		else if(cmd=="II")
		{
			std::vector<std::string> key_words = args;
			if(key_words.size()>0)
			{
				AloMovesResult result;
				cache.queryWithKeyWords(result, key_words);
				if(result.items().empty()){
					std::string encoded = StringJoiner('+')(key_words);
					std::cout<<"no result for key-word \""<<encoded<<"\""<<std::endl;
				}else{
					std::ostringstream s;
					cache.serialize(s, result);
					std::cout<<s.str()<<std::endl;
				}
			}
		}
	}

	}catch(std::runtime_error &e){
		std::cout<<e.what()<<std::endl;
	}

	return 0;
}

