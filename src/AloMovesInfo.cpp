#include "AloMovesInfo.h"
#include "dict.h"

/*
 * AloMovesPage
 */
bool AloMovesPage::operator<(const AloMovesPage &other) const
{
	return this->path()<other.path();
}

const std::string& AloMovesPage::topic() const
{
	return _topic;
}

const std::string& AloMovesPage::path() const
{
	return _path;
}

const std::list<std::string>& AloMovesPage::subPages() const
{
	return _sub_pages;
}

const std::list<std::string>& AloMovesPage::videos() const
{
	return _videos;
}

/*
 * AloMovesInfo
 */
const std::string& AloMovesInfo::baseUrl() const
{
	return _url;
}

const std::list<AloMovesPage>& AloMovesInfo::pages() const
{
	return _pages;
}

/*
 * AloMovesResultItem
 */
AloMovesResultItem::AloMovesResultItem(const AloMovesPage &page) : 
	_topic(page.topic()), 
	_path(page.path())
{
	this->_videos.insert(this->_videos.end(), page.videos().begin(), page.videos().end());
}

bool AloMovesResultItem::operator<(const AloMovesResultItem &other) const
{
	return this->topic()<other.topic();
}

const std::string& AloMovesResultItem::topic() const
{
	return this->_topic;
}

const std::string& AloMovesResultItem::path() const
{
	return this->_path;
}

const std::list<std::string>& AloMovesResultItem::videos() const
{
	return this->_videos;
}

/*
 * AloMovesResult
 */
void AloMovesResult::create(
	AloMovesResult &result, const AloMovesInfoCache *cache, 
	const AloMovesPage &page, int more_depth
)
{
	{
		AloMovesResultItem item(page);
		result.addItem(item);
	}

	if(more_depth>0)
	{
		for(std::list<std::string>::const_iterator iter=page.subPages().begin(); 
			iter!=page.subPages().end(); ++iter)
		{
			const std::string &sub_page_path = *iter;
			const AloMovesPage *page = cache->bufferedPageWithPath(sub_page_path);
			AloMovesResult::create(result, cache, *page, more_depth-1);
		}
	}
}

void AloMovesResult::addItem(const AloMovesResultItem &item)
{
	this->_items.insert(item);
}

const std::set<AloMovesResultItem>& AloMovesResult::items() const
{
	return this->_items;
}

/*
 * AloMovesInfoCache
 */
AloMovesInfoCache::AloMovesInfoCache(boost::shared_ptr<AloMovesInfo> info) : 
	_info(info), 
	_key_words(new Dict())
{
	for(std::list<AloMovesPage>::const_iterator iter=_info->pages().begin(); 
			iter!=_info->pages().end(); ++iter)
	{
		_info_map.insert(std::make_pair((*iter).path(), const_cast<AloMovesPage*>(&(*iter))));
	}
	
	this->build4KeyWords();
}

AloMovesPage* AloMovesInfoCache::bufferedPageWithPath(std::string path) const
{
	std::map<std::string, AloMovesPage*>::const_iterator iter = _info_map.find(path);
	if(iter==_info_map.end())
		return NULL;
	return iter->second;
}

void AloMovesInfoCache::serialize(std::ostringstream &s, AloMovesResult &result) const
{
	std::string escape = "\t";
	size_t escape_num = 1;

	for(std::set<AloMovesResultItem>::const_iterator result_item_iter=result.items().begin(); 
		result_item_iter!=result.items().end(); ++result_item_iter)
	{
		if(result_item_iter!=result.items().begin())
			s<<std::endl;
		const AloMovesResultItem &result_item = *result_item_iter;

		{
			s<<result_item.topic();
		}

		/*
		{
			
			for(size_t i=0; i<escape_num; ++i)
			{
				s<<std::endl;
				s<<escape;
			}
			s<<"url : "<<result_item.path();
		}
		*/

		for(std::list<std::string>::const_iterator video_iter=result_item.videos().begin(); 
			video_iter!=result_item.videos().end(); ++video_iter)
		{
			const std::string &video_url = *video_iter;
			for(size_t i=0; i<escape_num; ++i)
			{
				s<<std::endl;
				s<<escape;
			}
			s<<video_url;
		}
	}
}

static void createAloMovesResultWithPathConsidered(
	AloMovesResult &result, const AloMovesInfoCache *cache, const AloMovesPage &page)
{
	const std::string &path = page.path();
	if(path.find("/workouts", 0)==std::string::npos)
		AloMovesResult::create(result, cache, page);
	else
		AloMovesResult::create(result, cache, page, 0);
}

void AloMovesInfoCache::queryWithPath(AloMovesResult &result, std::string path) const
{
	const AloMovesPage *page = this->bufferedPageWithPath(path);
	if(page)
		createAloMovesResultWithPathConsidered(result, this, *page);
}

/*
void AloMovesInfoCache::queryWithKeyWord(AloMovesResult &result, std::string key_word) const
{
	const DictNode *node = NULL;
	if(!_key_words->hasWord(key_word, &node))
		return;
	std::map<const DictNode*, std::set<AloMovesPage*, AloMovesPageDuplicator> >::const_iterator iter = _key_words_map.find(node);
	assert(iter!=_key_words_map.end());
	const std::set<AloMovesPage*, AloMovesPageDuplicator> &pages = iter->second;
	for(std::set<AloMovesPage*, AloMovesPageDuplicator>::const_iterator pages_iter=pages.begin(); 
		pages_iter!=pages.end(); ++pages_iter)
	{
		const AloMovesPage *page = *pages_iter;
		createAloMovesResultWithPathConsidered(result, this, *page);
	}
}
*/

void AloMovesInfoCache::queryWithKeyWords(AloMovesResult &result, std::vector<std::string> &key_words) const
{
	for(size_t word_idx=0; word_idx<key_words.size(); ++word_idx)
	{
		std::string &key_word = key_words[word_idx];
		std::transform(key_word.begin(), key_word.end(), key_word.begin(), ::tolower);
	}

	for(std::map<std::string, AloMovesPage*>::const_iterator iter=_info_map.begin(); 
		iter!=_info_map.end(); ++iter)
	{
		const AloMovesPage *page = iter->second;
		std::string page_topic = page->topic();
		std::transform(page_topic.begin(), page_topic.end(), page_topic.begin(), ::tolower);

		size_t word_idx = 0;
		for(; word_idx<key_words.size(); ++word_idx)
		{
			const std::string &key_word = key_words[word_idx];
			if(page_topic.find(key_word, 0)==std::string::npos)
				break;
		}
		if(word_idx>=key_words.size())
			createAloMovesResultWithPathConsidered(result, this, *page);
	}
}

void AloMovesInfoCache::build4KeyWords()
{
	
}

