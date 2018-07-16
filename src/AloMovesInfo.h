#ifndef _ALOMOVES_INFO_
#define _ALOMOVES_INFO_

#include <string>
#include <list>
#include <set>
#include <boost/shared_ptr.hpp>
#include "cppbind/cppbind_json.hpp"

class Dict;
class DictNode;
class AloMovesResult;
class AloMovesInfoCache;

class AloMovesPage
{
public:
	AloMovesPage(){}
	bool operator<(const AloMovesPage &other) const;

	void setBind(cppbind::Binder *binder)
	{
		binder->bind("topic", _topic);
		binder->bind("path", _path);
		binder->bind("subPages", _sub_pages);
		binder->bind("videos", _videos);
	}

	const std::string& topic() const;
	const std::string& path() const;
	const std::list<std::string>& subPages() const;
	const std::list<std::string>& videos() const;

private:
	std::string _topic;
	std::string _path;
	std::list<std::string> _sub_pages;
	std::list<std::string> _videos;
};

struct AloMovesPageDuplicator
{
	bool operator()(const AloMovesPage *p1, const AloMovesPage *p2) const
	{
		return (*p1)<(*p2);
	}
};

class AloMovesInfo
{
public:
	AloMovesInfo(){}
	AloMovesInfo(std::string url);

	void setBind(cppbind::Binder *binder)
	{
		binder->bind("pages", _pages);
	}

	const std::string& baseUrl() const;
	const std::list<AloMovesPage>& pages() const;

private:
	std::string _url;
	std::list<AloMovesPage> _pages;
};

class AloMovesResultItem
{
public:
	AloMovesResultItem(const AloMovesPage &page);
	bool operator<(const AloMovesResultItem &other) const;

	const std::string& topic() const;
	const std::string& path() const;
	const std::list<std::string>& videos() const;

private:
	std::string _topic;
	std::string _path;
	std::list<std::string> _videos;

	friend class AloMovesResult;
};

class AloMovesResult
{
public:
	AloMovesResult(){}

	template<typename IteratorPageT> 
	static void create(AloMovesResult &result, const AloMovesInfoCache *cache, 
		IteratorPageT begin, IteratorPageT end);
	static void create(AloMovesResult &result, const AloMovesInfoCache *cache, 
		const AloMovesPage &page, int more_depth = 1);

	void addItem(const AloMovesResultItem &item);
	const std::set<AloMovesResultItem>& items() const;

private:
	std::set<AloMovesResultItem> _items;
};

template<typename IteratorPageT> 
void AloMovesResult::create(
	AloMovesResult &result, const AloMovesInfoCache *cache, 
	IteratorPageT begin, IteratorPageT end
)
{
	for(; begin!=end; ++begin)
		AloMovesResult::create(result, cache, *begin);
}

class AloMovesInfoCache
{
public:
	AloMovesInfoCache(boost::shared_ptr<AloMovesInfo> info);

	void queryWithPath(AloMovesResult &result, std::string path) const;
	void queryWithKeyWords(AloMovesResult &result, std::vector<std::string> &key_words) const;

	void serialize(std::ostringstream &s, AloMovesResult &result) const;

public:
	AloMovesPage* bufferedPageWithPath(std::string path) const;

private:
	boost::shared_ptr<AloMovesInfo> _info;
	std::map<std::string, AloMovesPage*> _info_map;

	boost::shared_ptr<Dict> _key_words;
	std::map<const DictNode*, std::set<AloMovesPage*, AloMovesPageDuplicator> > _key_words_map;
	void build4KeyWords();
};

#endif

